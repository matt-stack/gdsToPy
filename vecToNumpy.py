import numpy as np
import random

# processing flags
EXPORT = True

print(' * start processin ... *')

# volume dimensions
dx = 5
dy = 6
dz = 7

# define export type
export_type = 'uint16'

# data array export to binary file
#export_folder = "Z:/nvindex/incoming/AD102_NV_Sci_gds/sample-raw/"
#export_filename = export_folder + "export_boxes_"+str(dz)+"x"+str(dy)+"x"+str(dx)+"_"+export_type+".raw"

# convert arrays
volume_data = np.zeros((dx,dy,dz))

# generate some artificial boxes
num_boxes   = 5
min_size    = 2
max_size    = 3
max_height  = 7
min_height  = 2
max_value   = 10

# write the first layers
volume_data[:, :, 0:min_height] = max_value

print(' - generating volume with '+str(dx)+' x '+str(dy)+' x '+str(dz)+' entries and '+str(num_boxes)+' boxes: ')
for x in range(num_boxes):
  
    # generate random positions
    px = int(random.random() * (dx-min_size))
    py = int(random.random() * (dy-min_size))

    # generate random sizes
    sx = int(min(min_size + random.random() * (max_size-1), dx-1))
    sy = int(min(min_size + random.random() * (max_size-1), dy-1))
    sz = int(min(min_size + random.random() * (max_height-1), dz-1))

    # generate some label
    label = int(random.random() * max_value)

    # write data to volume
    volume_data[px:(px+sx), py:(py+sy), min_height:sz] = label
    print('  - creating box '+str(x)+' with label ' + str(label))

if EXPORT:
    volume_data[0, 2, 1] = 500
    print(volume_data);
    # export to file
   # print('  - exporting volume ... ')
   # volume_data.astype(export_type).tofile(export_filename)
   # print('  - saved array to: ' + export_filename)

print(' * finished processing! *')