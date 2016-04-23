
#include <cassert>

#include "Channels.hpp"
#include "SaliencyMath.hpp"

namespace sal {


Channels::Channels() {}
Channels::~Channels() {}

Channels Channels::angleBetweenChannels(
		const Channels other,
		const int actualChannelCount) const
{
	assert(actualChannelCount < MAX_CHANNEL_COUNT);

	Channels result;

	for (int channel=0; channel<actualChannelCount; channel++) {
		result[channel] = angleBetween( this->channels[channel], other[channel] );
	}
	// ensure result element in [-pi, pi]
	// ensure prefix (up to actualChannelCount) is valid
	return result;
}



float Channels::sumChannels( const int actualChannelCount)
{
	float result = 0;
	// Not require all channel values positive
	for (int channel=0; channel<actualChannelCount; channel++) {
		result += abs(channels[channel]);
	}
	assert(result>=0.f);
	return result;

}

// TODO const
Channels Channels::deltaChannels (
		Channels& other,
		const int actualChannelCount)
{
	Channels result;

	// Require values are not angles (since '-' is not well-defined, use angleBetween instead.)
	for (int channel=0; channel<actualChannelCount; channel++) {
		result[channel] = abs( this->channels[channel] - other[channel] );
	}
	// ensure every value is positive
	return result;
}

// Overloaded
//float& Channels::operator[] (int i) const
float& Channels::operator[] (int i)
{
	/*
		if( i > SIZE )
		{
			cout << "Index out of bounds" <<endl;
			// return first element.
			return arr[0];
		}
	 */
	return channels[i];
}

const float& Channels::operator[] (int i) const
{
	return channels[i];
}



}
