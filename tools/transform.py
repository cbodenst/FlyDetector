import h5py as h5
import numpy as np
import cv2
from PIL import Image
import os
from sys import argv, exit

if len(argv) < 3:
    print("Usage: python",argv[0],"input_dir output_file")
    exit(-1)
inputfolder = argv[1]
outputfile = argv[2]

vial_h = 350
vial_w = 350

crop = 200

images = []

  
out = h5.File(outputfile,"a")
if (not "data" in out):
    data = out.create_dataset('data', (0,3,vial_h, vial_w), maxshape=(None,3,vial_h, vial_w),dtype='u1')
    label = out.create_dataset('label', (0,1), maxshape=(None,1),dtype='u1')
out.close()

for subdir, dirs, files in os.walk(inputfolder):
    for file in files:
        images.append(os.path.join(subdir, file))

for image in images:
    print(image)
    img = cv2.imread(image)
    img = cv2.copyMakeBorder(img,vial_h,vial_h,vial_w,vial_w,cv2.BORDER_CONSTANT,value=[0,255,0])
    h = img.shape[0]
    w = img.shape[1]
    
    #find vials
    hsv = cv2.cvtColor(img, cv2.COLOR_RGB2HSV)
    mask = cv2.inRange(hsv, np.array([0,150,0]), np.array([255,255,255]))
    contours = cv2.findContours(mask, cv2.RETR_LIST,cv2.CHAIN_APPROX_NONE)

    vials = []
    vialSize = np.pi * 150**2 

    min = vialSize * 0.6
    max = vialSize * 1.4

    for c in contours[0]:
	    area =  cv2.contourArea(c) 
	    if area > min and area < max:
		    vials.append(c)
    
    for v in vials:
        center = v.mean(0).squeeze().astype(np.int)
        vial = img[center[1]-vial_h/2:center[1]+vial_h/2,center[0]-vial_w/2:center[0]+vial_w/2]
        Image.fromarray(vial).show()
        skipImage = False
        skipVial = False
        while True:
            try:
                s = raw_input("How many flies can you see? (x->skip vial; X->skip image; exit->exit)")
                if s=="exit":
                    exit(0)  
                elif s=="x":
                    skipVial = True
                    break
                elif s=="X":
                    skipImage = True
                    break
                flies = int(s)
                break
            except ValueError:
                print("Not a number try again!")
        os.system('killall display')
        if skipVial:
            continue
        if skipImage:
            break
        vial = np.rollaxis(vial,2)
        out = h5.File(outputfile,"a")
        data = out["data"]
        label = out["label"]
        idx = data.shape[0]
        new_data_shape = (idx+1,) + data.shape[1:]
        new_label_shape = (idx+1,) + label.shape[1:]
        data.resize(new_data_shape)
        label.resize(new_label_shape)
        data[idx] = vial
        label[idx]=int(flies)
        out.close()           

