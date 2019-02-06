import scipy.io as sio
from skimage import io
from paths import *
import cv2

# crop and tile image into 4x4 tiles
def tile_image(imgloadpath, imgsavepath):
    for img in os.listdir(imgloadpath):
        path = os.path.join(imgloadpath, img)
        if not os.path.isdir(path) and img.split('.')[1] == 'bmp':
            imgname, imgtype = img.split('.')
            imgname = '/'+imgname
            print path
            img_data = cv2.imread(path,cv2.IMREAD_GRAYSCALE)
            lx, ly = img_data.shape
            for y in xrange(4):
                for x in xrange(4):
                    img_crop = img_data[lx/3+x*2052: lx/3+(x+1)*2052, ly/3+y*2052: ly/3+(y+1)*2052]
                    # clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(50, 50))
                    # img_crop = clahe.apply(img_crop)
                    try:
                        os.makedirs(imgsavepath + imgname)
                    except OSError:
                        if not os.path.isdir(imgsavepath + imgname):
                            raise
                    io.imsave(imgsavepath + imgname + '/crop_' + str(x) + '_' + str(y) +'.'+ imgtype, img_crop)
                    sio.savemat(imgsavepath + imgname + '/crop_' + str(x) + '_' + str(y) +'.mat', dict(img = img_crop))


def init_tile_image():
    tile_image(TRAIN_FOLDER, CROP_PHANTAST_TRAIN)
    # tile_image(VAL_FOLDER, CROP_PHANTAST_VAL)