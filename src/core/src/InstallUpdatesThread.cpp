#include <aufw/core/InstallUpdatesThread.h>

#include <aufw/core/progress/ProgressReaderWriter.h>
#include <aufw/core/progress/Product.h>
#include "exceptions.h"
#include "package/FilePackage.h"
#include "package/sources.h"
#include <aufw/core/job/Job.h>
#include "job/MoveFilesStep.h"
#include "job/UnpackFilePackageStep.h"
#include "job/StartProgramStep.h"

#include <iostream>
#include <algorithm>
#include <boost/filesystem.hpp>

// Exceptions
#include <boost/exception_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <stdexcept>

namespace aufw {

const wxEventType InstallUpdatesThread::StateChangedEvent = wxNewEventType();
const wxEventType InstallUpdatesThread::InstallCompleteEvent = wxNewEventType();
const wxEventType InstallUpdatesThread::InstallFailedEvent = wxNewEventType();
const wxEventType InstallUpdatesThread::ServerErrorEvent = wxNewEventType();
const wxEventType InstallUpdatesThread::ThreadExceptionEvent = wxNewEventType();
const wxEventType InstallUpdatesThread::ThreadExitEvent = wxNewEventType();

InstallUpdatesThread::InstallUpdatesThread(wxEvtHandler* parent, aufw::progress::ProgressReaderWriter& progressFile, CancelCallback_t cancelCallback)
    : m_parent(parent), m_progressFile(progressFile), m_cancelCallback(cancelCallback), m_wasCanceled(false), wxThread() {}

void* InstallUpdatesThread::Entry() {
    if (TestDestroy()) {
        return nullptr;
    }

    // Clone exception for re-throwing in the main thread
    try {
        installNowInternal();
    }
    catch (...) {
        boost::exception_ptr* exception = new boost::exception_ptr(boost::current_exception());
        wxCommandEvent event(ThreadExceptionEvent);
        event.SetClientData(exception);
        wxQueueEvent(m_parent, event.Clone());
    }

    return nullptr;
}

void InstallUpdatesThread::OnExit() {
    wxCommandEvent event(ThreadExitEvent);
    wxQueueEvent(m_parent, event.Clone());
}

void InstallUpdatesThread::BeginInstall(wxEvtHandler* parent, aufw::progress::ProgressReaderWriter& progressFile, CancelCallback_t cancelCallback) {
    auto thread = new InstallUpdatesThread(parent, progressFile, cancelCallback);
    if (thread->Create() != wxTHREAD_NO_ERROR) {
        return;
    }

    thread->Run();
}

void InstallUpdatesThread::installNowInternal() {
    using namespace aufw::progress;

    auto& application = m_progressFile.GetApplicationWritable();

    if (m_progressFile.HasApplication())
    {
        if ((application.State >= State::VerifyComplete) && (application.State < State::InstallComplete)) {
            application.State = State::InstallPending;
            m_progressFile.Save();

            wxCommandEvent event(StateChangedEvent);
            event.SetClientData(static_cast<void*>(&application));
            wxQueueEvent(m_parent, event.Clone());

            try {
                install(application);
            }
            catch (std::exception&) {
                throw;
            }

            m_progressFile.Save();
        }
        else {
        }
    }

    auto& components = m_progressFile.GetComponentsWritable();
    std::for_each(components.begin(), components.end(), [&](Product& component) {
        using namespace aufw::progress;

        if ((component.State >= State::VerifyComplete) && (component.State < State::InstallComplete)) {
            component.State = State::InstallPending;

            wxCommandEvent event(StateChangedEvent);
            event.SetClientData(static_cast<void*>(&component));
            wxQueueEvent(m_parent, event.Clone());

            try {
                install(component);
            }
            catch (std::exception&) {
                throw;
            }

            m_progressFile.Save();
        }
        else {
        }
    });

    m_progressFile.Save();

    // Check if all are complete
    if (!m_progressFile.HasApplication() || (application.State >= State::InstallComplete)) {
        bool isComplete = true;
        std::for_each(components.begin(), components.end(), [&](Product& component) {
            using namespace aufw::progress;
            if (component.State < State::InstallComplete) {
                isComplete = false;
                return;
            }
        });

        if (isComplete) {
            wxCommandEvent event(InstallCompleteEvent);
            wxQueueEvent(m_parent, event.Clone());
        }
    }
}

void InstallUpdatesThread::install(aufw::progress::Product& product) {
    using namespace aufw;
    using namespace aufw::progress;
    namespace fs = boost::filesystem;

    try {
        if (product.State >= State::InstallComplete) {
            throw std::runtime_error("Already installed");
        }

        product.State = State::InstallPending;
        m_progressFile.Save();

        wxCommandEvent event(StateChangedEvent);
        event.SetClientData(static_cast<void*>(&product));
        wxQueueEvent(m_parent, event.Clone());

        if (!fs::exists(product.TempFilePath)) {
            throw FileException("File not found", product.TempFilePath);
        }

        bool needPrivileges = false;
        std::string exceptionStr;

        auto job = GetJob(product);

        job->OnExecutionFailed = [&](const job::JobFailureArg& arg) {
            using namespace aufw;
            exceptionStr = arg.Exception->what();
            //nowide::cout << std::endl << "Installation failed. " << arg.Exception.what() << std::endl;
            if (dynamic_cast<FileException*>(arg.Exception)) {
                // Probably need more privileges to do anything with files
                needPrivileges = true;
            }
            else {
                //nowide::cout << std::endl << "Exception: " << (arg.Exception ? arg.Exception->what() : "(unknown)") << std::endl;
            }
        };

        job->OnRollbackFailed = [](const job::JobFailureArg& arg) {
            //nowide::cout << std::endl << "Rollback failed. " << arg.Exception.what() << std::endl;
        };

        /*job.OnExecutionSuccess = []() {

        };

        job.OnRollbackSuccess = []() {

        };*/

        job->OnStepProgress = [](const job::StepProgressArg& arg) {
            
        };

        product.Job = job;
        product.State = State::Installing;
        m_progressFile.Save();

        {
            wxCommandEvent event(StateChangedEvent);
            event.SetClientData(static_cast<void*>(&product));
            wxQueueEvent(m_parent, event.Clone());
        }

        if (!job->Execute()) {
            if (needPrivileges) {
                product.State = State::InstallFailed;
                m_progressFile.Save();

                {
                    wxCommandEvent event(StateChangedEvent);
                    event.SetClientData(static_cast<void*>(&product));
                    wxQueueEvent(m_parent, event.Clone());
                }

                {
                    wxCommandEvent event(InstallFailedEvent);
                    wxQueueEvent(m_parent, event.Clone());
                }

                return;
            }
            else {
                std::string ex("Job failed to complete\n\n");
                if (exceptionStr.empty()) {
                    ex += "Reason is unknown.";
                }
                else {
                    ex += exceptionStr;
                }
                
                throw std::runtime_error(ex);
            }
        }

        product.Job = nullptr;
        product.State = State::InstallComplete;
        m_progressFile.Save();

        {
            wxCommandEvent event(StateChangedEvent);
            event.SetClientData(static_cast<void*>(&product));
            wxQueueEvent(m_parent, event.Clone());
        }
    }
    catch (std::exception&) {
        product.State = State::InstallFailed;
        m_progressFile.Save();

        wxCommandEvent event(StateChangedEvent);
        event.SetClientData(static_cast<void*>(&product));
        // Process event immediately so that the job is still available for additional processing
        m_parent->ProcessEvent(event);

        throw;
    }
}

aufw::job::Job* InstallUpdatesThread::GetJob(aufw::progress::Product& product) {
    using namespace aufw;

    m_package.reset(new package::FilePackage(product.TempFilePath));
    package::SourceBase& source = m_package->GetSource();

    source.GetFileList(m_fileList);

    if (!product.Job) {
        product.Job = new job::Job;

        // Backup files
        product.Job->AddStep(new job::MoveFilesStep(m_progressFile.TargetDir, m_fileList, m_progressFile.BackupDir));
        // Unpack
        product.Job->AddStep(new job::UnpackFilePackageStep(m_package.get(), m_progressFile.TargetDir));
        // Restart app
        //job->AddStep(new job::StartProgramStep(exePath.string()));
    }
    else {
        auto& steps = product.Job->GetSteps();

        if (steps.size() > 0) {
            auto step = steps[0].get() ? static_cast<job::MoveFilesStep*>(steps[0].get()) : new job::MoveFilesStep;
            if (!step) {
               throw std::runtime_error("Couldn't get existing/new MoveFilesStep");
            }

            steps[0] = std::unique_ptr<job::Step>(step);
            step->Init(m_progressFile.TargetDir, m_fileList, m_progressFile.BackupDir);
        }

        if (steps.size() > 1) {
            auto step = steps[1].get() ? static_cast<job::UnpackFilePackageStep*>(steps[1].get()) : new job::UnpackFilePackageStep;
            if (!step) {
                throw std::runtime_error("Couldn't get existing/new UnpackFilePackageStep");
            }

            steps[1] = std::unique_ptr<job::Step>(step);
            step->Init(m_package.get(), m_progressFile.TargetDir);
        }


        /*for (auto& step : steps) {

        }*/
    }

    return product.Job;
}

bool InstallUpdatesThread::testCanceled() {
    m_wasCanceled = (m_cancelCallback ? m_cancelCallback() : false);
    return m_wasCanceled;
}

} // namespace
