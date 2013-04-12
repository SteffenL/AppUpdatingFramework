#ifndef __aufw_progress_ProgressReaderWriter__
#define __aufw_progress_ProgressReaderWriter__

#include "ProgressReaderWriterBase.h"
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>

namespace aufw { namespace progress {

class ProgressReaderWriter : public ProgressReaderWriterBase {
public:
    ProgressReaderWriter(const std::string& filePath);
    void Load();
    void Save();
    std::string GetFilePath() const;
    void CleanupFiles() const;

private:
    std::fstream m_stream;
    std::string m_filePath;
    mutable std::mutex m_ioMutex;
};

} } // namespace
#endif // guard