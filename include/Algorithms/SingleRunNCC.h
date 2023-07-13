#ifndef ALGORITHMS_SINGLERUNNCC
#define ALGORITHMS_SINGLERUNNCC

#include <memory>
#include <ostream>
#include <utility>

#include "Algorithms/DeltaStrategies/DeltaStrategy.h"
#include "Algorithms/DetectionStrategies/NegativeCycleDetection.h"
#include "Algorithms/InitializationStrategies/InitializationStrategy.h"
#include "Algorithms/ResidualCosts/StandardResidualCostComputer.h"
#include "Algorithms/NegativeCycleCanceling.h"

#include "Auxiliary/ReturnCodes.h"

#include "DataStructures/Graphs/WindfarmResidual.h"
#include "DataStructures/Instance.h"

#include "IO/NCCLogger.h"

namespace wcp {

namespace Algorithms {

/**
 * Class that runs the hill-climbing version of
 * the negative cycle canceling algorithm on wind farms,
 * which corresponds to doi:10.4230/LIPIcs.ESA.2019.55.
 */
class SingleRunNCC : public NegativeCycleCanceling {
public:
    SingleRunNCC( DataStructures::Instance& instance,
                  WindfarmResidual& windfarm,
                  IO::NCCLogger& logger,
                  std::unique_ptr<Algorithms::InitializationStrategy>&& initializationStrategy,
                  std::unique_ptr<Algorithms::DeltaStrategy>&& deltaStrategy,
                  std::unique_ptr<Algorithms::NegativeCycleDetection>&& detectionStrategy,
                  int seedInput,
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
        residualCostComputer_(windfarm)
    {
        RunCompleteAlgorithm();
    }

private:
    StandardResidualCostComputer& ResidualCostComputer() { return residualCostComputer_; }

    StandardResidualCostComputer residualCostComputer_;

    ReturnCode RunCompleteAlgorithm() {
        ReturnCode stateOfAlgorithm = ComputeInitialFlow();
        if (stateOfAlgorithm == ReturnCode::SUCCESS) {
            stateOfAlgorithm = RunNCCforAllDeltas();
        }
        return stateOfAlgorithm;
    }
};

} // namespace Algorithms

} // namespace wcp

#endif