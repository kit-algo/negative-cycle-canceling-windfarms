#ifndef ALGORITHMS_NEGATIVECYCLECANCELING
#define ALGORITHMS_NEGATIVECYCLECANCELING

#include <cassert>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <vector>

#include "Algorithms/DeltaStrategies/DeltaStrategy.h"
#include "Algorithms/DetectionStrategies/NegativeCycleDetection.h"
#include "Algorithms/InitializationStrategies/InitializationStrategy.h"
#include "Algorithms/ResidualCosts/StandardResidualCostComputer.h"

#include "Auxiliary/Constants.h"
#include "Auxiliary/ReturnCodes.h"
#include "Auxiliary/Timer.h"

#include "DataStructures/Graphs/WindfarmResidual.h"
#include "DataStructures/Instance.h"

#include "IO/Auxiliary.h"
#include "IO/NCCLogger.h"

namespace wcp {

namespace Algorithms {

/**
 * Base class for an algorithm that computes a cable layout
 * of a windfarm using negative cycle canceling.
 * Use SingleRunNCC or EscapingNCC.
 */
class NegativeCycleCanceling {
public:
    NegativeCycleCanceling( DataStructures::Instance& instance,
                            WindfarmResidual& windfarm,
                            IO::NCCLogger& logger,
                            std::unique_ptr<Algorithms::InitializationStrategy>&& initializationStrategy,
                            std::unique_ptr<Algorithms::DeltaStrategy>&& deltaStrategy,
                            std::unique_ptr<Algorithms::NegativeCycleDetection>&& detectionStrategy,
                            int /*seedInput*/,
                            bool beVerbose,
                            std::ostream& verboseStream = std::cout,
                            double maxRunningTimeInMilliseconds = Auxiliary::DINFTY)
    : Timer(),
      beVerbose_(beVerbose),
      verboseStream_(verboseStream),
      windfarm_(windfarm),
      instance_(instance),
      maxRunningTimeInMilliseconds_(maxRunningTimeInMilliseconds),
      initializationStrategy_(std::move(initializationStrategy)),
      deltaStrategy_(std::move(deltaStrategy)),
      detectionStrategy_(std::move(detectionStrategy)),
      visitedVertex_(windfarm.NumberOfVertices()),
      logger_(logger),
      walkID_(0),
      solutionCosts_(Auxiliary::DINFTY) {}

    Auxiliary::Timer Timer;

    bool beVerbose_;
    std::ostream& verboseStream_;

    void ComputeResidualCosts(int changeOfFlow) {
        ResidualCostComputer().ComputeResidualCosts(changeOfFlow);
    }

    /*
     * This method is templated so that it can be used during a regular
     * NCC run with proper logging, as well as by an escaping strategy
     * which invokes NCC using special residual costs.
     * LoggingClass is responsible for outputting information obtained
     * from the cycle canceling routing and ResidualCostComputer may
     * apply non-standard residual cost computations.
     */
    template<typename LoggingClass, typename ResidualCostComputer>
    ReturnCode PerformNegativeCycleCancelingForSingleDelta( LoggingClass& logger,
                                                            ResidualCostComputer& residualCostComputer,
                                                            int delta,
                                                            bool isRegularNCCrun = true) {
        residualCostComputer.ComputeResidualCosts(delta);
        ReturnCode detectionCode = detectionStrategy_->Run();

        if (IsTimeLimitExceeded()) {
            return ReturnCode::TIME_LIMIT;
        }

        bool hasANegativeCycleBeenCanceled = false;
        if (detectionCode == ReturnCode::SUCCESS) {
            hasANegativeCycleBeenCanceled = ExtractWalkAndCancelCycle(logger, residualCostComputer, delta, isRegularNCCrun);

#ifndef NDEBUG
            AssertFeasibleFlow(windfarm_);
#endif
        }

        return (hasANegativeCycleBeenCanceled) ? ReturnCode::SUCCESS : ReturnCode::NOTHING_CHANGED;
    }

protected:
    bool IsTimeLimitExceeded() { return IsTimeLimitExceeded(Timer.ElapsedMilliseconds()); }
    bool IsTimeLimitExceeded(double elapsedMilliseconds) { return elapsedMilliseconds > maxRunningTimeInMilliseconds_; }

    virtual void dealWithBonbons(bool canceledAnyCycle, edgeID duplicateVertex, int changeOfFlow) {
        (void) canceledAnyCycle;
        (void) duplicateVertex;
        (void) changeOfFlow;
    };

    virtual void doSpecificReporting() {};

    ReturnCode ComputeInitialFlow();

    ReturnCode RunNCCforAllDeltas();

    int MaximumFlowChange() const {
        return 2 * windfarm_.Cabletypes().MaximumCapacity();
    }

    /*
     * The class which is responsible for computing residual cost.
     * Its inner workings depend on whether escaping strategies are used.
     */
    virtual StandardResidualCostComputer& ResidualCostComputer() = 0;

    template<typename LoggingClass, typename ResidualCostComputer>
    bool ExtractWalkAndCancelCycle(LoggingClass& logger, ResidualCostComputer& residualCostComputer, int changeOfFlow, bool isRegularNCCrun) {
        bool hasANegativeCycleBeenCanceled = false;
        while (!hasANegativeCycleBeenCanceled) {
            // ExtractNegativeClosedWalk for BF returns an empty walk if no edge can be relaxed further after n-1 iterations.
            auto negativeWalk = detectionStrategy_->ExtractNegativeClosedWalk();
            if (negativeWalk.empty()) {
                return false;
            }

            if (IsTimeLimitExceeded()) { return false; }

            // A negative closed walk has been found.
            hasANegativeCycleBeenCanceled = CancelNegativeWalk(logger, residualCostComputer, negativeWalk, changeOfFlow, isRegularNCCrun);
            // If no cycle has been canceled, find another walk. Otherwise, residual costs may not be valid anymore.
        }
        return hasANegativeCycleBeenCanceled;
    }

