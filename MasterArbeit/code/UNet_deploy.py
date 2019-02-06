import caffe
from paths import *
from img_property import *
import numpy as np
import cv2
import functions as fc
import time

def deploy_single():
    if not IS_GPU:
        caffe.set_mode_cpu()
    else:
        caffe.set_mode_gpu()
        caffe.set_device(0)

    folder = './phantast_val/2017-04-05_13-13-56_B1_universal'
    filename = 'crop_1_0'

    img_path = folder + '/' + filename + '.bmp'
    count = '_2018-10-12-1111_bat1_iter_182500'
    img_save_name = 'F:/MA_Yang/code/dump/' + filename + count
    net = caffe.Net(DEPLOY_PROTOTXT, caffe.TEST, weights='./snapshots/snapshot2018-10-12-1111_bat1_iter_182500.caffemodel')
    transformer = caffe.io.Transformer({'data': net.blobs['data'].data.shape})
    img = np.zeros((IMG_TILE_WIDTH, IMG_TILE_WIDTH, 1), dtype=np.uint8)
    img = cv2.imread(img_path, cv2.IMREAD_GRAYSCALE)
    color_img = cv2.cvtColor(img, cv2.COLOR_GRAY2RGB)
    padding = (IMG_EXT_SIZE - IMG_TILE_HEIGHT) / 2
    img = cv2.copyMakeBorder(img, padding, padding, padding, padding, cv2.BORDER_REFLECT_101)
    seg_mask_ptry = 0
    seg_mask = np.zeros((IMG_TILE_WIDTH, IMG_TILE_WIDTH, 1), dtype=np.uint8)
    for y in fc.my_range(0, IMG_EXT_SIZE - SEG_MASK_HEIGHT + 1, SEG_MASK_STRIDE):
        seg_mask_ptrx = 0
        for x in fc.my_range(0, IMG_EXT_SIZE - SEG_MASK_WIDTH + 1, SEG_MASK_STRIDE):
            tmp = img[y:y + SEG_MASK_WIDTH, x:x + SEG_MASK_HEIGHT]
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
    print np.unique(seg_mask)

    _, contours, _ = cv2.findContours(seg_mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    color_img = cv2.drawContours(color_img, contours, -1, (0, 255, 255), 1)
    cv2.imwrite(img_save_name + count + '_contour.png', color_img)
    cv2.imwrite(img_save_name + count + '.png', seg_mask)  # save image in the directory


def deploy_multi():
    if not IS_GPU:
        caffe.set_mode_cpu()
    else:
        caffe.set_mode_gpu()
        caffe.set_device(0)

    folder = '2017-04-05_13-00-50_B1_universal'
    filename = 'crop_0_3'
    img_path = 'F:/MA_Yang/code/phantast_train/' + folder + '/' + filename + '.bmp'
    img_save_name = 'F:/MA_Yang/code/dump/'+folder + '_' + filename

    img = np.zeros((IMG_TILE_WIDTH, IMG_TILE_WIDTH, 1), dtype=np.uint8)
    img = cv2.imread(img_path, cv2.IMREAD_GRAYSCALE)
    tmp_img = img
    padding = (IMG_EXT_SIZE - IMG_TILE_HEIGHT) / 2
    img = cv2.copyMakeBorder(img, padding, padding, padding, padding, cv2.BORDER_REFLECT_101)

    for x in xrange(200, 363):
        count = 500 * x
        net = caffe.Net(DEPLOY_PROTOTXT, caffe.TEST, weights='./snapshots/snapshot2018-10-17-1142_iter_'+str(count)+'.caffemodel')
        transformer = caffe.io.Transformer({'data': net.blobs['data'].data.shape})

        seg_mask_ptry = 0
        seg_mask = np.zeros((IMG_TILE_WIDTH, IMG_TILE_WIDTH, 1), dtype=np.uint8)
        for y in fc.my_range(0, IMG_EXT_SIZE - SEG_MASK_HEIGHT + 1, SEG_MASK_STRIDE):
            seg_mask_ptrx = 0
            for x in fc.my_range(0, IMG_EXT_SIZE - SEG_MASK_WIDTH + 1, SEG_MASK_STRIDE):
                tmp = img[y:y + SEG_MASK_WIDTH, x:x + SEG_MASK_HEIGHT]
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
        x, y, z = np.nonzero(seg_mask  == 1)
        seg_mask[x, y] = 255
        x, y, z = np.nonzero(seg_mask == 2)
        seg_mask[x, y] = 128
        print np.unique(seg_mask)

        _, contours, _ = cv2.findContours(seg_mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
        color_img = cv2.cvtColor(tmp_img, cv2.COLOR_GRAY2RGB)
        color_img = cv2.drawContours(color_img, contours, -1, (0, 255, 255), 1)
        cv2.imwrite(img_save_name + str(count) + '_contour.png', color_img)
        cv2.imwrite(img_save_name + str(count) + '.png', seg_mask)  # save image in the directory