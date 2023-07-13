#include <clocale>
#include <iostream>
#include <memory>

#include <QCoreApplication>

#include "Builders/AlgorithmControler.h"
#include "Builders/NCCControler.h"

#include "DataStructures/InputInformation.h"
#include "DataStructures/Instance.h"
#include "DataStructures/OutputInformation.h"

#include "IO/ParserFunctions.h"

std::unique_ptr<wcp::Builders::AlgorithmControler> DecideOnAlgorithm(const wcp::DataStructures::InputInformation& inputInformation) {
    if (inputInformation.algorithm_.endsWith("NCC")) {
        return std::make_unique<wcp::Builders::NCCControler>(inputInformation.algorithmStdString_);
    } else {
        throw "Unknown algorithm \"" + inputInformation.algorithm_.toStdString() + "\"";
        return std::make_unique<wcp::Builders::AlgorithmControler>("");
    }
}

void PrintGeneralInformationOnAlgorithmAndInstance(const wcp::DataStructures::OutputInformation& outputInfo, const std::string& algorithmName, wcp::DataStructures::Instance& instance) {
    if (outputInfo.ShouldBeVerbose()) {
        outputInfo.VerboseStream()  << "Run algorithm '" << algorithmName
                                    << "' on input from testset " << instance.TestsetNumber()
                                    << " instance " << instance.InstanceNumber()
                                    << ", which has " << instance.NumberOfTurbines()
                                    << " turbines and " << instance.NumberOfSubstations()
                                    << " substations. The cable types (capacity, cost) are";
        for (size_t i = 0; i < instance.CableTypes().NumberOfCables(); i++) {
            outputInfo.VerboseStream() << " (" << instance.CableTypes().CapacityBreakpoints()[i]
                                       << "," << instance.CableTypes().CostBreakpoints()[i] << ")";
        }
        outputInfo.VerboseStream() << ".\n";
    }
}

int main(int argc, char *argv[]) {
    try {
        auto application = std::make_unique<QCoreApplication>(argc, argv);
        QCoreApplication::setApplicationName("Windfarm Cabling Problem");
        QCoreApplication::setApplicationVersion("Version: 1");
        std::setlocale(LC_NUMERIC, "C");

        std::unique_ptr<wcp::DataStructures::InputInformation> allCommandLineInput = wcp::IO::SetUpParserAndProcessCommandLineInput(*application);

        wcp::DataStructures::OutputInformation outputInformation(allCommandLineInput->outputdir_.toStdString(),
                                                 allCommandLineInput->verbose_,
                                                 allCommandLineInput->printSummary_,
                                                 allCommandLineInput->printDetails_,
                                                 allCommandLineInput->printSolution_);

        wcp::DataStructures::Instance instance(allCommandLineInput->testsetNumber_, allCommandLineInput->instanceNumber_);
        instance.ReadFromGMLFile(allCommandLineInput->inputfile_.toStdString());

        std::unique_ptr<wcp::Builders::AlgorithmControler> algorithmControler = DecideOnAlgorithm(*allCommandLineInput);
        PrintGeneralInformationOnAlgorithmAndInstance(outputInformation, algorithmControler->AlgorithmName(), instance);
        algorithmControler->BuildAndRunAndOutput(instance, *allCommandLineInput, outputInformation);

        return 0;
    } catch (const std::string& message) {
        std::cerr << message << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unexpected error. Terminate." << std::endl;
        return 1;
    }
}
