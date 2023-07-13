#include <algorithm>

#include "Algorithms/Distance/Dijkstra.h"

namespace wcp {

namespace Algorithms {

ReturnCode Dijkstra::Run() {
    Reset();
    dist_[startVertex_] = 0.0;
    queue_.push(Entry{startVertex_, 0.0});

    while (!queue_.empty()) {
        Entry current = queue_.top();
        queue_.pop();

        if (dist_[current.vertex] < current.distance) {
            continue;
        }

        if (IsTarget(current.vertex)) {
            targetVertex_ = current.vertex;
            return ReturnCode::SUCCESS;
        }

        if (windfarm_.IsSubstation(current.vertex)) {
            numberOfSettledSubstations_++;
            if (windfarm_.HasFreeSubstationCapacity(current.vertex)) {
                lastFreeSubstationSettled_ = current.vertex;
            }
            continue;
        }

        if (IsBestEntryForVertex(current)) {
            RelaxAllNeighbors(current.vertex);
        }
    }

    if (targetVertex_ == Dijkstra::LAST_FREE_SUBSTATION) {
        if (lastFreeSubstationSettled_ != INVALID_VERTEX) {
            targetVertex_ = lastFreeSubstationSettled_;
            return ReturnCode::SUCCESS;
        }
    }

#ifdef NDEBUG
    std::cerr << "Infeasible" << std::endl;
#endif
    return ReturnCode::INFEASIBLE;
}

std::vector<edgeID> Dijkstra::ExtractWalk() const {
    std::vector<edgeID> walk;
    vertexID current = TargetVertex();
    while(current != StartVertex()) {
        edgeID edgeFromParent = parent_[current];
        walk.push_back(edgeFromParent);
        current = windfarm_.StartVertex(edgeFromParent);
    }

    std::reverse(walk.begin(), walk.end());

    return walk;
}

} // namespace Algorithms

} // namespace wcp
