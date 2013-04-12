#include "BoostFileSystemBugWorkaround.h"
#include <boost/filesystem.hpp>

boost::filesystem::path g_path;

BoostFileSystemBugWorkaround::BoostFileSystemBugWorkaround() {
    // Dummy path
    boost::filesystem::path p("a/b/c");
    g_path = p;
}
