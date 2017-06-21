#!/usr/bin/python
import os
import sys

if (len(sys.argv) != 3):
	raise Exception("Usage: " + sys.argv[0] + " result_file gt_file")

# Get command line arguments
res_file = sys.argv[1]
gt_file  = sys.argv[2]

# Read data from the results file
with open(res_file, "r") as rf:
	res_content = rf.readlines()
res_content = [x.strip() for x in res_content]

# Read data from the ground truth file
with open(gt_file, "r") as gf:
	gt_content = gf.readlines()
gt_content = [x.strip() for x in gt_content]

assert(len(res_content) == len(gt_content))

# Count the examples correctly detecting the same/different car
# and the ones correctly detecting the same size.
correct_class = 0
correct_index = 0
samples = len(res_content)

for i in range(len(res_content)):
	content   = res_content[i].split(" ")
	res_class = int(content[0])
	res_index = int(content[1])

	# class =  0 if we are dealing with different cars
	#       =  1 if we are dealing with the same car
	# index =  0 if the car in the first image has a larger size
	#       =  1 if the car in the second image has a larger size
	#       = -1 if the same car is not shown in both images

	content   = gt_content[i].split(" ")
	gt_class  = int(content[0])
	gt_index  = int(content[1])

	correct_class += res_class == gt_class
	correct_index += res_index == gt_index

print("Accuracy car type: %.2f (%d/%d)" % (float(correct_class) / samples, correct_class, samples))
print("Accuracy car size: %.2f (%d/%d)" % (float(correct_index) / samples, correct_index, samples))
