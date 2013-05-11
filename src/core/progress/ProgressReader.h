#ifndef __aufw_progress_ProgressReader__
#define __aufw_progress_ProgressReader__

#include "ProgressReaderWriterBase.h"
#include <iostream>
#include <string>
#include <nowide/fstream.hpp>

namespace aufw { namespace progress {

class ProgressReader : public ProgressReaderWriterBase {
public:
    ProgressReader(const std::string& filePath);
    void Load();

private:
    nowide::ifstream m_stream;

protected:
    // Hashes file
    // Status file
};

} } // namespace
#endif // guard