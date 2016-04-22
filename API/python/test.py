import numpy as np
from FlyDetector import *
import h5py as h5
from PIL import Image
import sys


if sys.argv !=  6:
    print("Usage: python test.py input.h5 treshold epsilon minPts PixelsPerFly")
input_path = sys.argv[1]
label_count = dict()
mse_count = dict()
flies = h5.File(input_path)
images = flies["data"][:]
label = flies["label"][:,0]
fc = FlyCounter()
fc.setThreshold(int(sys.argv[2]))
fc.setEpsilon(int(sys.argv[3]))
fc.setMinPoints(int(sys.argv[4]))
fc.setPixelsPerFly(int(sys.argv[5]))
mse = 0.0;
ssm = np.sum(np.square(label -label.mean()))
    
for i in range(images.shape[0]):
    print(i+1,"/",images.shape[0])
    img = np.rollaxis(images[i],0,3).copy()
    vials = findVials(img, 150)
    f = fc.count(img, vials)
    l =  label[i]
    mse += (f - l)**2
    if l in label_count:
        label_count[l] += 1
        mse_count[l] += (f-l)**2
    else:
        label_count[l]=1
        mse_count[l] = (f-l)**2 
r2 = 1 - mse / ssm
mse /= images.shape[0]
print("MSE:",mse)
print("R2:",r2)
print("Class Distribution:")
for k in label_count:
    print(k, label_count[k])
print("MSE Per Class:")
for k in mse_count:
    print(k, mse_count[k]/label_count[k])
