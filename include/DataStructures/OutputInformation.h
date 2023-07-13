#ifndef DATASTRUCTURES_OUTPUTINFORMATION
#define DATASTRUCTURES_OUTPUTINFORMATION

#include <ostream>

namespace wcp {

namespace DataStructures {

/**
 * This struct wraps all output related specifications
 */
struct OutputInformation {
public:
    OutputInformation(const std::string& outputPath)
      : OutputInformation(outputPath, true, true, true, true)
    {}

    OutputInformation(  const std::string& outputPath,
                        bool verbose,
                        bool printSummary,
                        bool printDetails,
                        bool printSolution,
                        std::ostream& verboseStream = std::cout)
      : outputPath_(outputPath),
        verbose_(verbose),
        printSummary_(printSummary),
        printDetails_(printDetails),
        printSolution_(printSolution),
        verboseStream_(verboseStream) {}

    const std::string& OutputPath() const { return outputPath_; }

    bool ShouldBeVerbose() const { return verbose_; }

    bool ShouldPrintSummary() const { return printSummary_; }

    bool ShouldPrintDetails() const { return printDetails_; }

    bool ShouldPrintSolution() const { return printSolution_; }

    std::ostream& VerboseStream() const { return verboseStream_; }
private:
    std::string outputPath_;

    bool verbose_;
    bool printSummary_;
    bool printDetails_;
    bool printSolution_;

    std::ostream& verboseStream_;
};

} // namespace DataStructures

} // namespace wcp

#endif
