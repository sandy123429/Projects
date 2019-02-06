import numpy as np
import cv2
from paths import *
from functions import *

c=0
for path in FilesInDir("F:/MA_Yang/code/data/phantast_val/2017-04-05_14-43-39_B1_universal", r'((^crop_\d_\d.bmp$))'):

    print path
    original_im = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
    im = original_im

    '''
    point_path = str.replace(path, 'crop', 'Results')
    point_path = str.replace(point_path, 'bmp', 'txt')
    print point_path
    points = []

    with open(point_path) as f:
        content = f.readlines()
        content = [x.strip() for x in content]
        for i in range(1, len(content)):
            pt = content[i].split('\t')
            points.append((int(pt[0]), int(pt[1])))

    label_path = path.replace('.bmp', '_phantast_result.tif')
    label_img = cv2.imread(label_path, cv2.IMREAD_GRAYSCALE)
    color = cv2.cvtColor(im, cv2.COLOR_GRAY2RGB)

    for pt in points:
        cv2.circle(color, pt, 6, (0, 0, 255), -1)
        cv2.circle(label_img, pt, 6, 85, -1)

    cv2.imwrite(label_path, label_img)
    cv2.namedWindow('deadcells', cv2.WINDOW_NORMAL)
    cv2.imshow('deadcells', color)
    cv2.namedWindow('deadcellslabel', cv2.WINDOW_NORMAL)
    cv2.imshow('deadcellslabel', label_img)
    cv2.waitKey(0)

    
    label_path = path.replace('.bmp', '_phantast_result.tif')
    label_img = cv2.imread(label_path, cv2.IMREAD_GRAYSCALE)
    color = cv2.cvtColor(im, cv2.COLOR_GRAY2RGB)

    for pt in points:
        cv2.circle(color, pt, 6, (0, 0, 255), -1)
        cv2.circle(label_img, pt, 6, 85, -1)

    cv2.imwrite(label_path, label_img)

    cv2.namedWindow('deadcells', cv2.WINDOW_NORMAL)
    cv2.imshow('deadcells', color)
    cv2.namedWindow('deadcellslabel', cv2.WINDOW_NORMAL)
    cv2.imshow('deadcellslabel', label_img)
    cv2.waitKey(0)
    '''

    kernel = np.ones((3, 3), np.uint8)
    im = cv2.dilate(im, kernel, iterations=3)

    label_path = path.replace('.bmp', '_phantast_result.tif')
    label_img = cv2.imread(label_path, cv2.IMREAD_GRAYSCALE)

    _, im = cv2.threshold(im, 200, 255, cv2.THRESH_BINARY)
    x, y = np.nonzero(im == 255)
    label_img[x,y] = 128
    cv2.imwrite(label_path, label_img)

    '''
    clahe = cv2.createCLAHE(clipLimit=3.0, tileGridSize=(50,50))
    im = clahe.apply(im)
    cv2.imshow("clahe", im)
    cv2.imwrite('CLAHE' + str(c) + '.bmp', im)
    color = cv2.cvtColor(im, cv2.COLOR_GRAY2RGB)
    cv2.waitKey(0)
    '''

    '''
    _, im = cv2.threshold(im,250, 255, cv2.THRESH_BINARY)
    # kernel = np.ones((3, 3), np.uint8)
    # im = cv2.dilate(im, kernel, iterations=1)
    # cv2.imshow("dilate", im)
    _, contours, _ = cv2.findContours(im, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    color = cv2.drawContours(color, contours, -1, (0 , 255, 255), 1)
    cv2.imshow("keypoints", color)
    cv2.imwrite('Keypoints'+str(c)+'.png', color)
    c+=1
    '''

    #kernel = np.ones((2,2), np.uint8)

    #im = img_dilation = cv2.dilate(im, kernel, iterations=1)
    #im = img_erosion = cv2.erode(im, kernel, iterations=1)
    #cv2.imshow("MORPH", im)

    
    # im = 255 - im
    #im = np.hstack((im, equ))  # stacking images side-by-side

    # Setup SimpleBlobDetector parameters.
    '''
    params = cv2.SimpleBlobDetector_Params()

    # Change thresholds
    params.minThreshold = 180
    params.maxThreshold = 200

    # Filter by Area.
    params.filterByArea = True
    params.minArea = 0
    params.maxArea = 50

    # Filter by Circularity
    params.filterByCircularity = True
    params.minCircularity = 0.08

    # Filter by Convexity
    params.filterByConvexity = False
    params.minConvexity = 0.01

    # Filter by Inertia
    params.filterByInertia = False
    params.minInertiaRatio = 0.2

    detector = cv2.SimpleBlobDetector_create(params)

    # Detect blobs.
    keypoints = detector.detect(im)
    # print keypoints
    # Draw detected blobs as red circles.
    # cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS ensures the size of the circle corresponds to the size of blob
    im_with_keypoints = cv2.drawKeypoints(im, keypoints, np.array([]), (0, 0, 255), cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS)

    # Show keypoints
    cv2.imshow("Keypoints", im_with_keypoints)
    cv2.imwrite('Keypoints.png',im_with_keypoints)
    c += 1
    cv2.waitKey(0)
    '''
