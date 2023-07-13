#include <utility>

#include "DataStructures/Cabletypes.h"

namespace wcp {

namespace DataStructures {

Cabletypes::Cabletypes(std::vector<int> vectorOfCapacities,
                       std::vector<int> vectorOfCosts)
  : numberOfCables_(vectorOfCosts.size()),
    vectorOfCapacities_(std::move(vectorOfCapacities)),
    vectorOfCosts_(std::move(vectorOfCosts)),
    maximumCableCapacity_(vectorOfCapacities_[numberOfCables_ - 1]),
    vectorOfCostsExpanded_(2 * maximumCableCapacity_ + 1, 0),
    isGivenByRealCables_(true)
{
    assert(vectorOfCosts_.size() == vectorOfCapacities_.size());
    assert(IsStrictlyIncreasing(vectorOfCosts_));
    assert(IsStrictlyIncreasing(vectorOfCapacities_));

    FillExpandedCosts();
}

Cabletypes::Cabletypes(std::vector<int> expandedVector)
  : numberOfCables_(0),
    vectorOfCapacities_(),
    vectorOfCosts_(),
    maximumCableCapacity_(expandedVector.size() / 2),
    vectorOfCostsExpanded_(std::move(expandedVector))
{
    assert(vectorOfCostsExpanded_.size() % 2 == 1);
    assert(vectorOfCostsExpanded_.size() == 2 * static_cast<std::size_t>(maximumCableCapacity_) + 1);
    assert(IsIncreasingFromMidpoint(vectorOfCostsExpanded_));

    const auto mid = vectorOfCostsExpanded_.size() / 2;
    isGivenByRealCables_ = (maximumCableCapacity_ > 0)
                           && (vectorOfCostsExpanded_[mid] == 0)
                           && IsSymmetric(vectorOfCostsExpanded_);

    if (isGivenByRealCables_) {
        ExtractCableCosts();
    }
}

std::size_t Cabletypes::NumberOfCables() const {
    return numberOfCables_;
}

int Cabletypes::MaximumCapacity() const {
    return maximumCableCapacity_;
}

int Cabletypes::CostOfFlow(int flow) const {
    if (flow < -MaximumCapacity() || flow > MaximumCapacity()) { return INFINITE_COST; }
    return vectorOfCostsExpanded_[vectorOfCostsExpanded_.size() / 2 + flow];
}

bool Cabletypes::IsStrictlyIncreasing(const std::vector<int>& v) {
    for (unsigned int i = 1; i < v.size(); ++i) {
        if (v[i - 1] >= v[i]) {
            return false;
        }
    }
    return true;
}

bool Cabletypes::IsIncreasingFromMidpoint(const std::vector<int>& expandedCosts) {
    assert(expandedCosts.size() % 2 == 1);
    const unsigned int mid = expandedCosts.size() / 2;

    for (unsigned int k = 0; k < mid; ++k) {
        assert(mid + k + 1 < expandedCosts.size());
        if (expandedCosts[mid + k] > expandedCosts[mid + k + 1]) {
            return false;
        }
        if (expandedCosts[mid - k] > expandedCosts[mid - k - 1]) {
            return false;
        }
    }
    return true;
}

bool Cabletypes::IsSymmetric(const std::vector<int>& expandedCosts) {
    assert(expandedCosts.size() % 2 == 1);
    const unsigned int mid = expandedCosts.size() / 2;

    for (unsigned int k = 1; k <= mid; ++k) {
        assert(mid + k < expandedCosts.size());
        if (expandedCosts[mid + k] != expandedCosts[mid - k]) {
            return false;
        }
    }
    return true;
}

void Cabletypes::FillExpandedCosts() {
    assert(vectorOfCostsExpanded_.size() == 2 * static_cast<std::size_t>(maximumCableCapacity_) + 1);

    int currentStep = 0;
    const int mid = vectorOfCostsExpanded_.size() / 2;
    vectorOfCostsExpanded_[mid] = 0;

    for (int i = 1; i <= mid; i++) {
        if (i > vectorOfCapacities_[currentStep]) {
            currentStep++;
        }
        vectorOfCostsExpanded_[mid + i] = vectorOfCosts_[currentStep];
        vectorOfCostsExpanded_[mid - i] = vectorOfCosts_[currentStep];
    }
}

void Cabletypes::ExtractCableCosts() {
    int previousCosts = 0;
    const int mid = vectorOfCostsExpanded_.size() / 2;
    for (int k = 1; k <= mid; ++k) {
        int currentCosts = vectorOfCostsExpanded_[mid + k];
        if (currentCosts != previousCosts) {
            vectorOfCapacities_.push_back(k);
            vectorOfCosts_.push_back(currentCosts);
            previousCosts = currentCosts;
        }
    }
    vectorOfCosts_.shrink_to_fit();
    vectorOfCapacities_.shrink_to_fit();

    assert(vectorOfCapacities_.size() == vectorOfCosts_.size());

    numberOfCables_ = vectorOfCosts_.size();
}

} // namespace DataStructures

} // namespace wcp
