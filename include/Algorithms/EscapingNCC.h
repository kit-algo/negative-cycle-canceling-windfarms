#ifndef ALGORITHMS_ESCAPINGNCC
#define ALGORITHMS_ESCAPINGNCC

#include <cassert>
#include <memory>
#include <ostream>
#include <utility>
#include <vector>

#include "Algorithms/DeltaStrategies/DeltaStrategy.h"
#include "Algorithms/DetectionStrategies/NegativeCycleDetection.h"
#include "Algorithms/EscapeStrategies/KeeperOfEscapingStrategies.h"
#include "Algorithms/InitializationStrategies/InitializationStrategy.h"
#include "Algorithms/ResidualCosts/EscapingResidualCostComputer.h"
#include "Algorithms/ResidualCosts/SpecializedResidualCosts.h"
#include "Algorithms/NegativeCycleCanceling.h"

#include "Auxiliary/Constants.h"
#include "Auxiliary/ReturnCodes.h"

#include "DataStructures/Graphs/WindfarmResidual.h"
#include "DataStructures/Instance.h"

#include "IO/NCCLogger.h"

namespace wcp {

namespace Algorithms {

/**
 * Struct to store an unhelpful closed walk ("bonbon").
 * Run the detection strategy again with changeOfFlow
 * to recover the bonbon.
 */
struct BonbonInformation {
    int changeOfFlow = -1; // Delta value for which bonbon was found
    edgeID edgeOnBonbon = INVALID_EDGE;

    void Reset() {
        changeOfFlow = -1;
        edgeOnBonbon = INVALID_EDGE;
    }
};

struct SpecialResidualCost;

/**
 * @brief Class that repeatedly alternates NCC runs
 * and escaping strategies to move away from local
 * minima, corresponding to doi:10.1145/3396851.3397754.
 */
class EscapingNCC : public NegativeCycleCanceling {
public:
    EscapingNCC(DataStructures::Instance& instance,
                WindfarmResidual& windfarm,
                IO::NCCLogger& logger,
                std::unique_ptr<Algorithms::InitializationStrategy>&& initializationStrategy,
                std::unique_ptr<Algorithms::DeltaStrategy>&& deltaStrategy,
                std::unique_ptr<Algorithms::NegativeCycleDetection>&& detectionStrategy,
                int seedInput,
                std::unique_ptr<Algorithms::KeeperOfEscapingStrategies>&& keeperOfEscapingStrategies,
                unsigned int iterationLimit,
                bool beVerbose,
                std::ostream& verboseStream = std::cout,
                double maxRunningTimeInMilliseconds = Auxiliary::DINFTY)
      : NegativeCycleCanceling( instance,
                                windfarm,
                                logger,
                                std::move(initializationStrategy),
                                std::move(deltaStrategy),
                                std::move(detectionStrategy),
                                seedInput,
                                beVerbose,
                                verboseStream,
                                maxRunningTimeInMilliseconds),
        keeperOfEscapingStrategies_(std::move(keeperOfEscapingStrategies)),
        iterationLimit_(iterationLimit),
        bestFlow_(windfarm_.NumberOfEdges(), Auxiliary::INFINITE_CAPACITY),
        bestSolutionValue_(Auxiliary::INFINITE_COST),
        residualCostComputer_(windfarm),
        runs_(0)
    {
        InferSettingsFromEscapingStrategies();

        InitializeKeeperOfEscapingStrategies(seedInput);

        RunCompleteAlgorithm();
    }

    BonbonInformation bonbon_;

    NegativeCycleDetection& DetectionStrategy() {
        return *detectionStrategy_;
    }

    double CostOfWalk(WalkIterator begin, WalkIterator end) {
        return NegativeCycleCanceling::CostOfWalk(begin, end);
    }

    void CopyAdaptedResidualCosts(const SpecialResidualCostCollection& newResidualCosts) {
        residualCostComputer_.CopyAdaptedResidualCosts(newResidualCosts);
    }

    void ClearListOfAdaptedResidualCosts() {
        residualCostComputer_.ClearListOfAdaptedResidualCosts();
    }

    void StealResidualCostCollectionInConstructionFrom(ResidualCostComputerFreeSatisfiedEdges& residualCostComputer) {
        residualCostComputer_.StealResidualCostCollectionInConstructionFrom(residualCostComputer);
    }

private:
    virtual StandardResidualCostComputer& ResidualCostComputer() { return residualCostComputer_; }

    ReturnCode RunCompleteAlgorithm() {
        ReturnCode valueForReturn = ComputeInitialFlow();

        if (valueForReturn == ReturnCode::SUCCESS && iterationLimit_ == 0) {
            valueForReturn = ReturnCode::ITERATION_LIMIT;
        }

        runs_ = 1; // the first run is NCC only, after that a run is Escaping+NCC

        if (valueForReturn == ReturnCode::SUCCESS) {
            valueForReturn = AlternateNCCandEscaping();
            CopyBestFlowToWindfarm();
        }

        logger_.CompleteSummaryWithInformationFromAlgorithmRun(windfarm_.ComputeTotalCostOfFlow(), valueForReturn);
        return valueForReturn;
    }

