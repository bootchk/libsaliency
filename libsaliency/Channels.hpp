
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

	Channels angleBetweenChannels(
			const Channels b,
			const int actualChannelCount) const;

	float sumChannels( const int actualChannelCount);

	Channels deltaChannels (
			Channels& other,
			const int actualChannelCount);

	float& operator[](int i);

private:
	std::array<float, MAX_CHANNEL_COUNT> channels;

};

}
