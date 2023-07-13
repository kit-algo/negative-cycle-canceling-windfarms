#ifndef ALGORITHMS_ESCAPESTRATEGIES_ESCAPINGSTRATEGY
#define ALGORITHMS_ESCAPESTRATEGIES_ESCAPINGSTRATEGY

#include <iostream>

#include "Auxiliary/ReturnCodes.h"

#include "DataStructures/Graphs/WindfarmResidual.h"

namespace wcp {

namespace Algorithms {

using DataStructures::WindfarmResidual;

class EscapingNCC;

/**
 * The base class for the escaping strategies.
 */
struct EscapingStrategy {
public:
    EscapingStrategy(WindfarmResidual& windfarm, std::string name)
    : numberOfCalls_(0),
      windfarm_(windfarm),
      name_(name) {}

    /**
     * Carries out the procedure to escape a local minimum.
     *
     * It is assumed that escaping takes place after the standard NCC algorithm
     * is not able to find and cancel more negative cycles.
     */
    virtual ReturnCode apply(EscapingNCC& ncc) = 0;

    unsigned int numberOfCalls_; // A counter for the number of times a specific strategy is invoked.

    WindfarmResidual& Windfarm() { return windfarm_; }

    const std::string& Name() { return name_; }

    virtual ~EscapingStrategy() = default;

protected:
    void CommonStartForAllEscapingStrategies(bool beVerbose, std::ostream& verboseStream = std::cout) {
        if (beVerbose) { verboseStream << "Attempt Escaping Strategy " << Name() << '\n'; }
        numberOfCalls_++;
    }

private:
    WindfarmResidual& windfarm_;

    const std::string name_;
};

} // namespace Algorithms

} // namespace wcp

#endif