    /**
     * Cancels all simple cycles included in the given walk
     * that have negative costs and length at least 3.
     */
    template<typename LoggingClass, typename ResidualCostComputer>
    bool CancelNegativeWalk(LoggingClass& logger, ResidualCostComputer& residualCostComputer, std::vector<edgeID>& walk, int changeOfFlow, bool isRegularNCCrun) {
        assert(!walk.empty());
        assert(CostOfWalk(walk.begin(), walk.end()) < 0);
        assert(std::all_of(visitedVertex_.begin(), visitedVertex_.end(), std::logical_not<bool>()));

        edgeID edgeOnWalk = walk.front(); // Needed for dealing with bonbons. We need to put it here, since walk will be emptied if there is a bonbon.

        bool canceledAnyCycle = false; // Will be set true once a cycle is canceled

        size_t currentIndex = 0;
        vertexID duplicateVertex = INVALID_VERTEX;
        while (!walk.empty()) {
            visitedVertex_[windfarm_.StartVertex(walk[currentIndex])] = true;

            // Traverse the wald to find index of the edge after the first simple cycle is closed and the first vertex that occurs twice
            size_t indexAfterCycle = FindClosingEdge(walk, currentIndex) + 1;
            duplicateVertex = windfarm_.EndVertex(walk[indexAfterCycle - 1]);

            // Find first occurence of an edge starting at duplicateVertex (= first edge in simple cycle)
            auto cycleEnd = walk.begin() + indexAfterCycle;
            auto cycleBegin = std::find_if(walk.begin(), cycleEnd, [&](edgeID edge) {
                return windfarm_.StartVertex(edge) == duplicateVertex;
            });

            canceledAnyCycle |= CancelCycle(logger, residualCostComputer, cycleBegin, cycleEnd, changeOfFlow);

            // Set up currentIndex for next iteration. currentIndex is on first edge after simple cycle
            currentIndex = std::distance(walk.begin(), cycleBegin);
            // Reset status on visited for all vertices
            for (auto it = cycleBegin; it != cycleEnd; ++it) {
                SetEndVertexVisited(*it, false);
            }

            walk.erase(cycleBegin, cycleEnd);
        }

        // Do whatever is necessary in terms of tracking bonbons. Note that the initial assert(!walk.empty())
        // makes sure that there was indeed a negative walk in the variable walk. If at this point canceledAnyCycle
        // is false, then the walk was an unhelpful bonbon and we might want to record it for potential escaping.
        dealWithBonbons(canceledAnyCycle, edgeOnWalk, changeOfFlow);

        if (isRegularNCCrun) { ++walkID_; }

        return canceledAnyCycle;
    }

    using WalkIterator = std::vector<edgeID>::iterator;

    /**
     * Cancel the cycle given by the two iterators if the cycle has length at least 3 and negative costs.
     */
    template<typename LoggingClass, typename ResidualCostComputer>
    bool CancelCycle(LoggingClass& logger, ResidualCostComputer& residualCostComputer, WalkIterator begin, WalkIterator end, int changeOfFlow) {
        size_t length = std::distance(begin, end);
        double costChange = CostOfWalk(begin, end);

        bool cycleWillBeCanceled = length > 2 && costChange < 0;
        logger.CycleHasBeenFound("NCC", changeOfFlow, walkID_, costChange,
                                    length, Timer.ElapsedMilliseconds(),
                                    solutionCosts_, cycleWillBeCanceled);

        if (cycleWillBeCanceled) {
            solutionCosts_ += costChange;
            for (WalkIterator it = begin; it != end; ++it) {
                residualCostComputer.CreateNewCableTypeWhileCanceling(*it, changeOfFlow, false, verboseStream_); // not needed for standard NCC runs, only during escaping.
                windfarm_.AddFlowOnEdge(*it, changeOfFlow);
            }

            if (beVerbose_) {
                verboseStream_ << "\t\tCancel negative cycle:";
                for (WalkIterator it = begin; it != end; ++it) {
                        verboseStream_ << " " <<*it;
                    }
                verboseStream_ << ".\n";
            }
        }

        return cycleWillBeCanceled;
    }

    double CostOfWalk(WalkIterator begin, WalkIterator end);

    bool IsEndVertexVisited(edgeID edge) const {
        vertexID endVertex = windfarm_.EndVertex(edge);
        return visitedVertex_[endVertex];
    }

    void SetEndVertexVisited(edgeID edge, bool visited) {
        vertexID endVertex = windfarm_.EndVertex(edge);
        visitedVertex_[endVertex] = visited;
    }

    /**
     * Finds the first edge in a walk that closes a simple cycle when starting from startIndex.
     * FindClosingEdge changes values in visitedVertex_.
     */
    size_t FindClosingEdge(const std::vector<edgeID>& walk, size_t startIndex);

    void PrintResidualCostsAndDetectionResults(int changeOfFlow);

    WindfarmResidual& windfarm_;
    DataStructures::Instance& instance_;
    double maxRunningTimeInMilliseconds_;

    std::unique_ptr<Algorithms::InitializationStrategy> initializationStrategy_;
    std::unique_ptr<Algorithms::DeltaStrategy> deltaStrategy_;
    std::unique_ptr<Algorithms::NegativeCycleDetection> detectionStrategy_;

    std::vector<bool> visitedVertex_;

    IO::NCCLogger& logger_;
    int walkID_;
    double solutionCosts_;
};

} // namespace Algorithms

} // namespace wcp

#endif