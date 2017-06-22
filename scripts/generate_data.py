#!/usr/bin/python
import math
import os
import random as rnd
import sys

def getImagePath(img_dir, car_id, seq_id):
	return os.path.join(img_dir, car_id + "_" + seq_id + ".png")

if __name__ == "__main__":
	if len(sys.argv) != 6:
		raise Exception("Usage " + sys.argv[0] + " img_dir samples_same samples_diff data_file gt_file")

	# Get command line arguments
	img_dir      = sys.argv[1]
	samples_same = int(sys.argv[2])
	samples_diff = int(sys.argv[3])
	data_file    = sys.argv[4]
	gt_file      = sys.argv[5]

	# Construct a dictionary with car_id as key and a list with seq_ids as value
	files = os.listdir(img_dir)
	sequences = {}

	for f in files:
		content = f.split("_")
		car_id  = content[0]
		seq_id  = content[1].split(".")[0]

		if car_id in sequences.keys():
			sequences[car_id].append(seq_id)
		else:
			sequences[car_id] = [seq_id]

	# Get the car ids
	keys = list(sequences.keys())

	# Make sure we have enough data to sample from
	if samples_same < 0 or samples_same > len(keys):
		raise Exception("'samples_same' must be between {0} and {1}.".format(0, len(keys)))

	if samples_diff < 0 or samples_diff > math.floor(len(keys) / 2):
		raise Exception("'samples_diff' must be between {0} and {1}.".format(0, math.floor(len(keys) / 2)))

	with open(data_file, "w") as df:
		with open(gt_file, "w") as gf:
			# Generate data corresponding to images showing the same car
			for i in range(samples_same):
				car_id  = keys[rnd.randint(0, len(keys) - 1)]
				path1   = getImagePath(img_dir, car_id, sequences[car_id][0])
				path2   = getImagePath(img_dir, car_id, sequences[car_id][1])
				index   = 0 if sequences[car_id][0] > sequences[car_id][1] else 1

				df.write("%s %s\n" % (path1, path2))
				gf.write("%d %d\n" % (1, index))

			# Generate data corresponding to images showing different cars
			for i in range(samples_diff):
				car_id1 = keys[rnd.randint(0, len(keys) - 1)]
				car_id2 = keys[rnd.randint(0, len(keys) - 1)]
				path1   = getImagePath(img_dir, car_id1, sequences[car_id1][0])
				path2   = getImagePath(img_dir, car_id2, sequences[car_id2][1])

				df.write("%s %s\n" % (path1, path2))
				gf.write("%d %d\n" % (0, -1))

				# Prevent those car ids from being used again
				sequences.pop(car_id1, None)
				sequences.pop(car_id2, None)
				keys = list(sequences.keys())
