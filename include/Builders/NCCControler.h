#ifndef BUILDERS_NCCCONTROLER
#define BUILDERS_NCCCONTROLER

#include <memory>
#include <QString>

#include "Algorithms/EscapeStrategies/EscapingStrategy.h"
#include "Algorithms/EscapeStrategies/KeeperOfEscapingStrategies.h"
#include "Algorithms/InitializationStrategies/InitializationStrategy.h"
#include "Algorithms/DeltaStrategies/DeltaStrategy.h"
#include "Algorithms/DetectionStrategies/NegativeCycleDetection.h"

#include "Builders/AlgorithmControler.h"

#include "DataStructures/InputInformation.h"
#include "DataStructures/Instance.h"
#include "DataStructures/OutputInformation.h"
#include "DataStructures/Graphs/WindfarmResidual.h"

#include "IO/NegativeCycleRuntimeCollection.h"
#include "IO/NegativeCycleSummaryRow.h"

namespace wcp {

namespace Builders {

/**
 * A builder class for all kinds of Negative Cycle Canceling.
 */
class NCCControler : public AlgorithmControler {
public:
    NCCControler(const std::string& algorithmName) 
      : AlgorithmControler(algorithmName) {};

    std::unique_ptr<Algorithms::InitializationStrategy> ConstructInitializationStrategy(QString& initializationInputString, Algorithms::WindfarmResidual& windfarm);

    std::unique_ptr<Algorithms::DeltaStrategy> ConstructDeltaStrategy(QString& deltaInputString, int seedInput, DataStructures::Instance& instance);

    std::unique_ptr<Algorithms::NegativeCycleDetection> ConstructDetectionStrategy(QString& detectionInputString, Algorithms::WindfarmResidual& windfarm);

    std::unique_ptr<Algorithms::KeeperOfEscapingStrategies> ConstructEscapingStrategies(DataStructures::InputInformation& inputInformation, Algorithms::WindfarmResidual& windfarm);

    double DetermineAndOutputTimeLimit(DataStructures::InputInformation& inputInformation, const DataStructures::OutputInformation& outputInformation);

    void OutputIterationLimit(DataStructures::InputInformation& inputInformation, const DataStructures::OutputInformation& outputInformation);

    void BuildAndRunAndOutput(  DataStructures::Instance& instance,
                                DataStructures::InputInformation& inputInformation,
                                const DataStructures::OutputInformation& outputInformation);

    void AddMetaToSummary(  IO::NegativeCycleSummaryRow& summaryRow,
                            DataStructures::Instance& instance,
                            DataStructures::InputInformation& inputInformation);
};

} // namespace Builders

} // namespace wcp

#endif
