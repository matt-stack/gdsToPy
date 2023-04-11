import numpy as np



volume_data = np.zeros((3, 11, 11))
  
# Text file data converted to integer data type
for x in range(3):
	filename = "fileout_%d.txt" % x
	volume_data[x][:][:] = np.loadtxt(filename, dtype=int)

#volume_data[1][:][:] = np.loadtxt("f_test.txt", dtype=int)


print(volume_data)