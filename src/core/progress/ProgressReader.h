#ifndef __aufw_progress_ProgressReader__
#define __aufw_progress_ProgressReader__

#include "ProgressReaderWriterBase.h"
#include <iostream>
#include <fstream>
#include <string>

namespace aufw { namespace progress {

class ProgressReader : public ProgressReaderWriterBase {
public:
    ProgressReader(const std::string& filePath);
    void Load();

private:
    std::ifstream m_stream;

protected:
    // Hashes file
    // Status file
};

} } // namespace
#endif // guard