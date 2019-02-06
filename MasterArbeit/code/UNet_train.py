import caffe
from paths import *
#import plot_training_log
from img_property import IS_GPU
import numpy as np
import cv2


def train(FromSolverState=False):
    if not IS_GPU:
        caffe.set_mode_cpu()
    else:
        caffe.set_mode_gpu()
        caffe.set_device(0)

    solver = caffe.get_solver(SOLVER_PROTOTXT)
    if FromSolverState:
        solver.restore(ROOT_DIR + '/snapshots/snapshot2018-05-22-1526_iter_12000.solverstate')
    solver.solve()
    solver.net.save('mymodel.caffemodel')
    net = solver.net
    for i in range(0, net.blobs['score'].data.shape[1]):
        out = net.blobs['score'].data[0, i, :, :]
        img = np.array(out).astype(np.uint8)
        cv2.imwrite(str(i) + "score.png", img)


def visualize_weight():
    weights = './snapshots/snapshot2018-06-05-1007_bat2_iter_96768.caffemodel'
    if not IS_GPU:
        caffe.set_mode_cpu()
    else:
        caffe.set_mode_gpu()

    solver = caffe.SGDSolver(SOLVER_PROTOTXT)
    solver.net.copy_from(weights)

    debug_weights = 1

    #if debug_weights:
    #    solver.step(1)
    net = solver.net
    print net.blobs['score'].data.shape
    for i in range(0, net.blobs['score'].data.shape[1]):
         out = net.blobs['score'].data[2, i, :, :]
         img = np.array(out).astype(np.uint8)
         cv2.imwrite(str(i) + "score.png", img)

    #result = solver.net.blobs['conv7_1'].data[0]
    #out = result.argmax(0)
    # out = result[0,:,:]
    #print out, result.shape
    #img = np.array(out).astype(np.uint8)
    # img = img > 0
    # img = img * 255
    #cv2.imwrite("data.png", img)  # save image in the directory

'''
def plot_acc_loss(timestamp):
    """
    0: Test accuracy vs. Iters
    1: Test accuracy vs. Seconds
    2: Test loss vs. Iters
    3: Test loss vs. Seconds
    4: Train learning rate vs. Iters
    5: Train learning rate vs. Seconds
    6: Train loss vs. Iters
    7: Train loss vs. Seconds
    """
    type = 0
    filepath = LOG_DIR + "/" + timestamp + ".log"

    imgpath = LOG_DIR + "/" + timestamp + "_" + str(type) + ".png"
    #plot_training_log.plot([0, imgpath, filepath])

    type = 6
    imgpath = LOG_DIR + "/" + timestamp + "_" + str(type) + ".png"
    plot_training_log.plot([6, imgpath, filepath])
'''