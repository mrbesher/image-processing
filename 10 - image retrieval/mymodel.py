import os
import json
import math
import myimagelib as mylib


def get_histograms(directory, tag, limit):
    """
    calculates and returns histograms for photos in `directory`
    until no photos left or number of photos reaches limit
    """
    files = next(os.walk(directory), (None, None, []))[2]
    files.sort()
    images = []
    print("> processing {} images in {}".format(limit, tag))
    # iterate files up till limit
    for filename in files[:limit]:
        print("{} ".format(filename), end='')
        filepath = os.path.join(directory, filename)
        img = mylib.get_img_data(filepath)
        # get rgb and hue histograms
        r, g, b, hue = mylib.calc_rgb_hue_hists(img)
        images.append({
            "filename": filename,
            "tag": tag,
            "rhist": r,
            "ghist": g,
            "bhist": b,
            "hhist": hue
        })
    print("\n")
    return images


def train(n=20, directory='assets', folders=['elephant', 'flamingo', 'kangaroo', 'leopards', 'octopus', 'seahorse'], dbfile='db.json'):
    images = []
    for foldername in folders:
        folderpath = os.path.join(directory, foldername)
        images.extend(get_histograms(folderpath, foldername, n))
    db = {
        "images": images
    }
    json_object = json.dumps(db, indent=4)
    with open(dbfile, "w") as outfile:
        outfile.write(json_object)


def to_similarity_obj(img, cmphists):
    """
    converts an img object to the following format:
    {
        "tag": img['tag'],
        "path": imgpath,
        "rgb": rgb similarity,
        "hue": hue similarity
    }
    """
    r, g, b, hue = cmphists
    rgbdist = mylib.dist_histograms(
        (r, g, b), (img['rhist'], img['ghist'], img['bhist']))
    rgbdist /= math.sqrt(3)
    huedist = mylib.dist_hist(hue, img['hhist'])
    huesim = 1-huedist
    rgbsim = 1-rgbdist
    return {
        "tag": img['tag'],
        "path": os.path.join('assets', img['tag'], img['filename']),
        "rgb": rgbsim,
        "hue": huesim
    }


def retrieve_similar_images(filepath, limit=5, db='db.json'):
    """
    returns a directory with smilar['hue'] list containing most similar images
    according to hue histograms and similar['rgb'] according to rgb histograms
    """
    with open(db, 'r') as openfile:
        images = json.load(openfile)['images']
    inputimg = mylib.get_img_data(filepath)

    r, g, b, hue = mylib.calc_rgb_hue_hists(inputimg)
    similar = {
        "rgb": [],
        "hue": []
    }

    # add first images up to limit since they are the closest considering only 0..limit
    for img in images[:limit]:
        imgobj = to_similarity_obj(img, (r, g, b, hue))
        similar['rgb'].append(imgobj)
        similar['hue'].append(imgobj)

    for img in images[limit:]:
        imgobj = to_similarity_obj(img, (r, g, b, hue))

        attr = 'rgb'
        mini = min(similar[attr], key=lambda x: x[attr])
        if imgobj[attr] > mini[attr]:
            similar[attr].remove(mini)
            similar[attr].append(imgobj)

        attr = 'hue'
        mini = min(similar[attr], key=lambda x: x[attr])
        if imgobj[attr] > mini[attr]:
            similar[attr].remove(mini)
            similar[attr].append(imgobj)
    
    similar['hue'].sort(key=lambda x: x['hue'], reverse=True)
    similar['rgb'].sort(key=lambda x: x['rgb'], reverse=True)

    return similar

def get_accuracy_category(category, directory='assets', offset=20, limit=10):
    """
    returns success rate for a `limit` images in `category` starting from `offset`
    as a tuple (rgb rate, hue rate)
    """
    categorypath = os.path.join(directory, category)
    files = next(os.walk(categorypath), (None, None, []))[2]
    files.sort()
    succhue = 0
    succrgb = 0
    # for `limit` files in directory after offset
    for filename in files[offset:offset+limit]:
        currimg = os.path.join(categorypath, filename)
        similar = retrieve_similar_images(currimg, limit=5)
        succhue += any(x['tag'] == category for x in similar['hue'])
        succrgb += any(x['tag'] == category for x in similar['rgb'])
    return (succrgb/limit, succhue/limit)

def print_accuracy_categories(categories=['elephant', 'flamingo', 'kangaroo', 'leopards', 'octopus', 'seahorse']):    
    print("category: (rgb accuracy, hue accuracy)")
    total = []
    for cat in categories:
        curr = get_accuracy_category(cat)
        total.append(curr)
        print("{}: {}".format(cat,curr))

    # sum all categories success rates and divide it by the number of categories to get the general accuracy
    curr = (sum(x for x,_ in total) / len(categories), sum(y for _,y in total) / len(categories))
    print("General: {}".format(tuple([round(x, 5) for x in curr])))