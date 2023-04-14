import numpy as np
import random
import sys

# processing flags
EXPORT = True

print(' * start processin ... *')

LAYERSIZE = 10

# volume dimensions
#1511, 1671, 163
dx = 1511
dy = 1671
#dz = (163*LAYERSIZE)
dz = 163

# define export type
export_type = 'uint16'

# data array export to binary file
#export_folder = "Z:/h/bigdata1/nvindex/incoming/AD102_NV_Sci_gds/gdsConverter/"
export_folder = "/h/bigdata1/nvindex/incoming/AD102_NV_Sci_gds/gdsConverter/"
export_filename = export_folder + "gds_test"+str(dz)+"x"+str(dy)+"x"+str(dx)+"_"+export_type+".raw"

# convert arrays
volume_data = np.zeros((dz,dy,dx))

# generate some artificial boxes
num_boxes   = 5
min_size    = 2
max_size    = 3
max_height  = 7
min_height  = 2
max_value   = 10

np.set_printoptions(threshold=sys.maxsize)

# Text file data converted to integer data type
for x in range(1, 163): # I dont know if you can use 0 as a layer in GDS, but change this if you can
	filename_previous = "fileout_%d.txt" % (x-1)
	temp = np.zeros((dz, dy, dx))
	temp = np.loadtxt(filename_previous, dtype=int)
	filename = "fileout_%d.txt" % x
	volume_data[x][:][:] = np.loadtxt(filename, dtype=int)
	res = np.maximum(temp, volume_data)

res[0:1][:][:] = 2000
#print(volume_data[49])

print(' - generating volume with '+str(dx)+' x '+str(dy)+' x '+str(dz)+' entries and '+str(num_boxes)+' boxes: ')

if EXPORT:
    # export to file
    print('  - exporting volume ... ')
    res.astype(export_type).tofile(export_filename)
    print('  - saved array to: ' + export_filename)

print(' * finished processing! *')
