
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
		result[channel] = angleBetween( this[channel], other[channel] );
	}
	// ensure result element in [-pi, pi]
	// ensure prefix (up to actualChannelCount) is valid
	return result;
}



float Channels::sumChannels( const int actualChannelCount)
{

}

Channels Channels::deltaChannels (
		Channels& other,
		const int actualChannelCount)
{

}

float& Channels::operator[](int i)
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



}