    ReturnCode AlternateNCCandEscaping() {
        ReturnCode valueForReturn = ReturnCode::IN_PROGRESS;

        while (valueForReturn == ReturnCode::IN_PROGRESS) {
            if (beVerbose_) {
                verboseStream_ << "Start iteration with number " << runs_ << ".\n";
                residualCostComputer_.OutputAdaptations(verboseStream_);
            }
            bonbon_.Reset();
            ReturnCode ncc_status = RunNCCforAllDeltas();

            // Since residual costs might have been adjusted due to escaping, we need to compute the total costs from scratch.
            double realSolutionCostsWithoutChanges = windfarm_.ComputeTotalCostOfFlow();

            logger_.SaveRuntimeDataAfterHalfIteration("NCC",
                                                realSolutionCostsWithoutChanges,
                                                realSolutionCostsWithoutChanges - solutionCosts_,
                                                ReturnCode::SUCCESS);
            SaveBestFlowIfNecessary(realSolutionCostsWithoutChanges);

            if (OverwriteReturnCodeIfNCCUnsuccessful(valueForReturn, ncc_status)) { break; }

            // NCC run is complete. Unless the time limit is exceeded or iteration limit is saturated, start escaping.
            valueForReturn = TryToChangeFlowByApplyingEscapingStrategies(realSolutionCostsWithoutChanges);
        }

        return valueForReturn;
    }

    ReturnCode TryToChangeFlowByApplyingEscapingStrategies(double realCostOfFlowInWindfarm) {
        ReturnCode valueForReturn = ReturnCode::IN_PROGRESS;
        while (valueForReturn == ReturnCode::IN_PROGRESS) {
            runs_++;
            assert(keeperOfEscapingStrategies_->CheckConsistency());
            if (IsTimeLimitExceeded()) { return ReturnCode::TIME_LIMIT; }
            if (IsIterationLimitExceeded()) { return ReturnCode::ITERATION_LIMIT; }

            keeperOfEscapingStrategies_->DrawNextEscapingStrategy();
            if (keeperOfEscapingStrategies_->HasCurrentStrategyBeenUsedInCurrentIteration()) {
                // Ignore this escaping strategy, skip NCC and pick again
                logger_.SaveRuntimeDataAfterHalfIteration(keeperOfEscapingStrategies_->NameOfCurrentStrategy(),
                                                    Timer.ElapsedMilliseconds(),
                                                    realCostOfFlowInWindfarm,
                                                    0, ReturnCode::EARLY_TERMINATION);

                assert(valueForReturn == ReturnCode::IN_PROGRESS);
                continue;
            } else {
                valueForReturn = keeperOfEscapingStrategies_->ApplyCurrentStrategy(*this);

                if (valueForReturn == ReturnCode::SUCCESS) {
                    DoBookkeepingAfterSuccessfulEscaping();
                    if (beVerbose_) {
                        verboseStream_ << "Escaping strategy " << keeperOfEscapingStrategies_->NameOfCurrentStrategy() << " successful.\n\n";
                    }
                    return ReturnCode::IN_PROGRESS; // Go back to NCC
                } else {
                    assert(valueForReturn == ReturnCode::NOTHING_CHANGED);
                    DoBookkeepingAfterUnsuccessfulEscaping(realCostOfFlowInWindfarm);
                    if (beVerbose_) {
                        verboseStream_ << "Escaping strategy " << keeperOfEscapingStrategies_->NameOfCurrentStrategy() << " not successful.\n\n";
                    }
                    valueForReturn = CanEscapingContinueAfterUnsuccessfulTry();
                }
            }
        }
        return valueForReturn;
    }

    void DoBookkeepingAfterSuccessfulEscaping() {
        double previousCostValueForCostChangeComputation = solutionCosts_;
        solutionCosts_ = windfarm_.ComputeTotalCostOfFlow(); // Does not use (special) residual costs in windfarm_._residualCosts
        SaveBestFlowIfNecessary(solutionCosts_);

        keeperOfEscapingStrategies_->SetCurrentStrategyAsUsedSuccessfully();

        logger_.SaveRuntimeDataAfterHalfIteration(keeperOfEscapingStrategies_->NameOfCurrentStrategy(),
                                            Timer.ElapsedMilliseconds(),
                                            solutionCosts_,
                                            solutionCosts_ - previousCostValueForCostChangeComputation,
                                            ReturnCode::SUCCESS);
    }

