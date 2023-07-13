#include "DataStructures/Graphs/Windfarm.h"

namespace wcp {

namespace DataStructures {

Windfarm::Windfarm(const Instance& instance)
  : numberOfOriginalVertices_(instance.Graph().numberOfNodes()),
    numberOfOriginalEdges_(instance.Graph().numberOfEdges()),
    numberOfTurbines_(0),
    numberOfSubstations_(0),
    listOfVertices_(numberOfOriginalVertices_ + 1, Vertex()),
    cabletypes_(instance.CableTypes())
{
    // Make sure that gml-file has been properly labeled
    assert(instance.Graph().maxNodeIndex() + 1 == instance.Graph().numberOfNodes());
    assert(instance.Graph().maxEdgeIndex() + 1 == instance.Graph().numberOfEdges());

    for (auto v : instance.Graph().nodes) {
        vertexID vIndex = v->index();
        Vertex& vertex = VertexAt(vIndex);

        vertex.Identifier() = vIndex;
        vertex.X() = instance.GraphAttributes().x(v);
        vertex.Y() = instance.GraphAttributes().y(v);
        // Of what type is newVertex? Turbine or Substation?
        if (instance.IsTurbine(v)) {
            vertex.Type() = VertexType::turbine;
            AddTurbine(vIndex);
        } else if (instance.IsSubstation(v)) {
            vertex.Type() = VertexType::substation;
            vertex.Capacity() = instance.SubstationCapacity(v);
            AddSubstation(vIndex);
        }
    }

    // Make sure that a type of every vertex has been recognized
    assert(NumberOfSubstations() + NumberOfTurbines() == NumberOfOriginalVertices());

    // Add super substation to the set of vertices
    Vertex& superSubstation = VertexAt(NumberOfOriginalVertices());
    superSubstation.Identifier() = NumberOfOriginalVertices();
    superSubstation.Type() = VertexType::supersubstation;
}

} // namespace DataStructures

} // namespace wcp
