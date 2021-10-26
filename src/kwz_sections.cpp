#include <string>
#include <vector>

#include "kwz_sections.hpp"
#include "types.hpp"
#include "tables.hpp"
#include "templates.hpp"

namespace KFH {

    /**
     * Get the size of the KFH section from a .kwz file buffer
     *
     * Parameters:
     * - input: a u8 vector that contains a .kwz file
     * Returns:
     * - the size of the KFH section as a u32
     */
    u32 getSize(std::vector<u8> input) {
        return getInt<u32>(input, 0x4);
    }

    /**
     * Get the CRC32 checksum of the KFH section from a .kwz file buffer
     *
     * Parameters:
     * - input: a u8 vector that contains a .kwz file
     * Returns:
     * - the CRC32 checksum of the KFH section as a u32
     */
    u32 getChecksum(std::vector<u8> input) {
        return getInt<u32>(input, 0x8);
    }

    /**
     * Get the creation timestamp from a .kwz file buffer
     *
     * Note: timstamps are stored as the number of seconds from the January 1, 2000 epoch instead of
     * the unix epoch (January 1, 1970). This conversion is configurable with the `unix` parameter.
     *
     * Parameters:
     * - input: a u8 vector that contains a .kwz file
     * - unix: whether or not to convert timestamp to unix epoch
     *      - true: will convert timestamp to unix epoch
     *      - false: will not convert timestamp to unix epoch (remains at Januray 1, 2000 epoch)
     * Returns:
     * - the creation timestamp as a u32
     */
    u32 getCreationTimestamp(std::vector<u8> input, bool unix) {
        if (unix) {
            return getInt<u32>(input, 0xC) + 946684800;
        }
        else {
            return getInt<u32>(input, 0xC);
        }
    }

    /**
     * Get the modified timestamp from a .kwz file buffer
     *
     * Note: timstamps are stored as the number of seconds from the January 1, 2000 epoch instead of
     * the unix epoch (January 1, 1970). This conversion is configurable with the `unix` parameter.
     *
     * Parameters:
     * - input: a u8 vector that contains a .kwz file
     * - unix: whether or not to convert timestamp to unix epoch
     *      - true: will convert timestamp to unix epoch
     *      - false: will not convert timestamp to unix epoch (remains at Januray 1, 2000 epoch)
     * Returns:
     * - the modified timestamp as a u32
     */
    u32 getModifiedTimestamp(std::vector<u8> input, bool unix) {
        if (unix) {
            return getInt<u32>(input, 0xD) + 946684800;
        }
        else {
            return getInt<u32>(input, 0xD);
        }
    }

    /**
     * Get the app version from a .kwz file buffer
     *
     * Parameters:
     * - input: a u8 vector that contains a .kwz file
     * Returns:
     * - the app version as a u32
     */
    u32 getAppVersion(std::vector<u8> input) {
        return getInt<u32>(input, 0x14);
    }

    /**
     * Get the root FSID as hex from a .kwz file buffer
     *
     * Parameters:
     * - input: a u8 vector that contains a .kwz file
     * Returns:
     * - the root FSID version as a string
     */
    std::string getRootFSID(std::vector<u8> input) {
        return getHexString(input, 0x18, 10);
    }

    /**
     * Get the parent FSID as hex from a .kwz file buffer
     *
     * Parameters:
     * - input: a u8 vector that contains a .kwz file
     * Returns:
     * - the parent FSID version as a string
     */
    std::string getParentFSID(std::vector<u8> input) {
        return getHexString(input, 0x22, 10);
    }

    /**
     * Get the current FSID as hex from a .kwz file buffer
     *
     * Parameters:
     * - input: a u8 vector that contains a .kwz file
     * Returns:
     * - the current FSID version as a string
     */
    std::string getCurrentFSID(std::vector<u8> input) {
        return getHexString(input, 0x2C, 10);
    }

    /**
     * Decode a username in a .kwz file buffer
     *
     * Parameters:
     * - input: a u8 vector that contains a .kwz file
     * - start: the starting index in input to get a username from
     * Returns:
     * - the decoded username as a u16string
     */
    std::u16string getUsername(std::vector<u8> input, int start) {
        std::u16string output;
        char16_t character;

        for (int i = start; i < start + 20; i += 2) {
            character = getInt<char16_t>(input, i);

            // Usernames are null padded, ignore padding.
            if (character != 0) output += character;
        }

        return output;
    }

