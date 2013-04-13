#ifndef __aufw_progress_ProgressReaderWriterBase__
#define __aufw_progress_ProgressReaderWriterBase__

#include "Product.h"
#include <iostream>
#include <list>

namespace aufw { namespace progress {

struct Product;

class ProgressReaderWriterBase {
public:
    ProgressReaderWriterBase();
    virtual ~ProgressReaderWriterBase();
    void SetApplication(const Product& application);
    void AddComponent(const Product& component);
    const Product& GetApplication() const;
    Product& GetApplicationWritable();
    const std::list<Product>& GetComponents() const;
    std::list<Product>& GetComponentsWritable();
    bool HasApplication() const;
    bool HaveComponents() const;
    bool IsReadyToDownload() const;
    bool IsReadyToVerify() const;
    bool IsReadyToInstall() const;
    bool InstallIsComplete() const;

    std::string TargetDir;
    std::string BackupDir;
    std::string DownloadDir;

protected:
    Product m_application;
    std::list<Product> m_components;
    bool m_hasApplication;
    bool m_haveComponents;
};

} } // namespace
#endif // guard
