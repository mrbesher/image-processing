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

def gakernel(sidesize=5, sig=1.0):
    """
    creates gaussian kernel with side size of `sidesize` and sigma of `sig`
    credit: https://stackoverflow.com/a/43346070
    """
    ax = np.linspace(-(sidesize - 1) / 2., (sidesize - 1) / 2., sidesize)
    gauss = np.exp(-0.5 * np.square(ax) / np.square(sig))
    kernel = np.outer(gauss, gauss)
    return kernel / np.sum(kernel)