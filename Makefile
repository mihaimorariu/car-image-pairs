LIBS = -lopencv_xfeatures2d \
       -lopencv_core \
       -lopencv_calib3d \
       -lopencv_highgui \
       -lopencv_imgcodecs \
       -lopencv_imgproc \
       -lopencv_features2d \
       -lopencv_flann \
       -lboost_program_options \
       -lyaml-cpp

ARGS += -std=c++11

all:
	g++ src/pairs.cpp -o pairs $(LIBS) $(ARGS)
clean:
	rm -rf pairs
