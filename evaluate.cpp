#include <fstream>
#include <iostream>

int main(int argc, char * argv[]) {
	if (argc != 3) {
		throw std::runtime_error("Usage: " + std::string(argv[0]) + " result_file gtruth_file");
	}

	std::ifstream result_file(argv[1]);
	if (!result_file.is_open()) {
		throw std::runtime_error("Unable to open result file.");
	}

	std::ifstream gtruth_file(argv[2]);
	if (!gtruth_file.is_open()) {
		throw std::runtime_error("Unable to open ground truth file.");
	}

	std::string line_result, line_gtruth;

	int result_class  = 0, gtruth_class  = 0;
	int result_index  = 0, gtruth_index  = 0;
	int correct_class = 0, correct_index = 0;

	int num_lines = 0;

	while (result_file >> result_class >> result_index) {
		gtruth_file >> gtruth_class >> gtruth_index;
		correct_class += result_class == gtruth_class;
		correct_index += result_index == gtruth_index;
		++num_lines;
	}

	float accuracy_type = float(correct_class) / num_lines;
	float accuracy_size = float(correct_index) / num_lines;

	std::cout << "Accuracy car type: " << accuracy_type    << "("
	          << correct_class  << "/" << num_lines << ")" << std::endl;
	std::cout << "Accuracy car size: " << accuracy_size    << "("
	          << correct_index  << "/" << num_lines << ")" << std::endl;

	result_file.close();
	gtruth_file.close();

	return 0;
}
