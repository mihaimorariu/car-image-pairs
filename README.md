# Pairs Test Case #

This repository contains my solution for the Pairs test case. Given two images, each showing a car, the program determined whether the same type/model is present in both of them. If so, it also provides information about which of the two images shows a large (zoomed-in) version of the car.

The tests are provided in a txt (e.g. `data.txt`) file. Each line in this file contains the paths (relative to the directory containing the executable) of the two images that need to be processed (one test), separated by a space.

The ground truth data is provided in another text file (e.g. `gtruth.txt`), which contains the same number of lines as the number of tests, in the following format `X Y`, where:

* X is 0 if the same car is **not** visible in both images, 1 otherwise.
* Y is 0 if the first image shows the zoomed-in car, 1 if it is the second image and -1 if the two images do not show the same car.

The result file (e.g. `result.txt`), containing the output of the program, contains data in the same format as the ground truth file.

### Dependencies ###

* OpenCV 3
* YAML-CPP
* Python 3 (for the data generation and evaluation scripts)

### How to run ###

1. Clone the repository. In the main directory, run `make`.
2. Generate new (random) data. In the main directory, run:

    `./scripts/generate_data.py img X data/data.txt data/gtruth.txt`

    where `X` is the total number of generated tests, out of which half will contain images showing the same car and the other half will not.

3. Configure the algorithm parameters in `data/params.yaml` or use the default ones.
4. Run the program. In the main directory, run:

    `./pairs data/params.yaml data/data.txt data/result.txt`

    The output of the program will be recorded in `result.txt`.

5. Evaluate the algorithm. In the main directory, run:

    `./scripts/evaluate.py data/result.txt data/gtruth.txt`

    The script will print :

     * The fraction of tests correctly marked as showing/not showing the same car.
     * The fraction of tests where the correct car size was detected.

### Limitations ###

### Future work ###