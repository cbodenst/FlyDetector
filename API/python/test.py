import numpy as np
from FlyDetector import FlyCounter
import h5py as h5
from PIL import Image

fc = FlyCounter()
flies = h5.File("/home/cboden/Extern/Data/Flies/flies2.h5")
images = flies["data"]
label = flies["label"]
fc.setVialSize(150)
out = h5.File("results.h5")
results = out.create_dataset('run', (0,5), maxshape=(None,5),dtype='f8')
out.close()
for t in range (50,150,5):
    for e in range(2,20):
        for m in range(10,100,2):
            for p in range(50, 400,10):
                fc.setThreshold(t)
                fc.setEpsilon(e)
                fc.setMinPoints(m)
                fc.setPixelsPerFly(p)
                mse = 0.0;
                for i in range(images.shape[0]):
                    img = np.rollaxis(images[i],0,3).copy()
                    f = fc.count(img)
                    mse += (f - label[i][0])**2

                mse /= images.shape[0]
                out = h5.File("results.h5")
                results = out["run"]
                idx = results.shape[0]
                results.resize((idx+1,5))
                results[idx] = np.array([t,e,m,p,mse])
                out.close()
                print("Threshold: %d Epsilon: %d MinPts: %d PPF: %d MSE:%f"%(t,e,m,p,mse))
