#include "DetailsPrinter.h"
#include <aufw/core/progress/Product.h>
#include <aufw/core/progress/State.h>
#include <nowide/iostream.hpp>

using aufw::progress::Product;
using aufw::progress::State;

void DetailsPrinter::Current(const Product& product) {
    switch (product.State) {
    case State::DownloadPending:
    case State::Downloading:
    case State::DownloadFailed:
    case State::DownloadComplete:
        DetailsPrinter::Download(product);
        break;

    case State::VerifyPending:
    case State::Verifying:
    case State::VerifyFailed:
    case State::VerifyComplete:
        DetailsPrinter::Verify(product);
        break;

    case State::UnpackPending:
    case State::Unpacking:
    case State::UnpackFailed:
    case State::UnpackComplete:
        DetailsPrinter::Unpack(product);
        break;

    case State::InstallPending:
    case State::Installing:
    case State::InstallFailed:
    case State::InstallComplete:
        DetailsPrinter::Install(product);
        break;

    case State::Complete:
        DetailsPrinter::Complete(product);
        break;

    default:
        assert(false);
        throw std::invalid_argument("Invalid state");
    }
}

void DetailsPrinter::Download(const Product& product) {
    before(product);
    nowide::cout << "File size: " << product.ProgressDetails.Download.FileSize << std::endl;
    nowide::cout << "Total downloaded: " << product.ProgressDetails.Download.TotalDownloaded << std::endl;
    after(product);
}

void DetailsPrinter::Verify(const Product& product) {
    before(product);

    after(product);
}

void DetailsPrinter::Unpack(const Product& product) {
    before(product);

    after(product);
}

void DetailsPrinter::Install(const Product& product) {
    before(product);

    after(product);
}

void DetailsPrinter::Complete(const Product& product) {
    before(product);

    after(product);
}

void DetailsPrinter::before(const Product& product) {
    nowide::cout << "Product: " << product.UpdateDetails.DisplayName << std::endl;
    nowide::cout << "State: " << State::GetStateText(product.State) << std::endl;
    if (!product.SourceFilePath.empty()) {
        nowide::cout << "Source file path: " << product.SourceFilePath << std::endl;
    }

    if (!product.TempFilePath.empty()) {
        nowide::cout << "Temp. file path: " << product.TempFilePath << std::endl;
    }
}

void DetailsPrinter::after(const Product& product) {
    nowide::cout << std::endl;
}
