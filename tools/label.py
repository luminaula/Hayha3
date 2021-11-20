import xml.etree.ElementTree as ET
import os
import random
import sys



classes = ["0","1","0head","1head"]


imageFolder = "images"
annotationFolder = "annotations"
labelFolder = "images"






def convert(size, box):
    dw = 1./(size[0])
    dh = 1./(size[1])
    x = (box[0] + box[1])/2.0 - 1
    y = (box[2] + box[3])/2.0 - 1
    w = box[1] - box[0]
    h = box[3] - box[2]
    x = x*dw
    w = w*dw
    y = y*dh
    h = h*dh
    return (x,y,w,h)


def main():
    if len(sys.argv) < 2:
        return

    wd = sys.argv[1]
    print(wd)

    trainFilename = "{}/train.txt".format(wd)
    valFilename = "{}/val.txt".format(wd)
    namesFilename = "{}/cs.names".format(wd)

    trainFile = open(trainFilename, "w")
    valFile = open(valFilename, "w")
    namesFile = open(namesFilename, "w")

    for cl in classes:
        namesFile.write(cl + "\n")

    path_imageFolder = "{}/{}".format(wd,imageFolder)
    path_annotation = "{}/{}".format(wd,annotationFolder)
    path_label = "{}/{}".format(wd,labelFolder)
    files = [f for f in os.listdir(path_annotation) if os.path.isfile(os.path.join(path_annotation,f))]

    count = 0
    for filu in files:
        path_imageFile = "{}/{}".format(path_imageFolder,filu)
        path_annotationFile = "{}/{}".format(path_annotation,filu)
        path_imagelabel = "{}/{}".format(path_label,filu)
        
        path_imagelabel = path_imagelabel[:-3]
        path_imagelabel += "txt"

        path_imageFile = path_imageFile[:-3]
        path_imageFile += "jpg"

        #print(path_annotationFile)
        #print(path_imagelabel)
        #print(path_imageFile)

        annotationFile = open(path_annotationFile)
        labelFile = open(path_imagelabel,"w")

        tree=ET.parse(annotationFile)
        root = tree.getroot()

        w = int(root.find('width').text)
        h = int(root.find('height').text)
        
        for obj in root.iter('object'):
            cls = obj.find('objName').text
            if cls not in classes:
                classes.append(cls)
            cls_id = classes.index(cls)
            b = (float(obj.find('xmin').text), float(obj.find('xmax').text), float(obj.find('ymin').text), float(obj.find('ymax').text))
            bb = convert((w,h), b)
            labelFile.write(str(cls_id) + " " + " ".join([str(a) for a in bb]) + '\n')


        randomNumber = random.randint(0,2)
        if randomNumber != 1:
            trainFile.write(path_imageFile + "\n")
            #print("train")
        else:
            valFile.write(path_imageFile + "\n")
            #print("validate")

        count += 1
        print(count)

        annotationFile.close()
        labelFile.close()


main()