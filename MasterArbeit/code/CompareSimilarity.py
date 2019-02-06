import numpy as np
import cv2
import functions as fc
from skimage.measure import compare_ssim, compare_mse

directory1 = 'D:/SublimeTextProject/MasterArbeit/code/RunFullImage/412/'
directory2 = 'D:/SublimeTextProject/MasterArbeit/code/RunFullImage/398/'

file_list1 = fc.FilesInDir(directory1, r'((^(\d)+_2018-10-12-1111_bat1_iter_178500.png$))')
file_list2 = fc.FilesInDir(directory2, r'((^(\d)+_2018-10-12-1111_bat1_iter_178500.png$))')

for i1, i2 in zip(file_list1, file_list2):
	img1 = cv2.imread(i1, cv2.IMREAD_GRAYSCALE)	
	img2 = cv2.imread(i2, cv2.IMREAD_GRAYSCALE)
	print img1.shape, img2.shape
	print img1-img2
	mse = compare_mse(img1, img2)
	print "MSE :", mse

