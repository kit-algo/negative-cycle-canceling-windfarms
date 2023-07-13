#include <cassert>
#include <ostream>
#include <vector>

#include "Algorithms/DeltaStrategies/DeltaStrategy.h"
#include "Algorithms/DeltaStrategies/IncreasingDeltaStrategy.h"
#include "Algorithms/DeltaStrategies/DecreasingDeltaStrategy.h"
#include "Algorithms/DeltaStrategies/IncreasingDecreasingStrategy.h"
#include "Algorithms/DeltaStrategies/RandomDeltaStrategy.h"
#include "Algorithms/DeltaStrategies/SameNextDeltaStrategy.h"

#include "Algorithms/DetectionStrategies/NegativeCycleDetection.h"
#include "Algorithms/DetectionStrategies/BellmanFord.h"

#include "Algorithms/Distance/Dijkstra.h"
#include "Algorithms/Distance/UnitLength.h"
#include "Algorithms/Distance/EdgeLength.h"
#include "Algorithms/Distance/EuclideanLength.h"

#include "Algorithms/InitializationStrategies/Collecting.h"
#include "Algorithms/InitializationStrategies/NonCollecting.h"
#include "Algorithms/InitializationStrategies/InitializationStrategy.h"

#include "Algorithms/EscapeStrategies/KeeperOfEscapingStrategies.h"
#include "Algorithms/EscapeStrategies/EscapingBonbons.h"
#include "Algorithms/EscapeStrategies/EscapingFreeCables.h"
#include "Algorithms/EscapeStrategies/EscapingLeaves.h"

#include "Algorithms/EscapingNCC.h"
#include "Algorithms/SingleRunNCC.h"

#include "Builders/NCCControler.h"

#include "IO/NCCLogger.h"

