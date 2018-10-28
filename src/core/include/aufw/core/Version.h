#ifndef __aufw_Version__
#define __aufw_Version__

#include <string>
#include <cstdint>

namespace aufw {

class Version
{
public:
	Version();
	Version(unsigned int major, unsigned int minor, unsigned int build, unsigned int private_);
	std::string ToString() const;
	// Assumes that each part is an 8 bit value
	unsigned int ToUInt() const;
	Version& operator=(const Version& v);
	// Assumes that each part is an 8 bit value
	// 0x01020304 = 1.2.3.4
	// 0 = 0.0.0.0
	Version& operator=(const unsigned int v);
	bool FromString(const std::string& s);
	Version& operator=(const std::string& versionString);
	unsigned int& Part(int index);
    void SetVersion(unsigned int major, unsigned int minor, unsigned int build, unsigned int private_);

public:
    unsigned int Major;
    unsigned int Minor;
    unsigned int Build;
    unsigned int Private;
};

} // namespace
#endif // guard
