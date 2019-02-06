from sklearn.metrics import f1_score, confusion_matrix, classification_report
import caffe
from paths import *
from img_property import *
import numpy as np
import cv2
import functions as fc
import time
import re

#Run Evaluation on Test Set (27 Images)
#Input: Root Folder of Test Images
#Print out the confusion matrix, classification report 

def CalculateMetricPerformance(root=CROP_PHANTAST_TEST):
    caffe.set_mode_gpu()
    caffe.set_device(0)
    file_list = []
    for subdir in fc.SubDirPath(root):
        file_list.append(fc.FilesInDir(subdir, r'((^crop_\d_\d_phantast_result.tif$)|(^crop_\d_\d.bmp$))'))

    file_list = [item for sublist in file_list for item in sublist]

    data_files = [file for file in file_list if '_phantast_result' not in file]
    label_files = [file for file in file_list if '_phantast_result' in file]
    print 'test:', len(data_files)
    print 'test', len(label_files)

    net = caffe.Net(DEPLOY_PROTOTXT, caffe.TEST, weights='snapshot2018-10-12-1111_bat1_iter_178500.caffemodel')
    transformer = caffe.io.Transformer({'data': net.blobs['data'].data.shape})
    
    cm = 0
    img_count = 0
    f1_avg = np.array([0.0,0.0,0.0], dtype='f')
    recall_avg = np.array([0.0,0.0,0.0], dtype='f')
    precision_avg = np.array([0.0,0.0,0.0], dtype='f')
    
    for img_path in data_files:
        print img_path
        label_path = img_path.replace('.bmp', '_phantast_result.tif')
        img = np.zeros((IMG_TILE_WIDTH, IMG_TILE_WIDTH, 1), dtype=np.uint8)
        img = cv2.imread(img_path, cv2.IMREAD_GRAYSCALE)
        #img = cv2.GaussianBlur(img, (13, 13), 0)

        #cv2.imwrite(str(img_count) + '_blur13.bmp', img)  # save image in the directory

        
        label = cv2.imread(label_path, cv2.IMREAD_GRAYSCALE)
        padding = (IMG_EXT_SIZE - IMG_TILE_HEIGHT) / 2
        img = cv2.copyMakeBorder(img, padding, padding, padding, padding, cv2.BORDER_REFLECT_101)
        pred_mask_ptry = 0
        pred = np.zeros((IMG_TILE_WIDTH, IMG_TILE_WIDTH, 1), dtype=np.uint8)
        for y in fc.my_range(0, IMG_EXT_SIZE - SEG_MASK_HEIGHT + 1, SEG_MASK_STRIDE):
            pred_mask_ptrx = 0
            for x in fc.my_range(0, IMG_EXT_SIZE - SEG_MASK_WIDTH + 1, SEG_MASK_STRIDE):
                tmp_cm = img[y:y + SEG_MASK_WIDTH, x:x + SEG_MASK_HEIGHT]
                net.blobs['data'].data[...] = transformer.preprocess('data', tmp_cm)
                out = net.forward()
                result = net.blobs['argmax'].data[0, 0, :, :]
                pred_mask_height = result.shape[0]
                pred_mask_width = result.shape[1]
                pred[pred_mask_ptry:pred_mask_ptry + pred_mask_height, pred_mask_ptrx:pred_mask_ptrx + pred_mask_width, 0] = \
                    net.blobs['argmax'].data[0, 0, :, :]
                pred_mask_ptrx = pred_mask_ptrx + pred_mask_width
            pred_mask_ptry = pred_mask_ptry + pred_mask_height

        pred = np.array(pred).astype(np.uint8)
        x, y, z = np.nonzero(pred == 1)
        pred[x, y] = 255
        x, y, z = np.nonzero(pred == 2)
        pred[x, y] = 128

        cv2.imwrite(str(img_count) + '.png', pred)  # save image in the directory

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
        
        img_count +=1
        
    
    cm = cm / img_count
    recall_avg = recall_avg / img_count
    f1_avg = f1_avg / img_count
    precision_avg = precision_avg / img_count

    print 'Confusion Matrix: \n', cm
    print 'recall_avg: \n', recall_avg
    print 'f1_avg: \n', f1_avg
    print 'precision_avg: \n', precision_avg
    

