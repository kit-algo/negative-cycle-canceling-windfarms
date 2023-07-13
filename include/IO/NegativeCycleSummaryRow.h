#ifndef IO_NEGATIVECYCLESUMMARYROW
#define IO_NEGATIVECYCLESUMMARYROW

#include <fstream>
#include <iomanip>

#include "Auxiliary/Constants.h"
#include "Auxiliary/ReturnCodes.h"

namespace wcp {

namespace IO {

/**
 * A struct that contains all information necessary for a summary of a run
 * of a Negative Cycle Canceling algorithm.
 */
struct NegativeCycleSummaryRow {
    std::string Name = "";
    std::size_t Testset = 0;
    std::size_t Instance = 0;
    std::string InitializationStrategy = "";
    std::string DeltaStrategy = "";
    std::string DetectionStrategy = "";

    std::size_t NumberOfTurbines = 0;
    std::size_t NumberOfSubstations = 0;
    std::size_t NumberOfEdges = 0;

    ReturnCode Status = ReturnCode::IN_PROGRESS;
    double BestSolution = Auxiliary::DINFTY;
    double InitialSolution = Auxiliary::DINFTY;

    std::size_t NumberOfDetectionRuns = 0;
    std::size_t NumberOfRelaxedEdges = 0;
    std::size_t NumberOfNegativeCycles = 0;

    std::size_t NumberOfSimpleCycles = 0;
    std::size_t NumberOfEdgesSimpleCycles = 0;
    std::size_t NumberOfEdgesOnCanceledCycles = 0;

    double GlobalElapsedMilliseconds = 0;
    double GlobalElapsedMillisecondsInitialFlowTime = 0;
    double GlobalElapsedMillisecondsNegativeCycleDeletion = 0;

    static void Header(std::ostream& os) {
        os  << "Name"                                           << ","
            << "Testset"                                        << ","
            << "Instance"                                       << ","
            << "InitializationStrategy"                         << ","
            << "DeltaStrategy"                                  << ","
            << "DetectionStrategy"                              << ","

            << "NumberOfTurbines"                               << ","
            << "NumberOfSubstations"                            << ","
            << "NumberOfEdges"                                  << ","
            << "NumberOfRelaxedEdges"                           << ","

            << "NumberOfEdgesOnCanceledCycles"                  << ","
            << "Status"                                         << ","
            << "BestSolution"                                   << ","
            << "InitialSolution"                                << ","

            << "NumberOfDetectionRuns"                          << ","
            << "NumberOfNegativeCycles"                         << ","

            << "GlobalElapsedMilliseconds"                      << ","
            << "GlobalElapsedMillisecondsInitialFlowTime"       << ","
            << "GlobalElapsedMillisecondsNegativeCycleDeletion" << "\n";
    }

    void Content(std::ostream& os) const {
        os  << Name                                             << ","
            << Testset                                          << ","
            << Instance                                         << ","
            << InitializationStrategy                           << ","
            << DeltaStrategy                                    << ","
            << DetectionStrategy                                << ","

            << NumberOfTurbines                                 << ","
            << NumberOfSubstations                              << ","
            << NumberOfEdges                                    << ","
            << NumberOfRelaxedEdges                             << ","

            << NumberOfEdgesOnCanceledCycles                    << ","

            << Status                                           << ","
            << BestSolution                                     << ","
            << InitialSolution                                  << ","

            << NumberOfDetectionRuns                            << ","
            << NumberOfNegativeCycles                           << ","

            << GlobalElapsedMilliseconds                        << ","
            << GlobalElapsedMillisecondsInitialFlowTime         << ","
            << GlobalElapsedMillisecondsNegativeCycleDeletion   << "\n";
    }

