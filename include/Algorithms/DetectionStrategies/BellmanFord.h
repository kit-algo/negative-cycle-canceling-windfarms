#ifndef ALGORITHMS_DETECTIONSTRATEGIES_BELLMANFORD
#define ALGORITHMS_DETECTIONSTRATEGIES_BELLMANFORD

#include <algorithm>
#include <iterator>
#include <vector>

#include "Algorithms/DetectionStrategies/NegativeCycleDetection.h"
#include "Auxiliary/ReturnCodes.h"
#include "DataStructures/Graphs/WindfarmResidual.h"
#include "DataStructures/Labels/OneDistanceLabel.h"
#include "DataStructures/Labels/TwoDistancesLabel.h"

namespace wcp {

namespace Algorithms {

using DataStructures::WindfarmResidual;

/**
 * The Bellman-Ford algorithm using a template type for the labels
 * stored at vertices. One label per vertex is stored and the type
 * reflects which version of the algorithm is run.
 * DataStructures::TwoDistancesLabel yields the BF-algorithm on the
 * linegraph as described in doi:10.4230/LIPIcs.ESA.2019.55.
 *
 * The template class LabelType must offer four methods with
 * the following signatures:
 * bool Update(double distance, edgeID incomingEdge);
 * bool WouldUpdate(double distance, edgeID incomingEdge) const;
 * double Distance(const WindfarmResidual &, edgeID) const;
 * edgeID Parent(const WindfarmResidual &, edgeID) const;
 * The first method is called whenever one of the incoming edges of the vertex
 * with the label was relaxed. It shall update the label based on this new
 * distance and return whether the label changed. The second method shall return
 * whether such an update would change the label but it should not change the
 * label.
 * The last two methods map a label at a vertex v and the ID of an outgoing edge to the
 * distance and parent edge of v that is appropriate for the outgoing edge,
 * respectively.
 */
template<typename LabelType>
class BellmanFord : public NegativeCycleDetection {
public:
    BellmanFord(const WindfarmResidual& windfarm)
      : windfarm_(windfarm),
        labels_(windfarm.NumberOfVertices()),
        indexOfWalkIncluded_(windfarm.NumberOfEdges()),
        currentWalkIndex_(0),
        updatedSinceLastTime_(windfarm.NumberOfVertices(), true),
        vertexOfLastUpdate_(windfarm.NumberOfVertices() - 1),
        earlyTermination_(false) {}

    /**
     * Runs the Bellman-Ford algorithm for the fixed number of rounds depending on the LabelType.
     * Returns ReturnCode::EARLY_TERMINATION if there is one iteration without
     * updates and returns ReturnCode::SUCCESS, otherwise.
     */
    ReturnCode Run() {
        Reset();
        InitializeLabels();

        // Main body of Bellman-Ford algorithm
        for (int i = 0; i < NumberOfIterations(); ++i) {
            for (vertexID vertex = 0; vertex < windfarm_.NumberOfVertices(); ++vertex) {
                // Going over all edges is the same as going over all vertices and their respective outgoing edges

                if (vertex == vertexOfLastUpdate_) {
                    // No changes to any label after this vertex was considered last round. Terminate now. 
                    earlyTermination_ = true;
                    return ReturnCode::EARLY_TERMINATION;
                }

                if (updatedSinceLastTime_[vertex]) {
                    RelaxOutgoingEdges(vertex);
                    updatedSinceLastTime_[vertex] = false; // Will be set to true if an incoming edge allowed a relaxation, i.e. the label at vertex is changed
                }
            }
        }
        return ReturnCode::SUCCESS;
    }

    /**
     * Extracts a negative closed walk from a still relaxable edge. The walk was
     * not returned before. The edges on the walk are ordered as follows: the
     * end vertex of the first edge equals the start vertex of the second edge.
     *
     * If no such negative closed walk exists (anymore), an empty vector is returned.
     */
    std::vector<edgeID> ExtractNegativeClosedWalk() {
        if (earlyTermination_) return {};

        // Increment number of walks found for the current set of residual costs
        ++currentWalkIndex_;

        // Implicit iteration over all edges
        for (vertexID vertex = 0; vertex < windfarm_.NumberOfVertices(); ++vertex) {
            edgeID relaxableEdge = FindRelaxableOutgoingEdge(vertex);
            if (relaxableEdge != INVALID_EDGE) {
                // There is a negative closed walk leading to relaxableEdge
                std::vector<edgeID> walk = ExtractWalk(relaxableEdge);
                if (!walk.empty()) {
                    // That walk has been returned before
                    return walk;
                }
                ++currentWalkIndex_;
            }
        }

        return {};
    }

