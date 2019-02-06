import numpy as np
from functions import *
from img_property import *
import cv2

for paths in FilesInDir("F:\MA_Yang\code\data\crop_phantast_train\\2017-03-07_09-34-04_A1_universal",r'((^crop_\d_\d_phantast_result.tif$))'):
    im = cv2.imread(paths, cv2.IMREAD_GRAYSCALE)
    color = cv2.cvtColor(im, cv2.COLOR_GRAY2RGB)
    print color.shape
    for y in xrange(0, im.shape[0]):
        for x in xrange(0, im.shape[1]):
            print y,x
            if y> 50 and x>50 and y<im.shape[0]-50 and x<im.shape[1]-50:
                sum = np.sum(im[y-50:y+50, x-50:x+50])
            elif x<50 and y >50:
                sum = np.sum(im[y - 50:y + 50, 0:x])
            elif y<50 and x >50:
                sum = np.sum(im[0:y, x-50:x+50])
            elif y>im.shape[0]-50 and x<im.shape[1]-50:
                sum = np.sum(im[y:im.shape[0], x - 50:x + 50])
            elif y < im.shape[0] - 50 and x > im.shape[1] - 50:
                sum = np.sum(im[y - 50:y + 50, x:im.shape[1]])
            if (sum > 255 * 100 * 100 * 0.5):
                color[y,x,0] = 0
    cv2.imshow("multi", color)
    cv2.waitKey(0)

