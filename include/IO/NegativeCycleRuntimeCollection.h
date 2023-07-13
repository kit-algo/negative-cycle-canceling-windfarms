#ifndef IO_NEGATIVECYCLERUNTIMECOLLECTION
#define IO_NEGATIVECYCLERUNTIMECOLLECTION

#include <string>
#include <iostream>
#include <iomanip>
#include <vector>

#include "IO/NegativeCycleRuntimeRow.h"
#include "IO/NegativeCycleSummaryRow.h"

namespace wcp {

namespace IO {

/**
 * A struct that contains all information from a Negative Cycle Canceling algorithm.
 *
 * It contains copies of the NegativeCycleRuntimeRow obtained at various points in the algorithm.
 */
class NegativeCycleRuntimeCollection {
public:
    NegativeCycleRuntimeCollection() : information_() { };

    NegativeCycleRuntimeCollection& operator+=(const NegativeCycleRuntimeRow& rhs) {
        information_.push_back(rhs);
        return *this;
    }

    void AddMeta(std::string name, std::size_t testset, std::size_t instance) {
      for (auto& info : information_) {
        info.Name     = name;
        info.Testset  = testset;
        info.Instance = instance;
      }
    }

    void AddMeta(const NegativeCycleSummaryRow& summary) {
        AddMeta(summary.Name, summary.Testset, summary.Instance);
    }

    void ComputeStatistics() {
        if (information_.empty()) {
            return;
        }

        avg = information_[0];
        min = information_[0];
        max = information_[0];
        for (std::size_t i = 1; i < information_.size(); ++i) {
            avg += information_[i];
            min.Min(information_[i]);
            max.Max(information_[i]);
        }
        avg /= information_.size();
    }

    void Dump(std::ostream& os) {
        if (information_.empty()) {
            return;
        }

        ComputeStatistics();
        os  << "\n"
            << "Statistics:" << "\n"
            << "-----------" << "\n"

            << std::setprecision(2)
            << std::fixed

            << std::setw(34) << "Problem: "             << std::setw(8)  << information_[0].NameOfProblem    << ", "     << "\n"
            << std::setw(34) << "Name: "                << std::setw(8)  << information_[0].Name             << ", "     << "\n"

            << std::setw(34) << "Testset: "             << std::setw(8)  << information_[0].Testset          << ", "     << "\n"
            << std::setw(34) << "Instance: "            << std::setw(8)  << information_[0].Instance         << ", "     << "\n"

            << std::setw(34) << "Current Procedure: "   << std::setw(8)  << information_[0].CurrentProcedure << ", "     << "\n"

            << std::setw(34) << "Walk ID: "             << std::setw(8)  << information_[0].WalkId           << ", "     << "\n"

            << std::setw(34) << "Total time: "          << std::setw(8)  << avg.GlobalElapsedMilliseconds    << std::setw(11) << " ms (avg), "
                                                        << std::setw(8)  << min.GlobalElapsedMilliseconds    << std::setw(11) << " ms (min), "
                                                        << std::setw(8)  << max.GlobalElapsedMilliseconds    << std::setw(11) << " ms (max)"    << "\n"

            << std::setw(34) << "Solutions: "           << std::setw(8)  << avg.Solution                     << std::setw(11) << " (avg), "
                                                        << std::setw(8)  << min.Solution                     << std::setw(11) << " (min), "
                                                        << std::setw(8)  << max.Solution                     << std::setw(11) << " (max)"       << "\n"

            << std::setw(34) << "ChangeInCost: "        << std::setw(8)  << avg.ChangeInCost                 << std::setw(11) << " (avg), "
                                                        << std::setw(8)  << min.ChangeInCost                 << std::setw(11) << " (min), "
                                                        << std::setw(8)  << max.ChangeInCost                 << std::setw(11) << " (max)"       << "\n"

            << std::setw(34) << "Number of edges per simple cycle: "
                                                        << std::setw(8)  << avg.NumberOfEdgesPerCycle        << std::setw(11) << " (avg), "
                                                        << std::setw(8)  << min.NumberOfEdgesPerCycle        << std::setw(11) << " (min), "
                                                        << std::setw(8)  << max.NumberOfEdgesPerCycle        << std::setw(11) << " (max)"       << "\n"

            << std::setw(34) << "Delta: "               << std::setw(8)  << avg.Delta                        << std::setw(11) << " (avg), "
                                                        << std::setw(8)  << min.Delta                        << std::setw(11) << " (min), "
                                                        << std::setw(8)  << max.Delta                        << std::setw(11) << " (max)"       << "\n"
            << "\n";
    }

    void Dump(const std::string& fileName, bool beVerbose, std::ostream& verboseStream = std::cout, bool overwrite = true) const {
        if (beVerbose) {
            verboseStream << "Print complete optimization information to: " << fileName << "\n";
        }

        auto mode = overwrite ? std::ofstream::trunc : std::ofstream::app;
      // Open output stream.
        std::ofstream f(fileName, mode);
        if (!f.is_open()) return;

        NegativeCycleRuntimeRow::Header(f);
        for (const auto& info : information_) {
            info.Content(f);
        }
    }

  void PrepareAndCompleteDumpToFile(const std::string& fileName, NegativeCycleSummaryRow& summary, bool beVerbose, std::ostream& verboseStream = std::cout) {
    AddMeta(summary);
    Dump(fileName, beVerbose, verboseStream);
  }

  private:
    std::vector<NegativeCycleRuntimeRow> information_;
    NegativeCycleRuntimeRow min, max, avg;
};

} // namespace IO

} // namespace wcp

#endif
