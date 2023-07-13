#ifndef DATASTRUCTURES_LABELS_ONEDISTANCELABEL
#define DATASTRUCTURES_LABELS_ONEDISTANCELABEL

#include "Auxiliary/Constants.h"

#include "DataStructures/Graphs/WindfarmResidual.h"

namespace wcp {

namespace DataStructures {

/**
 * A label that stores one distance and a parent.
 */
class OneDistanceLabel {
public:
    OneDistanceLabel()
      : dist_(Auxiliary::DINFTY),
        parent_(INVALID_VERTEX) {}

    bool WouldUpdate(double distance, edgeID /* parent */) const {
        return distance < dist_;
    }

    /**
     * Updates the distance if possible.
     *
     * After the update the label contains the smallest distance of the two
     * (the previously stored in the label and distance).
     */
    bool Update(double distance, edgeID parent) {
        if (distance < dist_) {
            parent_ = parent;
            dist_ = distance;
            return true;
        } else {
            return false;
        }
    }

    double Distance(const WindfarmResidual& /* windfarm */,
                    edgeID /* outgoingEdge */) const {
        return dist_;
    }

    edgeID Parent(const WindfarmResidual& /* windfarm */,
                  edgeID /* outgoingEdge */) const {
        return parent_;
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
};

} // namespace DataStructures

} // namespace wcp

#endif
