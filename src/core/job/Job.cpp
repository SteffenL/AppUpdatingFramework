#include "Job.h"
#include "StepFactory.h"

// Serialization
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>

#include <sstream>
#include <cassert>
#include <limits>

namespace aufw { namespace job {

Job::Job() : m_stepIndex(0) {}

bool Job::Execute() {
    bool isOk = true;
    m_stepIndex = (m_stepIndex < 0) ? 0 : m_stepIndex;
    // Start with the last attempted step
    for (int i = m_stepIndex, count = m_steps.size(); i < count;) {
        Step& step = *m_steps[i];

        try {
            step.Execute();
            ++i;
            onExecutionSuccess();
        }
        catch (std::exception& ex) {
            isOk = false;
            JobFailureArg arg;
            arg.Exception = &ex;
            onExecutionFailed(arg);
            if (!arg.ShouldRetry) {
                Rollback();
                break;
            }

            continue;
        }
        catch (...) {
            isOk = false;
            JobFailureArg arg;
            onExecutionFailed(arg);
            if (!arg.ShouldRetry) {
                Rollback();
                break;
            }

            continue;
        }

        m_stepIndex = i;
    }

    return isOk;
}

void Job::Rollback() {
    // Start with the last completed step
    for (int i = std::min<int>(m_stepIndex, m_steps.size()) - 1; i >= 0;) {
        Step& step = *m_steps[i];

        try {
            step.Rollback();
            --i;
            onRollbackSuccess();
        }
        catch (std::exception& ex) {
            JobFailureArg arg;
            arg.Exception = &ex;
            onRollbackFailed(arg);
            if (!arg.ShouldRetry) {
                break;
            }

            continue;
        }
        catch (...) {
            JobFailureArg arg;
            onRollbackFailed(arg);
            if (!arg.ShouldRetry) {
                break;
            }

            continue;
        }

        m_stepIndex = i;
    }
}

void Job::AddStep(Step* step) {
    assert(step != nullptr);
    if (!step) {
        throw std::logic_error("Step cannot be null");
    }

    step->OnProgress = [&](const Step& step_) {
        StepProgressArg arg;
        arg.Job = this;
        arg.Step = &step_;
        onStepProgress(arg);
    };

    m_steps.push_back(std::unique_ptr<Step>(step));
}

void Job::onExecutionFailed(JobFailureArg& arg) {
    if (OnExecutionFailed) {
        OnExecutionFailed(arg);
    }
}

void Job::onRollbackFailed(JobFailureArg& arg) {
    if (OnRollbackFailed) {
        OnRollbackFailed(arg);
    }
}

void Job::onExecutionSuccess() {
    if (OnExecutionSuccess) {
        OnExecutionSuccess();
    }
}

void Job::onRollbackSuccess() {
    if (OnRollbackSuccess) {
        OnRollbackSuccess();
    }
}

void Job::onStepProgress(StepProgressArg& arg) {
    if (OnStepProgress) {
        OnStepProgress(arg);
    }
}

struct StepSerializer {
    Step& m_step;

    StepSerializer(Step& step) : m_step(step) {}

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        m_step.Serialize(ar);
    }
};

struct StepItemsSerializer {
    std::vector<std::unique_ptr<Step>>& m_steps;
    int m_stepIndex;
    std::vector<std::string> m_stepTypes;

    StepItemsSerializer(std::vector<std::unique_ptr<Step>>& steps, int stepIndex, std::vector<std::string> stepTypes)
        : m_steps(steps), m_stepIndex(stepIndex), m_stepTypes(stepTypes) {}

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template<class Archive>
    void save(Archive& ar, const unsigned int version) const {
        using boost::serialization::make_nvp;

        for (int i = 0; i < m_stepIndex; ++i) {
            Step& step = *m_steps[i];
            ar & make_nvp("Step", StepSerializer(step));
        }
    }

    template<class Archive>
    void load(Archive& ar, const unsigned int version) {
        using boost::serialization::make_nvp;

        for (int i = 0; i < m_stepIndex; ++i) {
            auto step = StepFactory::CreateFromName(m_stepTypes[i]);
            try {
                ar & make_nvp("Step", StepSerializer(*step));
                m_steps[m_stepIndex] = std::unique_ptr<Step>(step);
            }
            catch (boost::archive::archive_exception&) {
                delete step;
                throw;
            }
        }
    }
};

struct StepsSerializer {
    std::vector<std::unique_ptr<Step>>& m_steps;
    int& m_stepIndex;

