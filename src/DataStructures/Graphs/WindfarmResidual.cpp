/*
 * Windfarm.cpp
 *
 *  Created on: Jun 29, 2018
 *      Author: sascha
 */

#include "DataStructures/Graphs/WindfarmResidual.h"

namespace wcp {

namespace DataStructures {

WindfarmResidual::WindfarmResidual(const Instance& instance)
  : Windfarm(instance)
{
    // Fill list of edges; vertices are constructed in the base class.
    InitializeEdges(2*NumberOfOriginalEdges() + 2*NumberOfSubstations());

    for (auto e : instance.Graph().edges) {
        edgeID edgeIndex        = e->index();
        vertexID sourceIndex    = e->source()->index();
        vertexID targetIndex    = e->target()->index();

        Edge& edge          = EdgeAt(edgeIndex);
        edge.Identifier()   = edgeIndex;
        edge.StartVertex()  = sourceIndex;
        edge.EndVertex()    = targetIndex;
        edge.Length()       = Length(e, instance.GraphAttributes());
        edge.Capacity()     = Cabletypes().MaximumCapacity();
        AddOutgoingEdge(sourceIndex, edgeIndex);

        edgeID reverseEdgeIndex     = edgeIndex + NumberOfOriginalEdges();
        Edge& reverseEdge           = EdgeAt(reverseEdgeIndex);
        reverseEdge.Identifier()    = edgeIndex;
        reverseEdge.StartVertex()   = targetIndex;
        reverseEdge.EndVertex()     = sourceIndex;
        reverseEdge.Length()        = Length(e, instance.GraphAttributes());
        reverseEdge.Capacity()      = Cabletypes().MaximumCapacity();
        AddOutgoingEdge(targetIndex, reverseEdgeIndex);
    }
    // We still need edges between substations and supersubstation
    for (int i = 0; i < NumberOfSubstations(); ++i) {
        vertexID substation = SubstationAt(i);

        // Edges towards supersubstation
        edgeID towardsEdgeIndex     = 2 * NumberOfOriginalEdges() + i;
        Edge& towardsEdge           = EdgeAt(towardsEdgeIndex);
        towardsEdge.Identifier()    = towardsEdgeIndex;
        towardsEdge.StartVertex()   = substation;
        towardsEdge.EndVertex()     = SuperSubstationIdentifier();
        towardsEdge.Capacity()      = VertexCapacity(substation);
        AddOutgoingEdge(substation, towardsEdgeIndex);

        // Edges from supersubstation
        edgeID fromEdgeIndex = towardsEdgeIndex + NumberOfSubstations();
        Edge& fromEdge = EdgeAt(fromEdgeIndex);
        fromEdge.Identifier()   = fromEdgeIndex;
        fromEdge.StartVertex()  = SuperSubstationIdentifier();
        fromEdge.EndVertex()    = substation;
        fromEdge.Capacity()     = VertexCapacity(substation);
        AddOutgoingEdge(SuperSubstationIdentifier(), fromEdgeIndex);
    }

    // Initialize Vector with Flow Values and Cost Values
    flowValues_.resize(NumberOfOriginalEdges() + NumberOfSubstations(), 0);
    residualCosts_.resize(NumberOfEdges(), 0);
}

edgeID WindfarmResidual::ReverseEdgeIndex(edgeID index) const {
    assert(index >= 0);
    assert(index < NumberOfEdges());
    if (index < NumberOfOriginalEdges()) {
        return index + NumberOfOriginalEdges();
    } else if (index < 2*NumberOfOriginalEdges()) {
        return index - NumberOfOriginalEdges();
    } else if (index < 2*NumberOfOriginalEdges() + NumberOfSubstations()) {
        return index + NumberOfSubstations();
    } else
        return index - NumberOfSubstations();
}

int WindfarmResidual::FlowOnEdge(edgeID index) const {
    assert(index >= 0);
    assert(index < NumberOfEdges());
    if (index < NumberOfOriginalEdges()) {
        return flowValues_[index];
    } else if (index < 2*NumberOfOriginalEdges()) {
        return -flowValues_[index - NumberOfOriginalEdges()];
    } else if (index < 2*NumberOfOriginalEdges() + NumberOfSubstations()) {
        return flowValues_[index - NumberOfOriginalEdges()];
    } else {
        return -flowValues_[index - NumberOfOriginalEdges() - NumberOfSubstations()];
    }
}

int WindfarmResidual::FindMinimumFlowOnEdges(std::vector<edgeID>& edges) const {
    int minimumFlow = Auxiliary::INFINITE_CAPACITY;
    for (edgeID edge : edges) {
        if (FlowOnEdge(edge) < minimumFlow) { minimumFlow = FlowOnEdge(edge); }
    }
    return minimumFlow;
}

void WindfarmResidual::SetFlowOnEdge(edgeID index, int newFlow) {
    assert(index >= 0);
    assert(index < NumberOfEdges());
    if (index < NumberOfOriginalEdges()) {
        flowValues_[index] = newFlow;
    } else if (index < 2*NumberOfOriginalEdges()) {
        flowValues_[index - NumberOfOriginalEdges()] = -newFlow;
    } else if (index < 2*NumberOfOriginalEdges() + NumberOfSubstations()) {
        flowValues_[index - NumberOfOriginalEdges()] = newFlow;
    } else {
        flowValues_[index - NumberOfOriginalEdges() - NumberOfSubstations()] = -newFlow;
    }
}

double WindfarmResidual::ComputeTotalCostOfFlow() const {
    double cost = 0.0;
    for (edgeID i = 0; i < NumberOfOriginalEdges(); ++i) {
        cost += ComputeCostOfFlowOnEdge(i);
    }
    return cost;
}

void WindfarmResidual::writeFlowToOGDF(ogdf::Graph& graph, ogdf::GraphAttributes& graphAttr) {
    for (auto edge = graph.firstEdge(); edge; edge = edge->succ()) {
        graphAttr.label(edge) = std::to_string(edge->index());
        graphAttr.doubleWeight(edge) = FlowOnEdge(edge->index());
    }
}

} // namespace DataStructures

} // namespace wcp
