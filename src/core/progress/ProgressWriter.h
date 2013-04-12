#ifndef __aufw_progress_ProgressWriter__
#define __aufw_progress_ProgressWriter__

#include "ProgressReaderWriterBase.h"
#include <iostream>
#include <fstream>
#include <string>

namespace aufw { namespace progress {

class ProgressWriter : public ProgressReaderWriterBase {
public:
    ProgressWriter(const std::string& filePath);
    //void Load(const std::string& filePath);
    //void Load(std::istream& stream);
    void Save();

private:
    std::ofstream m_stream;

protected:
    // Hashes file
    // Status file
};

} } // namespace
#endif // guard