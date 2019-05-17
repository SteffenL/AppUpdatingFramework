#include <aufw/core/Version.h>
#include <aufw/ui/FoundUpdatesDialog.h>

#include <iostream>

int main() {
    if (aufw::Version(1, 2, 3, 4).ToString() != "1.2.3.4") {
        return 1;
    }

    std::cout << "aufw works!" << std::endl;
    return 0;
}
