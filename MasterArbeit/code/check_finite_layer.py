import caffe
import numpy as np

class checkFiniteLayer(caffe.Layer):
    def setup(self, bottom, top):
        self.prefix = self.param_str

    def reshape(self, bottom, top):
        pass

    def forward(self, bottom, top):
        for i in xrange(len(bottom)):
            isbad = np.sum(1 - np.isfinite(bottom[i].data[...]))
            if isbad > 0:
                raise Exception("checkFiniteLayer: %s forward pass bottom %d has %.2f%% non-finite elements" %
                                (self.prefix, i, 100 * float(isbad) / bottom[i].count))

    def backward(self, top, propagate_down, bottom):
        for i in xrange(len(top)):
            if not propagate_down[i]:
                continue
            isf = np.sum(1 - np.isfinite(top[i].diff[...]))
            if isf > 0:
                raise Exception("checkFiniteLayer: %s backward pass top %d has %.2f%% non-finite elements" %
                                (self.prefix, i, 100 * float(isf) / top[i].count))
