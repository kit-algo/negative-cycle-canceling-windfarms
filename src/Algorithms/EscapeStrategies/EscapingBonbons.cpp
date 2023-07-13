#include "Algorithms/EscapeStrategies/EscapingBonbons.h"

#include "Algorithms/DetectionStrategies/NegativeCycleDetection.h"
#include "Algorithms/DetectionStrategies/BellmanFord.h"

#include "IO/Auxiliary.h"

namespace wcp {

namespace Algorithms {

void EscapingBonbons::SetIncomingBonbonEdgeForVertex(edgeID edgeInBonbon) {
    if (firstEdgeInWalkLeadingToVertex_[Windfarm().EndVertex(edgeInBonbon)] == INVALID_EDGE) {
        firstEdgeInWalkLeadingToVertex_[Windfarm().EndVertex(edgeInBonbon)] = edgeInBonbon;
    }
}

void EscapingBonbons::RecoverBonbon(EscapingNCC& ncc) {
    assert(ncc.bonbon_.changeOfFlow != -1);
    ncc.ComputeResidualCosts(ncc.bonbon_.changeOfFlow);

#ifndef NDEBUG
    ReturnCode detectionCode = ncc.DetectionStrategy().Run();
    assert(detectionCode == ReturnCode::SUCCESS);
#else
    (void) ncc.DetectionStrategy().Run();
#endif
    bonbon_ = ncc.DetectionStrategy().TraverseParentsUntilParticularEdgeRepeats(ncc.bonbon_.edgeOnBonbon);

    assert(bonbon_.size() > 1);
    assert(Windfarm().EndVertex(bonbon_.front()) == Windfarm().StartVertex(bonbon_.back()));
    assert(ncc.CostOfWalk(bonbon_.begin(), bonbon_.end()) < 0);

    if (ncc.beVerbose_) {
        ncc.verboseStream_ << "\tIdentified Bonbon for escaping:\n";
        ncc.verboseStream_ << "\t\t(edgeID, residual cost)";
        for (auto i : bonbon_) {
            ncc.verboseStream_ << " ("<< i << ", " << Windfarm().ResidualCostOnEdge(i) << ')';
        }
        ncc.verboseStream_ << '\n';
    }
}

ReturnCode EscapingBonbons::SpecialTreatmentIfTailIsClosed(EscapingNCC& ncc, std::vector<edgeID>& tail, double costOfTail) {
    if (Windfarm().EndVertex(tail.front()) == Windfarm().StartVertex(tail.back())) {
        if (costOfTail < 0) {
            IO::outputVectorToStream(  "\tFound a closed cycle in the tail : ", tail,
                                            "\tCancel the cycle...",
                                            true, ncc.beVerbose_, ncc.verboseStream_);
            Windfarm().AddFlowOnMultipleEdges(tail, ncc.bonbon_.changeOfFlow);
            AssertFeasibleFlow(Windfarm());
            return ReturnCode::SUCCESS;
        } else {
            return ReturnCode::NOTHING_CHANGED;
        }
    }
    return ReturnCode::IN_PROGRESS;
}

ReturnCode EscapingBonbons::CombineTailAndBonbon(EscapingNCC& ncc, std::vector<edgeID>& tail) {
    double costOfTail = ncc.CostOfWalk(tail.begin(), tail.end());

    ReturnCode resultOfCheckOnClosedWalk = SpecialTreatmentIfTailIsClosed(ncc, tail, costOfTail);
    if (resultOfCheckOnClosedWalk != ReturnCode::IN_PROGRESS) { return resultOfCheckOnClosedWalk; }

    double cumulativeCostWhileTraversing = costOfTail;
    size_t numberOfEdgesOnWalkToBeCancelled = tail.size();
    edgeID currentEdge = tail.back();

    std::vector<edgeID> partOfBonbon;

    while (Windfarm().StartVertex(currentEdge) != Windfarm().EndVertex(tail.front())) {
        currentEdge = firstEdgeInWalkLeadingToVertex_[Windfarm().StartVertex(currentEdge)];
        cumulativeCostWhileTraversing += Windfarm().ResidualCostOnEdge(currentEdge);
        numberOfEdgesOnWalkToBeCancelled++;
        partOfBonbon.push_back(currentEdge);
    }
    

    if (cumulativeCostWhileTraversing < 0 && numberOfEdgesOnWalkToBeCancelled > 2) {
        IO::outputVectorToStream(  "\tA suitable tail has been found: ", tail,
                                        "", false, ncc.beVerbose_, ncc.verboseStream_);
        IO::outputVectorToStream(  "\tBonbon edges that complement tail: ", partOfBonbon,
                                        "\tTail and bonbon combined form a negative cycle. Cancel the cycle...",
                                        true, ncc.beVerbose_, ncc.verboseStream_);
        Windfarm().AddFlowOnMultipleEdges(tail, ncc.bonbon_.changeOfFlow);
        Windfarm().AddFlowOnMultipleEdges(partOfBonbon, ncc.bonbon_.changeOfFlow);
        AssertFeasibleFlow(Windfarm());
        ncc.ClearListOfAdaptedResidualCosts();
        return ReturnCode::SUCCESS;
    }

    return ReturnCode::NOTHING_CHANGED;
}

ReturnCode EscapingBonbons::CheckAllIncomingEdgesForTail(EscapingNCC& ncc, edgeID currentNegativeEdgeOnBonbon) {
    for (edgeID outgoingEdge: Windfarm().OutgoingEdges(Windfarm().StartVertex(currentNegativeEdgeOnBonbon))) {
        if (outgoingEdge == currentNegativeEdgeOnBonbon) { continue; /* cycle of length 2 */ }

        // incomingEdge enters StartVertex of negative edge to (hopefully) form negative cycle
        edgeID incomingEdge = Windfarm().ReverseEdgeIndex(outgoingEdge);

        std::vector<edgeID> tail = ncc.DetectionStrategy().TraverseParentsUntilWalkStarts(bonbon_, incomingEdge);
        if (tail.empty()) { continue; }
        

        ReturnCode resultOfCombiningTailAndBonbon = CombineTailAndBonbon(ncc, tail);
        if (resultOfCombiningTailAndBonbon == ReturnCode::SUCCESS) { return resultOfCombiningTailAndBonbon; }
    }
    return ReturnCode::NOTHING_CHANGED;
}

void EscapingBonbons::UpdateVectorOfBonbonEdgesForVertices(unsigned int currentIndexInBonbon) {
    assert(currentIndexInBonbon < bonbon_.size());

    firstEdgeInWalkLeadingToVertex_ = std::vector<edgeID>(Windfarm().NumberOfVertices(), INVALID_EDGE);
    // Start at the current negative edge. With that, we make sure that we obtain a simple cycle
    // if we combine the tail with the edges as given in firstEdgeInWalkLeadingToVertex_.
    unsigned int index = currentIndexInBonbon;
    // read as: for (unsigned int k = currentIndexInBonbon; k >= 0 ; k--)
    while (true) {
        SetIncomingBonbonEdgeForVertex(bonbon_[index]);
        if (index == 0) { break; }
        else { index--; }
    }

    for (index = bonbon_.size()-1; index > currentIndexInBonbon; index--) {
        SetIncomingBonbonEdgeForVertex(bonbon_[index]);
    }
}

ReturnCode EscapingBonbons::apply(EscapingNCC& ncc) {
    // Iterate over all negative edges in a bonbon using (potentially) adjusted residual costs.
    // For each such edge (u,v), traverse parent pointers from any edge (w,u) until a simple cycle is
    // closed. If that cycle is negative, cancel it and go back to NCC.
    CommonStartForAllEscapingStrategies(ncc.beVerbose_, ncc.verboseStream_);

    if (ncc.bonbon_.edgeOnBonbon == INVALID_EDGE) {
        return ReturnCode::NOTHING_CHANGED;
    } else {
        RecoverBonbon(ncc);

        for (unsigned int walkIndex = 0; walkIndex + 1 < bonbon_.size(); walkIndex++) {
            if (Windfarm().ResidualCostOnEdge(bonbon_[walkIndex]) < 0) {
                if (ncc.beVerbose_) {
                    ncc.verboseStream_ << "\tIdentified negative edge on bonbon: ID " << bonbon_[walkIndex];
                    ncc.verboseStream_ << ". Try to find another path ('tail') from vertex " << Windfarm().StartVertex(bonbon_[walkIndex]) << ".\n";
                }
                UpdateVectorOfBonbonEdgesForVertices(walkIndex);

                ReturnCode resultForWalkIndex = CheckAllIncomingEdgesForTail(ncc, bonbon_[walkIndex]);

                if (resultForWalkIndex == ReturnCode::SUCCESS) { return resultForWalkIndex; }
            } else {
                continue;
            }
        }

        return ReturnCode::NOTHING_CHANGED;
    }
};

} // namespace Algorithms

} // namespace wcp
