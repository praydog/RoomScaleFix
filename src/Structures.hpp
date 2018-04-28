#pragma once

#include <cmath>

#include "skse64/NiTypes.h"
#include "skse64/NiNodes.h"

constexpr float SKYRIM_SCALE = 69.991249f;

class MyNiPoint3 : public NiPoint3 {
public:
    MyNiPoint3() 
        : NiPoint3{} 
    {
    }

    MyNiPoint3(const NiPoint3& other) 
        : NiPoint3{ other }
    {
    }

    MyNiPoint3(const MyNiPoint3& other)
        : NiPoint3{ other }
    {
    }

    MyNiPoint3(float x, float y, float z) 
        : NiPoint3{ x, y, z } 
    {
    }

    float Length2D() const {
        return sqrt(x * x + y * y);
    }

    float Length() const {
        return sqrt(x * x + y * y + z * z);
    }

    MyNiPoint3 operator*(float scalar) const {
        return MyNiPoint3{ x * scalar, y * scalar, z * scalar };
    }

    MyNiPoint3& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;

        return *this;
    }

    MyNiPoint3 operator*(const MyNiPoint3& other) const {
        return MyNiPoint3{ x * other.x, y * other.y, z * other.z };
    }

    MyNiPoint3& operator*= (const MyNiPoint3& other) {
        x *= other.x; 
        y *= other.y; 
        z *= other.z;

        return *this;
    }

    operator NiPoint3&() {
        return *this;
    }
};


// 0x20 bytes within an hkpCachingShapePhantom
struct PositionContainer {
    char pad_0000[256]; //0x0000
    NiPoint3 pos; //0x0100
    char pad_010C[64]; //0x010C
}; //Size: 0x014C

struct hkpCachingShapePhantom {
    char pad[0x20];
    PositionContainer container;
};

struct bhkCharProxyController {
    char pad_0000[504]; //0x0000
    void* N0000052A; //0x01F8
    char pad_0200[400]; //0x0200
    PositionContainer* positionContainer; //0x0390
    char pad_0398[1200]; //0x0398
}; //Size: 0x0848

enum class HMDFlag {
    FLAG_ALLOW_UPDATE = (1 << 0),
    FLAG_DIRTY = (1 << 2)
};

struct MyPlayer {
    char pad_0000[104]; //0x0000
    class MyLoadedState* loadedState; //0x0068
    char pad_0070[88]; //0x0070
    class N000002C0* N00000024; //0x00C8
    class N000002B5* N00000025; //0x00D0
    char pad_00D8[24]; //0x00D8
    class ActorProcessManager* processManager; //0x00F0
    char pad_00F8[760]; //0x00F8
    class NiNode* playerWorldNode; //0x03F0
    class NiNode* followNode; //0x03F8
    class NiNode* followOffset; //0x0400
    char pad_0408[8]; //0x0408
    class NiNode* roomNode; //0x0410
    char pad_0418[8]; //0x0418
    class NiNode* uiNode; //0x0420
    char pad_0428[320]; //0x0428
    class NiNode* hmdNode; //0x0568
    class NiNode* lastSyncPos; //0x0570
    class NiNode* uprightHmdNode; //0x0578
    char pad_0580[3398]; //0x0580
    uint8_t hmdFlags; //0x12C6
    char pad_12C7[121]; //0x12C7
}; //Size: 0x1040

struct MyNiNode {
    char crap[0x138];
    NiTArray <NiAVObject *>	children;
};