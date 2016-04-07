#include <cstdlib>
#include <iostream>
#include <cassert>
#include <opencv2/imgproc/imgproc.hpp>

#include "Gradienter.h"
#include "Smoother.h"


namespace sal {

// Class to hide implementation of gradient.
// Now adapts to openCL, original used de novo implementations.
// Owns the images it computes (magnitude and direction)

Gradienter::Gradienter() { }
Gradienter::~Gradienter() { }


void Gradienter::compute(const cv::Mat1f& src) {
	assert(!src.empty() && src.cols >= 3 && src.rows >= 3);


	cv::Mat1f deltaX, deltaY;

	// lkk Its not clear to me that smoothing is beneficial here ???
	cv::Mat1f smoothedImg;
	Smoother smoother(0);	// 0 means use default sigma

	std::cout << "Smoothing...\n";
	smoother.smoothImage(src, smoothedImg);

	// TODO: src or smoothedImg
	std::cout << "Derivatives...\n";
	calculateXAndYDerivatives(smoothedImg, deltaX, deltaY);

	std::cout << "Direction...\n";

	/* Obsolete
	calculateGradientDirections(deltaX, deltaY);
	calculateGradientMagnitudes(deltaX, deltaY);
	*/
	calculateGradientDirectionsAndMagnitudes(deltaX, deltaY);
}



void Gradienter::calculateXAndYDerivatives(
		const cv::Mat1f& inImage,
		cv::Mat1f& deltaX,
		cv::Mat1f& deltaY)
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
	deltaX.create(inImage.rows, inImage.cols);
	deltaY.create(inImage.rows, inImage.cols);

	// Handle pixels at borders?
	std::cout << "Sobel...\n";
	// 1,0 means first derivative x
	cv::Sobel( inImage, deltaX, ddepth, 1, 0, 3, scale, delta, cv::BORDER_DEFAULT );
	cv::Sobel( inImage, deltaY, ddepth, 0, 1, 3, scale, delta, cv::BORDER_DEFAULT );

	assert(!deltaX.empty());
}


void Gradienter::calculateGradientDirectionsAndMagnitudes(const cv::Mat1f& deltaX, const cv::Mat1f& deltaY)
{
	assert(deltaX.size == deltaY.size);
	this->gradMagnitudes.create(deltaX.rows, deltaX.cols);
	this->gradOrientations.create(deltaX.rows, deltaX.cols);
	cv::cartToPolar(deltaX, deltaY, this->gradMagnitudes, this->gradOrientations);
}


// Obsolete, not necessarily used
// Direction of gradient, in radians specified counteclockwise from the positive x-axis
void Gradienter::calculateGradientDirections(const cv::Mat1f& deltaX, const cv::Mat1f& deltaY)
{
	assert(deltaX.size == deltaY.size);
	this->gradOrientations.create(deltaX.rows, deltaX.cols);
	cv::phase(deltaX, deltaY, this->gradOrientations);
}


void Gradienter::calculateGradientMagnitudes(const cv::Mat1f& deltaX, const cv::Mat1f& deltaY)
{
	assert(deltaX.size == deltaY.size);
	this->gradMagnitudes.create(deltaX.rows, deltaX.cols);
	cv::magnitude(deltaX, deltaY, this->gradMagnitudes);
}


} /* namespace sal */
