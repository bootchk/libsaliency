/*
 * SaliencyMain.cpp
 *
 */
#include <cstdlib>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <libsaliency.hpp>	// ? Or " ", does this find the source file or the installed file?

using namespace std;

int main(int argc, char *argv[]) {
	// cv::Mat1f img = cv::imread("data/inGrayscale/croppedBike.jpeg", CV_LOAD_IMAGE_GRAYSCALE);
	// cv::Mat1f img = cv::imread("data/ads.ppm", CV_LOAD_IMAGE_GRAYSCALE);
	// cv::Mat1f img = cv::imread("data/IMG_9297.jpg", CV_LOAD_IMAGE_GRAYSCALE);

	// For color, 3 channels
	cv::Mat3f img = cv::imread("data/inColor/scenery.jpg", CV_LOAD_IMAGE_COLOR);

	if (img.empty()) {
		cout << "No image loaded. Exiting.\n";
		return (EXIT_FAILURE);
	}

	sal::ImageSaliencyDetector detector(img);
	detector.setSamplingPercentage(0.10f);

	cout << "Computing...\n";
	detector.compute();

	cout << "Post-processing...\n";
	detector.performPostProcessing();

	cv::imwrite("SaliencyTestOutput.jpg", detector.getSaliencyMap());

	return 0;
}



