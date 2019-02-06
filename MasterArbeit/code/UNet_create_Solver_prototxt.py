from caffe.proto import caffe_pb2
from paths import *
from img_property import *
import os
import datetime

now = datetime.datetime.now()


def generate_solver():
    s = caffe_pb2.SolverParameter()

    s.net = TRAIN_PROTOTXT
    s.test_net.append(TEST_PROTOTXT)
    s.test_iter.append(NUM_VAL_ITER_PER_EPOCH)
    s.test_interval = NUM_TRAIN_ITER_PER_EPOCH/2
    s.max_iter = MAX_ITER_TRAIN

    s.base_lr = 0.0005 #
    s.momentum = 0.9
    s.weight_decay = 0.001 #
    s.lr_policy = 'step'
    s.stepsize = NUM_TRAIN_ITER_PER_EPOCH*5
    s.gamma = 0.1
    s.display = 10
    s.snapshot = 500
    s.snapshot_prefix = SNAPSHOT_DIR + '/snapshot' + now.strftime("%Y-%m-%d-%H%M")
    s.type = "SGD"
    # s.debug_info = True

    if not IS_GPU:
        s.solver_mode = caffe_pb2.SolverParameter.CPU
    else:
        s.solver_mode = caffe_pb2.SolverParameter.GPU

    os.remove(SOLVER_PROTOTXT)
    with open(SOLVER_PROTOTXT, 'w') as f:
        f.write(str(s))
