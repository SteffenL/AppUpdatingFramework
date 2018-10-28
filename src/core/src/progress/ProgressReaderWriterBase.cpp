#include <aufw/core/progress/ProgressReaderWriterBase.h>

namespace aufw { namespace progress {

ProgressReaderWriterBase::ProgressReaderWriterBase()
    : m_hasApplication(false), m_haveComponents(false) {}

ProgressReaderWriterBase::~ProgressReaderWriterBase() {}

void ProgressReaderWriterBase::SetApplication(const Product& application) {
    m_application = application;
    m_hasApplication = true;
}

void ProgressReaderWriterBase::AddComponent(const Product& component) {
    m_components.push_back(component);
    m_haveComponents = true;
}

const Product& ProgressReaderWriterBase::GetApplication() const {
    return m_application;
}

Product& ProgressReaderWriterBase::GetApplicationWritable() {
    return m_application;
}

const std::list<Product>& ProgressReaderWriterBase::GetComponents() const {
    return m_components;
}

std::list<Product>& ProgressReaderWriterBase::GetComponentsWritable() {
    return m_components;
}

bool ProgressReaderWriterBase::HasApplication() const {
    return m_hasApplication;
}

bool ProgressReaderWriterBase::HaveComponents() const {
    return m_haveComponents;
}

bool ProgressReaderWriterBase::IsReadyToDownload() const {
    using namespace aufw::progress;

    auto& application = GetApplication();
    auto& components = GetComponents();

    int numProducts = 0;
    int n = 0;

    if (HasApplication()) {
        ++numProducts;
        n += (((application.State >= State::DownloadPending) && (application.State < State::DownloadComplete)) ? 1 : 0);
    }

    numProducts += components.size();
    std::for_each(components.begin(), components.end(), [&](const Product& component) {
        using namespace aufw::progress;
        if ((component.State >= State::DownloadPending) && (component.State < State::DownloadComplete)) {
            ++n;
        }
    });

    return (n == numProducts);
}

bool ProgressReaderWriterBase::IsReadyToVerify() const {
    using namespace aufw::progress;

    auto& application = GetApplication();
    auto& components = GetComponents();

    int numProducts = 0;
    int n = 0;

    if (HasApplication()) {
        ++numProducts;
        n += (((application.State >= State::DownloadComplete) && (application.State < State::VerifyComplete)) ? 1 : 0);
    }

    numProducts += components.size();
    std::for_each(components.begin(), components.end(), [&](const Product& component) {
        using namespace aufw::progress;
        if ((component.State >= State::DownloadComplete) && (component.State < State::VerifyComplete)) {
            ++n;
        }
    });

    return (n == numProducts);
}

bool ProgressReaderWriterBase::IsReadyToInstall() const {
    using namespace aufw::progress;

    auto& application = GetApplication();
    auto& components = GetComponents();

    int numProducts = 0;
    int n = 0;

    if (HasApplication()) {
        ++numProducts;
        n += (((application.State >= State::VerifyComplete) && (application.State < State::InstallComplete)) ? 1 : 0);
    }

    numProducts += components.size();
    std::for_each(components.begin(), components.end(), [&](const Product& component) {
        using namespace aufw::progress;
        if ((component.State >= State::VerifyComplete) && (component.State < State::InstallComplete)) {
            ++n;
        }
    });

    return (n == numProducts);
}

bool ProgressReaderWriterBase::InstallIsComplete() const {
    using namespace aufw::progress;

    auto& application = GetApplication();
    auto& components = GetComponents();

    int numProducts = 0;
    int n = 0;

    if (HasApplication()) {
        ++numProducts;
        n += ((application.State >= State::InstallComplete) ? 1 : 0);
    }

    numProducts += components.size();
    std::for_each(components.begin(), components.end(), [&](const Product& component) {
        using namespace aufw::progress;
        if (component.State >= State::InstallComplete) {
            ++n;
        }
    });

    return (n == numProducts);
}

bool ProgressReaderWriterBase::HasProgress() const
{
    using namespace aufw::progress;

    auto& application = GetApplication();
    auto& components = GetComponents();

    int numProducts = 0;
    int n = 0;

    if (HasApplication()) {
        ++numProducts;
        n += ((application.State > State::DownloadPending) ? 1 : 0);
    }

    numProducts += components.size();
    std::for_each(components.begin(), components.end(), [&](const Product& component) {
        using namespace aufw::progress;
        if (component.State > State::DownloadPending) {
            ++n;
        }
    });

    return (n == numProducts);
}

} } // namespace
