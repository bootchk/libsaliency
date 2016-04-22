/*
 * Software License Agreement (BSD License)
 *
 *  Saliency Detection
 *  Copyright (c) 2011, Kester Duncan
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *	\file	SaliencyDetector.h
 *	\brief	Defines the image and video saliency detection methods
 *	\author	Kester Duncan
 */

#pragma once

#include <opencv2/core/core.hpp>


namespace sal
{

/**
 * \brief
 * Abstract class for saliency algorithm, as described in:
 * "Relational Entropy-Based Saliency Detection in Images and Video"
 * by Kester Duncan and Sudeep Sarkar in IEEE International Conference on Image Processing (2012)
 * http://www.cse.usf.edu/~kkduncan/research/DuncanICIP2012.pdf
 *
 */
class SaliencyDetector {
public:
	/// Alias
	typedef cv::Point2i Location2D;
	typedef cv::Point3i Location3D;

public:
	SaliencyDetector() : samplingPercentage(0.25f), neighborhoodSize(5) { };
	virtual ~SaliencyDetector() { };

	/// Compute the saliency of an image or a 3D volume of frames
	virtual void compute() = 0;

	void setSamplingPercentage(const float& p) { if (p > 0) samplingPercentage = p; }
	void setNeighborhoodSize(const int& m) { if (m > 0) neighborhoodSize = m; }
	float getSamplingPercentage() const { return samplingPercentage; }
	int getNeighborhoodSize() const { return neighborhoodSize; }


protected:
	/// Process resultant saliency map to make it smooth and pretty
	void postProcessSaliencyMap(cv::Mat1f& salMap, const float& sigma = 18.0f);

	/// Percentage of image / frame pixels to use for calculations
	float samplingPercentage;

	/// Size of one dimension of the pixel neighborhood
	int neighborhoodSize;
};


/// Convenience aliases
// typedef SaliencyDetector::KernelDensityInfo KernelDensityInfo;



/**
 * \brief 2D Image saliency detector
 */
class ImageSaliencyDetector : public SaliencyDetector {
public:
	ImageSaliencyDetector (const cv::Mat& src);
	~ImageSaliencyDetector();

	/*!
	 * Computes the saliency of pixels based on the entropy of distance
	 * and gradient orientation relationships
	 */
	void compute();

	/*!
	 * Perform class specific post-processing of the saliency map
	 */
	void performPostProcessing();


public:
	void setMagnitudes(const cv::Mat& other) { magnitudes = other; }
	void setOrientations(const cv::Mat& other) { orientations = other; }
	void setSourceImage(const cv::Mat& theSrc) { srcImage = theSrc; }
	cv::Mat getMagnitudes() const { return magnitudes; }
	cv::Mat getOrientations() const { return orientations; }
	cv::Mat getSourceImage() const { return srcImage; }
	// !!! saliency map is one channel
	cv::Mat1f getSaliencyMap() const { return saliencyMap;}


private:

	/*
	/void inline calculateKernelsForChannels(
			AttributeVector& firstPairAttributes,
			AttributeVector& secondPairAttributes,
			float aNorm,
			float angleBinWidth,
			int channelCount);
	*/

	void extractChannelAttributes(
			const Location2D first,
			Channels& firstAttributes,
			cv::Mat image,
			const int channelCount
			);

	float calulateAngleKernel(
			const Channels& anglesFirst,
			const Channels& anglesSecond,
			const Channels& anglesThird,
			const Channels& anglesFourth,
			const float aNorm, const float angleBinWidth,
			const int channelCount);

	void calculateWeights(
			float& sample1Weight, float&sample2Weight,
			Channels&firstMags,
			Channels&secondMags,
			Channels&thirdMags,
			Channels&fourthMags,
			const int countChannels);

	/*
	void calculateChannelAttributes(
			const Location2D first,
			const Location2D second,
			Channels& firstAttributes,
			Channels& secondAttributes,
			const int channelCount);
	*/

	/*!
	 * Calculate the intermediate kernel sum from the contribution of the
	 * pixels given by samples
	 */
	// TODO use TSamples instead
	// How to include Samples.h without exposing it in the public API?
	void calculateKernelSum(const std::vector<Location2D>& samples, KernelDensityInfo& kernelInfo);


	/*!
	 * Update kernel sums of pixels within the applicable region given
	 */
	void updateApplicableRegion(const cv::Rect& bounds, const KernelDensityInfo& kernelSum);

	/*!
	 * Updates the saliency map using the most recent density estimates
	 */
	void createSaliencyMap();


private:
	// Pixel attributes
	cv::Mat magnitudes;
	cv::Mat orientations;

	cv::Mat srcImage;	// Unspecified channels
	cv::Mat1f saliencyMap;	// Grayscale

	sal::PDFEstimate pdfEstimate;

	cv::Size inImageSize;	// At time submitted.
};




} /* namespace sal */

