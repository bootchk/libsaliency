#include <cstdlib>
#include <iostream>
#include <cassert>
#include <opencv2/imgproc/imgproc.hpp>

#include "Gradienter.h"


namespace sal {

Gradienter::Gradienter() { }
Gradienter::~Gradienter() { }


void Gradienter::compute(const cv::Mat& src) {
	assert(!src.empty() && src.cols >= 3 && src.rows >= 3);

	cv::Mat deltaX, deltaY;

	std::cout << "Derivatives...\n";
	calculateXAndYDerivatives(src, deltaX, deltaY);

	std::cout << "Direction...\n";

	calculateGradientDirectionsAndMagnitudes(deltaX, deltaY);
}



void Gradienter::calculateXAndYDerivatives(
		const cv::Mat& inImage,	// In: many channels
		cv::Mat& deltaX,	// Out: one channel
		cv::Mat& deltaY)
{
	// Implementation: openCV.  Differs from original.
	// First derivative.
	int scale = 1;
	int delta = 0;
	int ddepth = inImage.depth();

	assert(!inImage.empty());
	assert(deltaX.empty());
	// Grayscale only ?

	std::cout << "Create...\n";
	//  !!! Create mat having same type i.e. channel count
	deltaX.create(inImage.rows, inImage.cols, inImage.type());
	deltaY.create(inImage.rows, inImage.cols, inImage.type());

	// Handle pixels at borders?
	std::cout << "Sobel...\n";
	// 1,0 means first derivative x
	// TODO rename deltaX => gradientX
	cv::Sobel( inImage, deltaX, ddepth, 1, 0, 3, scale, delta, cv::BORDER_DEFAULT );
	cv::Sobel( inImage, deltaY, ddepth, 0, 1, 3, scale, delta, cv::BORDER_DEFAULT );

	assert(!deltaX.empty());
}


void Gradienter::calculateGradientDirectionsAndMagnitudes(const cv::Mat& deltaX, const cv::Mat& deltaY)
{
	assert(deltaX.size == deltaY.size);
	// !!! Create mat having same type i.e. channel count
	this->gradMagnitudes.create(deltaX.rows, deltaX.cols, deltaX.type());
	this->gradOrientations.create(deltaX.rows, deltaX.cols, deltaX.type());
	cv::cartToPolar(deltaX, deltaY, this->gradMagnitudes, this->gradOrientations);
	// TODO merge mag and orient images into one image for memory locality
	// call it gradient of dimension [width, height, channels=2]

	/*
	cv::Mat h;
	std::vector<cv::Mat> g;
	g.push_back(gradMagnitudes);
	g.push_back(gradOrientations);
	merge(g, h);
	cv::Matx<float, 3, 2> element = h.at<cv::Matx<float, 3, 2>>(0,0);	// h.at<float>(0,0);
	printf("foo %f\n", element(2,0));
	*/

}


// Obsolete, not necessarily used
// Direction of gradient, in radians specified counteclockwise from the positive x-axis
void Gradienter::calculateGradientDirections(const cv::Mat& deltaX, const cv::Mat& deltaY)
{
	assert(deltaX.size == deltaY.size);
	this->gradOrientations.create(deltaX.rows, deltaX.cols, deltaX.type());
	cv::phase(deltaX, deltaY, this->gradOrientations);
}


void Gradienter::calculateGradientMagnitudes(const cv::Mat& deltaX, const cv::Mat& deltaY)
{
	assert(deltaX.size == deltaY.size);
	this->gradMagnitudes.create(deltaX.rows, deltaX.cols, deltaX.type());
	cv::magnitude(deltaX, deltaY, this->gradMagnitudes);
}


} /* namespace sal */
