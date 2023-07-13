#ifndef DATASTRUCTURES_GRAPHS_EDGES_EDGE
#define DATASTRUCTURES_GRAPHS_EDGES_EDGE

namespace wcp {

namespace DataStructures {

/**
 * Struct for edges in windfarm
 */
struct Edge {
private:
    int _identifier; // ID corresponding to ID in gml-file
    double _length;
    int _capacity;

    int _startvertex; // ID for vertex u with e = (u,v)
    int _endvertex; // ID for vertex v with e = (u,v)

public:
    int      Identifier() const  { return _identifier;  }
    int     &Identifier()        { return _identifier;  }
    double   Length() const      { return _length;      }
    double   &Length()           { return _length;      }
    int      Capacity() const    { return _capacity;    }
    int     &Capacity()          { return _capacity;    }
    int      StartVertex() const { return _startvertex; }
    int     &StartVertex()       { return _startvertex; }
    int      EndVertex() const   { return _endvertex;   }
    int     &EndVertex()         { return _endvertex;   }
};

} // namespace DataStructures

} // namespace wcp

#endif
