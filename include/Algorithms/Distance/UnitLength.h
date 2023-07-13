#ifndef ALGORITHMS_DISTANCE_UNITLENGTH
#define ALGORITHMS_DISTANCE_UNITLENGTH

#include "Algorithms/Distance/EdgeLength.h"

#include "DataStructures/Graphs/Windfarm.h"

namespace wcp {

namespace Algorithms {

/**
 * Class to return length of edges: All edges have unit length.
 */
struct UnitLength : EdgeLength {
    double ReturnEdgeLength(const DataStructures::Windfarm& /* windfarm */,
                            edgeID /* index */) override {
        return 1;
    }
};

} // namespace Algorithms

} // namespace wcp

#endif