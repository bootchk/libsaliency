#pragma once

namespace sal {

/*
\brief Class to quantize image gradient magnitudes
Suppressing lows and boosting highs
*/
class Quantizer {
public:
	explicit Quantizer();
	virtual ~Quantizer();

	void quantizeMagnitudes(const cv::Mat& magnitudes);
};

}
