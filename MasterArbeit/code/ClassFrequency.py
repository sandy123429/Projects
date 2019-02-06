import numpy as np
import cv2
import functions as fc
from img_property import *
from functions import *
import matplotlib.pyplot as plt 

count_dead=0
count_cell=0
count_background=0

data_dead = []
data_cell = []
data_back = []

pixels_back = 0
pixels_MSC = 0
pixels_dead = 0
"""
for folder in SubDirPath('D:/SublimeTextProject/MasterArbeit/data/phantast_train/'):
    for paths in FilesInDir(folder,r'((^crop_\d_\d_phantast_result.tif$))'):
        img = cv2.imread(paths, cv2.IMREAD_GRAYSCALE)
        
        for y in fc.my_range(0, IMG_EXT_SIZE - SEG_MASK_HEIGHT + 1, SEG_MASK_STRIDE):  # for (y=0; y<IMG_EXT_SIZE - SEG_MASK_HEIGHT + 1; y+=SEG_MASK_HEIGHT)
            count_d = 0.0
            count_c = 0.0
            count_b = 0.0
            for x in fc.my_range(0, IMG_EXT_SIZE - SEG_MASK_WIDTH + 1, SEG_MASK_STRIDE):
                tmp = img[y:y + SEG_MASK_HEIGHT, x:x + SEG_MASK_WIDTH]
                xx,yy = np.nonzero(tmp==255)
                if (xx.size!=0):
                    pixels_MSC += xx.size
                    count_cell += 1
                    count_c += len(xx)
                xx, yy = np.nonzero(tmp == 128)
                if (xx.size != 0):
                    pixels_dead += xx.size
                    count_dead += 1
                    count_d += len(xx)
                xx, yy = np.nonzero(tmp == 0)
                if (xx.size != 0):
                    pixels_back += xx.size
                    count_background += 1
                    count_b += len(xx)
                s = count_b+count_c+count_d
                count_b /=s
                count_c /=s
                count_d /=s
                data_back.append(count_b)
                data_cell.append(count_c)
                data_dead.append(count_d)
"""
for folder in SubDirPath('D:/SublimeTextProject/MasterArbeit/data/phantast_test/'):
    for paths in FilesInDir(folder,r'((^crop_\d_\d_phantast_result.tif$))'):
        img = cv2.imread(paths, cv2.IMREAD_GRAYSCALE)
        
        for y in fc.my_range(0, IMG_EXT_SIZE - SEG_MASK_HEIGHT + 1, SEG_MASK_STRIDE):  # for (y=0; y<IMG_EXT_SIZE - SEG_MASK_HEIGHT + 1; y+=SEG_MASK_HEIGHT)
            count_d = 0.0
            count_c = 0.0
            count_b = 0.0
            for x in fc.my_range(0, IMG_EXT_SIZE - SEG_MASK_WIDTH + 1, SEG_MASK_STRIDE):
                tmp = img[y:y + SEG_MASK_HEIGHT, x:x + SEG_MASK_WIDTH]
                xx,yy = np.nonzero(tmp==255)
                if (xx.size!=0):
                    pixels_MSC += xx.size
                    count_cell += 1
                    count_c += len(xx)
                xx, yy = np.nonzero(tmp == 128)
                if (xx.size != 0):
                    pixels_dead += xx.size
                    count_dead += 1
                    count_d += len(xx)
                xx, yy = np.nonzero(tmp == 0)
                if (xx.size != 0):
                    pixels_back += xx.size
                    count_background += 1
                    count_b += len(xx)
                s = count_b+count_c+count_d
                count_b /=s
                count_c /=s
                count_d /=s
                data_back.append(count_b)
                data_cell.append(count_c)
                data_dead.append(count_d)



print pixels_back, pixels_dead, pixels_MSC
print count_background, count_dead, count_cell

"""
print max(data_back), max(data_cell), max(data_dead)
print min(data_back), min(data_cell), min(data_dead)

data_to_plot = [data_cell, data_back, data_dead]

# Create a figure instance
fig = plt.figure(1, figsize=(9, 6))

# Create an axes instance
ax = fig.add_subplot(111)

# Create the boxplot
bp = ax.boxplot(data_to_plot)

ax.set_xticklabels(['MSCs', 'Background', 'Dead cells'])
ax.get_xaxis().tick_bottom()
ax.get_yaxis().tick_left()

# Save the figure
fig.savefig('boxplot_val.png', bbox_inches='tight')
"""
