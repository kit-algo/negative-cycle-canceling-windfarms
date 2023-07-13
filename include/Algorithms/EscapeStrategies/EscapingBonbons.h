#ifndef ALGORITHMS_ESCAPESTRATEGIES_ESCAPINGBONBONS
#define ALGORITHMS_ESCAPESTRATEGIES_ESCAPINGBONBONS

#include <vector>

#include "Algorithms/EscapeStrategies/EscapingStrategy.h"
#include "Algorithms/EscapingNCC.h"

#include "Auxiliary/ReturnCodes.h"

#include "DataStructures/Graphs/WindfarmResidual.h"

namespace wcp {

namespace Algorithms {

/**
 * Escaping strategy that uses unhelpful closed walks ('bonbons') from EscapingNCC
 */
struct EscapingBonbons : EscapingStrategy {
public:
    /// Constructor
    EscapingBonbons(WindfarmResidual& windfarm)
    : EscapingStrategy(windfarm, "Bonbon") {}

    ReturnCode apply(EscapingNCC& ncc); 

private:
    void SetIncomingBonbonEdgeForVertex(edgeID edgeInBonbon);

    void RecoverBonbon(EscapingNCC& ncc);

    ReturnCode SpecialTreatmentIfTailIsClosed(EscapingNCC& ncc, std::vector<edgeID>& tail, double costOfTail);

    ReturnCode CombineTailAndBonbon(EscapingNCC& ncc, std::vector<edgeID>& tail);

    ReturnCode CheckAllIncomingEdgesForTail(EscapingNCC& ncc, edgeID currentNegativeEdgeOnBonbon);

    void UpdateVectorOfBonbonEdgesForVertices(unsigned int currentIndexInBonbon);

    std::vector<edgeID> bonbon_;
    std::vector<edgeID> firstEdgeInWalkLeadingToVertex_;
};

} // namespace Algorithms

} // namespace wcp

#endif