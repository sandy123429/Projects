import os
ROOT_DIR = 'D:/SublimeTextProject/MasterArbeit/'
ROOT_DIR = ROOT_DIR.replace("\\", "/")
DATA_ROOT = ROOT_DIR + '/data'
TRAIN_FOLDER = DATA_ROOT + '/train'  # original training data
VAL_FOLDER = DATA_ROOT + '/val'  # original validation data
CROP_PHANTAST_TRAIN = DATA_ROOT + '/phantast_train'  # training data after running phantast
CROP_PHANTAST_VAL = DATA_ROOT + '/phantast_val'  # validation data after running phantast
CROP_PHANTAST_TEST = DATA_ROOT + '/phantast_test' # test data after training
TRAIN_DATA_LMDB = DATA_ROOT + '/train_data_lmdb'  # training data saved as lmdb
TRAIN_LABEL_LMDB = DATA_ROOT + '/train_label_lmdb'  # training label saved as lmdb
VAL_DATA_LMDB = DATA_ROOT + '/val_data_lmdb'  # validation data saved as lmdb
VAL_LABEL_LMDB = DATA_ROOT + '/val_label_lmdb'  # validation label saved as lmdb

TRAIN_FILELIST = DATA_ROOT + '/filelist_train.txt'
VAL_FILELIST = DATA_ROOT + '/filelist_val.txt'
TRAIN_MEAN_FILE = DATA_ROOT + '/mean.binaryproto'
TRAIN_PROTOTXT = DATA_ROOT + '/train.prototxt'
VAL_PROTOTXT = DATA_ROOT + '/val.prototxt'
DEPLOY_PROTOTXT = DATA_ROOT + '/deploy.prototxt'
SOLVER_PROTOTXT = DATA_ROOT + '/solver.prototxt'


DEPLOY_MODEL=''
DEPLOT_IMG=''


PHANTAST_DIR = ROOT_DIR + '/PHANTAST'  # matlab folder of phantast

TOOLS_DIR = ROOT_DIR + '/tools'
SNAPSHOT_DIR = ROOT_DIR + '/snapshots'
MEAN_TOOL_PATH = TOOLS_DIR + '/compute_image_mean.exe'  # the location of the caffe tool: compute_image_mean.exe
CAFFE_TOOL_PATH = TOOLS_DIR + '/caffe'

LOG_DIR = ROOT_DIR + '/log_results'
