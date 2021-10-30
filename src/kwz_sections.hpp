#pragma once

#include <string>
#include <vector>

#include "types.hpp"

namespace KFH {

    u32 getSize(std::vector<u8> input);

    u32 getChecksum(std::vector<u8> input);

    bool verifySection(std::vector<u8> input);

    u32 getAppVersion(std::vector<u8> input);

    u32 getCreationTimestamp(std::vector<u8> input);

    u32 getModifiedTimestamp(std::vector<u8> input);

    std::string getRootFSID(std::vector<u8> input);

    std::string getParentFSID(std::vector<u8> input);

    std::string getCurrentFSID(std::vector<u8> input);

    std::u16string getUsername(std::vector<u8> input, int start);

    std::u16string getRootUsername(std::vector<u8> input);

    std::u16string getParentUsername(std::vector<u8> input);

    std::u16string getCurrentUsername(std::vector<u8> input);

    std::string getRootFileName(std::vector<u8> input);

    std::string getParentFileName(std::vector<u8> input);

    std::string getCurrentFileName(std::vector<u8> input);

    u16 getFrameCount(std::vector<u8> input);

    u16 getThumbnailFrameIndex(std::vector<u8> input);

    u16 getFlags(std::vector<u8> input);

    float getFrameSpeed(std::vector<u8> input);

    u8 getLayerVisibility(std::vector<u8> input);

}

namespace KTN {

    u32 getSize(std::vector<u8> input, int offset);

    u32 getChecksum(std::vector<u8> input, int offset);

    bool verifySection(std::vector<u8> input, int offset);

    int getThumbnailOffset(int offset);

}

namespace KSN {

    u32 getSize(std::vector<u8> input, int offset);

    u32 getSpeedWhenRecorded(std::vector<u8> input, int offset);

    u32 getBGMSize(std::vector<u8> input, int offset);

    u32 getSE1Size(std::vector<u8> input, int offset);

    u32 getSE2Size(std::vector<u8> input, int offset);

    u32 getSE3Size(std::vector<u8> input, int offset);

    u32 getSE4Size(std::vector<u8> input, int offset);

    u32 getChecksum(std::vector<u8> input, int offset);

    bool verifySection(std::vector<u8> input, int offset);

    int getBGMOffset(std::vector<u8> input, int offset);

    int getSE1Offset(std::vector<u8> input, int offset);

    int getSE2Offset(std::vector<u8> input, int offset);

    int getSE3Offset(std::vector<u8> input, int offset);

    int getSE4Offset(std::vector<u8> input, int offset);

}

namespace KMI {

}

namespace KMC {

}
