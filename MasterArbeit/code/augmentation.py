# gray scale variation (contrastAdjustment)
# Rotation
# blur

import numpy as np
import cv2
import random
from img_property import NUM_AUG
import gc

def rotateImage(img, angle):
    row, col = img.shape
    center = tuple(np.array([row, col]) / 2)
    rot_mat = cv2.getRotationMatrix2D(center, angle, 1.0)
    new_img = cv2.warpAffine(img, rot_mat, (col, row))
    return new_img


def GaussianBlur(img, ks):
    blur = cv2.GaussianBlur(img, (ks, ks), 0)
    return blur


def contrastAdjustment(img):
    beta = random.randint(-50, 50)
    alpha = img * random.uniform(1.1, 1.8)
    img =  alpha + beta
    img = np.clip(img,0,255)
    del alpha,beta
    return img


def DoAugmentation(img):
    # do augmentation for data, and change its label accordingly
    # the structure of the augmented data is 3x2xHxW
    # in which axis 0 represents type of augmentation (org, rotation, contrast adjustment)
    # axis 1 represents data and label indices
    # axis 2 and 3 is the image data
    
    augimg = np.zeros((NUM_AUG, 2, img.shape[1], img.shape[2]))
    augimg[0, :, :, :] = img[:, :, :]

    augimg[1, 0, :, :] = contrastAdjustment(img[0, :, :])
    augimg[1, 1, :, :] = img[1, :, :]

    ks = random.randint(5, 11)
    if ks%2==0:
        ks=ks+1
    augimg[2, 0, :, :] = GaussianBlur(img[0, :, :], ks)
    augimg[2, 1, :, :] = img[1, :, :]

    augimg[3, 0, :, :] = contrastAdjustment(img[0, :, :])
    angle = random.randint(1, 360)
    augimg[3, 0, :, :] = rotateImage(augimg[3, 0, :, :], angle)
    augimg[3, 1, :, :] = rotateImage(img[1, :, :], angle)
    ks2 = random.randint(3, 11)
    if ks2%2==0:
        ks2=ks2+1
    augimg[3, 0, :, :] = GaussianBlur(augimg[3, 0, :, :], ks2)

    return augimg

def AugmentationSingleImg(path):
    img = cv2.imread(path,cv2.IMREAD_GRAYSCALE)
    

    blur = GaussianBlur(img, 17)
    cv2.imwrite('./blur.png', blur)

    grayV = contrastAdjustment(img)
    cv2.imwrite('./grayvariation.png', grayV)

    mix = contrastAdjustment(img)
    angle = random.randint(1, 360)
    mix = rotateImage(mix, 150)
    mix = GaussianBlur(mix, 11)
    cv2.imwrite('./mix.png', mix)

# [0,0,:,:] original data
# [0,1,:,:] original label
# [1,0,:,:] grayscale varied data
# [1,1,:,:] grayscale varied label
# [2,0,:,:] blurred data
# [2,1,:,:] blurred label
# [3,0,:,:] Mix data
# [3,1,:,:] Mix label

def main():
    AugmentationSingleImg('../data/phantast_train/2017-04-05_14-43-39_A1_universal/crop_1_3.bmp')

if __name__ == '__main__':
    main()