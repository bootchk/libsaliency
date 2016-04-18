#pragma once

namespace sal {


/*
\brief Class to hold PDF estimate

Valid calling sequence:  resizeTo() updateApplicableRegion()* maxEntropy()...quantizeAndCopyTo
*/
class PDFEstimate {
public:
	explicit PDFEstimate();
	virtual ~PDFEstimate();

	void resizeTo(cv::Size inImageSize, const int channelCount, const int neighborhoodSize);	// basically, init()
	bool isSane();
	void updateApplicableRegion(const cv::Rect& bounds, const KernelDensityInfo& kernelSum);
	float maxEntropy();
	float minEntropy(float maxEntropy);
	void shiftValuesSoMinIsZero(float minEntropy);
	void quantizeAndCopyTo(float maxEntropy, cv::Mat1f saliencyMap);

private:
	// vector because it is resized to source image size
	std::vector< std::vector<KernelDensityInfo> > densityEstimates;

	int width;	// valid after resizeTo()
	int height;
	int channelCount;

	int sampleCountLimit;
};

}
