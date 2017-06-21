#!/usr/bin/python
import os
import random as rnd
import sys

def getImagePath(img_dir, car_id, seq_id):
	return os.path.join(img_dir, car_id + "_" + seq_id + ".png")

if __name__ == "__main__":
	if len(sys.argv) != 5:
		raise Exception("Usage " + sys.argv[0] + " img_dir samples data_file gt_file")

	# Get command line arguments
	img_dir   = sys.argv[1]
	samples   = int(sys.argv[2])
	data_file = sys.argv[3]
	gt_file   = sys.argv[4]

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

	keys = list(sequences.keys())
	rnd.shuffle(keys)

	if len(keys) * 2 < samples:
		raise Exception("Number of samples must be less or equal to {0}.".format(len(keys) * 2))

	with open(data_file, "w") as df:
		with open(gt_file, "w") as gf:
			for i in range(samples // 2):
				car_id  = keys[i]
				path1   = getImagePath(img_dir, car_id, sequences[car_id][0])
				path2   = getImagePath(img_dir, car_id, sequences[car_id][1])
				index   = 0 if sequences[car_id][0] > sequences[car_id][1] else 1

				df.write("%s %s\n" % (path1, path2))
				gf.write("%d %d\n" % (1, index))

			for i in range(samples // 2, samples):
				car_id1 = keys[rnd.randint(0, len(keys) - 1)]
				car_id2 = keys[rnd.randint(0, len(keys) - 1)]
				path1   = getImagePath(img_dir, car_id1, sequences[car_id1][0])
				path2   = getImagePath(img_dir, car_id2, sequences[car_id2][1])

				df.write("%s %s\n" % (path1, path2))
				gf.write("%d %d\n" % (0, -1))
