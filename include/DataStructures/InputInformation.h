#ifndef DATASTRUCTURES_INPUTINFORMATION
#define DATASTRUCTURES_INPUTINFORMATION

#include <QString>

namespace wcp {

namespace DataStructures {

/**
 * This struct wraps all user input
 */
struct InputInformation {
public:
    QString inputfile_;
    QString outputdir_;
    QString algorithm_;
    std::string algorithmStdString_;

    int testsetNumber_;
    int instanceNumber_;

    bool isTimeLimitSet_;
    double timeLimitInSeconds_;

    int randomSeed_;

    // Negative Cycle Canceling input
    QString initialisationStrategy_;
    QString deltaStrategy_;
    QString detectionStrategy_;

    bool isIterationLimitSet_;
    unsigned int iterationLimit_;

    unsigned int weightEscapeLeaves_;
    unsigned int weightEscapeFreeCables_;
    unsigned int weightEscapeBonbon_;

    // Output Control
    bool verbose_;
    bool printSolution_;
    bool printSummary_;
    bool printDetails_;
};

} // namespace DataStructures

} // namespace wcp

#endif
