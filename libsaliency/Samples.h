#include <opencv2/core/core.hpp>

/// Alias for 2D Pixel location
typedef cv::Point2i Location2D;

// TODO not dynamic size, should be an array
typedef std::vector<Location2D> TSamples;

// Count of sample image locations
// TODO move this to constants.h
#define COUNT_SAMPLE_POINTS 4
