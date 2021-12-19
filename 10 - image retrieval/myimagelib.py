from PIL import Image
import math


def get_img_data(filepath):
    image_data = Image.open(filepath).getdata()
    return list(image_data)

def dist_images_data(a, b):
    """
    returns distances between rgb histograms of a and b
    and between hue histograms of a and b
    as a tuple (rgbdist, huedist)
    """
    rgbhistA = calc_rgb_histogram(a)
    rgbhistB = calc_rgb_histogram(b)
    rgbdist = dist_histograms(rgbhistA,rgbhistB)
    huehistA = calc_hue_histogram(a)
    huehistB = calc_hue_histogram(b)
    huedist = dist_hist(huehistA, huehistB)
    return (rgbdist, huedist)


def dist_histograms(ahists, bhists):
    """
    returns the euclidean distance between n histograms
    """
    distances = [dist_hist(a, b)**2 for a, b in zip(ahists, bhists)]
    return math.sqrt(sum(distances))


def dist_hist(a, b):
    """
    returns the euclidean distance between two histograms
    sqrt of sum of distance squares for each value in the histogram
    """
    dist = [(x-y)**2 for x, y in zip(a, b)]
    return math.sqrt(sum(dist))


def calc_rgb_hue_hists(imagedata):
    """
    returns the rgb and hue histograms for a list on pixels
    (histR, histG, histB, histHue)
    """
    histR, histG, histB = calc_rgb_histogram(imagedata)
    histHue = calc_hue_histogram(imagedata)
    return (histR, histG, histB, histHue)


def calc_rgb_histogram(imagedata):
    """
    calculates normalized R,G,B histograms
    returns histograms tuple (histR, histG, histB)
    """
    histR = [0] * 256
    histG = [0] * 256
    histB = [0] * 256
    for (r, g, b) in imagedata:
        histR[r] += 1
        histG[g] += 1
        histB[b] += 1
    histR = [x / len(imagedata) for x in histR]
    histG = [x / len(imagedata) for x in histG]
    histB = [x / len(imagedata) for x in histB]
    return (histR, histG, histB)


def calc_hue_histogram(imagedata):
    histHue = [0] * 360
    for t in imagedata:
        hue = rgbtohue(t)
        histHue[hue] += 1
    histHue = [x / len(imagedata) for x in histHue]
    return histHue


def rgbtohue(pixel):
    r, g, b = pixel
    r, g, b = r / 255, g / 255, b / 255
    maxChannel = max(r, g, b)
    minChannel = min(r, g, b)

    if minChannel == maxChannel:
        return 0

    if r == maxChannel:
        hue = (g-b) / (maxChannel-minChannel)

    elif g == maxChannel:
        hue = 2.0 + (b-r) / (maxChannel-minChannel)

    else:
        hue = 4.0 + (r-g)/(maxChannel - minChannel)

    return (round(hue * 60) + 360) % 360
