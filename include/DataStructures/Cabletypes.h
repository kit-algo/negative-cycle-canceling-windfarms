#ifndef DATASTRUCTURES_CABLETYPES
#define DATASTRUCTURES_CABLETYPES

#include <cassert>
#include <limits>
#include <vector>

namespace wcp {

namespace DataStructures {

/**
 * Class representing the cost function for flow in a windfarm.
 *
 * It can be used as a representation for real cable types or simply using
 * a vector that maps flow to costs.
 */
class Cabletypes {
public:
    /**
     * A constructor that uses equal sized vectors, from with the i-th entry
     * represents the i-th cable type.
     */
    Cabletypes(std::vector<int> vectorOfCapacities,
               std::vector<int> vectorOfCosts);

    /**
     * A constructor that uses a vector that maps flow amount to cost per unit of length.
     *
     * The vector must be of odd length and its middle entry corresponds to zero flow.
     * If the vector is symmetric and increasing after the middle entry, the vectors
     * vectorOfCapacities_ and vectorOfCosts_ are filled as well.
     */
    Cabletypes(std::vector<int> expandedVector);

    std::size_t NumberOfCables() const;

    int MaximumCapacity() const;

    int CostOfFlow(int flow) const;

    const std::vector<int>& CapacityBreakpoints() const {
        assert(isGivenByRealCables_);
        return vectorOfCapacities_;
    }

    const std::vector<int>& CostBreakpoints() const {
        assert(isGivenByRealCables_);
        return vectorOfCosts_;
    }

    const std::vector<int>& ExpandedCosts() const { return vectorOfCostsExpanded_; }

    static constexpr int INFINITE_COST = std::numeric_limits<int>::max() / 2;
    static constexpr int INFINITE_CAPACITY = std::numeric_limits<int>::max() / 2;

private:
    static bool IsStrictlyIncreasing(const std::vector<int>& v);
    static bool IsIncreasingFromMidpoint(const std::vector<int>& expandedCosts);
    static bool IsSymmetric(const std::vector<int>& expandedCosts);
    void FillExpandedCosts();
    void ExtractCableCosts();

    std::size_t numberOfCables_;
    std::vector<int> vectorOfCapacities_;
    std::vector<int> vectorOfCosts_;

    int maximumCableCapacity_;
    std::vector<int> vectorOfCostsExpanded_;

    bool isGivenByRealCables_;
};

} // namespace DataStructures

} // namespace wcp

#endif
