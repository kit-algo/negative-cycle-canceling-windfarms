#ifndef IO_AUXILIARY
#define IO_AUXILIARY

#include <ostream>
#include <sstream>
#include <vector>

#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/basic/GraphAttributes.h>

namespace wcp {

namespace IO {

enum InputFileFormat {
    GML = 0,
    GRAPHML = 1
};

inline void outputGraphAttributesToGRAPHML(const ogdf::GraphAttributes &graphAttr, const std::string filename, bool beVerbose, std::ostream& verboseStream = std::cout) {
    if (beVerbose) {
        verboseStream << "Print graph with graph attributes to:" << filename << std::endl;
    }
    std::ostringstream gmlOutput;
    gmlOutput << filename;

    ogdf::GraphIO::write(graphAttr, gmlOutput.str(), ogdf::GraphIO::writeGraphML);
}

template<typename T>
inline void outputVectorToStream(const std::string initialMessage, const std::vector<T>& vectorToOutput, const std::string messageAfterVector, bool endWithLineBreak, bool shouldOutput, std::ostream& verboseStream) {
    if (shouldOutput) {
        verboseStream << initialMessage;
        for (T entry : vectorToOutput) {
            verboseStream << entry << " ";
        }
        if (!messageAfterVector.empty()) { verboseStream << '\n' << messageAfterVector; }
        if (endWithLineBreak) { verboseStream << "\n"; }
    }
}

} // namespace IO

} // namespace wcp

#endif
