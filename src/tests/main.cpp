#include <UnitTest++.h>
#include <boost/locale.hpp>
#include <boost/filesystem/path.hpp>

int main() {
    // Setup locale
    std::locale::global(boost::locale::generator().generate(""));
    boost::filesystem::path::imbue(std::locale());

    return UnitTest::RunAllTests();
}