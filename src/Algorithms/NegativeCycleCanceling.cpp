#include "Algorithms/NegativeCycleCanceling.h"

namespace wcp {

namespace Algorithms {

ReturnCode NegativeCycleCanceling::ComputeInitialFlow() {
#ifndef NDEBUG
    AssertZeroFlow(windfarm_);
#endif

    if (beVerbose_) { verboseStream_ << "\tInitialize flow ...\n"; }

    Timer.Restart();
    ReturnCode initializationCode = initializationStrategy_->InitializeFlow();

    double elapsedMillisecondsAfterInitialization = Timer.ElapsedMilliseconds();
    solutionCosts_ = (initializationCode == ReturnCode::SUCCESS) ? windfarm_.ComputeTotalCostOfFlow() : Auxiliary::INFINITE_COST;
    if (IsTimeLimitExceeded(elapsedMillisecondsAfterInitialization)) {
        initializationCode = ReturnCode::TIME_LIMIT;
    }

    logger_.FillSummaryWithInitializationInformation(elapsedMillisecondsAfterInitialization, solutionCosts_, initializationCode);

    if (initializationCode == ReturnCode::SUCCESS) {
#ifndef NDEBUG
        AssertFeasibleFlow(windfarm_);
#endif
        if (beVerbose_) {
            verboseStream_  << "\t... completed after "
                            << elapsedMillisecondsAfterInitialization
                            << " milliseconds.\n"
                            << std::endl;
        }
    }

    return initializationCode;
}

ReturnCode NegativeCycleCanceling::RunNCCforAllDeltas() {
    int changeOfFlow = deltaStrategy_->FirstDelta();

    ReturnCode valueForReturn;

    while (!IsTimeLimitExceeded() && deltaStrategy_->Continue(changeOfFlow)) {
        logger_.AnotherNegativeCycleDetectionRunHasStarted();

        if (beVerbose_) { verboseStream_ << "\t" << "Compute residual graph for delta " << changeOfFlow << "..." << std::endl; }

        valueForReturn = PerformNegativeCycleCancelingForSingleDelta(logger_, ResidualCostComputer(), changeOfFlow);

        if (valueForReturn == ReturnCode::TIME_LIMIT) { break; }

        changeOfFlow = deltaStrategy_->NextDelta(changeOfFlow, valueForReturn == ReturnCode::SUCCESS);

        logger_.AnotherNumberOfEdgesHaveBeenRelaxed(detectionStrategy_->NumberOfRelaxedEdgesInLastRun());
    }

#ifndef NDEBUG
    AssertFeasibleFlow(windfarm_);
#endif

    double millisecondsAtCompletion = Timer.ElapsedMilliseconds();
    double totalCostOfFlowInWindfarm = windfarm_.ComputeTotalCostOfFlow();

    if (beVerbose_) {
        verboseStream_  << "Completed Negative Cycle Canceling after "
                        << millisecondsAtCompletion 
                        << " milliseconds. The cable layout has a cost of " 
                        << totalCostOfFlowInWindfarm << "." << std::endl;
    }
    doSpecificReporting();

    valueForReturn = (IsTimeLimitExceeded(millisecondsAtCompletion)) ? ReturnCode::TIME_LIMIT : ReturnCode::SUCCESS;
    logger_.CompleteSummaryWithInformationFromAlgorithmRun(millisecondsAtCompletion, totalCostOfFlowInWindfarm, valueForReturn);
    return valueForReturn;
}

double NegativeCycleCanceling::CostOfWalk(WalkIterator begin, WalkIterator end) {
    double cost = 0;
    for (WalkIterator it = begin; it != end; ++it) {
        cost += windfarm_.ResidualCostOnEdge(*it);
    }
    return cost;
}

size_t NegativeCycleCanceling::FindClosingEdge(const std::vector<edgeID>& walk, size_t startIndex) {
    assert(startIndex < walk.size());
    size_t index = startIndex;
    while (!IsEndVertexVisited(walk[index])) {
        SetEndVertexVisited(walk[index], true);
        assert(index < walk.size());
        ++index;
    }
    return index;
}

void NegativeCycleCanceling::PrintResidualCostsAndDetectionResults(int changeOfFlow) {
    verboseStream_ << "Debugging with Delta = " << changeOfFlow << ".\n";
    ComputeResidualCosts(changeOfFlow);
    detectionStrategy_->Run();
    detectionStrategy_->Print();
}

} // namespace Algorithms

} // namespace wcp
