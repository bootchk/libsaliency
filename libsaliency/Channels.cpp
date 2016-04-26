
#include <cassert>

#include "Channels.hpp"
#include "SaliencyMath.hpp"

namespace sal {


Channels::Channels() {}
Channels::~Channels() {}


// result of scalar type float

float Channels::sumChannels( const int actualChannelCount) const
{
	float result = 0;
	// Not require all channel values positive
	for (int channel=0; channel<actualChannelCount; channel++) {
		result += abs(channels[channel]);
	}
	assert(result>=0.f);
	return result;

}

float Channels::productChannels( const int actualChannelCount) const
{
	float result = 1;
	// Require all channel values positive
	for (int channel=0; channel<actualChannelCount; channel++) {
		assert(channels[channel] >= 0.f );
		result *= channels[channel];
	}
	assert(result>=0.f);
	return result;
}


// result is of type Channels

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


Channels Channels::deltaChannels (
		Channels& other,
		const int actualChannelCount) const
{
	Channels result;

	// Require values are not angles (since '-' is not well-defined, use angleBetween instead.)
	for (int channel=0; channel<actualChannelCount; channel++) {
		result[channel] = abs( this->channels[channel] - other[channel] );
	}
	// ensure every value is positive
	return result;
}

Channels Channels::gaussianChannels ( const int actualChannelCount, const float height, const float width) const
{
	Channels result;

	// Require values not isnan, since gaussian requires that
	for (int channel=0; channel<actualChannelCount; channel++) {
		result[channel] = gaussian( this->channels[channel], height, width );
	}
	// ensure every value is positive, since gaussian ensures that
	return result;
}

Channels Channels::logCauchyChannels ( const int actualChannelCount, const float height, const float width) const
{
	Channels result;

	// Require values not isnan, since gaussian requires that
	for (int channel=0; channel<actualChannelCount; channel++) {
		//result[channel] = proportionToNegLnCauchy( this->channels[channel] );
		result[channel] = logCauchy( this->channels[channel], height, width );
	}
	// ensure every value is positive, since kernel function ensures that
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
