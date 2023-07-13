#ifndef DATASTRUCTURES_GRAPHS_WINDFARMRESIDUAL
#define DATASTRUCTURES_GRAPHS_WINDFARMRESIDUAL

#include <cassert>
#include <string>
#include <vector>

#include <ogdf/basic/basic.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/Graph_d.h>
#include <ogdf/basic/GraphAttributes.h>

#include "Auxiliary/Constants.h"

#include "DataStructures/Graphs/Windfarm.h"
#include "DataStructures/Graphs/Edges/Edge.h"
#include "DataStructures/Graphs/Vertices/Vertex.h"
#include "DataStructures/Cabletypes.h"
#include "DataStructures/Instance.h"

namespace wcp {

namespace DataStructures {

/**
 * Struct representing the residual graph of a windfarm, which is used
 * during the negative cycle canceling algorithm.
 *
 *  Objects are solely to be constructed from an ogdf-graph.
 */
class WindfarmResidual : public Windfarm {
public:
    // Constructor for windfarm object based on graph with graph attributes from OGDF
    WindfarmResidual(const Instance& instance);

    const std::vector<edgeID>& OutgoingEdges(vertexID index) const {
        assert(index >= 0);
        assert(index < NumberOfVertices());
        return VertexAt(index).OutgoingEdges();
    }

    edgeID ReverseEdgeIndex(edgeID index) const;

    /**
     * Returns the flow value of the edge. For an edge (u,v), FlowOnEdge is positive
     * if flow goes from u to v.
     */
    int FlowOnEdge(edgeID index) const;

    int FindMinimumFlowOnEdges(std::vector<edgeID>& edges) const;

    void SetFlowOnEdge(edgeID index, int newFlow);

    void AddFlowOnEdge(edgeID index, int delta) {
        SetFlowOnEdge(index, FlowOnEdge(index) + delta);
    }

    void AddFlowOnMultipleEdges(const std::vector<edgeID>& edges, int delta) {
        for (edgeID edge : edges) {
            AddFlowOnEdge(edge, delta);
        }
    }

    double ResidualCostOnEdge(edgeID index) const {
        assert(index >= 0);
        assert(index < NumberOfEdges());
        return residualCosts_[index];
    }

    double& ResidualCostOnEdge(edgeID index) {
        assert(index >= 0);
        assert(index < NumberOfEdges());
        return residualCosts_[index];
    }

    /**
     * Computes the cost of the flow on the edge based on the cabletypes.
     */
    double ComputeCostOfFlowOnEdge(edgeID index) const {
        assert(index >= 0);
        assert(index < NumberOfEdges());
        return Cabletypes().CostOfFlow(FlowOnEdge(index)) * Length(index);
    }

    double ComputeTotalCostOfFlow() const;

    /**
     * Returns the edge from the given substation to the supersubstation.
     */
    edgeID EdgeToSupersubstation(vertexID substation) const {
        assert(IsSubstation(substation));
        edgeID edge = VertexAt(substation).OutgoingEdges().back();
        assert(IsSuperSubstation(EndVertex(edge)));
        return edge;
    }

    int FlowToSubstation(vertexID substation) const {
        assert(IsSubstation(substation));
        edgeID edgeToSupersubstation = EdgeToSupersubstation(substation);
        return FlowOnEdge(edgeToSupersubstation);
    }


    int FreeSubstationCapacity(vertexID substation) const {
        return VertexCapacity(substation) - FlowToSubstation(substation);
    }

    bool HasFreeSubstationCapacity(vertexID substation) const {
        return FreeSubstationCapacity(substation) > 0;
    }

    /**
     * Returns the free capacity of the edge. The edge must not be
     * incident to the supersubstation.
     */
    int FreeEdgeCapacity(edgeID edge) const {
        return Cabletypes().MaximumCapacity() - FlowOnEdge(edge);
    }

