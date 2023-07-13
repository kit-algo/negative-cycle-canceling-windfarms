#ifndef ALGORITHMS_DELTASTRATEGIES_SAMENEXTDELTASTRATEGY
#define ALGORITHMS_DELTASTRATEGIES_SAMENEXTDELTASTRATEGY

#include <memory>

#include "Algorithms/DeltaStrategies/DeltaStrategy.h"

namespace wcp {

namespace Algorithms {

/**
 * A delta strategy on top of other delta strategies.
 * SameNextDeltaStrategy modifies the other delta strategies
 * so that after a cancelation, the same value of delta
 * is chosen again, until no more cycle can be canceled with
 * this delta value. Then, a new value is chosen according
 * to the other delta strategy.
 */
class SameNextDeltaStrategy : public DeltaStrategy {
public:

    SameNextDeltaStrategy(std::unique_ptr<DeltaStrategy>&& deltaStrategy)
      : DeltaStrategy(deltaStrategy->MaximumFlowChange()),
        deltaStrategy_(std::move(deltaStrategy)),
        lastCanceled_(false)
    {}

    int FirstDelta() {
        return deltaStrategy_->FirstDelta();
    }
    
    int NextDelta(int currentDelta, bool canceledNegativeCycle) {
        if (canceledNegativeCycle) {
            lastCanceled_ = true;
            lastCanceledDelta_ = currentDelta;
            return currentDelta;
        } else {
            if (lastCanceled_) {
                lastCanceled_ = false;
                currentDelta = deltaStrategy_->NextDelta(currentDelta, true);
            } else { // lastCanceled_ == false
                currentDelta = deltaStrategy_->NextDelta(currentDelta, false);
            }
            if (currentDelta == lastCanceledDelta_) {
                return deltaStrategy_->NextDelta(currentDelta, false);
            } else {
                return currentDelta;
            }
        }
    }
    
    bool Continue(int upcomingDelta) {
        return deltaStrategy_->Continue(upcomingDelta);
    }
private:
    std::unique_ptr<DeltaStrategy>  deltaStrategy_;
    bool                            lastCanceled_; // equals canceledNegativeCycle from the last invocation of NextDelta
    int                             lastCanceledDelta_;
};

} // namespace Algorithms

} // namespace wcp

#endif
