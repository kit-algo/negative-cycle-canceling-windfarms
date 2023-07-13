#ifndef IO_DUMMYNCCLOGGER
#define IO_DUMMYNCCLOGGER

namespace wcp {

namespace IO {

/**
 * A do-nothing logger struct used in a template in NegativeCycleCanceling.
 *
 * We use this struct to perform an iteration of NCC in which no logging takes place.
 */
struct DummyNCCLogger {
    void CycleHasBeenFound(const std::string& /*currentProcedure*/,
                            int /*currentDelta*/,
                            size_t /*walkID*/,
                            double /*costOfCycle*/,
                            size_t /*lengthOfCycle*/,
                            double /*elapsedMilliseconds*/,
                            double /*solutionValueBeforeCanceling*/,
                            bool /*cycleWillBeCanceled*/) {}
    
};

} // namespace IO

} // namespace wcp

#endif
