#ifndef DATASTRUCTURES_GRAPHS_VERTICES_VERTEX
#define DATASTRUCTURES_GRAPHS_VERTICES_VERTEX

#include <type_traits>
#include <vector>

#include <ogdf/basic/graphics.h>

namespace wcp {

namespace DataStructures {

enum class VertexType : std::underlying_type<ogdf::Shape>::type {
    turbine         = static_cast<std::underlying_type<ogdf::Shape>::type>(ogdf::Shape::Rect),
    substation      = static_cast<std::underlying_type<ogdf::Shape>::type>(ogdf::Shape::Ellipse),
    supersubstation = static_cast<std::underlying_type<ogdf::Shape>::type>(ogdf::Shape::Triangle)
};

inline bool operator==(const VertexType &lhs, const ogdf::Shape &rhs) {
    using type = std::underlying_type<ogdf::Shape>::type;
    return static_cast<type>(lhs) == static_cast<type>(rhs);
}

inline bool operator==(const ogdf::Shape &lhs, const VertexType &rhs) {
    return rhs == lhs;
}

inline bool operator!=(const VertexType &lhs, const ogdf::Shape &rhs) {
    return !(lhs == rhs);
}

inline bool operator!=(const ogdf::Shape &lhs, const VertexType &rhs) {
    return !(lhs == rhs);
}


/**
 * Struct for vertices in windfarm
 */
struct Vertex {
public:
    Vertex()
      : _identifier(-1),
        _xCoordinate(-1),
        _yCoordinate(-1),
        _capacity(-1),
        _type(VertexType::turbine) {}

    int Identifier() const {
        return _identifier;
    }

    int &Identifier() {
        return _identifier;
    }

    const std::vector<int>& IncidentEdges() const {
        return _incidentEdges;
    }

    std::vector<int>& IncidentEdges() {
        return _incidentEdges;
    }

    const std::vector<int>& OutgoingEdges() const {
        return _outgoingEdges;
    }

    std::vector<int>& OutgoingEdges() {
        return _outgoingEdges;
    }

    int Capacity() const {
        return _capacity;
    }

    int &Capacity() {
        return _capacity;
    }

    VertexType Type() const {
        return _type;
    }

    VertexType &Type() {
        return _type;
    }

    double X() const {
        return _xCoordinate;
    }

    double &X() {
        return _xCoordinate;
    }

    double Y() const {
        return _yCoordinate;
    }

    double &Y() {
        return _yCoordinate;
    }

private:
    int _identifier; // ID corresponding to ID in gml-file
    double _xCoordinate;
    double _yCoordinate;
    int _capacity;
    VertexType _type;

    /**
     * Includes indices of all outgoing edges, including reverse and supersubstation-edges,
     * needed for WindfarmResidual.h
     */
    std::vector<int> _outgoingEdges;

    /**
     * Includes indices of all incident edges, including supersubstation-edges,
     * needed for WindfarmCMST.h
     */
    std::vector<int> _incidentEdges;
};

} // namespace DataStructures

} // namespace wcp

#endif