    void DoBookkeepingAfterUnsuccessfulEscaping(double realCostOfFlowInWindfarm) {
        keeperOfEscapingStrategies_->SetCurrentStrategyAsUsedUnsuccessfully();

        logger_.SaveRuntimeDataAfterHalfIteration(keeperOfEscapingStrategies_->NameOfCurrentStrategy(),
                                            Timer.ElapsedMilliseconds(),
                                            realCostOfFlowInWindfarm,
                                            0, ReturnCode::NOTHING_CHANGED);
    }

    ReturnCode CanEscapingContinueAfterUnsuccessfulTry() {
        if (keeperOfEscapingStrategies_->HaveAllStrategiesBeenUsedInCurrentIteration()) {
            return ReturnCode::EARLY_TERMINATION;
        } else {
            return ReturnCode::IN_PROGRESS;
        }
    }

    void InitializeKeeperOfEscapingStrategies(int seedInput) {
        keeperOfEscapingStrategies_->InitializeRNGandTracking(runs_, seedInput);
    }

    void InferSettingsFromEscapingStrategies() {
        CapIterationLimitInAbsenceOfEscapingStrategies();
    }

    bool IsIterationLimitExceeded() {
        return runs_ > iterationLimit_;
    }

    bool OverwriteReturnCodeIfTimeLimitExceeded(ReturnCode& statusOfProgram) {
        if (IsTimeLimitExceeded()) {
            statusOfProgram = ReturnCode::TIME_LIMIT;
            return true;
        } else { return false; }
    }

    bool OverwriteReturnCodeIfIterationLimitExceeded(ReturnCode& statusOfProgram) {
        if (IsIterationLimitExceeded()) {
            statusOfProgram = ReturnCode::ITERATION_LIMIT;
            return true;
        } else { return false; }
    }

    bool OverwriteReturnCodeIfNCCUnsuccessful(ReturnCode& statusOfProgram, ReturnCode& statusOfNCC) {
        if (statusOfNCC != ReturnCode::SUCCESS) {
            statusOfProgram = statusOfNCC;
            return true;
        }
        return false;
    }

    void CapIterationLimitInAbsenceOfEscapingStrategies() {
        if (!keeperOfEscapingStrategies_->AtLeastOneEscapingStrategy()) {
            iterationLimit_ = 1;
        }
    }

    void SaveBestFlowIfNecessary(double costOfFlowInWindfarm) {
        if (costOfFlowInWindfarm < bestSolutionValue_) {
            UpdateBestFlow(costOfFlowInWindfarm);

            if (beVerbose_) {
                WriteBestFlowToStream(verboseStream_);
            }
        }
    }

    void UpdateBestFlow(double newBestCost) {
        bestSolutionValue_ = newBestCost;
        CopyBestFlowFromWindfarm();
    }

    void CopyBestFlowFromWindfarm() {
        assert(bestFlow_.size() == static_cast<size_t>(windfarm_.NumberOfEdges()));
        for (int edge = 0; edge < windfarm_.NumberOfEdges(); edge++) {
            bestFlow_[edge] = windfarm_.FlowOnEdge(edge);
        }
    }

    void CopyBestFlowToWindfarm() {
        assert(bestFlow_.size() == static_cast<size_t>(windfarm_.NumberOfEdges()));
        for (int edge = 0; edge < windfarm_.NumberOfEdges(); edge++) {
            windfarm_.SetFlowOnEdge(edge, bestFlow_[edge]);
        }
    }

    void WriteBestFlowToStream(std::ostream& os) {
        os << "A new best flow has been found. Flow values are: ";
        for (int edge = 0; edge < windfarm_.NumberOfOriginalEdges(); edge++) {
            os << bestFlow_[edge] << " ";
        }
        os << "\n";
    }
    
    /**
     * This function maintains the bonbon_ member of this class
     * Make sure to only call this function with canceledAnyCycle == false
     * if there really was an unhelpful bonbon.
     * This function records an unhelpful bonbon if one occured and
     * deletes the information once some cycle has been cancelled.
     */
    void dealWithBonbons(bool canceledAnyCycle, edgeID edgeOnWalk, int changeOfFlow) override {
        if (canceledAnyCycle) {
            bonbon_.changeOfFlow = -1;
            bonbon_.edgeOnBonbon = INVALID_EDGE;
        } else {
            bonbon_.changeOfFlow = changeOfFlow;
            bonbon_.edgeOnBonbon = edgeOnWalk;
        }
    }

    void doSpecificReporting() override {
        if (beVerbose_) {
            verboseStream_ << "BonbonInformation is as follows. Delta: " << bonbon_.changeOfFlow << ". Edge: " << bonbon_.edgeOnBonbon << ".\n";
        }
    }

    std::unique_ptr<KeeperOfEscapingStrategies> keeperOfEscapingStrategies_;
    unsigned int iterationLimit_;
    std::vector<int> bestFlow_;
    double bestSolutionValue_;
    ResidualCostComputerWithAdaptedEdges residualCostComputer_;

    unsigned int runs_;
};

} // namespace Algorithms

} // namespace wcp

#endif