    /**
     * Get the root username from a .kwz file buffer
     *
     * Parameters:
     * - input: a u8 vector that contains a .kwz file
     * Returns:
     * - the root username as a u16string
     */
    std::u16string getRootUsername(std::vector<u8> input) {
        return getUsername(input, 0x36);
    }

    /**
     * Get the parent username from a .kwz file buffer
     *
     * Parameters:
     * - input: a u8 vector that contains a .kwz file
     * Returns:
     * - the parent username as a u16string
     */
    std::u16string getParentUsername(std::vector<u8> input) {
        return getUsername(input, 0x4C);
    }

    /**
     * Get the current username from a .kwz file buffer
     *
     * Parameters:
     * - input: a u8 vector that contains a .kwz file
     * Returns:
     * - the current username as a u16string
     */
    std::u16string getCurrentUsername(std::vector<u8> input) {
        return getUsername(input, 0x62);
    }

    /**
     * Get the root file name from a .kwz file buffer
     *
     * Parameters:
     * - input: a u8 vector that contains a .kwz file
     * Returns:
     * - the root file name as a u16string
     */
    std::string getRootFileName(std::vector<u8> input) {
        return getHexString(input, 0x78, 28);
    }

    /**
     * Get the parent file name from a .kwz file buffer
     *
     * Parameters:
     * - input: a u8 vector that contains a .kwz file
     * Returns:
     * - the parent file name as a u16string
     */
    std::string getParentFileName(std::vector<u8> input) {
        return getHexString(input, 0x94, 28);
    }

    /**
     * Get the current file name from a .kwz file buffer
     *
     * Parameters:
     * - input: a u8 vector that contains a .kwz file
     * Returns:
     * - the current file name as a u16string
     */
    std::string getCurrentFileName(std::vector<u8> input) {
        return getHexString(input, 0xB0, 28);
    }

    /**
     * Get the frame count from a .kwz file buffer
     *
     * Parameters:
     * - input: a u8 vector that contains a .kwz file
     * Returns:
     * - the frame count as a u16
     */
    u16 getFrameCount(std::vector<u8> input) {
        return getInt<u16>(input, 0xCC);
    }

    /**
     * Get the frame count from a .kwz file buffer
     *
     * Parameters:
     * - input: a u8 vector that contains a .kwz file
     * Returns:
     * - the frame count as a u16
     */
    u16 getThumbnailFrameIndex(std::vector<u8> input) {
        return getInt<u16>(input, 0xCE);
    }

    /**
     * Get the KFH flags from a .kwz file buffer
     *
     * See: https://github.com/Flipnote-Collective/flipnote-studio-3d-docs/wiki/kwz-format#kfh-flags
     *
     * Parameters:
     * - input: a u8 vector that contains a .kwz file
     * Returns:
     * - the frame count as a u16
     */
    u16 getFlags(std::vector<u8> input) {
        return getInt<u16>(input, 0xD0);
    }

    /**
     * Get the flipnote's framerate from a .kwz file buffer.
     *
     * See: https://github.com/Flipnote-Collective/flipnote-studio-3d-docs/wiki/kwz-format#flipnote-playback-speeds
     *
     * Parameters:
     * - input: a u8 vector that contains a .kwz file
     * Returns:
     * - the framerate as a float
     */
    float getFrameSpeed(std::vector<u8> input) {
        return KWZ_FRAMERATES[getInt<u8>(input, 0xD2)];
    }

    /**
     * Get the layer visibility flags from a .kwz file buffer
     *
     * See: https://github.com/Flipnote-Collective/flipnote-studio-3d-docs/wiki/kwz-format#layer-visibility-flags
     *
     * Parameters:
     * - input: a u8 vector that contains a .kwz file
     * Returns:
     * - the layer visibility flags as a u8
     */
    u8 getLayerVisibility(std::vector<u8> input) {
        return getInt<u8>(input, 0xD3);
    }

}
