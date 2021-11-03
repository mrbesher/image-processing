import numpy as np
def conv2d(a, f):
    s = f.shape + tuple(np.subtract(a.shape, f.shape) + 1)
    strd = np.lib.stride_tricks.as_strided
    subM = strd(a, shape = s, strides = a.strides * 2)
    return np.einsum('ij,ijkl->kl', f, subM)

def minmaxnorm(data):
    max = np.max(data)
    min = np.min(data)
    scaleval = 255/(max-min)
    return data * scaleval