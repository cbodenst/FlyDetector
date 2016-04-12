import numpy as np
from FlyDetector import FlyCounter
import h5py as h5
from PIL import Image

fc = FlyCounter()
flies = h5.File("/home/cboden/Extern/Data/Flies/flies2.h5")
images = flies["data"]
fc.setVialSize(150)
fc.setThreshold(100)
fc.setEpsilon(5)
fc.setMinPoints(40)
fc.setPixelsPerFly(200)
for i in range(1):#images.shape[0]):
    img = np.rollaxis(images[i],0,3)
    g = fc.generateThresholdImage(img)
    Image.fromarray(g).show()
    f = fc.count(img)
    print(f)
