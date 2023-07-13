#ifndef ALGORITHMS_DELTASTRATEGIES_RANDOMDELTASTRATEGY
#define ALGORITHMS_DELTASTRATEGIES_RANDOMDELTASTRATEGY

#include "Algorithms/DeltaStrategies/DeltaStrategy.h"

#include <algorithm>
#include <cassert>
#include <random>

namespace wcp {

namespace Algorithms {

/**
 * A delta strategy that randomly and uniformly picks new deltas
 * from the set of deltas that have not been tried since the last
 * time a negative cycle was canceled.
 */
struct RandomDeltaStrategy : DeltaStrategy {
    RandomDeltaStrategy(int maximumFlowChange, unsigned int randomSeed)
      : DeltaStrategy(maximumFlowChange),
        allDeltasRandomized_(maximumFlowChange),
        numberOfDeltasSinceUpdate_(0),
        rng_(randomSeed)
    {
        for (int delta = 1; delta <= maximumFlowChange; ++delta) {
            allDeltasRandomized_[delta - 1] = delta;
        }
    }

    int FirstDelta() {
        assert(numberOfDeltasSinceUpdate_ == 0);
        ShuffleVector();
        numberOfDeltasSinceUpdate_++;
        return allDeltasRandomized_.front();
    }

    int NextDelta(int /* currentDelta */, bool canceledNegativeCycle) {
        if (canceledNegativeCycle) {
            ShuffleVector();
            numberOfDeltasSinceUpdate_ = 1;
            return allDeltasRandomized_.front();
        } else {
            if (Continue()) {
                int delta = allDeltasRandomized_[numberOfDeltasSinceUpdate_];
                ++numberOfDeltasSinceUpdate_;
                return delta;
            } else {
                // We considered all deltas
                return -1;
            }
        }
    }

    bool Continue(int /* upcomingDelta */) {
        return Continue();
    }

private:
    std::vector<int> allDeltasRandomized_;
    unsigned int numberOfDeltasSinceUpdate_;
    std::mt19937 rng_;

    bool Continue() {
        return numberOfDeltasSinceUpdate_ <= allDeltasRandomized_.size();
    }

    void ShuffleVector() {
        std::shuffle(allDeltasRandomized_.begin(), allDeltasRandomized_.end(), rng_);
    }
};

} // namespace Algorithms

} // namespace wcp

#endif
