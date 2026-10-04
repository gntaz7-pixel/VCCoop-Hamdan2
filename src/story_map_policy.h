// Self-contained policy used by the VCCoop story-map synchronization.
// No game or network dependencies: tested on Linux and Windows.
#pragma once
#include <stdint.h>
#include <string.h>
#include <cmath>

namespace story_map {
    enum { MAX_CONTACTS = 28, PACKET_KIND = 6, PACKET_SIZE = 2 + 16 * MAX_CONTACTS };
    struct Key { uint32_t x, y, z, sprite; };
    static_assert(PACKET_SIZE <= 480, "Story contacts must fit one reliable VCCoop packet");
    inline Key Make(float x, float y, float z, int sprite) {
        Key k = {};
        memcpy(&k.x, &x, sizeof(float));
        memcpy(&k.y, &y, sizeof(float));
        memcpy(&k.z, &z, sizeof(float));
        k.sprite = (uint32_t)sprite;
        return k;
    }
    inline bool Same(const Key &a, const Key &b) {
        if (a.sprite != b.sprite) return false;
        float ax, ay, az, bx, by, bz;
        memcpy(&ax, &a.x, 4); memcpy(&ay, &a.y, 4); memcpy(&az, &a.z, 4);
        memcpy(&bx, &b.x, 4); memcpy(&by, &b.y, 4); memcpy(&bz, &b.z, 4);
        // Mission contact coordinates are static in main.scm. A little tolerance
        // allows benign float roundoff but not unrelated contacts across town.
        return std::isfinite(ax) && std::isfinite(ay) && std::isfinite(az) &&
               std::isfinite(bx) && std::isfinite(by) && std::isfinite(bz) &&
               std::fabs(ax - bx) <= 0.5f && std::fabs(ay - by) <= 0.5f && std::fabs(az - bz) <= 1.0f;
    }
    inline bool Included(const Key &what, const Key *available, int n) {
        for (int i = 0; i < n; i++) if (Same(what, available[i])) return true;
        return false;
    }
    // Only actual mission *contact point* icons. Shop / property / objective /
    // side challenge blips use different opcodes and must stay local.
    inline bool IsContact(uint16_t opcode) { return opcode == 0x02A7 || opcode == 0x0570; }
    inline bool ValidCount(unsigned n) { return n <= MAX_CONTACTS; }
}
