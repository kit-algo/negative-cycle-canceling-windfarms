#ifndef ALGORITHMS_DISTANCE_EUCLIDEANLENGTH
#define ALGORITHMS_DISTANCE_EUCLIDEANLENGTH

#include "Algorithms/Distance/EdgeLength.h"

#include "DataStructures/Graphs/Windfarm.h"

namespace wcp {

namespace Algorithms {

/**
 * Class to return length of edges: All edges have Euclidean length
 * given by the coordinates of the endvertices.
 */
struct EuclideanLength : EdgeLength {
    double ReturnEdgeLength(const DataStructures::Windfarm& windfarm, edgeID index) { return windfarm.Length(index); }
};

} // namespace Algorithms

} // namespace wcp

#endif