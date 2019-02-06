import caffe
from paths import *
from img_property import *
import numpy as np
import cv2
import functions as fc
import time
import os

def deploy_Full_Image(img_path=DEPLOY_IMG, caffemodel_path=DEPLOY_MODEL, deploy_prototxt_path=DEPLOY_PROTOTXT):
    caffe.set_mode_gpu()
    caffe.set_device(0)

    start_time = time.time()
    img_data = cv2.imread(img_path, cv2.IMREAD_GRAYSCALE)
    lx, ly = img_data.shape

    save_dir = './FullSegmentationResult/'
    net = caffe.Net(deploy_prototxt_path, caffe.TEST, weights=caffemodel_path)
    transformer = caffe.io.Transformer({'data': net.blobs['data'].data.shape})
    if not os.path.exists(save_dir):
        os.makedirs(save_dir)


    c=0
    for yy in xrange(4):
        for xx in xrange(4):
            img_save_name = 'tile_'+str(c)
            img_crop = img_data[lx / 3 + xx * IMG_TILE_WIDTH: lx / 3 + (xx + 1) * IMG_TILE_WIDTH, ly / 3 + yy * IMG_TILE_WIDTH: ly / 3 + (yy + 1) * IMG_TILE_WIDTH]
            
            color_img = cv2.cvtColor(img_crop, cv2.COLOR_GRAY2RGB)
            padding = (IMG_EXT_SIZE - IMG_TILE_HEIGHT) / 2
            img_crop = cv2.copyMakeBorder(img_crop, padding, padding, padding, padding, cv2.BORDER_REFLECT_101)
            seg_mask_ptry = 0
            seg_mask = np.zeros((IMG_TILE_WIDTH, IMG_TILE_WIDTH, 1), dtype=np.uint8)

            for y in fc.my_range(0, IMG_EXT_SIZE - SEG_MASK_HEIGHT + 1, SEG_MASK_STRIDE):
                seg_mask_ptrx = 0
                for x in fc.my_range(0, IMG_EXT_SIZE - SEG_MASK_WIDTH + 1, SEG_MASK_STRIDE):
                    tmp = img_crop[y:y + SEG_MASK_WIDTH, x:x + SEG_MASK_HEIGHT]
                    net.blobs['data'].data[...] = transformer.preprocess('data', tmp)
                    out = net.forward()                     
                    result = net.blobs['argmax'].data[0, 0, :, :]
                    seg_mask_height = result.shape[0]
                    seg_mask_width = result.shape[1]
                    seg_mask[seg_mask_ptry:seg_mask_ptry + seg_mask_height, seg_mask_ptrx:seg_mask_ptrx + seg_mask_width, 0] = \
                        net.blobs['argmax'].data[0, 0, :, :]
                    seg_mask_ptrx = seg_mask_ptrx + seg_mask_width
                seg_mask_ptry = seg_mask_ptry + seg_mask_height

            seg_mask = np.array(seg_mask).astype(np.uint8)
            x, y, z = np.nonzero(seg_mask == 1)
            seg_mask[x, y] = 255
            x, y, z = np.nonzero(seg_mask == 2)
            seg_mask[x, y] = 128

            _, contours, _ = cv2.findContours(seg_mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
            color_img = cv2.drawContours(color_img, contours, -1, (0, 255, 255), 1)
            cv2.imwrite(save_dir + img_save_name + '_contour.png', color_img)
            cv2.imwrite(save_dir + img_save_name + '.png', seg_mask)  # save image in the directory
            c+=1



    end_time = time.time()
    image_processing = end_time - start_time
    print 'processing time : ' + image_processing

def main():
    deploy_Full_Image()

if __name__ == '__main__':
    main()