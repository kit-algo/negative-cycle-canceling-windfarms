#ifndef AUXILIARY_CONSTANTS
#define AUXILIARY_CONSTANTS

#include <limits>

namespace wcp {

namespace Auxiliary {

const       int    INFTY       = std::numeric_limits<int>::max();
const       double DINFTY      = std::numeric_limits<double>::max() / 2;

const       double DOUBLE_ROUNDING_PRECISION = 0.001;

const       double INFINITE_COST             = std::numeric_limits<double>::max() / 2.0;
const       int    INFINITE_CAPACITY         = std::numeric_limits<int>::max() / 2;

} // namespace Auxiliary

} // namespace wcp

#endif
