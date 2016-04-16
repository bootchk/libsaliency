
#include <opencv2/core/core.hpp>
#include "Quantizer.h"

namespace sal {

// Quantize a single value
void quantizeValue(float& value, float maxMag, cv::Vec3f thresholds) {
	if (value >= 0 && value < thresholds[0]) {
		value = 0.05 * maxMag;
	} else if (value >= thresholds[0] && value < thresholds[1]) {
		value = 0.25 * maxMag;
	} else if (value >= thresholds[1] && value < thresholds[2]) {
		value = 0.75 * maxMag;
	} else if (value >= thresholds[2]){
		value = maxMag;
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
	assert(magnitudes.channels() > 1);

	int width = magnitudes.cols;
	int height = magnitudes.rows;

	// Allow up to four channels
	cv::Vec4f maxMag = (FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN);

	// Use pointer arithmetic into data.
	// !!! cast
	float *magnitudeData = (float*) magnitudes.data;
	int stride = magnitudes.channels();

	// Iterate over channels in data
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			// float *magElement = magnitudeData[row * stride + col];
			int pixelAddress = row * (stride * magnitudes.cols) + col * stride;
			for(int channel=0; channel<magnitudes.depth(); channel++ ) {
				float magValue = magnitudeData[pixelAddress + channel];
				if (magValue > maxMag[channel]) {
					maxMag[channel] = magValue;
				}
				// Equivalent without address arithmetic, but slower:
				// if (magnitudes.at<float>(i, j)[channel] > maxMag[channel]) {
				//	maxMag[channel] = magnitudes.at<float>(i, j)[channel]);
			}
		}
	}

	// Thresholds
	// Each channel has its own thresholds
	// Thresholds for a channel are function of maxMag for the channel.
	std::vector<cv::Vec3f> thresholds;

	for(int channel=0; channel<magnitudes.depth(); channel++ ) {
		thresholds[channel] =
				cv::Vec3f(
						maxMag[channel] / 3.0f,	// binning constants??
						maxMag[channel] / 2.0f,
						maxMag[channel] / 2.0f
				);
	}


	// This differs from the original as a result of using OpenCV
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			for(int channel=0; channel<magnitudes.depth(); channel++ ) {
				quantizeValue(
						magnitudeData[row * stride + col + channel],	// pass by reference
						maxMag[channel],
						thresholds[channel]);
			}
		}
	}
};



}
