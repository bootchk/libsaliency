
#pragma once

#include <array>


namespace sal {

/*!
 Short container of attributes, one per channel (e.g. color)
 OpenCV requires def of inner datatype, i.e. custom type.....   typedef cv::Vec<Attribute, 4>  AttributeVector;
 std::array requires C++11
 A math 'vector' of fixed length, but a C++ array
 */

#define MAX_CHANNEL_COUNT 4


//class Channels;

class Channels {
public:
	explicit Channels();
	virtual ~Channels();


	// Scalar results (reduce)

	// Scalar sum of abs of values of Channels
	float sumChannels( const int actualChannelCount)const;
	// Scalar product of channels
	float productChannels( const int actualChannelCount) const;


	// Channel results (element-wise)
	// All are const functions: do not alter self

	// Return Channels whose values are angle between this and other Channels (element-wise)
	Channels angleBetweenChannels(
			const Channels b,
			const int actualChannelCount) const;

	// Return Channels whose values are abs of difference this and other (element-wise)
	// !!! Not to be used for angles.
	Channels deltaChannels (
			Channels& other,
			const int actualChannelCount) const;

	// Return Channels whose values are gaussians of this
	Channels gaussianChannels(
			const int actualChannelCount,
			const float height, const float width) const;

	Channels logCauchyChannels (
			const int actualChannelCount,
			const float height, const float width) const;

	// Obscure C++, see Scott Meyers "Effective C++"
	// Overloaded operator
	// Returns pointer to non-const (can be used on lhs) to float
	float& operator[] (int i);
	// Return pointer to const
	const float& operator[] (int i) const;

private:
	std::array<float, MAX_CHANNEL_COUNT> channels;

};

}
