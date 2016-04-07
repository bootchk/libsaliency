#include <cstdlib>
#include <iostream>
#include <cassert>
#include <opencv2/imgproc/imgproc.hpp>

#include "Smoother.h"


namespace sal {

// Class to hide implementation of smooth or blur.
// Now adapts to openCL, original used de novo implementations.

Smoother::Smoother(const float& sig) {
	if (sig > 0) {
		this->sigma = sig;
	} else {
		this->sigma = 1.20; // Default
	}

	windowSize = static_cast<int> (1 + (2 * ceil(2.5 * sigma)));

}

Smoother::~Smoother() { }


void Smoother::smoothImage(const cv::Mat1f& src, cv::Mat1f& smoothedImg) {
	assert(!src.empty() && src.cols >= 3 && src.rows >= 3);

	if (!smoothedImg.empty()) {
			std::cout << "Smoother WARNING: Expected an empty matrix for [smoothedImg]\n";
			std::cout << "\t... may have unexpected behavior!\n";
	}

	cv::GaussianBlur( src, smoothedImg, cv::Size(this->windowSize, this->windowSize ), 0, 0 );

	assert(!smoothedImg.empty() && smoothedImg.cols >= 3 && smoothedImg.rows >= 3);
}


} /* namespace sal */
