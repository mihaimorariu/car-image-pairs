#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <yaml-cpp/yaml.h>

#include <fstream>

struct CompareResult {
	bool same_car;
	int index_large;
};

/** \brief Loads an image from file and throws exception if it does not exist. */
cv::Mat loadImage(std::string const & image_file) {
	cv::Mat image = cv::imread(image_file);
	if (image.empty()) {
		throw std::runtime_error("Unable to load image '" + image_file + "'.");
	}

	return image;
}

/** \brief Visualizes keypoints.
 *  \param [in] image       Image that the keypoints will be drawn onto.
 *  \param [in] keypoints   Vector of keypoints.
 *  \param [in] window_name Name of display window.
 */
void visualizeKeypoints(cv::Mat                   const & image,
                        std::vector<cv::KeyPoint> const & keypoints,
                        std::string               const & window_name)
{
	cv::Mat image_keypoints = image.clone();

	for (auto const & p : keypoints) {
		cv::circle(image_keypoints, p.pt, 1, cv::Scalar(0, 200, 0), -1);
	}

	//cv::drawKeypoints(image, keypoints, image_keypoints, cv::Scalar::all(-1));
	cv::imshow(window_name, image_keypoints);
}

/** \brief Visualizes keypoint matches.
 *  \param [in] image1      First image.
 *  \param [in] image2      Second image.
 *  \param [in] keypoints1  Vector with keypoints from the first image.
 *  \param [in] keypoints2  Vector with keypoints from the second image.
 *  \param [in] matches     Vector with matches.
 *  \param [in] window_name Name of display window.
 */
