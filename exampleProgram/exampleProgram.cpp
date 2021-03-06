/*
 * SaliencyMain.cpp
 *
 */
#include <cstdlib>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


#include <kernelDensityInfo.hpp>
// TODO this should be in the library include directory
#include <SaliencyMath.hpp>
#include "../libsaliency/Channels.hpp"
#include "../libsaliency/PDFEstimate.hpp"
#include <libsaliency.hpp>	// ? Or " ", does this find the source file or the installed file?

using namespace std;

int main(int argc, char *argv[]) {
	// original test data: has a thin black line on lower and right edge
	//cv::Mat1f img = cv::imread("data/inGrayscale/bike.jpeg", CV_LOAD_IMAGE_GRAYSCALE);
	// Same as above but with thin black line cropped out
	//cv::Mat1f img = cv::imread("data/inGrayscale/croppedBike.jpeg", CV_LOAD_IMAGE_GRAYSCALE);
	// Same as above but also thresholded to B&W
	//cv::Mat1f img = cv::imread("data/inGrayscale/croppedThresholdedBike.jpeg", CV_LOAD_IMAGE_GRAYSCALE);
	// Same as above but posterized to 3 colors
	// cv::Mat1f img = cv::imread("data/inGrayscale/croppedBikePosterized3.jpeg", CV_LOAD_IMAGE_GRAYSCALE);
	// cv::Mat1f img = cv::imread("data/inGrayscale/ads.ppm", CV_LOAD_IMAGE_GRAYSCALE);
	// Very large image
	//cv::Mat1f img = cv::imread("data/inGrayscale/IMG_9297.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	//cv::Mat1f img = cv::imread("data/inGrayscale/scenery.jpg", CV_LOAD_IMAGE_GRAYSCALE);

	// Color, 3 channels
	//cv::Mat3f img = cv::imread("data/inColor/scenery.jpg", CV_LOAD_IMAGE_COLOR);
	// cv::Mat3f img = cv::imread("data/inColor/sceneryThresholded.jpg", CV_LOAD_IMAGE_COLOR);
	//cv::Mat3f img = cv::imread("data/inColor/miti2.jpg", CV_LOAD_IMAGE_COLOR);

	// RGBD image and images of its color and depth components
	// RGB w Depth as fourth channel
	//cv::Mat4f img = cv::imread("data/RGBD/rgbdFromTUMunichStructureNoTexture.png", CV_LOAD_IMAGE_UNCHANGED);
	cv::Mat3f img = cv::imread("data/RGBD/rgbFromTUMunichStructureNoTexture.png", CV_LOAD_IMAGE_UNCHANGED);

	if (img.empty()) {
		cout << "No image loaded. Exiting.\n";
		return (EXIT_FAILURE);
	}

	sal::ImageSaliencyDetector detector(img);
	detector.setSamplingPercentage(0.10f);
	//detector.setNeighborhoodSize(10);

	cout << "Computing...\n";
	detector.compute();

	cout << "Post-processing...\n";
	// TODO TEST detector.postProcessSaliencyMap();

	cv::imwrite("SaliencyTestOutput.jpg", detector.getSaliencyMap());

	return 0;
}