def sort_by_name(name):    
    name = name.split('/')[-1].split('.')[0].split('_')[0]
    return int(name)


def Label_Pred_comparison(root = './RunValidationImage/results/'):
    file_list = []
    file_list.append(fc.FilesInDir(root, r'((^(\d)+_phantast_result.tif$)|(^(\d)+.bmp$)|(^(\d)+.png$))'))

    file_list = [item for sublist in file_list for item in sublist]
    data_files = [file for file in file_list if 'bmp' in file]
    label_files = [file for file in file_list if '_phantast_result' in file]
    pred_files = [file for file in file_list if 'png' in file]

    data_files.sort(key=sort_by_name)
    label_files.sort(key=sort_by_name)
    pred_files.sort(key=sort_by_name)

    img_count = 0
    
    for img_path,label_path, pred_path in zip(data_files, label_files, pred_files):
        img = np.zeros((IMG_TILE_WIDTH, IMG_TILE_WIDTH, 1), dtype=np.uint8)
        label =  np.zeros((IMG_TILE_WIDTH, IMG_TILE_WIDTH, 1), dtype=np.uint8)
        pred = np.zeros((IMG_TILE_WIDTH, IMG_TILE_WIDTH, 1), dtype=np.uint8)
        mask = np.zeros((IMG_TILE_WIDTH, IMG_TILE_WIDTH, 3), dtype=np.uint8)
        img = cv2.imread(img_path, cv2.IMREAD_COLOR)
        mask = cv2.imread(img_path, cv2.IMREAD_COLOR)
        label = cv2.imread(label_path, cv2.IMREAD_COLOR)
        pred = cv2.imread(pred_path, cv2.IMREAD_COLOR)

        label_pred = cv2.bitwise_and(label,pred)
        label_only = cv2.bitwise_xor(label_pred,label)
        pred_only = cv2.bitwise_xor(label_pred,pred)

        #print len(np.nonzero(label_pred!=[0,0,0])[0])/3, len(np.nonzero(label_only!=[0,0,0])[0])/3, len(np.nonzero(pred_only!=[0,0,0])[0])/3

        mask[np.where((label_pred == [255,255,255]).all(axis = 2))] = [0,255,0] #green
        mask[np.where((label_pred == [128,128,128]).all(axis = 2))] = [0,255,0] #green
        #mask[np.where((label_pred == [0,0,0]).all(axis = 2))] = [0,255,0] #green
        mask[np.where((label_only == [255,255,255]).all(axis = 2))] = [0,0,255] # red
        mask[np.where((label_only == [128,128,128]).all(axis = 2))] = [0,0,255] #red
        #mask[np.where((label_only == [0,0,0]).all(axis = 2))] = [0,0,255] #red
        mask[np.where((pred_only == [255,255,255]).all(axis = 2))] = [255,0,0] # blue
        mask[np.where((pred_only == [128,128,128]).all(axis = 2))] = [255,0,0] # blue
        #mask[np.where((pred_only == [0,0,0]).all(axis = 2))] = [255,0,0] # blue

        overlay = cv2.addWeighted(img, 1.0, mask, 0.3, 0)
        # cv2.imshow('overlay',mask)
        # cv2.waitKey(0)
        # cv2.destroyAllWindows()
        cv2.imwrite(root+str(img_count) + '_overlay_deadcells.png', overlay)
        img_count +=1
    
def main():
    CalculateMetricPerformance()
    #Label_Pred_comparison()

if __name__ == '__main__':
    main()