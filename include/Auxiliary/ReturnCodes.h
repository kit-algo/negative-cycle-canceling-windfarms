#ifndef AUXILIARY_RETURNCODES
#define AUXILIARY_RETURNCODES

#include <ostream>

namespace wcp {

enum class ReturnCode {
    SUCCESS                     = 0,
    INFEASIBLE                  = 3,
    ITERATION_LIMIT             = 7,
    TIME_LIMIT                  = 9,
    IN_PROGRESS                 = 14,
    NO_FEASIBLE_SOLUTION_FOUND  = 16,
    EARLY_TERMINATION           = 17,
    BOUND_TO_TERMINATE          = 18,
    NOTHING_CHANGED             = 19
};

inline std::ostream& operator<<(std::ostream& stream, ReturnCode code) {
    return stream << static_cast<int>(code);
}

} // namespace wcp

#endif
