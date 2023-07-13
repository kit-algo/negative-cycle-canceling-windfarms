#ifndef IO_NCCLOGGER
#define IO_NCCLOGGER

#include "Auxiliary/Constants.h"
#include "Auxiliary/ReturnCodes.h"

#include "DataStructures/InputInformation.h"
#include "DataStructures/Instance.h"
#include "DataStructures/OutputInformation.h"

#include "IO/DummyNCCLogger.h"
#include "IO/NegativeCycleRuntimeCollection.h"
#include "IO/NegativeCycleRuntimeRow.h"
#include "IO/NegativeCycleSummaryRow.h"

namespace wcp {

namespace IO {

/**
 * The struct to manage the bookkeeping in NegativeCycleCanceling.
 *
 * It coordinates the NegativeCycleRuntimeCollection, -RuntimeRow and -SummaryRow.
 */
struct NCCLogger : public DummyNCCLogger {

public:
    void AddMeta(DataStructures::Instance& instance, DataStructures::InputInformation& inputInformation, const std::string& algorithmName) {
        AddMetaToSummary(instance, inputInformation, algorithmName);
    }

    void AddMetaToSummary(DataStructures::Instance& instance, DataStructures::InputInformation& inputInformation, const std::string& algorithmName) {
        summaryRow_.Name = algorithmName
                    + "-" + instance.longTestsetInstanceString()
                    + "-" + inputInformation.initialisationStrategy_.toStdString()
                    + "-" + inputInformation.deltaStrategy_.toStdString()
                    + "-" + inputInformation.detectionStrategy_.toStdString();
        summaryRow_.Testset = instance.TestsetNumber();
        summaryRow_.Instance = instance.InstanceNumber();
        summaryRow_.InitializationStrategy = inputInformation.initialisationStrategy_.toStdString();
        summaryRow_.DeltaStrategy = inputInformation.deltaStrategy_.toStdString();
        summaryRow_.DetectionStrategy = inputInformation.detectionStrategy_.toStdString();
        summaryRow_.NumberOfTurbines = instance.NumberOfTurbines();
        summaryRow_.NumberOfSubstations = instance.NumberOfSubstations();
        summaryRow_.NumberOfEdges = instance.NumberOfEdges();
    }

    void CompleteOutput(DataStructures::Instance& instance,
                        DataStructures::InputInformation& inputInformation,
                        const DataStructures::OutputInformation& outputInformation,
                        Algorithms::WindfarmResidual& windfarm,
                        const std::string& algorithmName) {
        if (outputInformation.ShouldPrintSummary() || outputInformation.ShouldPrintDetails() || outputInformation.ShouldBeVerbose()) {
            // ShouldPrintDetails() as well, since we copy metadata from summaryRow_ to runtimeCollection_
            AddMetaToSummary(instance, inputInformation, algorithmName);
        }

        if (outputInformation.ShouldPrintSummary()) {
            summaryRow_.Dump(outputInformation.OutputPath() + "WCP-" + algorithmName + "-general.csv", outputInformation.ShouldBeVerbose(), outputInformation.VerboseStream(), false);
        }

        if (outputInformation.ShouldPrintSolution()) {
            windfarm.writeFlowToOGDF(instance.Graph(), instance.GraphAttributes());
            outputGraphAttributesToGRAPHML(instance.GraphAttributes(),
                                        outputInformation.OutputPath()
                                        + "WCP" 
                                        + "-" + algorithmName
                                        + "-" + instance.testsetInstanceString()
                                        + "-" + inputInformation.initialisationStrategy_.toStdString()
                                        + "-" + inputInformation.deltaStrategy_.toStdString()
                                        + "-" + inputInformation.detectionStrategy_.toStdString()
                                        + ".graphml",
                                        outputInformation.ShouldBeVerbose(),
                                        outputInformation.VerboseStream());
        }

        if (outputInformation.ShouldPrintDetails()) {
            runtimeCollection_.PrepareAndCompleteDumpToFile(outputInformation.OutputPath()
                    + "WCP"
                    + "-" + algorithmName
                    + "-" + instance.testsetInstanceString()
                    + "-" + inputInformation.initialisationStrategy_.toStdString()
                    + "-" + inputInformation.deltaStrategy_.toStdString()
                    + "-" + inputInformation.detectionStrategy_.toStdString()
                    + ".csv",
                    summaryRow_,
                    outputInformation.ShouldBeVerbose(),
                    outputInformation.VerboseStream());
        }

        if (outputInformation.ShouldBeVerbose()) {
            summaryRow_.Dump(outputInformation.VerboseStream());
        }
    }

    void SaveRuntimeDataAfterHalfIteration(const std::string& procedure, double elapsedMilliseconds, double newSolutionValue, double changeInCost, ReturnCode code) {
        RuntimeRow().CurrentProcedure = procedure;
        RuntimeRow().WalkId = 0;
        RuntimeRow().Solution = newSolutionValue;
        RuntimeRow().ChangeInCost = changeInCost;
        RuntimeRow().GlobalElapsedMilliseconds = elapsedMilliseconds;
        RuntimeRow().NumberOfEdgesPerCycle = 0;
        RuntimeRow().Delta = (unsigned int) code;
        // Delta normally refers to the value by how much flow is changed in NCC. Here, it saves the status of the half iteration.

        StoreRuntimeRowWithReset(false);
    }

