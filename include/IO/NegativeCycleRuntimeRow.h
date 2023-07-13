#ifndef IO_NEGATIVECYCLERUNTIMEROW
#define IO_NEGATIVECYCLERUNTIMEROW

#include <algorithm>
#include <iostream>
#include <vector>

namespace wcp {

namespace IO {

/**
 * A struct that contains all information from a (half-)iteration
 * of a Negative Cycle Canceling algorithm.
 */
struct NegativeCycleRuntimeRow {
    std::string NameOfProblem = "NegativeCycleCanceling";
    std::string Name = "";

    std::size_t Testset = 0;
    std::size_t Instance = 0;
    std::string CurrentProcedure = "";

    std::size_t WalkId = 0;

    double Solution = 0.;
    double ChangeInCost = 0.;
    double GlobalElapsedMilliseconds = 0.;

    std::size_t NumberOfEdgesPerCycle = 0;
    unsigned int Delta = 0;

    static void Header(std::ostream& os) {
        os << "NameOfProblem"              << ","
           << "Name"                       << ","

           << "Testset"                    << ","
           << "Instance"                   << ","

           << "CurrentProcedure"           << ","

           << "WalkId"                     << ","

           << "Solution"                   << ","
           << "ChangeInCost"               << ","
           << "GlobalElapsedMilliseconds"  << ","

           << "NumberOfEdgesPerCycle"      << ","
           << "Delta"                      << "\n";
    }

    void Content(std::ostream& os) const {
        os << NameOfProblem                << ","
           << Name                         << ","

           << Testset                      << ","
           << Instance                     << ","

           << CurrentProcedure             << ","

           << WalkId                       << ","

           << Solution                     << ","
           << ChangeInCost                 << ","
           << GlobalElapsedMilliseconds    << ","

           << NumberOfEdgesPerCycle        << ","
           << Delta                        << "\n";
    }

    NegativeCycleRuntimeRow& operator+=(const NegativeCycleRuntimeRow& rhs) {
        Solution                  += rhs.Solution;
        ChangeInCost              += rhs.ChangeInCost;
        GlobalElapsedMilliseconds += rhs.GlobalElapsedMilliseconds;

        NumberOfEdgesPerCycle     += rhs.NumberOfEdgesPerCycle;
        Delta                     += rhs.Delta;

        return *this;
    }

    NegativeCycleRuntimeRow& operator/= (int rhs) {
        Solution                  /= rhs;
        ChangeInCost              /= rhs;
        GlobalElapsedMilliseconds /= rhs;

        NumberOfEdgesPerCycle     /= rhs;
        Delta                     /= rhs;

        return *this;
    }

    void Min(const NegativeCycleRuntimeRow& rhs) {
        Solution                  = std::min(Solution, rhs.Solution);
        ChangeInCost              = std::min(ChangeInCost, rhs.ChangeInCost);
        GlobalElapsedMilliseconds = std::min(GlobalElapsedMilliseconds, rhs.GlobalElapsedMilliseconds);

        NumberOfEdgesPerCycle     = std::min(NumberOfEdgesPerCycle, rhs.NumberOfEdgesPerCycle);
        Delta                     = std::min(Delta, rhs.Delta);
    }

    void Max(const NegativeCycleRuntimeRow& rhs) {
        Solution                   = std::max(Solution, rhs.Solution);
        ChangeInCost               = std::max(ChangeInCost, rhs.ChangeInCost);
        GlobalElapsedMilliseconds  = std::max(GlobalElapsedMilliseconds, rhs.GlobalElapsedMilliseconds);

        NumberOfEdgesPerCycle      = std::max(NumberOfEdgesPerCycle, rhs.NumberOfEdgesPerCycle);
        Delta                      = std::max(Delta, rhs.Delta);
    }
};

} // namespace IO

} // namespace wcp

#endif
