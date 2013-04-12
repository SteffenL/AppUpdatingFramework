#ifndef __aufw_package_sources__
#define __aufw_package_sources__

#include <vector>
#include <string>
#include <memory>

namespace Partio { class ZipFileReader; }

namespace aufw { namespace package {

class SourceBase {
public:
    virtual void GetFileList(std::vector<std::string>& list) const = 0;
    virtual std::istream* GetFile(std::string path) = 0;
    virtual ~SourceBase();
};

class ZipFileSource : public SourceBase {
public:
    ZipFileSource(const std::string& path);
    void GetFileList(std::vector<std::string>& list) const;
    std::istream* GetFile(std::string path);

private:
    const std::shared_ptr<Partio::ZipFileReader> m_reader;
};

} } // namespace
#endif // guard
