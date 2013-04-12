#include "Version.h"
#include <Poco/Format.h>
#include <Poco/NumberParser.h>
#include <Poco/StringTokenizer.h>

namespace aufw {

Version::Version() : Major(0), Minor(0), Build(0), Private(0) {}

Version::Version(unsigned int major, unsigned int minor, unsigned int build, unsigned int private_)
    : Major(major), Minor(minor), Build(build), Private(private_) {}

std::string Version::ToString() const {
    return Poco::format("%u.%u.%u.%u", Major, Minor, Build, Private);
}

// Assumes that each part is an 8 bit value
unsigned int Version::ToUInt() const {
    return (Major << 24) | (Minor << 16) | (Build << 8) | Private;
}

Version& Version::operator=(const Version& v) {
    if (this != &v) {
        SetVersion(v.Major, v.Minor, v.Build, v.Private);
    }

    return *this;
}

// Assumes that each part is an 8 bit value
// 0x01020304 = 1.2.3.4
// 0 = 0.0.0.0
Version& Version::operator=(const unsigned int v) {
    if (v == 0) {
        SetVersion(0, 0, 0, 0);
        return *this;
    }

    SetVersion(
        (v & 0xff000000) >> 24,
        (v & 0xff0000) >> 16,
        (v & 0xff00) >> 8,
        (v & 0xff));
    return *this;
}

bool Version::FromString(const std::string& s) {
    if (s.empty()) {
        return false;
    }

    Poco::StringTokenizer tokenizer(s, ".");
    int i = 0;
    for (auto it = tokenizer.begin(), end(tokenizer.end()); (it != end) && (i < 4); ++it, ++i) {
        unsigned int partValue;
        if (!Poco::NumberParser::tryParseUnsigned(*it, partValue)) {
            return false;
        }

        Part(i) = partValue;
    }

    return true;
}

Version& Version::operator=(const std::string& versionString) {
    if (!FromString(versionString)) {
        throw std::invalid_argument("Invalid version string");
    }

    return *this;
}

unsigned int& Version::Part(int index) {
    unsigned int* parts[] = { &Major, &Minor, &Build, &Private };
    return *parts[index];
}

void Version::SetVersion(unsigned int major, unsigned int minor, unsigned int build, unsigned int private_) {
    Major = major;
    Minor = minor;
    Build = build;
    Private = private_;
}

} // namespace
