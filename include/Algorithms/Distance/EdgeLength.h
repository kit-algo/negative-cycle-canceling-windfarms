#ifndef ALGORITHMS_DISTANCE_EDGELENGTH
#define ALGORITHMS_DISTANCE_EDGELENGTH

#include "DataStructures/Graphs/Windfarm.h"

namespace wcp {

namespace Algorithms {

typedef int edgeID;

/**
 * Base class to return length of edges.
 */
struct EdgeLength {
    virtual double ReturnEdgeLength(const DataStructures::Windfarm& windfarm, edgeID index) = 0;

    virtual ~EdgeLength() = default;
};

} // namespace Algorithms

} // namespace wcp

#endif