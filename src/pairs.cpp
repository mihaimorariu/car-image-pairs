#include <boost/program_options.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <yaml-cpp/yaml.h>

#include <fstream>
#include <limits>

struct DetectionResult {
	bool same_car;
	int index_large;
};

cv::Mat loadImage(std::string const & image_file) {
	cv::Mat image = cv::imread(image_file);
	if (image.empty()) {
		throw std::runtime_error("Unable to load image '" + image_file + "'.");
	}

	return image;
}

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

std::vector<cv::DMatch> computeGoodMatches(cv::Mat                          const & image1,
                                           cv::Mat                          const & image2,
                                           std::vector<cv::KeyPoint>              & keypoints1,
                                           std::vector<cv::KeyPoint>              & keypoints2,
                                           cv::Ptr<cv::DescriptorExtractor> const & extractor,
                                           cv::Ptr<cv::DescriptorMatcher>   const & matcher,
                                           int                              const threshold)
{
	cv::Mat descriptors1, descriptors2;
	extractor->compute(image1, keypoints1, descriptors1);
	extractor->compute(image2, keypoints2, descriptors2);

	std::vector<cv::DMatch> matches;
	matcher->match(descriptors1, descriptors2, matches);

	std::vector<cv::DMatch> good_matches;
	for (int i = 0; i < descriptors1.rows; ++i) {
		if (matches[i].distance < threshold) good_matches.push_back(matches[i]);
	}

	cv::Mat i1 = image1.clone();
	cv::Mat i2 = image2.clone();
	cv::Mat i3 = cv::Mat::zeros(image1.size(), CV_8UC3);

	std::sort(good_matches.begin(), good_matches.end(),
		[](cv::DMatch const & m1, cv::DMatch const & m2) { return m1.distance < m2.distance; });

	std::vector<cv::Point2f> points1;
	std::vector<cv::Point2f> points2;

	for (int i = 0; i < good_matches.size(); ++i) {
		points1.push_back(keypoints1[good_matches[i].queryIdx].pt);
		points2.push_back(keypoints2[good_matches[i].trainIdx].pt);
		//cv::circle(i1, keypoints1[good_matches[i].queryIdx].pt, 2, cv::Scalar(0, 255, 0), -1);
		//cv::circle(i2, keypoints2[good_matches[i].trainIdx].pt, 2, cv::Scalar(0, 255, 0), -1);
	}

	//if (points1.size() && points2.size()) {
	//cv::Mat H = cv::findHomography(points1, points2, cv::LMEDS);
	//std::cout << H << std::endl;
	//cv::warpPerspective(image1, i3, H, i2.size());
	//}


	//cv::imshow("i2", i2);
	//cv::imshow("i3", i3);
	//cv::waitKey(0);

	return good_matches;
}

int computeIndexLarge(std::vector<cv::KeyPoint> const & keypoints1,
                      std::vector<cv::KeyPoint> const & keypoints2,
                      std::vector<cv::DMatch>   const & matches)
{
	cv::Mat points1(matches.size(), 2, CV_16U);
	cv::Mat points2(matches.size(), 2, CV_16U);

	for (std::size_t i = 0; i < matches.size(); ++i) {
		points1.at<uint16_t>(i, 0) = keypoints1[matches[i].queryIdx].pt.x;
		points1.at<uint16_t>(i, 1) = keypoints1[matches[i].queryIdx].pt.y;
		points2.at<uint16_t>(i, 0) = keypoints2[matches[i].trainIdx].pt.x;
		points2.at<uint16_t>(i, 1) = keypoints2[matches[i].trainIdx].pt.y;
	}

	cv::PCA pca1(points1, cv::Mat(), CV_PCA_DATA_AS_ROW);
	cv::PCA pca2(points2, cv::Mat(), CV_PCA_DATA_AS_ROW);

	double max1, max2;
	cv::minMaxLoc(pca1.eigenvalues, 0, &max1);
	cv::minMaxLoc(pca2.eigenvalues, 0, &max2);

	std::cout << max1 << " " << max2 << std::endl;

	return max1 > max2 ? 0 : 1;
}

DetectionResult compareImages(cv::Mat const & image1, cv::Mat const & image2, YAML::Node const & params) {
	int fast_threshold = params["fast_threshold"].as<int>(5);
	bool visualize     = params["visualize"].as<bool>(true);

	cv::Ptr<cv::FeatureDetector> detector = cv::FastFeatureDetector::create(fast_threshold);

	std::vector<cv::KeyPoint> keypoints1, keypoints2;
	detector->detect(image1, keypoints1);
	detector->detect(image2, keypoints2);

	std::cout << "Found " << keypoints1.size() << " keypoints in the first image. " << std::endl;
	std::cout << "Found " << keypoints2.size() << " keypoints in the second image." << std::endl;

	//if (visualize) {
		//visualizeKeypoints(image1, keypoints1, "Keypoints1");
		//visualizeKeypoints(image2, keypoints2, "Keypoints2");
	//}

	cv::Ptr<cv::DescriptorExtractor> extractor = cv::xfeatures2d::SIFT::create();
	cv::Ptr<cv::DescriptorMatcher> matcher     = cv::DescriptorMatcher::create("FlannBased");

	int rejection_threshold = params["rejection_threshold"].as<int>(300);
	std::vector<cv::DMatch> good_matches = computeGoodMatches(image1, image2, keypoints1, keypoints2,
	                                                          extractor, matcher, rejection_threshold);
	std::cout << "Found " << good_matches.size() << " point correspondences." << std::endl;

	//if (visualize) {
		//visualizeMatches(image1, image2, keypoints1, keypoints2, good_matches, "Matches");
	//}

	if (good_matches.size() > params["similarity_threshold"].as<int>(30)) {
		int index_large = computeIndexLarge(keypoints1, keypoints2, good_matches);
		//std::cout << 1 << " " << index_large << std::endl;
		//cv::waitKey(0);
		return { true, index_large };
	}

	return { false, -1 };
}


bool parseArguments(std::string & params_file,
                    std::string & data_file,
                    std::string & result_file,
                    int argc, char * argv[])
{
	namespace po = boost::program_options;

	po::options_description desc("Pairs test case arguments");
	desc.add_options()
		("help,h",        "Display this message")
		("params_file,p", po::value<std::string>(&params_file)->required(), "Path to the parameters file")
		("data_file,d",   po::value<std::string>(&data_file)->required(),   "Path to the input data file")
		("result_file,r", po::value<std::string>(&result_file)->required(), "Path to the output (result) file");

	po::variables_map vm;

	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);

		if (vm.count("help")) {
			std::cout << desc << std::endl;
			return true;
		}

		po::notify(vm);
	} catch (std::exception & e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return false;
	}

	return true;
}

int main(int argc, char *argv[]) {
	std::string params_file, data_file, result_file;

	bool parse_result = parseArguments(params_file, data_file, result_file, argc, argv);
	if (!parse_result) return -1;

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
		DetectionResult detection_result = compareImages(image1, image2, params);
		rf << detection_result.same_car << " " << detection_result.index_large << std::endl;
	}

	df.close();
	rf.close();

	return 0;
}
