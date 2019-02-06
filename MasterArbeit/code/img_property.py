import math

# IS_GPU = True
IS_GPU = False

NUM_TRAIN_TILE = 140
NUM_VAL_TILE = 35
NUM_AUG = 4
IMG_TILE_WIDTH = 2052
IMG_TILE_HEIGHT = 2052
IMG_EXT_SIZE =  2236#2236
SEG_MASK_WIDTH = 412#412
SEG_MASK_HEIGHT = 412#412
SEG_MASK_STRIDE = 228#228

BATCHSIZE = 1
NUM_TRAIN_EPOCH =14
NUM_VAL_EPOCH = 14

NUM_TRAIN_SEG_MASK = int(math.pow((IMG_EXT_SIZE - SEG_MASK_WIDTH) / SEG_MASK_STRIDE+1, 2) * NUM_TRAIN_TILE * NUM_AUG)
NUM_TRAIN_ITER_PER_EPOCH = int(math.ceil(NUM_TRAIN_SEG_MASK / BATCHSIZE))
MAX_ITER_TRAIN = int(NUM_TRAIN_EPOCH * NUM_TRAIN_ITER_PER_EPOCH)

NUM_VAL_SEG_MASK = int(math.pow((IMG_EXT_SIZE - SEG_MASK_WIDTH) / SEG_MASK_STRIDE+1, 2) * NUM_VAL_TILE * NUM_AUG)
NUM_VAL_ITER_PER_EPOCH = int(math.ceil(NUM_VAL_SEG_MASK / BATCHSIZE))
MAX_ITER_VAL = int(NUM_VAL_EPOCH * NUM_VAL_ITER_PER_EPOCH)