    /**
     * Caution: Might incur infinity loop!
     * Very similar to method ExtractWalk, but that one yields the walk in opposite order
     * and deletes any extracted edges outside the negative walk. This method can 
     * and should only be used if edgeToStartFrom is on the closed walk.
     *
     * We use it for the DealWithBonbon escaping strategy.
     */
    std::vector<edgeID> TraverseParentsUntilParticularEdgeRepeats(edgeID edgeToStartFrom) {
        if (earlyTermination_) return {};
        currentWalkIndex_ = 1;

        std::vector<edgeID> walk;
        edgeID currentEdge = edgeToStartFrom;
        if (currentEdge == INVALID_EDGE) return {};
        do {
            SetToCurrentWalk(currentEdge);
            walk.push_back(currentEdge);
            vertexID startVertex = windfarm_.StartVertex(currentEdge);
            currentEdge = LabelAt(startVertex).Parent(windfarm_, currentEdge);
        } while (currentEdge != walk.front());

        return walk;
    }

    /**
     * Extracts a simple walk between closedWalk and firstEdge by traversing
     * parent pointers from firstEdge until a vertex in closedWalk is visited
     * or a vertex in the parent pointer chain is repeated.

     * The last edge in the vector is that edge whose StartVertex has been visited before,
     * either by the closedWalk or by the chain of parent pointers. In the
     * latter case, firstEdge need not be the first edge in the returned vector
     * but a simple cycle is returned. In both cases, vector[i+1] is the
     * parent of vector[i]. If an invalid edge is found as a parent, then an
     * empty vector is returned.
     */
    std::vector<edgeID> TraverseParentsUntilWalkStarts(std::vector<edgeID>& closedWalk, edgeID firstEdge) {
        assert(firstEdge != INVALID_EDGE);
        
        std::vector<edgeID> walk;
        std::vector<bool> isVertexVisited(windfarm_.NumberOfVertices(), false);
        std::vector<bool> isVertexOnWalk(windfarm_.NumberOfVertices(), false);
        for (edgeID i : closedWalk) {
            // Visit all vertices on closedWalk
            isVertexOnWalk[windfarm_.EndVertex(i)] = true;
        }

        edgeID currentEdge = firstEdge;
        isVertexVisited[windfarm_.EndVertex(currentEdge)] = true;
        while (currentEdge != INVALID_EDGE) {
            walk.push_back(currentEdge);
            if (isVertexOnWalk[windfarm_.StartVertex(currentEdge)]) {
                // at this point, walk is a path between two vertices on closedWalk
                // walk is simple because currentEdge is the first edge on walk that starts in a vertex on closedWalk and
                // because no vertex had been repeated before.
                return walk;
            }
            if (isVertexVisited[windfarm_.StartVertex(currentEdge)]) {
                // traverse walk to front to find edge s.t. TargetVertex(edge) = StartVertex(currentEdge) and return everything in between
                vertexID duplicateVertex = windfarm_.StartVertex(currentEdge);
                int vectorIndexOfClosingEdge = walk.size()-1;
                while (windfarm_.EndVertex(walk[vectorIndexOfClosingEdge]) != duplicateVertex) {
                    vectorIndexOfClosingEdge--;
                    // cannot become negative becaue of if-condition above!
                }
                std::vector<edgeID> v;
                std::copy(walk.begin()+vectorIndexOfClosingEdge, walk.end(), std::back_inserter(v));
                assert(windfarm_.EndVertex(v.front()) == windfarm_.StartVertex(v.back()));
                return v;
            } else {
                isVertexVisited[windfarm_.StartVertex(currentEdge)] = true;
            }
            
            currentEdge = LabelAt(windfarm_.StartVertex(currentEdge)).Parent(windfarm_, currentEdge);
            // if currentEdge is invalid, while-loop will be broken and an empty vector returned
        }
        std::vector<edgeID> empty_walk;
        return empty_walk;
    }


