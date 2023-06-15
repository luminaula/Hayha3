import os
import xml.etree.ElementTree as ET
import numpy as np
import cv2
import sys
from tqdm import tqdm
from albumentations import *

from multiprocessing import Process

BOX_COLOR = (255, 0, 0)
TEXT_COLOR = (255, 255, 255)

classes = ["0","1","0head","1head"]

imageFolder = "images"
annotationFolder = "annotations"

if len(sys.argv) < 2:
    quit()

wd = sys.argv[1]
print(wd)

path_images = "{}/{}".format(wd, imageFolder)
path_annotations = "{}/{}".format(wd, annotationFolder)

files = [f for f in os.listdir(path_annotations) if os.path.isfile(os.path.join(path_annotations, f))]

pbar = tqdm(total=len(files))

def get_aug(aug, min_area=0., min_visibility=0.):
    return Compose(aug, bbox_params={'format': 'pascal_voc', 'min_area': min_area, 'min_visibility': min_visibility, 'label_fields': ['category_id']})







def write_annotation(path_annotation, annotations):
    height, width = annotations['image'].shape[:2]
    #print("{}".format(path_annotation))
    annotation_file = open(path_annotation, "w")
    annotation_file.write("<annotation>\n")
    annotation_file.write("<width>{}</width>\n".format(width))
    annotation_file.write("<height>{}</height>\n".format(height))

    for idx, bbox in enumerate(annotations['bboxes']):
        annotation_file.write("<object>\n")
        annotation_file.write("\t<objName>{}</objName>\n".format(annotations['category_id'][idx]))
        annotation_file.write("\t<xmin>{}</xmin>\n".format(int(bbox[0])))
        annotation_file.write("\t<xmax>{}</xmax>\n".format(int(bbox[2])))
        annotation_file.write("\t<ymin>{}</ymin>\n".format(int(bbox[1])))
        annotation_file.write("\t<ymax>{}</ymax>\n".format(int(bbox[3])))
        annotation_file.write("</object>\n")
        #print("{} {} {} {} {}".format(bbox[0], bbox[1], bbox[2], bbox[3], annotations['category_id'][idx]))

    annotation_file.write("</annotation>\n")
    annotation_file.close()



def process_images(annotations):

    aug = get_aug([
    OneOf([
        RandomBrightness(),
        RandomContrast(),
        RandomGamma()
    ],p=0.8),
    #RGBShift(p=0.1),

    #HueSaturationValue(p=0.1),
    OneOf([
        Blur(),
        MedianBlur(),
        MotionBlur(),
        GaussNoise()
    ],p=0.2),
    OneOf([
        CLAHE(),
        IAAAdditiveGaussianNoise(),
        IAASharpen(),
        IAAEmboss()
    ],p=0.15),
    #Normalize(p=0.1),
    #RandomRotate90(p=0.1),
    #JpegCompression(p=0.1, quality_lower=45),
    #OneOf([
    #    RandomCrop(height=900,width=1600),
    #    RandomCrop(height=800,width=800),
    #    RandomSizedCrop(min_max_height=[100,500],height=800,width=1000),
    #    ShiftScaleRotate(p=0.1, scale_limit=0.1),
    #    Cutout(num_holes=512, max_h_size=16, max_w_size=16),
    #    RandomCrop(height=1000, width=600)
    #],p=0.5)
    ],min_visibility=0.33)



    for filu in annotations:
        path_image = "{}/{}".format(path_images, filu)
        path_annotation = "{}/{}".format(path_annotations, filu)

        path_image = path_image[:-3]
        path_image += "jpg"

        annotation_file = open(path_annotation)
        image = cv2.imread(path_image)
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
            bboxes.append([xmin, ymin, xmax, ymax])
            cat_id.append(classes.index(clas))

        annotations = {'image': image, 'bboxes': bboxes, 'category_id': cat_id}
        category_id_to_name = {0: "0", 1: "1"}

        try:
            augmented = aug(**annotations)
        except:
            #print("Some error")
            continue

        if not augmented['bboxes']:
            continue

        annotation_file.close()

        cv2.imwrite(path_image, augmented['image'])
        write_annotation(path_annotation, augmented)
    
        

chunks = [files[x:x+100] for x in range(0, len(files), 100)]

thread_count = 16


while chunks:
    threads = []
    for i in range(0,thread_count):
        if not chunks:
            break
        chunk = chunks.pop()
        
        p = Process(target=process_images, args=([chunk]))
        p.start()
        threads.append(p)
        #process_images(aug, chunk)
    for thread in threads:
        thread.join()
        pbar.update(100)

pbar.close()