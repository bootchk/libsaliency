#include <cstdlib>
#include <iostream>

#include <opencv2/core/core.hpp>
#include "Quantizer.h"

namespace sal {

// Quantize a single value.
// Bin pixel value into bins defined by thresholds on the maximum value in the channel
void quantizeValue(float& value, const float maxMagnitude, const cv::Vec3f thresholds) {
	if (value >= 0 && value < thresholds[0]) {
		value = 0.05 * maxMagnitude;
	} else if (value >= thresholds[0] && value < thresholds[1]) {
		value = 0.25 * maxMagnitude;
	} else if (value >= thresholds[1] && value < thresholds[2]) {
		value = 0.75 * maxMagnitude;
	} else if (value >= thresholds[2]){
		value = maxMagnitude;
	}
}

// Quantizer does not own any data
Quantizer::Quantizer() {};
Quantizer::~Quantizer() {};


void Quantizer::quantizeMagnitudes(const cv::Mat& magnitudes) {
	if (magnitudes.empty()) {
		throw std::logic_error("ImageSaliencyDetector: There must be magnitudes info to process!");
	}
	// Quantize all channels
	assert(magnitudes.channels() >= 1);
	// TODO symbolic constant for MAX_SUPPORTED_CHANNELS = 4
	assert(magnitudes.channels() < 5); 	// Allow for future RGBAlpha or RGBDepth

	int width = magnitudes.cols;
	int height = magnitudes.rows;

	// Allow up to four channels
	cv::Vec4f channelMaxMagnitudes = (FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN);

	// Use pointer arithmetic into data.
	// !!! cast
	float *magnitudeData = (float*) magnitudes.data;
	int stride = magnitudes.channels();

	// Iterate over channels in data
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			// row * rowLength + col * elementLength
			// step is count of bytes in a row
			assert(stride * magnitudes.cols == magnitudes.step / sizeof(float));	// row length
			int pixelAddress = row * (stride * magnitudes.cols) + col * stride;
			for(int channel=0; channel<magnitudes.channels(); channel++ ) {
				// non-compiling attempt:  float magValue = CV_MAT_ELEM(magnitudes, float, col, row*stride + channel);
				float magValue = magnitudeData[pixelAddress + channel];
				if (magValue > channelMaxMagnitudes[channel]) {
					channelMaxMagnitudes[channel] = magValue;
				}
				// Equivalent without address arithmetic, but slower:
				// if (magnitudes.at<float>(i, j)[channel] > channelMaxMagnitudes[channel]) {
				//	channelMaxMagnitudes[channel] = magnitudes.at<float>(i, j)[channel]);
			}
		}
	}

	std::cout << "Thresholds...\n";
	for(int channel=0; channel<magnitudes.channels(); channel++ ) {
		thresholds[channel] =
				cv::Vec3f(
						channelMaxMagnitudes[channel] / 3.0f,	// binning constants??
						channelMaxMagnitudes[channel] / 2.0f,
						channelMaxMagnitudes[channel] / 2.0f
				);
	}

	std::cout << "Quantizing...\n";
	// This differs from the original as a result of using OpenCV
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			int pixelAddress = row * (stride * magnitudes.cols) + col * stride;
			for(int channel=0; channel<magnitudes.channels(); channel++ ) {
				quantizeValue(
						magnitudeData[pixelAddress + channel],	// pass by reference
						channelMaxMagnitudes[channel],
						thresholds[channel]);
			}
		}
	}
	// assert magnitudes has been changed, quantized
	// thresholds also changed but discarded.
	std::cout << "Return from quantizing...\n";
};

}
