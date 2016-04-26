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
	// assert -infinity < difference < +infinity i.e. difference is any real number
	float result = (1.f / height) * exp( (pow(difference, 2) / (-2.f * pow(width, 2))));

	// not assert(height < 1) => not assert(result<1)

	assert(("Gaussian is a pdf", result >= 0.f));
	// assert(difference is small and width is small) => Result may not underflow, i.e. infinitely small or zero
	// But it seems to happen anyway, so not assert(result > 0)
	assert( not isnan(result));
	return result;
}


/*
A function that is proportional to the negative of the natural log of the cauchy PDF
Note in C log() is natural log (ln in some math notation.)

cauchyParam, is a user-passed parameter to the algorithm,
In earlier versions, cauchyParam was hardcoded scaled by 256.
If the size of a Pixelel changes (if we go from 8-bit color to 16-bit), this code might still work.

In math, the denominator is the parameter to the cauchy PDF.
Here, the user's cauchyParam is a fraction (say 0.117).
*/
//static inline float
float
proportionToNegLnCauchy(float x )
{
	// float cauchyParam
	float alpha = 0.117*257;

    return log(
       (x/alpha)
      *(x/alpha)  // squared
      + 1.0
      );
}

float logCauchy(const float difference, const float height, const float width){
	// height not used

	// This accepts any real number but
	// logCauchy is defined for strictly positive numbers
	float absDifference = abs(difference);
	// avoid division by zero
	if (absDifference == 0.f) return 1.0f;

	float result = 1/(absDifference*3.14256f*width*(1+(pow((log(absDifference))/width, 2))));
	assert(result >= 0.f);
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