    const std::vector<LabelType>& Labels() const { return labels_; }
    const LabelType& LabelAt(vertexID vertex) const { return labels_[vertex]; }
    int NumberOfIterations() const;

private:
    /**
     * Gives acess to label at the vertex.
     */
    LabelType& MutableLabelAt(vertexID vertex) { return labels_[vertex]; }

    /**
     * Resets members used in previous runs of BellmanFord, e.g. with different residual costs
     */
    void Reset() {
        std::fill(labels_.begin(), labels_.end(), LabelType());
        std::fill(indexOfWalkIncluded_.begin(), indexOfWalkIncluded_.end(), 0);
        currentWalkIndex_ = 0;

        std::fill(updatedSinceLastTime_.begin(), updatedSinceLastTime_.end(), true);
        vertexOfLastUpdate_ = windfarm_.NumberOfVertices() - 1;
        earlyTermination_ = false;
        numberOfRelaxedEdgesInLastRun_ = 0;
    }

    /**
     * Sets all labels to their initial value given by the residual costs
     */
    void InitializeLabels() {
        for (edgeID edge = 0; edge < windfarm_.NumberOfEdges(); ++edge) {
            vertexID endVertex = windfarm_.EndVertex(edge);
            MutableLabelAt(endVertex).Update(windfarm_.ResidualCostOnEdge(edge), edge);
        }
    }

    void RelaxOutgoingEdges(vertexID vertex) {
        bool atLeastOneUpdate = false;
        for (edgeID edge : windfarm_.OutgoingEdges(vertex)) {
            // Compute the distance to the endvertex of edge using the label at vertex
            vertexID endVertex = windfarm_.EndVertex(edge);
            double distance = LabelAt(vertex).Distance(windfarm_, edge)
                                + windfarm_.ResidualCostOnEdge(edge);
#ifndef NDEBUG
            // For debugging: Find out whether distance allows an update of the label at endvertex
            bool willUpdate = LabelAt(endVertex).WouldUpdate(distance, edge);
#endif
            // Update the label at endvertex if possible
            bool didUpdate = MutableLabelAt(endVertex).Update(distance, edge);
            assert(didUpdate == willUpdate);

            // Maintain information for early termination
            updatedSinceLastTime_[endVertex] = updatedSinceLastTime_[endVertex] | didUpdate;
            numberOfRelaxedEdgesInLastRun_ += didUpdate;
            atLeastOneUpdate |= didUpdate;
        }

        if (atLeastOneUpdate) {
            vertexOfLastUpdate_ = vertex;
        }
    }

    /**
     * Finds a relaxable outgoing edge of a given vertex. Edges
     * that have already been on any walk are ignored.
     */
    edgeID FindRelaxableOutgoingEdge(vertexID vertex) {
        for (edgeID edge : windfarm_.OutgoingEdges(vertex)) {
            // If edge was on any walk extracted in the current residual graph, then 
            // traversion the parent chain from edge would yield the same walk again.
            if (OnAnyWalk(edge)) continue;

            vertexID endVertex = windfarm_.EndVertex(edge);
            double distance = LabelAt(vertex).Distance(windfarm_, edge)
                                                + windfarm_.ResidualCostOnEdge(edge);
            if (LabelAt(endVertex).WouldUpdate(distance, edge)) {
                return edge;
            }
        }
        return INVALID_EDGE;
    }

    bool OnAnyWalk(edgeID edge) {
        // index != 0 means that edge was on the walk numbered by index
        return indexOfWalkIncluded_[edge];
    }

    bool OnCurrentWalk(edgeID edge) {
        return indexOfWalkIncluded_[edge] == currentWalkIndex_;
    }

    void SetToCurrentWalk(edgeID edge) {
        assert(!OnAnyWalk(edge));
        indexOfWalkIncluded_[edge] = currentWalkIndex_;
    }

