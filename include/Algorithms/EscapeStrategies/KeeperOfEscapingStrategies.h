#ifndef ALGORITHMS_ESCAPESTRATEGIES_KEEPEROFESCAPINGSTRATEGIES
#define ALGORITHMS_ESCAPESTRATEGIES_KEEPEROFESCAPINGSTRATEGIES

#include <cassert>
#include <limits>
#include <memory>
#include <random>
#include <vector>

#include "Algorithms/EscapeStrategies/EscapingStrategy.h"

#include "DataStructures/Graphs/WindfarmResidual.h"

namespace wcp {

namespace Algorithms {

class EscapingNCC;

/**
 * A class to store escaping strategies and to pick them during EscapingNCC.
 */
class KeeperOfEscapingStrategies {
public:
    template<typename Strategy>
    void AddEscapingStrategy(DataStructures::WindfarmResidual& windfarm, unsigned int weightOfNewStrategy) {
        vectorOfEscapingStrategiesWithWeights.push_back(std::make_pair(std::make_unique<Strategy>(windfarm),weightOfNewStrategy));
        fieldsUpdatedSinceLastAdd = false;
    }

    void OutputInfoOnEscapingStrategies(std::ostream& os) {
        for (auto&& strategyWithWeight : vectorOfEscapingStrategiesWithWeights) {
            os << "Escaping strategy " << strategyWithWeight.first->Name() << " has weight " << strategyWithWeight.second << ".\n";
        }
    }

    bool AtLeastOneEscapingStrategy() { return !vectorOfEscapingStrategiesWithWeights.empty(); }

    void DrawNextEscapingStrategy() {
        indexOfCurrentEscapingStrategy_ = distributionForPickingEscapingStrategies_(rng_);
    }

    const std::string& NameOfCurrentStrategy() {
        assert(indexOfCurrentEscapingStrategy_ < vectorOfEscapingStrategiesWithWeights.size());
        return vectorOfEscapingStrategiesWithWeights[indexOfCurrentEscapingStrategy_].first->Name();
    }

    void InitializeRNGandTracking(unsigned int& iterationCounter, int seedInput) {
        currentIteration_ = &iterationCounter;
        iterationOfLastUse_ = std::vector<unsigned int>(vectorOfEscapingStrategiesWithWeights.size(),0);
        fieldsUpdatedSinceLastAdd = true;

        assert(CheckConsistency());

        rng_.seed(seedInput);
        std::vector<unsigned int> weightsOfStrategies = CopyVectorOfWeights();
        distributionForPickingEscapingStrategies_ = std::discrete_distribution<unsigned int>(weightsOfStrategies.begin(),weightsOfStrategies.end());
    }

    ReturnCode ApplyCurrentStrategy(EscapingNCC& escapingNCC) {
        return vectorOfEscapingStrategiesWithWeights[indexOfCurrentEscapingStrategy_].first->apply(escapingNCC);
    }

    bool CheckConsistency() {
        return std::count_if(iterationOfLastUse_.begin(), iterationOfLastUse_.end(), [&](unsigned int iteration) { return iteration > iterationOfLastChange_; }) == numberOfStrategiesUsedUnsuccessfully_;
    }

    bool HasCurrentStrategyBeenUsedInCurrentIteration() {
        assert(IsValidIndex(indexOfCurrentEscapingStrategy_));
        return iterationOfLastUse_[indexOfCurrentEscapingStrategy_] > iterationOfLastChange_;
    }

    void SetCurrentStrategyAsUsedSuccessfully() {
        assert(IsValidIndex(indexOfCurrentEscapingStrategy_));
        iterationOfLastUse_[indexOfCurrentEscapingStrategy_] = *currentIteration_;
        iterationOfLastChange_ = *currentIteration_;
        numberOfStrategiesUsedUnsuccessfully_ = 0;
    }

    void SetCurrentStrategyAsUsedUnsuccessfully() {
        assert(IsValidIndex(indexOfCurrentEscapingStrategy_));
        iterationOfLastUse_[indexOfCurrentEscapingStrategy_] = *currentIteration_;
        numberOfStrategiesUsedUnsuccessfully_++;
    }

    bool HaveAllStrategiesBeenUsedInCurrentIteration() { return numberOfStrategiesUsedUnsuccessfully_ == iterationOfLastUse_.size(); }

private:
    std::vector<std::pair<std::unique_ptr<EscapingStrategy>, unsigned int>> vectorOfEscapingStrategiesWithWeights;

    unsigned int* currentIteration_;
    std::vector<unsigned int> iterationOfLastUse_;
    unsigned int numberOfStrategiesUsedUnsuccessfully_ = 0;
    unsigned int iterationOfLastChange_ = 0;

    unsigned int indexOfCurrentEscapingStrategy_ = std::numeric_limits<unsigned int>::max();

    bool fieldsUpdatedSinceLastAdd = true;

    std::mt19937 rng_;
    std::discrete_distribution<unsigned int> distributionForPickingEscapingStrategies_;

    bool IsValidIndex(unsigned int index) {
        return index < iterationOfLastUse_.size();
    }

    std::vector<unsigned int> CopyVectorOfWeights() {
        std::vector<unsigned int> vectorOfWeights;
        for (size_t i = 0; i < vectorOfEscapingStrategiesWithWeights.size(); i++) {
            vectorOfWeights.push_back(vectorOfEscapingStrategiesWithWeights[i].second);
        }
        return vectorOfWeights;
    }
};

} // namespace Algorithms

} // namespace wcp

#endif
