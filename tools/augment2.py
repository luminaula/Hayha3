import imgaug as ia
from imgaug import augmenters as iaa

import os
import xml.etree.ElementTree as ET
import numpy as np
import cv2
import sys

ia.seed(1)

BOX_COLOR = (255, 0, 0)
TEXT_COLOR = (255, 255, 255)

classes = ["0","1"]

imageFolder = "images"
annotationFolder = "annotations"

def draw_bbox(img, bbox, id, color=BOX_COLOR, thickness=2):
    x_min = bbox.x1
    y_min = bbox.y1
    x_max = bbox.x2
    y_max = bbox.y2
    x_min, y_min, x_max, y_max = int(x_min),int(y_min),int(x_max),int(y_max)
    cv2.rectangle(img, (x_min, y_min), (x_max, y_max), color=color, thickness=thickness)

    print("{} {} {} {}".format(x_min,y_min,x_max,y_max))

    class_name = str(id)
    ((text_width, text_height), _) = cv2.getTextSize(class_name, cv2.FONT_HERSHEY_SIMPLEX, 0.35, 1)
    cv2.rectangle(img, (x_min, y_min - int(1.3 * text_height)), (x_min + text_width, y_min), BOX_COLOR, -1)
    cv2.putText(img, class_name, (x_min, y_min - int(0.3 * text_height)), cv2.FONT_HERSHEY_SIMPLEX, 0.35, TEXT_COLOR, lineType=cv2.LINE_AA)
    return img


def visualize(img, bboxes, ids):
    index = 0
    for bbox in bboxes.bounding_boxes:
        img = draw_bbox(img, bbox, ids[index])
        index += 1
    cv2.imshow("a",img)
    cv2.waitKey(0)

def main():
    if len(sys.argv) < 2:
        return

    wd = sys.argv[1]
    print(wd)
    path_imageFolder = "{}/{}".format(wd, imageFolder)
    path_annotation = "{}/{}".format(wd, annotationFolder)

    files = [f for f in os.listdir(path_annotation) if os.path.isfile(os.path.join(path_annotation, f))]

    
    aug = iaa.SomeOf((0,4),[
            iaa.Affine(rotate=45),
            iaa.AdditiveGaussianNoise(scale=0.2*255),
            iaa.Add(50, per_channel=True),
            iaa.Sharpen(alpha=0.5)
        ], random_order=True)

    aug_det = aug._to_deterministic()

    for filu in files:

        

        path_imageFile = "{}/{}".format(path_imageFolder, filu)
        path_annotationFile = "{}/{}".format(path_annotation, filu)

        path_imageFile = path_imageFile[:-3]
        path_imageFile += "jpg"

        img = cv2.imread(path_imageFile)
        annotation_file = open(path_annotationFile)
        tree = ET.parse(annotation_file)
        root = tree.getroot()

        bboxes = []
        cat_id = []

        for obj in root.iter('object'):
            clas = obj.find('objName').text
            xmin = int(obj.find('xmin').text)
            xmax = int(obj.find('xmax').text)
            ymin = int(obj.find('ymin').text)
            ymax = int(obj.find('ymax').text)
            bboxes.append(ia.BoundingBox(x1 = xmin, y1 = ymin, x2 = xmax, y2 = ymax))
            cat_id.append(classes.index(clas))


        ia_boxes = ia.BoundingBoxesOnImage(bboxes, shape=img.shape)
        

        aug_img = aug.augment_images([img])[0]
        aug_bbox = aug.augment_bounding_boxes([ia_boxes])[0]

        visualize(aug_img, aug_bbox, cat_id)

main()
