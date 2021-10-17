#include <string>
#include <vector>

#include "kwz_sections.hpp"
#include "types.hpp"
#include "templates.hpp"

namespace KFH {

    u32 getSize(std::vector<u8> input) {
        return getInt<u32>(input, 0x4);
    }

    u32 getChecksum(std::vector<u8> input) {
        return getInt<u32>(input, 0x8);
    }

    u32 getCreationTimestamp(std::vector<u8> input, bool unix) {
        if (unix) {
            return getInt<u32>(input, 0xC) + 946684800;
        }
        else {
            return getInt<u32>(input, 0xC);
        }
    }

    u32 getModifiedTimestamp(std::vector<u8> input, bool unix) {
        if (unix) {
            return getInt<u32>(input, 0xD) + 946684800;
        }
        else {
            return getInt<u32>(input, 0xD);
        }
    }

    u32 getAppVersion(std::vector<u8> input) {
        return getInt<u32>(input, 0x14);
    }

    std::string getRootFSID(std::vector<u8> input) {
        return getHexString(input, 0x18, 10);
    }

    std::string getParentFSID(std::vector<u8> input) {
        return getHexString(input, 0x22, 10);
    }

    std::string getCurrentFSID(std::vector<u8> input) {
        return getHexString(input, 0x2C, 10);
    }

    // TODO: Implement these
    std::u16string getRootUsername(std::vector<u8> input) {
        return;
    }

    std::u16string getParentUsername(std::vector<u8> input) {
        return;
    }

    std::u16string getCurrentUsername(std::vector<u8> input) {
        return;
    }

    std::string getRootFileName(std::vector<u8> input) {
        return getHexString(input, 0x78, 28);
    }

    std::string getParentFileName(std::vector<u8> input) {
        return getHexString(input, 0x94, 28);
    }

    std::string getCurrentFileName(std::vector<u8> input) {
        return getHexString(input, 0xB0, 28);
    }

    u16 getFrameCount(std::vector<u8> input) {
        return getInt<u16>(input, 0xCC);
    }

    u16 getThumbnailFrameIndex(std::vector<u8> input) {
        return getInt<u16>(input, 0xCE);
    }

    u16 getFlags(std::vector<u8> input) {
        return getInt<u16>(input, 0xD0);
    }

    u8 getFrameSpeed(std::vector<u8> input) {
        return getInt<u8>(input, 0xD2);
    }

    u8 getLayerVisibility(std::vector<u8> input) {
        return getInt<u8>(input, 0xD3);
    }

}