    StepsSerializer(std::vector<std::unique_ptr<Step>>& steps, int& stepIndex)
        : m_steps(steps), m_stepIndex(stepIndex) {}

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template<class Archive>
    void save(Archive& ar, const unsigned int version) const {
        using boost::serialization::make_nvp;

        ar & make_nvp("Index", m_stepIndex);
        auto stepCount = m_steps.size();
        ar & make_nvp("Count", stepCount);

        std::vector<std::string> stepTypes;
        for (int i = 0; i < m_stepIndex; ++i) {
            Step& step = *m_steps[i];
            stepTypes.push_back(step.GetTypeName());
        }

        ar & make_nvp("Types", stepTypes);
        ar & make_nvp("Items", StepItemsSerializer(m_steps, m_stepIndex, stepTypes));
    }

    template<class Archive>
    void load(Archive& ar, const unsigned int version) {
        using boost::serialization::make_nvp;

        ar & make_nvp("Index", m_stepIndex);
        int stepCount = 0;
        ar & make_nvp("Count", stepCount);

        // Make room for the steps
        m_steps.clear();
        m_steps.resize(stepCount);

        std::vector<std::string> stepTypes;
        ar & make_nvp("Types", stepTypes);
        ar & make_nvp("Items", StepItemsSerializer(m_steps, m_stepIndex, stepTypes));
    }
};

struct JobSerializer {
    std::vector<std::unique_ptr<Step>>& m_steps;
    int& m_stepIndex;

    JobSerializer(std::vector<std::unique_ptr<Step>>& steps, int& stepIndex)
        : m_steps(steps), m_stepIndex(stepIndex) {}

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template<class Archive>
    void save(Archive& ar, const unsigned int version) const {
        using boost::serialization::make_nvp;

        ar & make_nvp("Steps", StepsSerializer(m_steps, m_stepIndex));
    }

    template<class Archive>
    void load(Archive& ar, const unsigned int version) {
        using boost::serialization::make_nvp;

        ar & make_nvp("Steps", StepsSerializer(m_steps, m_stepIndex));
    }
};

void Job::Serialize(boost::archive::xml_iarchive& ar) {
    using boost::serialization::make_nvp;
#if 0
    ar & make_nvp("StepIndex", m_stepIndex);

    // Make room for the steps
    m_steps.resize(m_stepIndex + 1);

    std::string stepType;
    ar & make_nvp("StepType", stepType);

    auto step = StepFactory::CreateFromName(stepType);
    step->Serialize(ar);
    m_steps[m_stepIndex] = std::unique_ptr<Step>(step);

    /*std::vector<std::string> stepTypes;
    ar & make_nvp("StepTypes", stepTypes);

    m_steps.clear();

    // Walk through steps, up to the last successful one
    for (int i = 0; i < m_stepIndex; ++i) {
        auto step = StepFactory::CreateFromName(stepTypes[i]);
        step->Serialize(ar);
        m_steps.push_back(std::unique_ptr<Step>(step));
    }*/
#endif

    ar & make_nvp("Job", JobSerializer(m_steps, m_stepIndex));

    /*for (int i = 0; i < m_stepIndex; ++i) {
        auto step = StepFactory::CreateFromName(stepTypes[i]);
        try {
            step->Serialize(ar);
            m_steps[m_stepIndex] = std::unique_ptr<Step>(step);
        }
        catch (boost::archive::archive_exception&) {
            delete step;
            throw;
        }
    }*/
}

void Job::Serialize(boost::archive::xml_oarchive& ar) {
    using boost::serialization::make_nvp;

#if 0
    /*std::vector<std::string> stepTypes;
    // Walk through steps, up to the last successful one
    for (int i = 0; i < m_stepIndex; ++i) {
        Step& step = *m_steps[i];
        stepTypes.push_back(step.GetTypeName());
        }*/

    ar & make_nvp("StepIndex", m_stepIndex);

    Step& step = *m_steps[m_stepIndex];
    auto& stepType = step.GetTypeName();
    ar & make_nvp("StepType", stepType);

    step.Serialize(ar);

    // Walk through steps, up to the last successful one
    /*for (int i = 0; i < m_stepIndex; ++i) {
        Step& step = *m_steps[i];
        step.Serialize(ar);
        }*/
#endif

    ar & make_nvp("Job", JobSerializer(m_steps, m_stepIndex));

    /*for (int i = 0; i < m_stepIndex; ++i) {
        Step& step = *m_steps[i];
        step.Serialize(ar);
    }*/
}

std::vector<std::unique_ptr<Step>>& Job::GetSteps() {
    return m_steps;
}

} } // namespace
