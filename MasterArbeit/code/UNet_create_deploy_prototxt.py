from caffe import layers as L, params as P
import caffe
from paths import *
from img_property import *


def conv_relu(bottom, ks, stride, nout, pad, runBatch):
    conv = L.Convolution(bottom, kernel_size=ks, stride=stride, num_output=nout, pad=pad,
                         convolution_param={'weight_filler': dict(type='xavier')},
                         param=[{'lr_mult': 1, 'decay_mult': 1}, {'lr_mult': 2, 'decay_mult': 0}])

    if runBatch:
        bn = L.BatchNorm(conv)
        scale = L.Scale(bn, scale_param={'bias_term': True})
        return conv, L.ReLU(scale, in_place=True)
    else:
        return conv, L.ReLU(conv, in_place=True)


def upconv_concat(bottom, ks, stride, nout, pad, crop_offset, cat):
    upconv = L.Deconvolution(bottom, convolution_param=dict(num_output=nout, kernel_size=ks, stride=stride, pad=pad,
                                                            weight_filler=dict(type='constant', value=1)
                                                            ),param=[{'lr_mult': 1, 'decay_mult': 1},{'lr_mult': 2, 'decay_mult': 0}])
    crop_cat = L.Crop(cat, upconv, axis=2, offset=crop_offset)
    return upconv, L.Concat(upconv, crop_cat)  # channel up to nout *2


def create_UNet():
    n = caffe.NetSpec()
    n.data = L.Input(include={'phase': caffe.TEST}, input_param={'shape': {'dim': [1, 1, SEG_MASK_WIDTH, SEG_MASK_HEIGHT]}})

    # encoder => level 1
    n.conv1_1, n.relu1_1 = conv_relu(n.data, 3, 1, 64, 0, True)  # conv_relu(bottom, kernel_size, stride, nout, pad):
    n.conv1_2, n.relu1_2 = conv_relu(n.relu1_1, 3, 1, 64, 0, True)
    n.pool1 = L.Pooling(n.relu1_2, pool=P.Pooling.MAX, kernel_size=2, stride=2)

    # encoder => level 2
    n.conv2_1, n.relu2_1 = conv_relu(n.pool1, 3, 1, 128, 0, True)
    n.conv2_2, n.relu2_2 = conv_relu(n.relu2_1, 3, 1, 128, 0, True)
    n.pool2 = L.Pooling(n.relu2_2, pool=P.Pooling.MAX, kernel_size=2, stride=2)

    # encoder => level 3
    n.conv3_1, n.relu3_1 = conv_relu(n.pool2, 3, 1, 256, 0, True)
    n.conv3_2, n.relu3_2 = conv_relu(n.relu3_1, 3, 1, 256, 0, True)
    n.pool3 = L.Pooling(n.relu3_2, pool=P.Pooling.MAX, kernel_size=2, stride=2)

    # encoder => level 4
    n.conv4_1, n.relu4_1 = conv_relu(n.pool3, 3, 1, 512, 0, True)
    n.conv4_2, n.relu4_2 = conv_relu(n.relu4_1, 3, 1, 512, 0, True)
    n.pool4 = L.Pooling(n.relu4_2, pool=P.Pooling.MAX, kernel_size=2, stride=2)

    # encoder => level 5
    n.conv5_1, n.relu5_1 = conv_relu(n.pool4, 3, 1, 1024, 0, True)
    n.conv5_2, n.relu5_2 = conv_relu(n.relu5_1, 3, 1, 1024, 0, True)

    # (bottom, ks, stride, nout, pad, crop_offset, cat)
    n.upconv1, n.concat1 = upconv_concat(n.relu5_2, 2, 2, 512, 0, 4, n.relu4_2)

    # decoder => level 1
    n.conv6_1, n.relu6_1 = conv_relu(n.concat1, 3, 1, 512, 0, True)
    n.conv6_2, n.relu6_2 = conv_relu(n.relu6_1, 3, 1, 512, 0, True)
    n.upconv2, n.concat2 = upconv_concat(n.relu6_2, 2, 2, 256, 0, 16, n.relu3_2)

    # decoder => level 2
    n.conv7_1, n.relu7_1 = conv_relu(n.concat2, 3, 1, 256, 0, True)
    n.conv7_2, n.relu7_2 = conv_relu(n.relu7_1, 3, 1, 256, 0, True)
    n.upconv3, n.concat3 = upconv_concat(n.relu7_2, 2, 2, 128, 0, 40, n.relu2_2)

    # decoder => level 3
    n.conv8_1, n.relu8_1 = conv_relu(n.concat3, 3, 1, 128, 0, True)
    n.conv8_2, n.relu8_2 = conv_relu(n.relu8_1, 3, 1, 128, 0, True)
    n.upconv4, n.concat4 = upconv_concat(n.relu8_2, 2, 2, 64, 0, 88, n.relu1_2)

    # decoder => level 4
    n.conv9_1, n.relu9_1 = conv_relu(n.concat4, 3, 1, 64, 0, True)
    n.conv9_2, n.relu9_2 = conv_relu(n.relu9_1, 3, 1, 64, 0, True)
    n.score = L.Convolution(n.relu9_2, kernel_size=1, num_output=3, pad=0)

    # n.labelcrop = L.Crop(n.label, n.score, crop_param={'axis': 2, 'offset': 92})
    # n.loss = L.SoftmaxWithLoss(n.score, n.labelcrop, loss_param={'ignore_label': 3}, propagate_down=[True, False])
    n.argmax = L.ArgMax(n.score, argmax_param={'axis': 1}, include=dict(phase=caffe.TEST))
    # n.acc, accuracy_by_class = L.Accuracy(n.score, n.labelcrop, accuracy_param={'axis': 1}, include=dict(phase=caffe.TEST), ntop=2)
    # n.confmat = L.Python(n.argmax, n.labelcrop, python_param={'module': 'python_confmat', 'layer': 'PythonConfMat', 'param_str': '{"test_iter":3780}'}, include=dict(phase=caffe.TEST))

    return n.to_proto()


def write_net():
    with open(DEPLOY_PROTOTXT, 'w') as f:
        f.write(str(create_UNet()))

