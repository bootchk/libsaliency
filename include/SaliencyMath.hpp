
#pragma once


// Pi is not exactly expressable in float.  Value of M_PI from math.h depends on C implementation
#define LARGER_THAN_PI  3.1416


namespace sal {

// TODO Angle type

float angleBetween(const float angle1, const float angle2);

float gaussian(const float difference, const float height, const float width);

}