    /**
     * Returns whether the edge has free capacity. The edge must not be
     * incident to the supersubstation.
     */
    bool HasFreeEdgeCapacity(edgeID edge) const {
        return FreeEdgeCapacity(edge) > 0;
    }

    /**
     * Writes flow values as edge weights in ogdf graph.
     *
     * This method does not perform a sanity check whether the ogdf graph
     * is consistent with the wind farm residual graph, nor does it check
     * whether edge label is an enabled attribute in the ogdf graph.
     */
    void writeFlowToOGDF(ogdf::Graph& graph, ogdf::GraphAttributes& graphAttr);

private:
    std::vector<int>& FlowValues() { return flowValues_; }

    void AddOutgoingEdge(vertexID vertexIndex, edgeID edgeIndex) {
        assert(vertexIndex >= 0);
        assert(vertexIndex < NumberOfVertices());
        assert(edgeIndex >= 0);
        assert(edgeIndex < NumberOfEdges());
        VertexAt(vertexIndex).OutgoingEdges().push_back(edgeIndex);
    }

    /**
     * Vector of flow values on original edges and supersubstation edges
     *
     * Flow values are only saved for those edges in gml-file (original edges) and edges towards the supersubstation,
     * i.e. entries 0 to m-1 correspond to entries 0 to m-1 in listOfEdges_ and entries m to m+#V_S-1 correspond
     * to entries 2m to 2m+#V_S-1.
     */
    std::vector<int> flowValues_;

    /**
     * Cost values for all edges in the windfarm, including reverse and supersubstation edges
     */
    std::vector<double> residualCosts_;
};

inline void AssertZeroFlow(const WindfarmResidual& windfarm) {
    for (int e = 0; e < windfarm.NumberOfEdges(); ++e) {
        assert(windfarm.FlowOnEdge(e) == 0);
    }
}

inline void AssertFeasibleFlow(const WindfarmResidual& windfarm) {
    // Edge capacities
    for (edgeID edge = 0; edge < windfarm.NumberOfEdges(); ++edge) {
        assert(windfarm.FlowOnEdge(edge) == -windfarm.FlowOnEdge(windfarm.ReverseEdgeIndex(edge)));
        if (!windfarm.IsSuperSubstation(windfarm.StartVertex(edge))
                && !windfarm.IsSuperSubstation(windfarm.EndVertex(edge))) {
            assert(windfarm.FlowOnEdge(edge) <= windfarm.Cabletypes().MaximumCapacity());
        }
    }

    // Flow conservation at turbines
    for (vertexID turbine : windfarm.Turbines()) {
        int sumOfFlow = 0;
        for (edgeID edge : windfarm.OutgoingEdges(turbine)) {
            sumOfFlow += windfarm.FlowOnEdge(edge);
        }
        assert(sumOfFlow == 1);
    }

    // Flow conservation at substations
    for (vertexID substation : windfarm.Substations()) {
        int sumOfFlow = 0;
        for (edgeID edge : windfarm.OutgoingEdges(substation)) {
            sumOfFlow += windfarm.FlowOnEdge(edge);

            vertexID endVertex = windfarm.EndVertex(edge);
            if (windfarm.IsTurbine(endVertex)) {
                assert(windfarm.FlowOnEdge(edge) <= 0);
            } else if (windfarm.IsSuperSubstation(endVertex)) {
                assert(windfarm.FlowOnEdge(edge) >= 0);
                assert(windfarm.FlowToSubstation(substation)
                        == windfarm.FlowOnEdge(edge));
            } else {
                assert(windfarm.FlowOnEdge(edge) == 0);
            }
        }
        assert(sumOfFlow == 0);
        assert(windfarm.FlowToSubstation(substation) >= 0);
        assert(windfarm.FlowToSubstation(substation) <= windfarm.VertexCapacity(substation));
    }
}

} // namespace DataStructures

} // namespace wcp

#endif
