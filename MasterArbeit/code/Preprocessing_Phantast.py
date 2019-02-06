import scipy.io as sio
from skimage import io
import os
import matlab.engine
import functions as fc
import cv2
from paths import *


# run matlab phantast code
def generate_phantast_segmentation():
    eng = matlab.engine.start_matlab()
    eng.cd(PHANTAST_DIR)
    eng.PHANTAST_generate_mat_files(nargout=0)
    eng.quit()


# display phantast result
def save_phantast_mask_and_draw_contour(mat_root):
    for mat_dir in fc.SubDirPath(mat_root):
        dirpath = os.path.join(mat_root, mat_dir)
        for img in os.listdir(dirpath):
            matname, mattype = img.split('.')
            mat_path = os.path.join(dirpath, img)
            if not os.path.isdir(mat_path) and mattype == 'mat':
                mat = sio.loadmat(mat_path)
                mat_path_noextension = os.path.join(dirpath, matname)
                if(mat.has_key('ImagePhantast')):
                    img = mat['ImagePhantast']*255
                    io.imsave(os.path.join(dirpath, matname) + '.tif', img)
                    im = cv2.imread(mat_path_noextension + '.tif', 0)
                    imgdrawpath = mat_path_noextension.replace("_phantast_result", "")
                    imgdraw = cv2.imread(imgdrawpath + '.bmp', 0)
                    _, contours, _ = cv2.findContours(im.copy(), cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
                    img = cv2.drawContours(imgdraw, contours, -1, (255, 255, 255), 3)
                    io.imsave(mat_path_noextension + '_contour.tif', img)


def init_save_mask_draw_contour():
    save_phantast_mask_and_draw_contour(CROP_PHANTAST_TRAIN)
    save_phantast_mask_and_draw_contour(CROP_PHANTAST_VAL)

