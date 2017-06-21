#!/usr/bin/python
import os
import sys

if (len(sys.argv) != 3):
	raise Exception("Usage: " + sys.argv[0] + " result_file gt_file")

res_file = sys.argv[1]
gt_file  = sys.argv[2]

with open(res_file, "r") as rf:
	res_content = rf.readlines()
res_content = [x.strip() for x in res_content]

with open(gt_file, "r") as gf:
	gt_content = gf.readlines()
gt_content = [x.strip() for x in gt_content]

assert(len(res_content) == len(gt_content))

correct_class = 0
correct_index = 0
samples = len(res_content)

for i in range(len(res_content)):
	content   = res_content[i].split(" ")
	res_class = int(content[0])
	res_index = int(content[1])

	content   = gt_content[i].split(" ")
	gt_class  = int(content[0])
	gt_index  = int(content[1])

	correct_class += res_class == gt_class
	correct_index += res_index == gt_index

print("Accuracy car type: %.2f (%d/%d)" % (float(correct_class) / samples, correct_class, samples))
print("Accuracy car size: %.2f (%d/%d)" % (float(correct_index) / samples, correct_index, samples))