    NegativeCycleSummaryRow& operator+=(const NegativeCycleSummaryRow& rhs) {
        NumberOfTurbines                                += rhs.NumberOfTurbines;
        NumberOfSubstations                             += rhs.NumberOfSubstations;
        NumberOfEdges                                   += rhs.NumberOfEdges;

        NumberOfEdgesOnCanceledCycles                   += rhs.NumberOfEdgesOnCanceledCycles;
        BestSolution                                    += rhs.BestSolution;
        InitialSolution                                 += rhs.InitialSolution;

        NumberOfDetectionRuns                           += rhs.NumberOfDetectionRuns;
        NumberOfNegativeCycles                          += rhs.NumberOfNegativeCycles;

        GlobalElapsedMilliseconds                       += rhs.GlobalElapsedMilliseconds;
        GlobalElapsedMillisecondsInitialFlowTime        += rhs.GlobalElapsedMillisecondsInitialFlowTime;
        GlobalElapsedMillisecondsNegativeCycleDeletion  += rhs.GlobalElapsedMillisecondsNegativeCycleDeletion;

        return *this;
    }

    void Dump(const std::string& fileName, bool beVerbose, std::ostream& verboseStream = std::cout, bool overwrite = true) const {
        if (beVerbose) {
            verboseStream << "Print summary to: " << fileName << std::endl;
        }

        // Open output stream.
        auto mode = overwrite ? std::ofstream::trunc : std::ofstream::app;
        std::ofstream f(fileName, mode);
        if (!f.is_open()) return;

        // Write header if file is empty
        f.seekp(0, std::ios::end);
        if (f.tellp() == 0) {
            NegativeCycleSummaryRow::Header(f);
        }

        Content(f);
    }

    void Dump(std::ostream& os) const {
        os << "\n"
           << "Statistics:" << "\n"
           << "-----------" << "\n"
           << std::setprecision(2)
           << std::fixed
           << std::setw(27) << "Name: "                      << std::setw(10) << Name                                            << ", "      << "\n"
           << std::setw(27) << "Testset: "                   << std::setw(10) << Testset                                         << ", "      << "\n"
           << std::setw(27) << "Instance: "                  << std::setw(10) << Instance                                        << ", "      << "\n"
           << std::setw(27) << "Initialization Strategy: "   << std::setw(10) << InitializationStrategy                          << ", "      << "\n"
           << std::setw(27) << "Delta Strategy: "            << std::setw(10) << DeltaStrategy                                   << ", "      << "\n"
           << std::setw(27) << "Detection Strategy: "        << std::setw(10) << DetectionStrategy                               << ", "      << "\n"
           << std::setw(27) << "Number of Turbines: "        << std::setw(10) << NumberOfTurbines                                << ", "      << "\n"
           << std::setw(27) << "Number of Substations: "     << std::setw(10) << NumberOfSubstations                             << ", "      << "\n"
           << std::setw(27) << "Number of Edges: "           << std::setw(10) << NumberOfEdges                                   << ", "      << "\n"

           << std::setw(27) << "Total Time: "                << std::setw(10) << GlobalElapsedMilliseconds                       << " ms, "   << "\n"
           << std::setw(27) << "Initial Flow Time: "         << std::setw(10) << GlobalElapsedMillisecondsInitialFlowTime        << " ms, "   << "\n"
           << std::setw(27) << "Cycle Deletion Time: "       << std::setw(10) << GlobalElapsedMillisecondsNegativeCycleDeletion  << " ms, "   << "\n"

           << std::setw(27) << "Status Code: "               << std::setw(10) << Status                                          << ", "      << "\n"
           << std::setw(27) << "Best Solution: "             << std::setw(10) << BestSolution                                    << ", "      << "\n"
           << std::setw(27) << "Initial Solution: "          << std::setw(10) << InitialSolution                                 << ", "      << "\n"
           << std::setw(27) << "Number of Detection Runs: "  << std::setw(10) << NumberOfDetectionRuns                           << ", "      << "\n"
           << std::setw(27) << "Number of Negative Cycles: " << std::setw(10) << NumberOfNegativeCycles                          << ", "      << "\n"
           << std::setw(27) << "Edges on Canceled Cycles: "  << std::setw(10) << NumberOfEdgesOnCanceledCycles                   << ", "      << "\n"
           << std::setw(27) << "Relaxed Edges: "             << std::setw(10) << NumberOfRelaxedEdges                            << ", "      << "\n"
           << "\n";
    }
};

} // namespace IO

} // namespace wcp

#endif
