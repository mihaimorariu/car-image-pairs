# Pairs Test Case #

This repository contains my solution for the Pairs test case. Given two images, each showing a car, the program determines whether the same type/model is present in both of them. If so, it provides information about which of the two images shows a large (zoomed-in) version of the car.

The tests are provided in a txt (e.g. `data.txt`) file. Each line in this file contains the paths (relative to the directory containing the executable) of the two images that need to be processed (one test), separated by a space.

The ground truth data is provided in another text file (e.g. `gtruth.txt`), which contains the same number of lines as the number of tests, in the format `X Y`, where:

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

    `./scripts/generate_data.py img X Y data/data.txt data/gtruth.txt`

    where `X` is the number of tests where the two images will show the same car and `Y` is the number of tests where they will be different.

3. Configure the algorithm parameters in `data/params.yaml` or use the default ones.
4. Run the program. In the main directory, run:

    `./pairs data/params.yaml data/data.txt data/result.txt`

    The output of the program will be recorded in `data/result.txt`.

5. Evaluate the algorithm. In the main directory, run:

    `./scripts/evaluate.py data/result.txt data/gtruth.txt`

    The script will print :

     * The fraction of tests correctly marked as showing/not showing the same car.
     * The fraction of tests where the correct car size was detected.

### Limitations ###

This implementation uses SIFT features and the Flann matcher to determine whether the same car is seen in both images, based on similar point correspondences. While hand-crafted features have proven successful in many applications, they are not guaranteed to be the optimal choice for the problem at hand. Moreover, they depend on parameters which could be learned from the data, but the current implementation only allows for tweaking such parameters manually.

To determine the image which shows a large (zoomed-in) version of the car, the PCA algorithm is used. After obtaining point correspondences between the two images, the principal components and their eigenvalues are computed, for each of the two sets of points. Intuitively, the set of points from the image with the zoomed-in car should be more "spread out" than their correspondences in the other image, because the car is closer to the camera. We can use this intuition to check which image has the largest eigenvalue (i.e. the largest variation on a principal component) and mark that as being the one showing the zoomed-in version of the car. While this idea is easy to implement, it is sensitive to outliers.

### Future work ###

To improve the performance of the algorithm, several ideas can be employed:

* One could try to experiment with other popular descriptors (e.g. FAST, ORB, SURF, BRISK etc) and descriptor matchers, to see if better point correspondences are obtained that way. To determine the image with the larger car size, another idea could be computing the homography between the two images, based on the computed point correspondences, and check whether the area enclosed by them is scaled up or not.

* If more images are available per car type, then this problem could be treated as a multi-class classification problem and a supervised learning algorithm could be used to learn the correct car type for each image.

* Deep Learning is a popular algorithm nowadays, which has proven successful in many applications. Siamese neural networks, in particular, have proven useful in finding similarities between images, so they could be a solution to determining if the two images show the same car. The advantage is that features would automatically be learned from the data, however more data would be needed for this purpose.

* A deep neural network that can classify and provide a segmentation of the image (e.g. Multitask Network Cascades, Jifeng Dai et al., CVPR, 2106) could also be a solution. If the images are classified as having the same label (i.e. the same car is seen in both), then the area of the segmentation could be used to determine which one is larger.