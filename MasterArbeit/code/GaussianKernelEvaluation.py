import numpy as np
from functions import *
from paths import *
import cv2
from sklearn.metrics import f1_score, confusion_matrix, classification_report
import caffe
from img_property import *

imgs = []
labels = []
subDir = SubDirPath(CROP_PHANTAST_VAL)
for dir in subDir:
    files = FilesInDir(dir, r'((^crop_\d_\d_phantast_result.tif$)|(^crop_\d_\d.bmp$))')
    for file in files:
        if '_phantast_result' not in file:
            imgs.append(file)
        else:
            labels.append(file)

caffe.set_mode_gpu()
caffe.set_device(0)
net = caffe.Net(DEPLOY_PROTOTXT, caffe.TEST, weights='snapshot2018-10-17-1142_iter_178500.caffemodel')
transformer = caffe.io.Transformer({'data': net.blobs['data'].data.shape})

ks_list = range(5, 7, 2)
for ks in ks_list:
    cm = 0
    img_count = 0
    f1_avg = np.array([0.0,0.0,0.0], dtype='f')
    recall_avg = np.array([0.0,0.0,0.0], dtype='f')
    precision_avg = np.array([0.0,0.0,0.0], dtype='f')
    for img_path, label_path in zip(imgs, labels):
        img = cv2.imread(img_path, cv2.IMREAD_GRAYSCALE)
        img = cv2.GaussianBlur(img, (ks, ks), 0)
        
        cv2.imshow('image',img)
        cv2.waitKey(0)
        

        label = cv2.imread(label_path, cv2.IMREAD_GRAYSCALE)
        padding = (IMG_EXT_SIZE - IMG_TILE_HEIGHT) / 2
        img = cv2.copyMakeBorder(img, padding, padding, padding, padding, cv2.BORDER_REFLECT_101)
        pred_mask_ptry = 0
        pred = np.zeros((IMG_TILE_WIDTH, IMG_TILE_WIDTH, 1), dtype=np.uint8)
        for y in my_range(0, IMG_EXT_SIZE - SEG_MASK_HEIGHT + 1, SEG_MASK_STRIDE):
            pred_mask_ptrx = 0
            for x in my_range(0, IMG_EXT_SIZE - SEG_MASK_WIDTH + 1, SEG_MASK_STRIDE):
                tmp_cm = img[y:y + SEG_MASK_WIDTH, x:x + SEG_MASK_HEIGHT]
                net.blobs['data'].data[...] = transformer.preprocess('data', tmp_cm)
                out = net.forward()
                result = net.blobs['argmax'].data[0, 0, :, :]
                pred_mask_height = result.shape[0]
                pred_mask_width = result.shape[1]
                pred[pred_mask_ptry:pred_mask_ptry + pred_mask_height, pred_mask_ptrx:pred_mask_ptrx + pred_mask_width,
                0] = \
                    net.blobs['argmax'].data[0, 0, :, :]
                pred_mask_ptrx = pred_mask_ptrx + pred_mask_width
            pred_mask_ptry = pred_mask_ptry + pred_mask_height

        pred = np.array(pred).astype(np.uint8)
        x, y, z = np.nonzero(pred == 1)
        pred[x, y] = 255
        x, y, z = np.nonzero(pred == 2)
        pred[x, y] = 128

        pred = pred.flatten()
        label = label.flatten()
        tmp_cm = confusion_matrix(label, pred, [0, 128, 255])
        cm += tmp_cm.astype('float') / tmp_cm.sum(axis=1)[:, np.newaxis]  # normalized
        tmp_cr = classification_report(label, pred, output_dict=True)
        
        recall_avg[0] += tmp_cr['0']['recall']
        recall_avg[1] += tmp_cr['128']['recall']
        recall_avg[2] += tmp_cr['255']['recall']
        f1_avg[0] += tmp_cr['0']['f1-score']
        f1_avg[1] += tmp_cr['128']['f1-score']
        f1_avg[2] += tmp_cr['255']['f1-score']
        precision_avg[0] += tmp_cr['0']['precision']
        precision_avg[1] += tmp_cr['128']['precision']
        precision_avg[2] += tmp_cr['255']['precision']

        print tmp_cm, tmp_cr,img_count
        img_count += 1

    cm = cm / img_count
    print 'Confusion Matrix: \n', cm


    print recall_avg, f1_avg, precision_avg
    recall_avg = recall_avg / img_count
    f1_avg = f1_avg / img_count
    precision_avg = precision_avg / img_count
    print 'recall_avg: \n', recall_avg
    print 'f1_avg: \n', f1_avg
    print 'precision_avg: \n', precision_avg