    /**
     * Extracts the closed walk which led to the relaxable edge lastEdge.
     * The edges on the walk are ordered as follows: the end vertex of the first edge
     * equals the start vertex of the second edge.
     *
     * If during the traversal of parent pointers an edge is encountered
     * that lies on any previous walk the extraction is aborted
     * and an empty vector is returned.
     */
    std::vector<edgeID> ExtractWalk(edgeID lastEdge) {
        std::vector<edgeID> walk;
        edgeID currentEdge = lastEdge;

        while (!OnAnyWalk(currentEdge)) {
            walk.push_back(currentEdge); // inserts edges in reverse order
            SetToCurrentWalk(currentEdge);
            vertexID startVertex = windfarm_.StartVertex(currentEdge);

            currentEdge = LabelAt(startVertex).Parent(windfarm_, currentEdge);
            assert(windfarm_.EndVertex(currentEdge) == startVertex);
        }
        // If traversing the parent chain repeats an edge, the condition is violated
        // because that edge is on the current walk. If an edge is on an earlier walk,
        // walk is not enlarged any more and the currentwalkindex does not give a closed walk.

        // If the loop is broken due to a repetition in the current walk,
        // walk includes a sequence of edges which go into a negative closed subwalk.
        // We want that closed subwalk.
        std::vector<edgeID> walkWithoutTail;
        if (OnCurrentWalk(currentEdge)) { // while-loop was broken due to repetition of currentEdge, i.e.
                // we have a closed walk that was not returned earlier
            auto firstOccurenceOfCurrent = std::find(walk.begin(), walk.end(), currentEdge);
            assert(firstOccurenceOfCurrent != walk.end());
            // copy walk between the first and last occurence of currentEdge. The last occurence is also the end of walk.
            std::copy(walk.rbegin(), make_reverse_iterator(firstOccurenceOfCurrent),
                    back_inserter(walkWithoutTail));

#ifndef NDEBUG
            assert(!walkWithoutTail.empty());
            assert(walkWithoutTail.back() == currentEdge);
            assert(windfarm_.StartVertex(walkWithoutTail.front())
                    == windfarm_.EndVertex(walkWithoutTail.back()));

            for (size_t i = 0; i < walkWithoutTail.size() - 1; ++i) {
                assert(windfarm_.EndVertex(walkWithoutTail[i])
                        == windfarm_.StartVertex(walkWithoutTail[i+1]));
            }
#endif

        }
        return walkWithoutTail;
    }

    /**
     * Prints residual costs, parent labels, and negative closed walk to std::cout.
     */
    void Print() {
        std::cout << "Residual Costs and Parents:\n";
        for (edgeID edge = 0; edge < windfarm_.NumberOfOriginalEdges(); ++edge) {
            std::cout << "Edge " << edge << ": " << windfarm_.ResidualCostOnEdge(edge) << " with Parent " << labels_[windfarm_.StartVertex(edge)].Parent(windfarm_,edge);
            std::cout << " Reverse Edge " << windfarm_.NumberOfOriginalEdges() + edge << ": " << windfarm_.ResidualCostOnEdge(windfarm_.NumberOfOriginalEdges() + edge) << " with Parent " << labels_[windfarm_.StartVertex(windfarm_.NumberOfOriginalEdges() + edge)].Parent(windfarm_,windfarm_.NumberOfOriginalEdges() + edge) << "\n";
        }
        std::cout << '\n';

        auto walk = ExtractNegativeClosedWalk();
        for (auto i : walk) {
            std::cout << " " << i << " \n";
        }
    }

    const WindfarmResidual& windfarm_;

    std::vector<LabelType> labels_;

    /**
     * Maps an edge to the index of the walk in which the edge is included.
     * A value of 0 indicates that the edge has not been on any walk so far.
     */
    std::vector<unsigned> indexOfWalkIncluded_;
    unsigned currentWalkIndex_;
    std::vector<bool> updatedSinceLastTime_;
    vertexID vertexOfLastUpdate_;
    bool earlyTermination_;
    /// @}
};

using SideTripFreeBellmanFord = BellmanFord<DataStructures::TwoDistancesLabel>;
using StandardBellmanFord = BellmanFord<DataStructures::OneDistanceLabel>;

template<>
inline int SideTripFreeBellmanFord::NumberOfIterations() const {
    return 2 * windfarm_.NumberOfVertices() - 1;
}

template<>
inline int StandardBellmanFord::NumberOfIterations() const {
    return windfarm_.NumberOfVertices() - 1;
}

} // namespace Algorithms

} // namespace wcp

#endif
