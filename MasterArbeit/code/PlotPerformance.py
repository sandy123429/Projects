from paths import *
from img_property import *
import numpy as np
import matplotlib.pyplot as plt


data_x = range(1, 17, 2)
print data_x
data_y = range(0,24,3)
recall = [0.9880221, 0.85097396, 0.7291291,
		  0.9875679, 0.7445166, 0.723944,
		  0.9883618, 0.63313556, 0.6762467,
		  0.98956734, 0.45351195, 0.5767498,
		  0.9907384, 0.34363112, 0.48277652,
		  0.99238104, 0.22552763, 0.37804344,
		  0.9934531, 0.18062066, 0.29302788,
		  0.9950587, 0.12812068, 0.20716475]

f1 = [0.8589938, 0.6810698, 0.83298725,
	  0.8553156, 0.71428424, 0.8291845,
	  0.8372909, 0.6801152, 0.795392,
	  0.80152553, 0.5632211, 0.71810395,
	  0.76990217, 0.46014875, 0.6364745,
	  0.7378633, 0.32720572, 0.534223,
	  0.71465755, 0.2619931, 0.44013655,
	  0.69388515, 0.19446346, 0.33299515]

precision = [0.77114064, 0.5915648, 0.9820794,
			 0.7663209, 0.7191943, 0.98058385,
			 0.7395423, 0.7719552, 0.9803326,
			 0.68944633, 0.79322594, 0.97922105,
			 0.6482224, 0.7354279, 0.9779753,
			 0.6088139, 0.7368638, 0.9769794,
			 0.5814338, 0.6690605, 0.9751396,
			 0.5573082, 0.64308983, 0.9743183]

recall_background = []
f1_background = []
precision_background = []
for x in range(2,len(recall),3):
	recall_background.append(recall[x])
	f1_background.append(f1[x])
	precision_background.append(precision[x])


path_to_png = './gaussian_MSCs.png'
plt.plot(data_x, recall_background, label='recall', color=[1,0,0], linewidth=0.75)
plt.plot(data_x, f1_background, label='f1', color=[0,1,0], linewidth=0.75)
plt.plot(data_x, precision_background, label='precision', color=[0,0,1], linewidth=0.75)
plt.legend(title= 'Gaussian with different kernel size',loc='lower right', ncol=1)  # ajust ncol to fit the space
#plt.title(logfile+'.log\n'+ get_chart_type_description(chart_type))
plt.xlabel('Gaussian Kernel Size')
plt.ylabel('Measurement of performance')
plt.ylim(-0.05, 1.2)
fig = plt.gcf()
fig.set_size_inches(8, 4)
plt.savefig(path_to_png)
plt.show()