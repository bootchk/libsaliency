
#pragma once

/*
Compile time constants that limit this algorithm.
Referenced by several class definitions.
*/

/*
Max image channels this algorithm supports.

Actual channel count from source image.
Allow for future RGBAlpha or RGBDepth
Usually actual channel count is 1:grayscale or 3:color

See also definitions of Vec4f not covered by this constant
*/
#define MAX_CHANNEL_COUNT 4
