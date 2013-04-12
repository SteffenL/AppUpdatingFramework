#ifndef __aufw_progress_DetailsPrinter__
#define __aufw_progress_DetailsPrinter__

namespace aufw { namespace progress { struct Product; } }

struct DetailsPrinter {
    // Based on current state
    static void Current(const aufw::progress::Product& product);

    static void Download(const aufw::progress::Product& product);
    static void Verify(const aufw::progress::Product& product);
    static void Unpack(const aufw::progress::Product& product);
    static void Install(const aufw::progress::Product& product);
    static void Complete(const aufw::progress::Product& product);

private:
    static void before(const aufw::progress::Product& product);
    static void after(const aufw::progress::Product& product);
};

#endif // guard
