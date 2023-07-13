#ifndef DATASTRUCTURES_INSTANCE
#define DATASTRUCTURES_INSTANCE

#include <string>
#include <utility>

#include <ogdf/basic/Array.h>
#include <ogdf/basic/NodeArray.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/fileformats/GraphIO.h>

#include "DataStructures/Graphs/Vertices/Vertex.h"
#include "DataStructures/Cabletypes.h"

namespace wcp {

namespace DataStructures {

/**
 * This class describes an instance. It contains a graph and its attributes
 * as well as the testset number and the instance number and the cable types.
 */
class Instance {
public:
    Instance(int testsetNumber,
             int instanceNumber,
             long graphAttributeSpecification = ogdf::GraphAttributes::nodeGraphics   |
                                                ogdf::GraphAttributes::nodeLabel      |
                                                ogdf::GraphAttributes::nodeWeight     |
                                                ogdf::GraphAttributes::edgeLabel      |
                                                ogdf::GraphAttributes::edgeIntWeight  |
                                                ogdf::GraphAttributes::edgeDoubleWeight,
             std::vector<int> vectorOfCapacities = {5, 8, 12, 15},
             std::vector<int> vectorOfCosts      = {20, 25, 27, 41})
      : graph_(),
        graphAttributes_(graph_, graphAttributeSpecification),
        testsetNumber_(testsetNumber),
        instanceNumber_(instanceNumber),
        cabletypes_(std::move(vectorOfCapacities), std::move(vectorOfCosts)) {}

    Instance(const Instance&) = delete;
    Instance(Instance&&) = delete;
    void operator=(const Instance&) = delete;
    void operator=(Instance&&) = delete;

    ogdf::Graph& Graph() {
        return graph_;
    }

    const ogdf::Graph& Graph() const {
        return graph_;
    }

    ogdf::GraphAttributes& GraphAttributes() {
        return graphAttributes_;
    }

    const ogdf::GraphAttributes& GraphAttributes() const {
        return graphAttributes_;
    }

    int& TestsetNumber() {
        return testsetNumber_;
    }

    int TestsetNumber() const {
        return testsetNumber_;
    }

    int& InstanceNumber() {
        return instanceNumber_;
    }

    int InstanceNumber() const {
        return instanceNumber_;
    }

    std::string testsetInstanceString() {
        return "t-" + std::to_string(TestsetNumber())
                + "-i-" + std::to_string(InstanceNumber());
    }

    std::string longTestsetInstanceString() {
        return "testset-" + std::to_string(TestsetNumber())
                + "-instance-" + std::to_string(InstanceNumber());
    }

    const Cabletypes& CableTypes() const {
        return cabletypes_;
    }

    int NumberOfNodes() const {
        return graph_.numberOfNodes();
    }

    int NumberOfEdges() const {
        return graph_.numberOfEdges();
    }

    bool IsTurbine(ogdf::node node) const {
        return graphAttributes_.shape(node) == DataStructures::VertexType::turbine;
    }

    bool IsSubstation(ogdf::node node) const {
        return graphAttributes_.shape(node) == DataStructures::VertexType::substation;
    }

    int NumberOfTurbines() const { return numberOfTurbines_; }

    int NumberOfSubstations() const { return numberOfSubstations_; }

    int SubstationCapacity(ogdf::node node) const {
        assert(IsSubstation(node));
        return std::stoi(graphAttributes_.label(node));
    }

    int TotalSubstationCapacity() const { return totalSubstationCapacity_; }

    double SubstationCapacityTightness() const { return substationCapacityTightness_; }

    void ReadFromGMLFile(const std::string& pathToInputFile) {
        ogdf::GraphIO::read(GraphAttributes(),
                            Graph(),
                            pathToInputFile,
                            ogdf::GraphIO::readGML);
        UpdateMembers();
        EnsurePrerequisites();
    }

    void UpdateMembers() {
        numberOfTurbines_ = 0;
        numberOfSubstations_ = 0;
        totalSubstationCapacity_ = 0;

        for (ogdf::node v : graph_.nodes) {
            try {
                if (IsTurbine(v)) {
                    ++numberOfTurbines_;
                } else if (IsSubstation(v)) {
                    ++numberOfSubstations_;
                    totalSubstationCapacity_ += SubstationCapacity(v);
                } else {
                    throw v->index();
                }
            } catch (int i) {
                throw "Could not identify type of ogdf-node " + std::to_string(i) + '\n';
            }
        }

        substationCapacityTightness_ = NumberOfTurbines() / (double) totalSubstationCapacity_;
    }

    void EnsurePrerequisites() {
        if (NumberOfNodes() == 0) {
            throw std::string("Input file is empty or does not exist.");
        }
        if (Graph().maxNodeIndex() != Graph().numberOfNodes() - 1) {
            throw "The vertex indices are not in the range from 0 to n-1 (="
                    + std::to_string(Graph().numberOfNodes() - 1) + ")";
        }
        if (Graph().maxEdgeIndex() != Graph().numberOfEdges() - 1) {
            throw "The edge indices are not in the range from 0 to n-1 (="
                    + std::to_string(Graph().numberOfNodes() - 1) + ")";
        }
    }

    /**
     * Checks if the graphs of two instances are indexed identically in both vertices and edges.
     * Returns the boolean value whether the check is successful or not and a string
     * with a reason why the graphs are not indexed identically.
     */
    std::pair<bool, std::string> GraphIsSimilarTo(const Instance& otherInstance) {
        bool equalGraphs = true;

        if (NumberOfNodes() != otherInstance.NumberOfNodes()) {
            std::cout << NumberOfNodes() << " " << otherInstance.NumberOfNodes() << std::endl;
            return std::make_pair(false, "Graphs do not have the same number of nodes.");
        }

        if (NumberOfEdges() != otherInstance.NumberOfEdges()) {
            return std::make_pair(false, "Graphs do not have the same number of edges.");
        }

        if (Graph().maxNodeIndex() != otherInstance.Graph().maxNodeIndex()) {
            return std::make_pair(false, "Max Node IDs do not match.");
        }

        if (Graph().maxEdgeIndex() != otherInstance.Graph().maxEdgeIndex()) {
            return std::make_pair(false, "Max Edge IDs do not match.");
        }

        auto runningEdgeSecondInstance = otherInstance.Graph().firstEdge();
        for (ogdf::edge e = graph_.firstEdge(); e; e = e->succ()) {
            equalGraphs &= (e->source()->index() == runningEdgeSecondInstance->source()->index());
            equalGraphs &= (e->target()->index() == runningEdgeSecondInstance->target()->index());
            if (!equalGraphs) {
                return std::make_pair(equalGraphs, "Vertices at edge " + std::to_string(e->index()) + " do not match.");
            }
            runningEdgeSecondInstance = runningEdgeSecondInstance->succ();
        }

        assert(equalGraphs);
        return std::make_pair(equalGraphs, "");
    }

private:
    ogdf::Graph graph_;
    ogdf::GraphAttributes graphAttributes_;
    int testsetNumber_;
    int instanceNumber_;

    Cabletypes cabletypes_;

    int numberOfTurbines_;
    int numberOfSubstations_;

    int totalSubstationCapacity_;
    double substationCapacityTightness_;
};

} // namespace DataStructures

} // namespace wcp

#endif
