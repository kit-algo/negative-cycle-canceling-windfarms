#ifndef ALGORITHMS_DETECTIONSTRATEGIES_NEGATIVECYCLEDETECTION
#define ALGORITHMS_DETECTIONSTRATEGIES_NEGATIVECYCLEDETECTION

#include <vector>

#include "Auxiliary/ReturnCodes.h"
#include "DataStructures/Graphs/Windfarm.h" // for vertexID, edgeID

namespace wcp {

namespace Algorithms {

/**
 * @brief A base class for strategies for detecting negative closed walks.
 */
class NegativeCycleDetection {
public:
    /**
     * Runs the negative walk detection.
     *
     * Returns ReturnCode::EARLY_TERMINATION if it was detected that there is
     * no negative closed walk in the graph. Otherwise, ReturnCode::SUCCESS
     * is returned. In that case, a negative walk may or may not exist. If it
     * does, it can be retrieved using ExtractNegativeClosedWalk().
     */
    virtual ReturnCode Run() = 0;

    /**
     * Extracts a negative closed walk, in which the edges are ordered
     * as in the graph, i.e., the end vertex of the first edge
     * equals the start vertex of the second edge.
     *
     * If no such negative closed walk exists, an empty vector is returned.
     */
    virtual std::vector<edgeID> ExtractNegativeClosedWalk() = 0;
    virtual std::vector<edgeID> TraverseParentsUntilParticularEdgeRepeats(vertexID vertexToStartFrom) = 0;
    virtual std::vector<edgeID> TraverseParentsUntilWalkStarts(std::vector<edgeID>& closedWalk, edgeID firstEdge) = 0;
    virtual void Print() = 0;

    size_t NumberOfRelaxedEdgesInLastRun() const { return numberOfRelaxedEdgesInLastRun_; }

    virtual ~NegativeCycleDetection() = default;

protected:
    size_t numberOfRelaxedEdgesInLastRun_ = 0;
};

} // namespace Algorithms

} // namespace wcp

#endif
