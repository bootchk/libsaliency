// Local math functions
// Ideally, some of these should be in another library, such as openCV

#include <cmath>	// isnan, pow, etc.
#include <cassert>

// TODO re tree include dependencies
#include "kernelDensityInfo.hpp"	// MAX_CHANNEL_COUNT
#include "SaliencyMath.hpp"

namespace sal {

// Kernel function
float gaussian(const float difference, const float height, const float width) {
	// Standard gaussian function
	// For convenience, the standard minus sign is attached to the denominator at '-2.f'
	assert(not isnan(difference));
	float result = (1.f / height) * exp( (pow(difference, 2) / (-2.f * pow(width, 2))));

	// not assert(height < 1) => not assert(result<1)

	assert(("Gaussian is a pdf", result >= 0.f));
	// assert(difference is small and width is small) => Result may not underflow, i.e. infinitely small or zero
	// But it seems to happen anyway, so not assert(result > 0)
	assert( not isnan(result));
	return result;
}





float angleBetween(const float angle1, const float angle2)
{
	// Smaller of subtended angles
	// Not just: float result = angle1 - angle2;

	// Require angles are in units radians (because sin requires it)
	// Require angles <= infinity and angles >= -infinity
	float result = atan2(sin(angle1-angle2), cos(angle1-angle2));
	assert(result >= -LARGER_THAN_PI and result <= LARGER_THAN_PI);
	// Result is signed (later we square it), but abs(value) < number slightly larger than PI
	return result;
}

}
