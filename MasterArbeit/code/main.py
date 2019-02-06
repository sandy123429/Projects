# import Preprocessing_Tile_Images
# import Preprocessing_Phantast
import augmentation
import UNet_create_lmdb_meanfile

import UNet_create_Train_Val_prototxt
import UNet_create_Solver_prototxt
import UNet_train
import UNet_create_deploy_prototxt
import UNet_deploy
import functions
import paths
import cv2
import datetime as dt

# data preprocessing
# Preprocessing_Tile_Images.init_tile_image()
# Preprocessing_Phantast.generate_phantast_segmentation()
# Preprocessing_Phantast.init_save_mask_draw_contour()

# create lmdb data from images
# UNet_create_lmdb_meanfile.init()
# UNet_create_lmdb_meanfile.visualize_lmdb()

# data training with UNet
# UNet_create_Train_Val_prototxt.write_net()
# UNet_create_Solver_prototxt.generate_solver()
# UNet_train.train(FromSolverState=False)
# UNet_train.plot_acc_loss('2018-05-15_16-29')
# UNet_train.visualize_weight()

# data testing with deploy network
# UNet_create_deploy_prototxt.write_net()
# UNet_deploy.deploy_Full_Image()
# UNet_deploy.deploy_single()
