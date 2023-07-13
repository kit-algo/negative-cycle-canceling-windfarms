#ifndef BUILDERS_ALGORITHMCONTROLER
#define BUILDERS_ALGORITHMCONTROLER

#include <string>

#include "DataStructures/InputInformation.h"
#include "DataStructures/Instance.h"
#include "DataStructures/OutputInformation.h"

namespace wcp {

namespace Builders {

/**
 * A base builder class from which builders for algorithms can be derived.
 */
class AlgorithmControler {
public:
    AlgorithmControler(const std::string& algorithmName)
      : algorithmName_("WCP-" + algorithmName) {};

    /**
     * This method takes care of everything.
     */
    virtual void BuildAndRunAndOutput(  DataStructures::Instance& /*instance*/,
                                        DataStructures::InputInformation& /*inputInformation*/,
                                        const DataStructures::OutputInformation& outputInformation) {
        if (outputInformation.ShouldBeVerbose()) {
            outputInformation.VerboseStream() << "Nothing to do. Terminate." <<std::endl;
        }
    }

    const std::string& AlgorithmName() const { return algorithmName_; }
protected:
    const std::string algorithmName_;

    std::string testsetInstanceString(const DataStructures::Instance& instance) {
        return "-t-" + std::to_string(instance.TestsetNumber())
                + "-i-" + std::to_string(instance.InstanceNumber());
    }

    std::string longTestsetInstanceString(const DataStructures::Instance& instance) {
        return "-testset-" + std::to_string(instance.TestsetNumber())
                + "-instance-" + std::to_string(instance.InstanceNumber());
    }
};

} // namespace Builders

} // namespace wcp

#endif
