#ifndef IO_PARSERFUNCTIONS
#define IO_PARSERFUNCTIONS

#include <memory>
#include <stdio.h>

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QRegularExpression>

#include "Auxiliary/Constants.h"
#include "DataStructures/InputInformation.h"

namespace wcp {

namespace IO {

inline void addCommandLineOptionsForNCC(QCommandLineParser& parser) {
    QCommandLineOption initOption(
            "init",
            "Initialization strategy of form '(COLLECTING)?(DIJKSTRA|BFS)(ANY|LAST)' (default: DIJKSTRAANY).",
            "Strategy", "DijkstraAny"
    );
    parser.addOption(initOption);

    QCommandLineOption deltaOption(
            "delta",
            "Delta strategy of form '*(STAY)?(INC|DEC|INCDEC|RANDOM)' (default: INC).",
            "Strategy", "INC"
    );
    parser.addOption(deltaOption);

    QCommandLineOption detectOption(
            "detect",
            "Negative cycle detection algorithm variant: 'STFBF' (default) or 'BF'.",
            "Strategy", "STFBF"
    );
    parser.addOption(detectOption);

    QCommandLineOption iterationLimitOption(
        QStringList() << "l" << "iterations",
        "Specify maximum number of iterations (default: infinity).",
        "Limit"
    );
    parser.addOption(iterationLimitOption);

    QCommandLineOption escapeLeavesOption(
            "escapeLeaves",
            "Weight of escaping strategy: Move Leaf (default: 0).",
            "Weight", "0"
    );
    parser.addOption(escapeLeavesOption);

    QCommandLineOption escapeFreeCablesOption(
            "escapeFreeCables",
            "Weight of escaping strategy: Free Cable Upgrade (default: 0).",
            "Weight", "0"
    );
    parser.addOption(escapeFreeCablesOption);

    QCommandLineOption escapeBonbonOption(
            "escapeBonbon",
            "Weight of escaping strategy: Cancel Bonbon (default: 0).",
            "Weight", "0"
    );
    parser.addOption(escapeBonbonOption);
}

inline void addCommandLineOptions(QCommandLineParser& parser) {
    parser.setApplicationDescription("Wind Farm Cabling");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption algorithmOption(
        QStringList() << "a" << "algorithm", 
        "Choose which algorithm to run: NCC (default) or EscapingNCC.",
        "NameOfAlgorithm", "NCC"
    );
    parser.addOption(algorithmOption);

    QCommandLineOption inputFileOption(
        QStringList() << "f" << "inputFile", 
        "Specify the instance on which to run the algorithm (default: testset-0-instance-1.gml).",
        "FileName", "testset-0-instance-1.gml"
    );
    parser.addOption(inputFileOption);

    QCommandLineOption outputDirOption(
        QStringList() << "o" << "output", 
        "Specify the directory where output files are written (default: ./). Make sure to end with a separator, i.e. '\\' or '/'",
        "Path", "./"
    );
    parser.addOption(outputDirOption);

    QCommandLineOption timeLimitOption(
        QStringList() << "z" << "time", 
        "Specify the maximum running time in seconds for the algorithm (default: no time limit).",
        "LimitInSeconds"
    );
    parser.addOption(timeLimitOption);

    QCommandLineOption seedOption(
            "seed",
            QCoreApplication::translate("main", "Specify an integer for the random seed (default: 0)."),
            QCoreApplication::translate("main", "integer"), "0"
    );
    parser.addOption(seedOption);

    parser.addOption({"verbose", "Verbose mode. Prints out real-time information from the algorithm to standard-out."});
    parser.addOption({"print-solution", "Write a file with the best solution."});
    parser.addOption({"print-summary", "Add a summary line of information after termination to an overview file."});
    parser.addOption({"print-details", "Write a file with step-by-step optimization information."});

    addCommandLineOptionsForNCC(parser);
}

inline void ProcessNCCInput(QCommandLineParser& parser,
                            DataStructures::InputInformation& inputInfo) {
    if (parser.isSet("iterations")) {
        inputInfo.isIterationLimitSet_ = true;
        inputInfo.iterationLimit_ = parser.value("iterations").toUInt();
    } else {
        inputInfo.isIterationLimitSet_ = false;
        inputInfo.iterationLimit_ = std::numeric_limits<unsigned int>::max();
    }

    inputInfo.initialisationStrategy_ = parser.value("init").toUpper();
    inputInfo.deltaStrategy_ = parser.value("delta").toUpper();
    inputInfo.detectionStrategy_ = parser.value("detect").toUpper();

    bool weightConversionWorked = true;
    bool didAllconversionWork = true;
    inputInfo.weightEscapeLeaves_ = parser.value("escapeLeaves").toUInt(&weightConversionWorked);
    didAllconversionWork &= weightConversionWorked;
    inputInfo.weightEscapeBonbon_ = parser.value("escapeBonbon").toUInt(&weightConversionWorked);
    didAllconversionWork &= weightConversionWorked;
    inputInfo.weightEscapeFreeCables_ = parser.value("escapeFreeCables").toUInt(&weightConversionWorked);
    didAllconversionWork &= weightConversionWorked;

    if (!didAllconversionWork) {
        throw std::string("Could not identify weight for an escaping strategy.");
    }
}

inline void ProcessCommandLineInput(const QCoreApplication& application,
                                    QCommandLineParser& parser,
                                    DataStructures::InputInformation& inputInfo) {
    parser.process(application);

    const QStringList args = parser.optionNames();
    if (args.size() < 1) {
        fprintf(stderr, "%s\n", "Error: Must specify an argument.");
        parser.showHelp(1);
        exit(1);
    }

    inputInfo.outputdir_  = parser.value("output");
    inputInfo.algorithm_   = parser.value("algorithm").toUpper();
    inputInfo.algorithmStdString_ = inputInfo.algorithm_.toStdString();
    inputInfo.isTimeLimitSet_ = parser.isSet("time");
    if (inputInfo.isTimeLimitSet_) {
        inputInfo.timeLimitInSeconds_ = parser.value("time").toDouble();
    } else {
        inputInfo.timeLimitInSeconds_ = Auxiliary::DINFTY;
    }

    if (inputInfo.timeLimitInSeconds_ <= 0) {
        fprintf(stderr, "%s\n", "Error: Non-positive time limit is invalid.");
        exit(1);
    }

    if (parser.isSet("inputFile")) {
        inputInfo.inputfile_  = parser.value("inputFile");

        QRegularExpression rx("-(\\d+)");

        QList<int> list;
        QRegularExpressionMatchIterator i = rx.globalMatch(parser.value("inputFile"));
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            QString word = match.captured(1);
            list << word.toInt();
        }
        if (list.size() >= 2) {
            inputInfo.testsetNumber_ = list[0];
            inputInfo.instanceNumber_ = list[1];
        } else {
            fprintf(stderr, "%s\n", "Warning: Could not identify testset or instance number. Inserting standard values ...");
            if (list.size() == 1) {
                inputInfo.testsetNumber_ = list[0];
                inputInfo.instanceNumber_ = Auxiliary::INFTY;
            } else {
                inputInfo.testsetNumber_ = Auxiliary::INFTY;
                inputInfo.instanceNumber_ = Auxiliary::INFTY;
            }
        }
    } else {
        fprintf(stderr, "%s\n", "Error: Must specify an input file.");
        parser.showHelp(1);
        exit(1);
    }

    inputInfo.randomSeed_ = parser.value("seed").toInt();

    inputInfo.verbose_ = parser.isSet("verbose");
    inputInfo.printSolution_ = parser.isSet("print-solution");
    inputInfo.printSummary_ = parser.isSet("print-summary");
    inputInfo.printDetails_ = parser.isSet("print-details");

    ProcessNCCInput(parser, inputInfo);
}

inline std::unique_ptr<DataStructures::InputInformation> SetUpParserAndProcessCommandLineInput(const QCoreApplication& application) {
    DataStructures::InputInformation inputInfo;

    QCommandLineParser parser;
    addCommandLineOptions(parser);

    ProcessCommandLineInput(application, parser, inputInfo);

    return std::make_unique<DataStructures::InputInformation>(inputInfo);
}

} // namespace IO

} // namespace wcp

#endif