void visualizeMatches(cv::Mat                   const & image1,
                      cv::Mat                   const & image2,
                      std::vector<cv::KeyPoint> const & keypoints1,
                      std::vector<cv::KeyPoint> const & keypoints2,
                      std::vector<cv::DMatch>   const & matches,
                      std::string               const & window_name)
{
	cv::Mat image_matches;
	cv::drawMatches(image1, keypoints1, image2, keypoints2, matches, image_matches,
	                cv::Scalar::all(-1), cv::Scalar::all(-1), std::vector<char>(),
	                cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	cv::imshow(window_name, image_matches);
}

/** \brief Computes matches between keypoints with similar descriptors.
 *  \param [in] image1     First image.
 *  \param [in] image2     Second image.
 *  \param [in] keypoints1 Vector with keypoints from the first image.
 *  \param [in] keypoints2 Vector with keypoints from the second image.
 *  \param [in] extractor  Points to descriptor extractor.
 *  \param [in] matcher    Pointer to descriptor matcher.
 *  \param [in] threshold  Threshold used for matching.
 *  \return Vector with matches.
 */
std::vector<cv::DMatch> computeGoodMatches(cv::Mat                          const & image1,
                                           cv::Mat                          const & image2,
                                           std::vector<cv::KeyPoint>              & keypoints1,
                                           std::vector<cv::KeyPoint>              & keypoints2,
                                           cv::Ptr<cv::DescriptorExtractor> const & extractor,
                                           cv::Ptr<cv::DescriptorMatcher>   const & matcher,
                                           int                              const threshold)
{
	cv::Mat descriptors1, descriptors2;

	// Extract descriptors.
	extractor->compute(image1, keypoints1, descriptors1);
	extractor->compute(image2, keypoints2, descriptors2);

	// Compute matches.
	std::vector<cv::DMatch> matches;
	matcher->match(descriptors1, descriptors2, matches);

	// Threshold matches.
	std::vector<cv::DMatch> good_matches;
	for (int i = 0; i < descriptors1.rows; ++i) {
		if (matches[i].distance < threshold) good_matches.push_back(matches[i]);
	}

	return good_matches;
}

/** \brief Computes the index of the image showing the large car.
 *  \param [in] keypoints1 Vector with keypoints from the first image.
 *  \param [in] keypoints2 Vector with keypoints from the second image.
 *  \param [in] matches    Vector with matches.
 *  \return 0 if the first image shows the large car, 1 otherwise.
 */
int computeIndexLarge(std::vector<cv::KeyPoint> const & keypoints1,
                      std::vector<cv::KeyPoint> const & keypoints2,
                      std::vector<cv::DMatch>   const & matches)
{
	cv::Mat points1(matches.size(), 2, CV_16U);
	cv::Mat points2(matches.size(), 2, CV_16U);

	// Store the keypoints in two matrices, as required by the PCA class.
	for (std::size_t i = 0; i < matches.size(); ++i) {
		points1.at<uint16_t>(i, 0) = keypoints1[matches[i].queryIdx].pt.x;
		points1.at<uint16_t>(i, 1) = keypoints1[matches[i].queryIdx].pt.y;
		points2.at<uint16_t>(i, 0) = keypoints2[matches[i].trainIdx].pt.x;
		points2.at<uint16_t>(i, 1) = keypoints2[matches[i].trainIdx].pt.y;
	}

	// Apply PCA and get the directions with the largest variation.
	cv::PCA pca1(points1, cv::Mat(), CV_PCA_DATA_AS_ROW);
	cv::PCA pca2(points2, cv::Mat(), CV_PCA_DATA_AS_ROW);

	// Get the maximum eigen values for each of the two images.
	double max1, max2;
	cv::minMaxLoc(pca1.eigenvalues, 0, &max1);
	cv::minMaxLoc(pca2.eigenvalues, 0, &max2);

	return max1 > max2 ? 0 : 1;
}

/** \brief Compares two images to see if they contain the same car type/size.
 *  \param [in] image1 First image.
 *  \param [in] image2 Second image.
 *  \param [in] params Algorithm parameters.
 *  \return Result of the image comparison.
 */
CompareResult compareImages(cv::Mat const & image1, cv::Mat const & image2, YAML::Node const & params) {
	int fast_threshold = params["fast_threshold"].as<int>(5);
	bool visualize     = params["visualize"].as<bool>(true);

	cv::Ptr<cv::FeatureDetector> detector = cv::FastFeatureDetector::create(fast_threshold);

	// Detect keypoints in the two images.
	std::vector<cv::KeyPoint> keypoints1, keypoints2;
	detector->detect(image1, keypoints1);
	detector->detect(image2, keypoints2);

	std::cout << "Found " << keypoints1.size() << " keypoints in the first image. " << std::endl;
	std::cout << "Found " << keypoints2.size() << " keypoints in the second image." << std::endl;

	// Display keypoints, if 'visualize' is set to true.
	if (visualize) {
		visualizeKeypoints(image1, keypoints1, "Keypoints1");
		visualizeKeypoints(image2, keypoints2, "Keypoints2");
	}

	// Create SIFT descriptors around the detected keypoints and match them with Flann.
	cv::Ptr<cv::DescriptorExtractor> extractor = cv::xfeatures2d::SIFT::create();
	cv::Ptr<cv::DescriptorMatcher> matcher     = cv::DescriptorMatcher::create("FlannBased");

	// Compute matches corresponding to similar descriptors
	int rejection_threshold = params["rejection_threshold"].as<int>(300);
	std::vector<cv::DMatch> good_matches = computeGoodMatches(image1, image2, keypoints1, keypoints2,
	                                                          extractor, matcher, rejection_threshold);
	std::cout << "Found " << good_matches.size() << " point correspondences." << std::endl;

	// Display matches, if 'visualize' is set to true.
	if (visualize) {
		visualizeMatches(image1, image2, keypoints1, keypoints2, good_matches, "Matches");
	}

	// If a certain number of matches was found, the same car is likely found in both images.
	if (good_matches.size() > params["similarity_threshold"].as<int>(30)) {
		// Compute the index of the image showing the larger car (0 for the first, 1 for the second).
		int index_large = computeIndexLarge(keypoints1, keypoints2, good_matches);
		if (visualize) cv::waitKey(0);

		return { true, index_large };
	}

	return { false, -1 };
}

int main(int argc, char *argv[]) {
	if (argc != 4) {
		throw std::runtime_error("Usage " + std::string(argv[0]) + " params_file data_file result_file");
	}

	std::string params_file(argv[1]);
	std::string data_file(argv[2]);
	std::string result_file(argv[3]);

	YAML::Node params = YAML::LoadFile(params_file);
	if (!params) {
		throw std::runtime_error("Unable to open parameters file for reading, aborting execution.");
	}

	std::ifstream df(data_file);
	if (!df.is_open()) {
		throw std::runtime_error("Unable to open data file for reading, aborting execution.");
	}

	std::ofstream rf(result_file);
	if (!rf.is_open()) {
		throw std::runtime_error("Unable to open results file for writing, aborting execution.");
	}

	std::string image_file1, image_file2;
	while (df >> image_file1 >> image_file2) {
		std::cout << "Processing images '" << image_file1 << "' and '" << image_file2 << "'." << std::endl;

		cv::Mat image1 = loadImage(image_file1);
		cv::Mat image2 = loadImage(image_file2);

		bool visualize = params["visualize"].as<bool>(true);
		CompareResult compare_result = compareImages(image1, image2, params);
		rf << compare_result.same_car << " " << compare_result.index_large << std::endl;
	}

	df.close();
	rf.close();

	return 0;
}
