#pragma once

namespace sal {

/*
\brief Class to quantize image gradient magnitudes
Suppressing lows and boosting highs
*/
class Quantizer {
public:
	Quantizer();
	~Quantizer();

	void quantizeMagnitudes(const cv::Mat& magnitudes);

private:
	// Thresholds
	// Each channel has its own thresholds
	// Thresholds for a channel are function of maxMag for the channel.
	// 3 is the arbitrary count of bins
	// 4 is max count of channels
	std::array<cv::Vec3f, 4> thresholds;
};

}
