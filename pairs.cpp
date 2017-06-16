#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>

#include <fstream>
#include <limits>

std::vector<std::pair<std::string, std::string>> file_pairs;

float threshold = 0;

cv::Mat loadImage(const std::string &image_file) {
	cv::Mat image = cv::imread(image_file);
	if (image.empty()) {
		throw std::runtime_error("Unable to load image '" + image_file + "'.");
	}

	return image;
}

bool compareImages(const cv::Mat &image1, const cv::Mat &image2) {
	cv::Ptr<cv::xfeatures2d::SIFT> extractor = cv::xfeatures2d::SIFT::create();

	std::vector<cv::KeyPoint> keypoints1;
	std::vector<cv::KeyPoint> keypoints2;

	extractor->detect(image1, keypoints1);
	extractor->detect(image2, keypoints2);

	cv::Mat descriptors1;
	cv::Mat descriptors2;

	extractor->compute(image1, keypoints1, descriptors1);
	extractor->compute(image2, keypoints2, descriptors2);

	cv::BFMatcher matcher;
	std::vector<cv::DMatch> matches;
	matcher.match(descriptors1, descriptors2, matches);

	if (true) {
		cv::Mat image_keypoints1;
		cv::Mat image_keypoints2;
		cv::drawKeypoints(image1, keypoints1, image_keypoints1, cv::Scalar::all(-1));
		cv::drawKeypoints(image2, keypoints2, image_keypoints2, cv::Scalar::all(-1));
		cv::imshow("Keypoints1", image_keypoints1);
		cv::imshow("Keypoints2", image_keypoints2);
		cv::waitKey(0);
	}

	double max_dist = std::numeric_limits<double>::min();
	double min_dist = std::numeric_limits<double>::max();

	for (int i = 0; i < descriptors1.rows; ++i) {
		double dist = matches[i].distance;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}

	std::cout << "Max dist: " << max_dist << std::endl;
	std::cout << "Min dist: " << min_dist << std::endl;

	std::vector<cv::DMatch> good_matches;

	for (int i = 0; i < descriptors1.rows; ++i) {
		if (matches[i].distance < 350) good_matches.push_back(matches[i]);
	}

	cv::Mat image_matches;
	cv::drawMatches(image1, keypoints1, image2, keypoints2, good_matches, image_matches,
	                cv::Scalar::all(-1), cv::Scalar::all(-1), std::vector<char>(),
	                cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

	cv::imshow("Matches", image_matches);
	cv::waitKey(0);

	return true;
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		throw std::runtime_error("Usage: " + std::string(argv[0]) + " [data_file] [results_file]");
	}

	std::ifstream data_file(argv[1]);
	if (!data_file.is_open()) {
		throw std::runtime_error("Unable to open data file for reading, will abort execution.");
	}

	std::ofstream results_file(argv[2]);
	if (!results_file.is_open()) {
		throw std::runtime_error("Unable to open results file for writing, will abort execution.");
	}

	std::string image_file1, image_file2;

	data_file >> threshold;
	while (data_file >> image_file1 >> image_file2) {
		std::cout << "Processing images '" << image_file1 << "' and '" << image_file2 << "'." << std::endl;
		cv::Mat image1 = loadImage(image_file1);
		cv::Mat image2 = loadImage(image_file2);

		bool result = compareImages(image1, image2);
		results_file << result ? "true" : "false";
	}

	data_file.close();
	results_file.close();

	return 0;
}
