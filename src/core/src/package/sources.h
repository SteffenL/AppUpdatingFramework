#ifndef __aufw_package_sources__
#define __aufw_package_sources__

#include <vector>
#include <string>
#include <memory>
#include <iosfwd>

namespace Poco { namespace Zip { class ZipArchive; } }

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
	std::unique_ptr<std::istream> m_file;
	std::unique_ptr<Poco::Zip::ZipArchive> m_zip;
};

} } // namespace
#endif // guard