    void SaveRuntimeDataAfterHalfIteration(const std::string& procedure, double newSolutionValue, double changeInCost, ReturnCode code) {
        RuntimeRow().CurrentProcedure = procedure;
        RuntimeRow().WalkId = 0;
        RuntimeRow().Solution = newSolutionValue;
        RuntimeRow().ChangeInCost = changeInCost;
        RuntimeRow().GlobalElapsedMilliseconds = SummaryRow().GlobalElapsedMilliseconds;
        RuntimeRow().NumberOfEdgesPerCycle = 0;
        RuntimeRow().Delta = (unsigned int) code;
        // Delta normally refers to the value by how much flow is changed in NCC.  Here, it saves the status of the half iteration.

        StoreRuntimeRowWithReset(false);
    }

    void CompleteSummaryWithInformationFromAlgorithmRun(double bestSolutionValue, ReturnCode code) {
        SummaryRow().GlobalElapsedMilliseconds = RuntimeRow().GlobalElapsedMilliseconds;
        SummaryRow().GlobalElapsedMillisecondsNegativeCycleDeletion = SummaryRow().GlobalElapsedMilliseconds - SummaryRow().GlobalElapsedMillisecondsInitialFlowTime;
        SummaryRow().BestSolution = bestSolutionValue;

        SummaryRow().Status = code;
    }

    void CompleteSummaryWithInformationFromAlgorithmRun(double elapsedMilliseconds, double bestSolutionValue, ReturnCode code) {
        SummaryRow().GlobalElapsedMilliseconds = elapsedMilliseconds;
        SummaryRow().GlobalElapsedMillisecondsNegativeCycleDeletion = SummaryRow().GlobalElapsedMilliseconds - SummaryRow().GlobalElapsedMillisecondsInitialFlowTime;
        SummaryRow().BestSolution = bestSolutionValue;

        SummaryRow().Status = code;
    }

    void FillSummaryWithInitializationInformation(double elapsedMilliseconds, double solutionValue, ReturnCode code) {
        assert(code == ReturnCode::SUCCESS || solutionValue > Auxiliary::INFINITE_COST / 2.0);

        SummaryRow().InitialSolution = solutionValue;
        SummaryRow().GlobalElapsedMillisecondsInitialFlowTime = elapsedMilliseconds;

        if (code != ReturnCode::SUCCESS) {
            SummaryRow().GlobalElapsedMillisecondsNegativeCycleDeletion = 0;
            SummaryRow().GlobalElapsedMilliseconds = elapsedMilliseconds;
            SummaryRow().Status = code;
        }
    }

    void CycleHasBeenFound(const std::string& currentProcedure, int currentDelta, size_t walkID, double costOfCycle, size_t lengthOfCycle, double elapsedMilliseconds, double solutionValueBeforeCanceling, bool cycleWillBeCanceled) {
        RuntimeRow().ChangeInCost = costOfCycle;
        RuntimeRow().Delta = currentDelta;
        RuntimeRow().GlobalElapsedMilliseconds = elapsedMilliseconds;
        RuntimeRow().NumberOfEdgesPerCycle = lengthOfCycle;
        RuntimeRow().Solution = solutionValueBeforeCanceling;
        RuntimeRow().WalkId = walkID;
        RuntimeRow().CurrentProcedure = currentProcedure;

        SummaryRow().NumberOfEdgesSimpleCycles += lengthOfCycle;
        SummaryRow().NumberOfSimpleCycles++;

        if (cycleWillBeCanceled) {
            SummaryRow().NumberOfNegativeCycles++;
            SummaryRow().NumberOfEdgesOnCanceledCycles += lengthOfCycle;
            RuntimeRow().Solution = solutionValueBeforeCanceling + costOfCycle;
        }

        StoreRuntimeRowWithReset(false);
    }

    void AnotherNegativeCycleDetectionRunHasStarted() {
        SummaryRow().NumberOfDetectionRuns++;
    }

    void AnotherNumberOfEdgesHaveBeenRelaxed(size_t newlyRelaxedEdges) {
        SummaryRow().NumberOfRelaxedEdges += newlyRelaxedEdges;
    }

    int NumberOfDetectionRuns() { return SummaryRow().NumberOfDetectionRuns; }

    IO::NegativeCycleRuntimeCollection& RuntimeCollection() { return runtimeCollection_; }
    IO::NegativeCycleRuntimeRow& RuntimeRow() { return runtimeRow_; }
    IO::NegativeCycleSummaryRow& SummaryRow() { return summaryRow_; }

private:
    IO::NegativeCycleRuntimeCollection runtimeCollection_;
    IO::NegativeCycleRuntimeRow runtimeRow_;
    IO::NegativeCycleSummaryRow summaryRow_;

    void StoreRuntimeRowWithReset(bool reset = true) {
        RuntimeCollection() += RuntimeRow();
        if (reset) { RuntimeRow() = NegativeCycleRuntimeRow(); }
    }
    
};

} // namespace IO

} // namespace wcp

#endif
