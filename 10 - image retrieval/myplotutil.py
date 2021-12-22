from PIL import Image
import matplotlib.pyplot as plt
import os
import mymodel

def remove_ticks(a):
    a.set_xticks([])
    a.set_yticks([])
    a.spines['top'].set_visible(False)
    a.spines['right'].set_visible(False)
    a.spines['bottom'].set_visible(False)
    a.spines['left'].set_visible(False)
    
def add_images_plt_row(similar, attr='rgb', row = 0):
    cols = len(similar[attr])+1
    for i, img in enumerate(similar[attr], 1):
        im = Image.open(img['path'])
        im = im.resize((50, 50))
        ax = plt.subplot2grid((2,cols), (row,i))
        if (i==1):
            ax.set_ylabel(attr)
        ax.imshow(im, vmin=0, vmax=255)
        ax.set_title("{}".format(round(img[attr], 3)))
        imgno = os.path.basename(img['path'])
        imgno = os.path.splitext(imgno)[0][-2:]
        ax.set_xlabel("{}/{}".format(img['tag'],imgno))
        remove_ticks(ax)
        
def show_results_img(currimg,label=None):
    similar = mymodel.retrieve_similar_images(currimg, limit=5)
    cols = len(similar['hue'])+1
    
    fig = plt.figure()

    im = Image.open(currimg)
    im = im.resize((40, 40))
    axmain = plt.subplot2grid((2, cols), (0, 0), rowspan=2)
    axmain.imshow(im, vmin=0, vmax=255)
    remove_ticks(axmain)
    imgname = os.path.basename(currimg)
    imgname = os.path.splitext(imgname)[0]
    axmain.set_xlabel(imgname)
    if label is not None:
        axmain.set_title(label)
    
    add_images_plt_row(similar,attr='rgb', row=0)
    add_images_plt_row(similar, attr='hue', row=1)

    fig.set_size_inches(12, 5)
    plt.show()

def show_results_category(category, directory='assets', offset=20, limit=10):
    """
    returns success rate for a `limit` images in `category` starting from `offset`
    as a tuple (rgb rate, hue rate)
    """
    categorypath = os.path.join(directory, category)
    files = next(os.walk(categorypath), (None, None, []))[2]
    files.sort()
    # for `limit` files in directory after offset
    for filename in files[offset:offset+limit]:
        currimg = os.path.join(categorypath, filename)
        show_results_img(currimg, label=category)
        
def show_results_cats(categories=['elephant', 'flamingo', 'kangaroo', 'leopards', 'octopus', 'seahorse']):
    for cat in categories:
        show_results_category(cat)