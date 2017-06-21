# Pairs #

This repository contains my solution for the Pairs test case. Given two images, each showing a car, the program determined whether the same type/model is present in both of them. If so, it also provides information about which of the two images shows a large (zoomed-in) version of the car.

The tests are provided in a txt (e.g. `data.txt`) file. Each line in this file contains the paths (relative to the directory containing the executable) of the two images that need to be processed (one test), separated by a space.

The ground truth data is provided in another text file (e.g. `gtruth.txt'), which contains the same number of lines as the number of tests, in the following format `X Y`, where:

* X is 0 if the same car is **not** visible in both images, 1 otherwise.
* Y is 0 if the first image shows the zoomed-in car, 1 if it is the second image and -1 if the two images do not show the same car.

The result file (e.g. `result.txt`), containing the output of the program, contains data in the same format as the ground truth file.

### Dependencies ###

* OpenCV 3
* YAML-CPP
* Python 3 (for the data generation and evaluation scripts)

### How do I get set up? ###

* Summary of set up
* Configuration
* Dependencies
* Database configuration
* How to run tests
* Deployment instructions

### Contribution guidelines ###

* Writing tests
* Code review
* Other guidelines

### Who do I talk to? ###

* Repo owner or admin
* Other community or team contact