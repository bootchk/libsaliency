#pragma once

namespace sal {


/*
\brief Class to hold PDF estimate that is iteratively computed.

The pdf is of order 2 (an image)

Uses KernalDensityInfo.

Valid calling sequence:  resizeTo() updateApplicableRegion()* copyResultToGrayscaleImage()
*/
class PDFEstimate {
public:
	explicit PDFEstimate();
	virtual ~PDFEstimate();

	void resizeTo(cv::Size srcSize, const int channelCount, const int neighborhoodSize);	// basically, init()
	void updateApplicableRegion(const cv::Rect& bounds, const KernelDensityInfo& kernelSum);
	void copyResultToGrayscaleImage(cv::Mat1f& saliencyMap);

private:

	bool isSane();

	float maxEntropy();
	float minEntropy();
	/* obsolete
	float minEntropy(float maxEntropy);
	void shiftValuesSoMinIsZero(float minEntropy);
	void quantizeAndCopyTo(float maxEntropy, cv::Mat1f saliencyMap);
	*/
	void shiftQuantizeAndCopyTo(float overallminEntropy, float overallmaxEntropy, cv::Mat1f& saliencyMap);


	// Data

	// vector because it is resized to source image size
	std::vector< std::vector<KernelDensityInfo> > densityEstimates;

	int width;	// valid after resizeTo()
	int height;

	int sampleCountLimit;
};

}
