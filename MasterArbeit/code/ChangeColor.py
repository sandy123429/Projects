import numpy as np
import cv2

img = cv2.imread("C:/Users/Sandy/Pictures/2.tif", cv2.IMREAD_COLOR)

x, y, z = np.nonzero( img !=  0 )
img[x,y]=(0,0,255)
cv2.imshow("test", img)
cv2.waitKey(0)
cv2.imwrite("C:/Users/Sandy/Pictures/2_color.tif", img)