namespace wcp {

namespace Builders {

std::unique_ptr<Algorithms::InitializationStrategy> NCCControler::ConstructInitializationStrategy(QString& initializationInputString, Algorithms::WindfarmResidual& windfarm) {
    try {
        QString init(initializationInputString);
        // It should be of form "(COLLECTING)?(DIJKSTRA|BFS)(ANY|LAST)"
        vertexID targetForInitialization;

        if (initializationInputString.endsWith("ANY")) {
            targetForInitialization = Algorithms::Dijkstra::ANY_FREE_SUBSTATION;
            init.chop(3);
        } else if (init.endsWith("LAST")) {
            targetForInitialization = Algorithms::Dijkstra::LAST_FREE_SUBSTATION;
            init.chop(4);
        } else {
            throw "Could not identify a suitable initialization strategy from your input '" + initializationInputString.toStdString() 
                    + "'. Your input must be of format '(COLLECTING)?(DIJKSTRA|BFS)(ANY|LAST)'.";
        }
        // Now, init is of form "(COLLECTING)?(DIJKSTRA|BFS)"

        // Decide which edge lengths to use
        std::unique_ptr<Algorithms::EdgeLength> edgeLengthPointer;
        if (init.endsWith("BFS")) {
            edgeLengthPointer = std::make_unique<Algorithms::UnitLength>();
            init.chop(3);
        } else if (init.endsWith("DIJKSTRA")) {
            edgeLengthPointer = std::make_unique<Algorithms::EuclideanLength>();
            init.chop(8);
        } else {
            throw "Could not identify a suitable initialization strategy from your input '" + initializationInputString.toStdString() 
                    + "'. Your input must be of format '(COLLECTING)?(DIJKSTRA|BFS)(ANY|LAST)'.";
        }
        std::unique_ptr<Algorithms::Dijkstra> dijkstraPointer = std::make_unique<Algorithms::Dijkstra>(windfarm, std::move(edgeLengthPointer));
        // Now, init is of form "(COLLECTING)?"

        if (init == "COLLECTING") {
            return std::make_unique<Algorithms::Collecting>(windfarm, std::move(dijkstraPointer), targetForInitialization);
        } else if (init.size() == 0) {
            return std::make_unique<Algorithms::NonCollecting>(windfarm, std::move(dijkstraPointer), targetForInitialization);
        } else {
            throw "Could not identify a suitable initialization strategy from your input '" + initializationInputString.toStdString() 
                    + "'. Your input must be of format '(COLLECTING)?(DIJKSTRA|BFS)(ANY|LAST)'.";
        }
    } catch (const std::string& e) {
        throw;
    }
}

std::unique_ptr<Algorithms::DeltaStrategy> NCCControler::ConstructDeltaStrategy(QString& deltaInputString, int seedInput, DataStructures::Instance& instance) {
    try {
        QString delta(deltaInputString);
        // should be of form "*(STAY)?(INC|DEC|INCDEC|RANDOM)"
        int maximumFlowChange = 2 * instance.CableTypes().MaximumCapacity();

        std::unique_ptr<Algorithms::DeltaStrategy> auxiliaryDeltaStrategy_;
        if (delta.endsWith("INC")) {
            auxiliaryDeltaStrategy_ = std::make_unique<Algorithms::IncreasingDeltaStrategy>(maximumFlowChange);
            delta.chop(3);
        } else if (delta.endsWith("DEC") && !delta.endsWith("INCDEC")) {
            auxiliaryDeltaStrategy_ = std::make_unique<Algorithms::DecreasingDeltaStrategy>(maximumFlowChange);
            delta.chop(3);
        } else if (delta.endsWith("INCDEC")) {
            auxiliaryDeltaStrategy_ = std::make_unique<Algorithms::IncreasingDecreasingStrategy>(maximumFlowChange);
            delta.chop(6);
        } else if (delta.endsWith("RANDOM")) {
            unsigned int seed = static_cast<unsigned int>( seedInput );
            auxiliaryDeltaStrategy_ = std::make_unique<Algorithms::RandomDeltaStrategy>(maximumFlowChange, seed);
            delta.chop(6);
        } else {
            throw "Could not identify a suitable delta strategy from your input '" + deltaInputString.toStdString()
                    + "'. Your input must be of the form '(STAY)?(INC|DEC|INCDEC|RANDOM)'.";
        }

        if (delta == "STAY") {
            return std::make_unique<Algorithms::SameNextDeltaStrategy>(std::move(auxiliaryDeltaStrategy_));
        } else if (delta.size() == 0) {
            return auxiliaryDeltaStrategy_;
        } else {
            throw "Could not identify a suitable delta strategy from your input '" + deltaInputString.toStdString()
                    + "'. Your input must be of the form '(STAY)?(INC|DEC|INCDEC|RANDOM)'.";
        }
    } catch (const std::string& e) {
        throw;
    }
}

std::unique_ptr<Algorithms::NegativeCycleDetection> NCCControler::ConstructDetectionStrategy(QString& detectionInputString, Algorithms::WindfarmResidual& windfarm) {
    try {
        if (detectionInputString == "STFBF") {
            return std::make_unique<Algorithms::SideTripFreeBellmanFord>(windfarm);
        } else if (detectionInputString == "BF") {
            return std::make_unique<Algorithms::StandardBellmanFord>(windfarm);
        } else {
            throw "Could not identify a suitable detection strategy from your input '" + detectionInputString.toStdString()
                    + "'. Your input must be 'BF' or 'STFBF'.";
        }
    } catch (const std::string& e) {
        throw;
    }
}

std::unique_ptr<Algorithms::KeeperOfEscapingStrategies> NCCControler::ConstructEscapingStrategies(DataStructures::InputInformation& inputInformation, Algorithms::WindfarmResidual& windfarm) {
    std::unique_ptr<Algorithms::KeeperOfEscapingStrategies> keeperOfEscapingStrategies = std::make_unique<Algorithms::KeeperOfEscapingStrategies>();

    if (inputInformation.weightEscapeLeaves_ > 0) {
        keeperOfEscapingStrategies->AddEscapingStrategy<Algorithms::EscapingLeaves>(windfarm, inputInformation.weightEscapeLeaves_);
    }

    if (inputInformation.weightEscapeBonbon_ > 0) {
        keeperOfEscapingStrategies->AddEscapingStrategy<Algorithms::EscapingBonbons>(windfarm, inputInformation.weightEscapeBonbon_);
    }

    if (inputInformation.weightEscapeFreeCables_ > 0) {
        keeperOfEscapingStrategies->AddEscapingStrategy<Algorithms::EscapingFreeCables>(windfarm, inputInformation.weightEscapeFreeCables_);
    }

    return keeperOfEscapingStrategies;
}

double NCCControler::DetermineAndOutputTimeLimit(DataStructures::InputInformation& inputInformation, const DataStructures::OutputInformation& outputInformation) {
    double timeLimitInMilliseconds;
    if (inputInformation.isTimeLimitSet_) {
        timeLimitInMilliseconds = inputInformation.timeLimitInSeconds_ * 1000;
    } else {
        timeLimitInMilliseconds = Auxiliary::DINFTY;
    }

    if (outputInformation.ShouldBeVerbose()) {
        outputInformation.VerboseStream() << "The time limit in milliseconds is set to ";
        if (inputInformation.isTimeLimitSet_) {
            outputInformation.VerboseStream() << timeLimitInMilliseconds;
        } else {
            outputInformation.VerboseStream() << "infinity";
        }
        outputInformation.VerboseStream() << ".\n";
    }
    return timeLimitInMilliseconds;
}

void NCCControler::OutputIterationLimit(DataStructures::InputInformation& inputInformation, const DataStructures::OutputInformation& outputInformation) {
    if (outputInformation.ShouldBeVerbose()) {
        outputInformation.VerboseStream() << "The maximum number of iterations is set to ";
        if (inputInformation.isIterationLimitSet_) {
            outputInformation.VerboseStream() << inputInformation.iterationLimit_;
        } else {
            outputInformation.VerboseStream() << "infinity";
        }  
        outputInformation.VerboseStream() << ".\n";
    }
}

void NCCControler::BuildAndRunAndOutput(DataStructures::Instance& instance,
                                        DataStructures::InputInformation& inputInformation,
                                        const DataStructures::OutputInformation& outputInformation) {
    assert(inputInformation.algorithm_.endsWith("NCC"));

    Algorithms::WindfarmResidual windfarm(instance);

    double timeLimitInMilliseconds = DetermineAndOutputTimeLimit(inputInformation, outputInformation);

    IO::NCCLogger logger;

    std::unique_ptr<Algorithms::InitializationStrategy> initializationStrategy = ConstructInitializationStrategy(inputInformation.initialisationStrategy_, windfarm);
    std::unique_ptr<Algorithms::DeltaStrategy> deltaStrategy = ConstructDeltaStrategy(inputInformation.deltaStrategy_, inputInformation.randomSeed_, instance);
    std::unique_ptr<Algorithms::NegativeCycleDetection> detectionStrategy = ConstructDetectionStrategy(inputInformation.detectionStrategy_, windfarm);
    if (inputInformation.algorithm_.startsWith("ESCAPING")) {
        OutputIterationLimit(inputInformation, outputInformation);

        std::unique_ptr<Algorithms::KeeperOfEscapingStrategies> keeperOfEscapingStrategies = ConstructEscapingStrategies(inputInformation, windfarm);

        if (keeperOfEscapingStrategies->AtLeastOneEscapingStrategy()) {
            if (outputInformation.ShouldBeVerbose()) {
                keeperOfEscapingStrategies->OutputInfoOnEscapingStrategies(outputInformation.VerboseStream());
            }
            Algorithms::EscapingNCC negativeCycleCanceling( instance,
                                                            windfarm,
                                                            logger,
                                                            std::move(initializationStrategy),
                                                            std::move(deltaStrategy),
                                                            std::move(detectionStrategy),
                                                            inputInformation.randomSeed_,
                                                            std::move(keeperOfEscapingStrategies),
                                                            inputInformation.iterationLimit_,
                                                            outputInformation.ShouldBeVerbose(),
                                                            outputInformation.VerboseStream(),
                                                            timeLimitInMilliseconds);
        } else {
            throw std::string("No escaping strategies found for Negative Cycle Canceling with Escaping!");
        }
    } else {
        Algorithms::SingleRunNCC negativeCycleCanceling(instance,
                                                        windfarm,
                                                        logger,
                                                        std::move(initializationStrategy),
                                                        std::move(deltaStrategy),
                                                        std::move(detectionStrategy),
                                                        inputInformation.randomSeed_,
                                                        outputInformation.ShouldBeVerbose(),
                                                        outputInformation.VerboseStream(),
                                                        timeLimitInMilliseconds);
    }

    logger.CompleteOutput(instance, inputInformation, outputInformation, windfarm, "NCC");
}


void NCCControler::AddMetaToSummary(IO::NegativeCycleSummaryRow& summaryRow,
                                    DataStructures::Instance& instance,
                                    DataStructures::InputInformation& inputInformation) {
    summaryRow.Name = algorithmName_
                        + "-" + longTestsetInstanceString(instance)
                        + "-" + inputInformation.initialisationStrategy_.toStdString()
                        + "-" + inputInformation.deltaStrategy_.toStdString()
                        + "-" + inputInformation.detectionStrategy_.toStdString();
    summaryRow.Testset = instance.TestsetNumber();
    summaryRow.Instance = instance.InstanceNumber();
    summaryRow.InitializationStrategy = inputInformation.initialisationStrategy_.toStdString();
    summaryRow.DeltaStrategy = inputInformation.deltaStrategy_.toStdString();
    summaryRow.DetectionStrategy = inputInformation.detectionStrategy_.toStdString();
    summaryRow.NumberOfTurbines = instance.NumberOfTurbines();
    summaryRow.NumberOfSubstations = instance.NumberOfSubstations();
    summaryRow.NumberOfEdges = instance.NumberOfEdges();
}

} // namespace Builders

} // namespace wcp
