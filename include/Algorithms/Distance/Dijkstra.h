#ifndef ALGORITHMS_DISTANCE_DIJKSTRA
#define ALGORITHMS_DISTANCE_DIJKSTRA

#include <cassert>
#include <limits>
#include <memory>
#include <queue>
#include <vector>

#include "Algorithms/Distance/EdgeLength.h"

#include "DataStructures/Graphs/WindfarmResidual.h"

#include "Auxiliary/ReturnCodes.h"
#include "Auxiliary/Constants.h"

namespace wcp {

namespace Algorithms {

using DataStructures::WindfarmResidual;

/**
 * Class to compute shortest paths using Dijkstra's algorithm in a
 * wind farm residual graph in which edge lengths can be specified.
 *
 * One needs to specify TargetVertex() and StartVertex() separately
 * before using the method Run().
 */
class Dijkstra {
public:
    Dijkstra(WindfarmResidual& windfarm, std::unique_ptr<EdgeLength>&& edgeLength)
      : windfarm_(windfarm),
        edgeLength_(std::move(edgeLength)),
        startVertex_(0),
        targetVertex_(0),
        numberOfSettledSubstations_(0),
        lastFreeSubstationSettled_(INVALID_VERTEX),
        dist_(windfarm.NumberOfVertices(), Auxiliary::DINFTY),
        parent_(windfarm.NumberOfVertices(), INVALID_VERTEX),
        queue_() {}

    /**
     * Runs the shortest path computation.
     *
     * After a successful run TargetVertex() contains the target
     * vertex that was found. In particular, if one searches for a substation
     * with free capacity, TargetVertex() contains the closest such substation.
     * Returns ReturnCode::Success if the target was found, ReturnCode::Infeasible
     * otherwise.
     */
    ReturnCode Run();

    /**
     * Obtain path from start to target vertex after successful run.
     * In the returned vector of edgeIDs, the first edge starts in StartVertex(),
     * the last edge ends in TargetVertex() and the i-th edge is the parent pointer
     * of the (i+1)-th edge.
     */
    std::vector<edgeID> ExtractWalk() const;

    vertexID& StartVertex()        { return startVertex_; }
    vertexID  StartVertex()  const { return startVertex_; }
    vertexID& TargetVertex()       { return targetVertex_; }
    vertexID  TargetVertex() const { return targetVertex_; }
    bool      IsAnyFreeSubstationTarget() const { return targetVertex_ == ANY_FREE_SUBSTATION; }
    bool      IsLastFreeSubstationTarget() const { return targetVertex_ == LAST_FREE_SUBSTATION; }

    static const vertexID ANY_FREE_SUBSTATION = std::numeric_limits<vertexID>::max();
    static const vertexID LAST_FREE_SUBSTATION = std::numeric_limits<vertexID>::max() - 1;

private:
    /**
     * An entry of the priority queue.
     */
    struct Entry {
        vertexID vertex;
        double distance;

        bool operator<(const Entry& other) const { return distance < other.distance; }
        bool operator>(const Entry& other) const { return distance > other.distance; }
        bool operator<=(const Entry& other) const { return distance <= other.distance; }
        bool operator>=(const Entry& other) const { return distance >= other.distance; }
    };

    using QueueType = std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>>;


    void Reset() {
        std::fill(dist_.begin(), dist_.end(), Auxiliary::DINFTY);
        // parent_ does not have to be reset because its content is only meaningful
        // if the distance is not infinity.
        queue_ = QueueType();

        numberOfSettledSubstations_ = 0;
        lastFreeSubstationSettled_ = INVALID_VERTEX;
    }

    bool IsTarget(vertexID vertex) const {
        if (targetVertex_ == ANY_FREE_SUBSTATION) {
            return windfarm_.IsSubstation(vertex)
                    && windfarm_.HasFreeSubstationCapacity(vertex);
        } else if (targetVertex_ == LAST_FREE_SUBSTATION) {
            return windfarm_.IsSubstation(vertex)
                    && windfarm_.HasFreeSubstationCapacity(vertex)
                    && numberOfSettledSubstations_ == windfarm_.NumberOfSubstations() - 1;
        } else {
            return vertex == targetVertex_;
        }
    }

    /**
     * Creates new distance label at target vertex of the parameter edge if sensible.
     */
    void RelaxEdge(edgeID edge) {
        if (!windfarm_.HasFreeEdgeCapacity(edge)) return;

        vertexID current = windfarm_.StartVertex(edge);
        vertexID end = windfarm_.EndVertex(edge);
        double distance = dist_[current] + edgeLength_->ReturnEdgeLength(windfarm_, edge);

        if (dist_[end] > distance) {
            dist_[end] = distance;
            queue_.push(Entry{end, distance});
            parent_[end] = edge;
            // This could be replaced by a decreaseKey operation (if we use a priority queue that supports this) or lazy deletion,
            // see https://stackoverflow.com/questions/9209323/easiest-way-of-using-min-priority-queue-with-key-update-in-c.
        }
    }

    void RelaxAllNeighbors(vertexID vertex) {
        for (edgeID edge : windfarm_.OutgoingEdges(vertex)) {
            RelaxEdge(edge);
        }
    }

    bool IsBestEntryForVertex(Entry current) {
        assert(current.distance >= dist_[current.vertex]);
        return current.distance == dist_[current.vertex];
    }

    WindfarmResidual& windfarm_;
    std::unique_ptr<EdgeLength> edgeLength_;
    vertexID startVertex_;
    vertexID targetVertex_;

    int numberOfSettledSubstations_;
    vertexID lastFreeSubstationSettled_;

    std::vector<double> dist_; // Contains the currently known shortest distance from the start vertex.
    std::vector<edgeID> parent_;
    QueueType queue_;
};

} // namespace Algorithms

} // namespace wcp

#endif
