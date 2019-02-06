import cv2
import functions as fc
from caffe.proto import caffe_pb2
import lmdb
import numpy as np
from paths import *
from sklearn.utils import shuffle
from img_property import *
import os
import shutil
import augmentation as aug
import itertools
import time
import caffe
import gc

def createFileList(img_root, txt_save_path):
    fw = open(txt_save_path, "w")
    for subdir in fc.SubDirPath(img_root):
        dirpath = os.path.join(img_root, subdir)
        file_list = fc.FilesInDir(dirpath, r'((^crop_\d_\d_phantast_result.tif$)|(^crop_\d_\d.bmp$))')
        for img in file_list:
            print img
            fw.write(img + '\n')
    print 'txtfile generated'
    fw.close()

def create_lmdb(mode, txt_save_path, data_lmdb_path, label_lmdb_path, num_tiles):
    if os.path.exists(data_lmdb_path):
        shutil.rmtree(data_lmdb_path)
    if os.path.exists(label_lmdb_path):
        shutil.rmtree(label_lmdb_path)

    txtfile = open(txt_save_path, "r")
    imagelist = [line.split(' ')[0].replace('\n', '') for line in txtfile.readlines()]
    data = [img for img in imagelist if '_phantast_result' not in img]
    label = [img for img in imagelist if '_phantast_result' in img]

    print '\nCreating ' + mode
    data_datums = np.zeros(81 * NUM_AUG * num_tiles, dtype=caffe_pb2.Datum)
    label_datums = np.zeros(81 * NUM_AUG * num_tiles, dtype=caffe_pb2.Datum)
    counter = 0
    c = 0
    start_time = time.time()
    for data_path, label_path in itertools.izip(data, label):
        c+=1
        print data_path, label_path,c
        data_img = cv2.imread(data_path, cv2.IMREAD_GRAYSCALE)
        label_img = cv2.imread(label_path, cv2.IMREAD_GRAYSCALE)

        # pad image into 2236x2236
        padding = (IMG_EXT_SIZE - IMG_TILE_HEIGHT) / 2
        data_img = cv2.copyMakeBorder(data_img, padding, padding, padding, padding, cv2.BORDER_REFLECT_101)
        label_img = cv2.copyMakeBorder(label_img, padding, padding, padding, padding, cv2.BORDER_REFLECT_101)

        # save image and label as 2xCxHxW
        img = np.zeros((2, 1, IMG_EXT_SIZE, IMG_EXT_SIZE), dtype=np.uint8)
        img[0, 0, :, :] = data_img
        sx, sy = np.nonzero(label_img == 255)
        if sx.size !=0:
            label_img[sx, sy] = 1
        sx, sy = np.nonzero(label_img == 128)
        if sx.size !=0:
            label_img[sx, sy] = 2
        img[1, 0, :, :] = label_img

        del sx,sy,data_img,label_img

        for y in fc.my_range(0, IMG_EXT_SIZE - SEG_MASK_HEIGHT + 1,
                             SEG_MASK_STRIDE):  # for (y=0; y<IMG_EXT_SIZE - SEG_MASK_HEIGHT + 1; y+=SEG_MASK_HEIGHT)
            for x in fc.my_range(0, IMG_EXT_SIZE - SEG_MASK_WIDTH + 1, SEG_MASK_STRIDE):
                tmp = img[:, 0, y:y + SEG_MASK_HEIGHT, x:x + SEG_MASK_WIDTH]

                augimg = aug.DoAugmentation(tmp)
                del tmp

                for i in xrange(len(augimg)):
                    datum = caffe_pb2.Datum()
                    datum.channels = 1
                    datum.height = SEG_MASK_HEIGHT
                    datum.width = SEG_MASK_WIDTH
                    datum.data = augimg[i, 0, :, :].astype(np.uint8).tobytes()  # or .tostring() if numpy < 1.9
                    datum.label = 0
                    data_datums[counter] = datum

                    datum_label = caffe_pb2.Datum()
                    datum_label.channels = 1
                    datum_label.height = SEG_MASK_HEIGHT
                    datum_label.width = SEG_MASK_WIDTH
                    datum_label.data = augimg[i, 1, :, :].astype(np.uint8).tobytes()  # or .tostring() if numpy < 1.9
                    datum_label.label = 0
                    label_datums[counter] = datum_label
                    counter += 1
                    del datum_label, datum
                    
                del augimg

    end_time = time.time()
    print end_time - start_time

    if mode == 'val':
        map_size = 2e9
    else :
        map_size = 8e9

    data_datums, label_datums = shuffle(data_datums, label_datums)
    
    in_db = lmdb.open(data_lmdb_path, map_size=float(map_size))
    with in_db.begin(write=True) as in_txn:
        for count in xrange(len(data_datums)):
            in_txn.put('{:05}'.format(count).encode('ascii'), (data_datums[count]).SerializeToString())
    in_db.close()
    
    in_db = lmdb.open(label_lmdb_path, map_size=float(map_size))
    with in_db.begin(write=True) as in_txn:
        for count in xrange(len(label_datums)):
            in_txn.put('{:05}'.format(count).encode('ascii'), label_datums[count].SerializeToString())
    in_db.close()
    
    print '\nFinished processing all ' + mode + ' images'


def create_mean_file(mean_tool_path, lmdb_path, mean_file_path):
    cmd = '%s -backend=lmdb %s %s ' % (mean_tool_path, lmdb_path, mean_file_path)
    os.system(cmd)


def init():
    #createFileList(CROP_PHANTAST_TRAIN, TRAIN_FILELIST)
    createFileList(CROP_PHANTAST_VAL, VAL_FILELIST)
    #create_lmdb('train', TRAIN_FILELIST, TRAIN_DATA_LMDB, TRAIN_LABEL_LMDB, NUM_TRAIN_TILE)
    create_lmdb('val', VAL_FILELIST, VAL_DATA_LMDB, VAL_LABEL_LMDB, NUM_VAL_TILE)
    #if os.path.exists(TRAIN_MEAN_FILE):
    #     os.remove(TRAIN_MEAN_FILE)
    #create_mean_file(MEAN_TOOL_PATH, TRAIN_DATA_LMDB, TRAIN_MEAN_FILE)


def visualize_lmdb():
    lmdb_env = lmdb.open(TRAIN_LABEL_LMDB)
    lmdb_txn = lmdb_env.begin()
    lmdb_cursor = lmdb_txn.cursor()
    datum = caffe.proto.caffe_pb2.Datum()

    for key, value in lmdb_cursor:
        datum.ParseFromString(value)
        data = caffe.io.datum_to_array(datum)
        test = np.array(data[0, :, :]).astype(np.uint8)
        x,y = np.nonzero(test > 0)
        test[x,y]=255/test[x,y]
        cv2.imwrite('./dump/test' + key + '.png', test)
        if int(key) > 50:
            break
