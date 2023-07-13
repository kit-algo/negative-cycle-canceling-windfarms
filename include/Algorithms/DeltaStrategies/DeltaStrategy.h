#ifndef ALGORITHMS_DELTASTRATEGIES_DELTASTRATEGY
#define ALGORITHMS_DELTASTRATEGIES_DELTASTRATEGY

namespace wcp {

namespace Algorithms {

/**
 * The base class for all delta strategies.
 */
class DeltaStrategy {
public:
    DeltaStrategy(int maximumFlowChange)
    : maximumFlowChange_(maximumFlowChange) {}

    /**
     * Returns the delta used for the first iteration of Negative Cycle Canceling.
     */
    virtual int FirstDelta() = 0;

    /**
     * Returns the next value of delta based on the delta 'currentDelta' from the
     * last iteration of Negative Cycle Canceling and based on whether a negative
     * cycle was canceled in that iteration. Returns a negative value if no
     * further iteration of NCC should take place after all deltas have been tried.
     */
    virtual int NextDelta(int currentDelta, bool canceledNegativeCycle) = 0;

    /**
     * Returns whether to continue canceling negative cycles.
     */
    virtual bool Continue(int upcomingDelta) = 0;

    /**
     * Returns the highest possible delta.
     */
    int MaximumFlowChange() { return maximumFlowChange_; }

    virtual ~DeltaStrategy() = default;

private:
    /**
     * The highest possible delta to be defined in the subclasses.
     */
    int maximumFlowChange_;
};

} // namespace Algorithms

} // namespace wcp

#endif
