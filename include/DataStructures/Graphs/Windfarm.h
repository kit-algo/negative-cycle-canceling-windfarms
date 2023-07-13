#ifndef DATASTRUCTURES_GRAPHS_WINDFARM
#define DATASTRUCTURES_GRAPHS_WINDFARM

#include <cassert>
#include <vector>

#include <ogdf/basic/basic.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/Graph_d.h>
#include <ogdf/basic/GraphAttributes.h>

#include "DataStructures/Graphs/Edges/Edge.h"
#include "DataStructures/Graphs/Vertices/Vertex.h"
#include "DataStructures/Cabletypes.h"
#include "DataStructures/Instance.h"

using vertexID = int;
using edgeID = int;

const vertexID INVALID_VERTEX = -1;
const edgeID INVALID_EDGE = -1;

namespace wcp {

namespace DataStructures {

/**
 * Base class for the wind farms.
 *
 * This wind farm contains only the vertices of the graph and not the edges.
 * The subclasses are responsible to store the edges because different algorithms
 * have different requirements regarding which and how edges are stored.
 */
struct Windfarm {
public:
    Windfarm(const Instance& instance);

    int NumberOfOriginalVertices() const { return numberOfOriginalVertices_; }     ///< returns number of vertices without supersubstation
    int NumberOfVertices()         const { return listOfVertices_.size();    }     ///< returns number of vertices including supersubstation
    int NumberOfTurbines()         const { return numberOfTurbines_;         }     ///< returns number of turbines
    int NumberOfSubstations()      const { return numberOfSubstations_;      }     ///< returns number of substations
    int NumberOfOriginalEdges()    const { return numberOfOriginalEdges_;    }     ///< returns number of original edges without supersubstation edges
    int NumberOfEdges()            const { return listOfEdges_.size();       }     ///< returns total number of edges including supersubstation edges

    vertexID SuperSubstationIdentifier() const {
        assert(IsSuperSubstation(NumberOfVertices() - 1));
        return NumberOfVertices() - 1;
    }

    const std::vector<vertexID>& Turbines()    const { return turbineIdentifiers_;   }   ///< returns const reference on vector with turbine identifiers
    const std::vector<vertexID>& Substations() const { return substationIdentifiers_; }  ///< returns const reference on vector with substation indices without supersubstation

    const std::vector<edgeID>& IncidentEdges(vertexID index) const {
        assert(index >= 0);
        assert(index < NumberOfVertices());
        return VertexAt(index).IncidentEdges();
    }

    int VertexCapacity(vertexID index) const {
        assert(index >= 0);
        assert(index < NumberOfVertices());
        return VertexAt(index).Capacity();
    }

    double X(vertexID index) const {
        assert(index >= 0);
        assert(index < NumberOfVertices());
        return VertexAt(index).X();
    }

    double Y(vertexID index) const {
        assert(index >= 0);
        assert(index < NumberOfVertices());
        return VertexAt(index).Y();
    }

    vertexID StartVertex(edgeID index) const {
        assert(index >= 0);
        assert(index < NumberOfEdges());
        return EdgeAt(index).StartVertex();
    }

    vertexID EndVertex(edgeID index) const {
        assert(index >= 0);
        assert(index < NumberOfEdges());
        return EdgeAt(index).EndVertex();
    }

    /**
     * Returns already computed length of the edge
     */
    double Length(edgeID index) const {
        assert(index >= 0);
        assert(index < NumberOfEdges());
        return EdgeAt(index).Length();
    }

    /**
     * Computes and returns Euclidean length of edge from ogdf::GraphAttributes
     */
    double Length(ogdf::edge e, const ogdf::GraphAttributes& graphAttributes) const {
        return std::sqrt(std::pow(graphAttributes.x(e->source()) - graphAttributes.x(e->target()), 2)
                + std::pow(graphAttributes.y(e->source()) - graphAttributes.y(e->target()), 2));
    }

    int EdgeCapacity(edgeID index) const {
        assert(index >= 0);
        assert(index < NumberOfEdges());
        return EdgeAt(index).Capacity();
    }

    const wcp::DataStructures::Cabletypes& Cabletypes() const {
        return cabletypes_;
    }

    bool IsTurbine(vertexID index) const {
        assert(index >= 0);
        assert(index < NumberOfVertices());
        return VertexAt(index).Type() == VertexType::turbine;
    }

    bool IsSubstation(vertexID index) const {
        assert(index >= 0);
        assert(index < NumberOfVertices());
        return VertexAt(index).Type() == VertexType::substation;
    }

    bool IsSuperSubstation(vertexID index) const {
        assert(index >= 0);
        assert(index < NumberOfVertices());
        return VertexAt(index).Type() == VertexType::supersubstation;
    }

protected:
    vertexID SubstationAt(int index) const {
        assert(index < NumberOfSubstations());
        return Substations()[index];
    }

    void AddTurbine(vertexID index) {
        turbineIdentifiers_.push_back(index);
        ++numberOfTurbines_;
    }

    void AddSubstation(vertexID index) {
        substationIdentifiers_.push_back(index);
        ++numberOfSubstations_;
    }

    Vertex& VertexAt(vertexID index) {
        assert(index >= 0);
        assert(index < NumberOfVertices());
        return listOfVertices_[index];
    }

    const Vertex& VertexAt(vertexID index) const {
        assert(index >= 0);
        assert(index < NumberOfVertices());
        return listOfVertices_[index];
    }

    Edge& EdgeAt(edgeID index) {
        assert(index >= 0);
        assert(index <= NumberOfEdges());
        return listOfEdges_[index];
    }

    const Edge& EdgeAt(edgeID index) const {
        assert(index >= 0);
        assert(index <= NumberOfEdges());
        return listOfEdges_[index];
    }

    void InitializeEdges(size_t edgeCount) {
        listOfEdges_.resize(edgeCount, Edge());
    }

private:
    int numberOfOriginalVertices_; // number of vertices as in gml-file
    int numberOfOriginalEdges_; // number of edges as in gml-file
    int numberOfTurbines_;
    int numberOfSubstations_;

    std::vector<Vertex>   listOfVertices_; // vector of original vertices with supersubstation as last entry

    std::vector<Edge>     listOfEdges_; // vector of original edges and edges between substation and supersubstation

    std::vector<vertexID> turbineIdentifiers_;
    std::vector<vertexID> substationIdentifiers_;

    const wcp::DataStructures::Cabletypes&   cabletypes_;
};

} // namespace DataStructures

} // namespace wcp

#endif
