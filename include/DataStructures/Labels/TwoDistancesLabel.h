#ifndef DATASTRUCTURES_LABELS_TWODISTANCELABEL
#define DATASTRUCTURES_LABELS_TWODISTANCELABEL

#include "Auxiliary/Constants.h"

#include "DataStructures/Graphs/WindfarmResidual.h"

namespace wcp {

namespace DataStructures {

/**
 * A label that stores two distances that were computed via two different
 * parent edges.
 */
class TwoDistancesLabel {
public:
    TwoDistancesLabel()
      : dist_(Auxiliary::DINFTY),
        parent_(INVALID_VERTEX),
        distSecond_(Auxiliary::DINFTY),
        parentSecond_(INVALID_VERTEX) {}

    /**
     * Returns the distance when the last edge must not be forbiddenParent
     */
    double DistanceForbidding(edgeID forbiddenParent) const {
        return parent_ == forbiddenParent ? distSecond_ : dist_;
    }

    /**
     * Returns the parent when the last edge must not be forbiddenParent
     */
    edgeID ParentForbidding(edgeID forbiddenParent) const {
        return parent_ == forbiddenParent ? parentSecond_ : parent_;
    }

    bool WouldUpdate(double distance, edgeID parent) const {
        return distance < dist_
                || (distance < distSecond_ && parent != parent_);
    }

    /**
     * Updates the distances if possible.
     *
     * After the update the label contains the two smallest distances of the three
     * (the two previously stored in the label and distance)
     * such that they were computed via two different parent edges.
     */
    bool Update(double distance, edgeID parent) {
        if (distance < dist_) {
            if (parent_ == parent) {
                dist_ = distance;
            } else {
                distSecond_ = dist_;
                parentSecond_ = parent_;
                dist_ = distance;
                parent_ = parent;
            }
            return true;
        } else if (distance < distSecond_ && parent_ != parent) {
            distSecond_ = distance;
            parentSecond_ = parent;
            return true;
        }
        return false;
    }

    double Distance(const WindfarmResidual& windfarm,
            edgeID outgoingEdge) const {
        edgeID reverseEdge = windfarm.ReverseEdgeIndex(outgoingEdge);
        return DistanceForbidding(reverseEdge);
    }

    edgeID Parent(const WindfarmResidual& windfarm,
            edgeID outgoingEdge) const {
        edgeID reverseEdge = windfarm.ReverseEdgeIndex(outgoingEdge);
        return ParentForbidding(reverseEdge);
    }

    double FirstDistance() const {
        return dist_;
    }

    edgeID FirstParent() const {
        return parent_;
    }

private:
    double dist_;
    edgeID parent_;
    double distSecond_;
    edgeID parentSecond_;
};

} // namespace DataStructures

} // namespace wcp

#endif
