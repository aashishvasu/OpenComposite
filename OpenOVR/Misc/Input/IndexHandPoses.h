#pragma once
#include "generated/interfaces/public_vrtypes.h"
#include <array>

using BoneArray = std::array<vr::VRBoneTransform_t, 31>;

namespace knuckles {
const BoneArray leftBindPose {
    vr::VRBoneTransform_t { // Bone 0
        .position = { 0.00000f, 0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 1
        .position = { -0.00016f, -0.00003f, 0.00063f },
        .orientation = { .w = 0.00000f, .x = 0.00000f, .y = 1.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 2
        .position = { -0.01791f, 0.02918f, 0.02530f },
        .orientation = { .w = 0.27639f, .x = 0.54119f, .y = 0.18203f, .z = 0.77304f }
    },
    vr::VRBoneTransform_t { // Bone 3
        .position = { 0.04041f, -0.00000f, 0.00000f },
        .orientation = { .w = 0.96917f, .x = 0.00006f, .y = -0.00137f, .z = 0.24638f }
    },
    vr::VRBoneTransform_t { // Bone 4
        .position = { 0.03252f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.98817f, .x = 0.00010f, .y = 0.00140f, .z = 0.15334f }
    },
    vr::VRBoneTransform_t { // Bone 5
        .position = { 0.03046f, 0.00000f, -0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 6
        .position = { -0.00156f, 0.02107f, 0.01479f },
        .orientation = { .w = 0.55075f, .x = 0.53106f, .y = -0.35143f, .z = 0.53958f }
    },
    vr::VRBoneTransform_t { // Bone 7
        .position = { 0.07380f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.96898f, .x = 0.00162f, .y = -0.05289f, .z = 0.24140f }
    },
    vr::VRBoneTransform_t { // Bone 8
        .position = { 0.04329f, 0.00000f, -0.00000f },
        .orientation = { .w = 0.98277f, .x = -0.00009f, .y = 0.00504f, .z = 0.18476f }
    },
    vr::VRBoneTransform_t { // Bone 9
        .position = { 0.02828f, 0.00000f, -0.00000f },
        .orientation = { .w = 0.99707f, .x = 0.00003f, .y = -0.00117f, .z = 0.07646f }
    },
    vr::VRBoneTransform_t { // Bone 10
        .position = { 0.02282f, 0.00000f, -0.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 11
        .position = { 0.00218f, 0.00712f, 0.01632f },
        .orientation = { .w = 0.53342f, .x = 0.56175f, .y = -0.41974f, .z = 0.47299f }
    },
    vr::VRBoneTransform_t { // Bone 12
        .position = { 0.07089f, 0.00000f, -0.00000f },
        .orientation = { .w = 0.97339f, .x = -0.00000f, .y = -0.00019f, .z = 0.22916f }
    },
    vr::VRBoneTransform_t { // Bone 13
        .position = { 0.04311f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.98753f, .x = 0.00009f, .y = -0.00369f, .z = 0.15740f }
    },
    vr::VRBoneTransform_t { // Bone 14
        .position = { 0.03327f, -0.00000f, 0.00000f },
        .orientation = { .w = 0.98996f, .x = -0.00011f, .y = 0.00413f, .z = 0.14128f }
    },
    vr::VRBoneTransform_t { // Bone 15
        .position = { 0.02589f, -0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = -0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 16
        .position = { 0.00051f, -0.00655f, 0.01635f },
        .orientation = { .w = 0.51669f, .x = 0.55014f, .y = -0.49555f, .z = 0.42989f }
    },
    vr::VRBoneTransform_t { // Bone 17
        .position = { 0.06597f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.97456f, .x = -0.00090f, .y = -0.04096f, .z = 0.22037f }
    },
    vr::VRBoneTransform_t { // Bone 18
        .position = { 0.04033f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.99100f, .x = -0.00007f, .y = 0.00253f, .z = 0.13383f }
    },
    vr::VRBoneTransform_t { // Bone 19
        .position = { 0.02849f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.99079f, .x = 0.00020f, .y = -0.00426f, .z = 0.13535f }
    },
    vr::VRBoneTransform_t { // Bone 20
        .position = { 0.02243f, 0.00000f, -0.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = -0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 21
        .position = { -0.00248f, -0.01898f, 0.01521f },
        .orientation = { .w = -0.48576f, .x = -0.51533f, .y = 0.61502f, .z = -0.34675f }
    },
    vr::VRBoneTransform_t { // Bone 22
        .position = { 0.06286f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.99349f, .x = 0.00394f, .y = 0.02816f, .z = 0.11031f }
    },
    vr::VRBoneTransform_t { // Bone 23
        .position = { 0.02987f, -0.00000f, 0.00000f },
        .orientation = { .w = 0.99111f, .x = 0.00038f, .y = -0.01146f, .z = 0.13252f }
    },
    vr::VRBoneTransform_t { // Bone 24
        .position = { 0.01798f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.99401f, .x = -0.00054f, .y = 0.01270f, .z = 0.10858f }
    },
    vr::VRBoneTransform_t { // Bone 25
        .position = { 0.01802f, -0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = -0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 26
        .position = { 0.03928f, 0.06008f, -0.08449f },
        .orientation = { .w = 0.04504f, .x = 0.81959f, .y = -0.04861f, .z = -0.56911f }
    },
    vr::VRBoneTransform_t { // Bone 27
        .position = { 0.01823f, 0.03728f, -0.14896f },
        .orientation = { .w = 0.59723f, .x = 0.70842f, .y = 0.20956f, .z = -0.31233f }
    },
    vr::VRBoneTransform_t { // Bone 28
        .position = { 0.01256f, 0.00787f, -0.15469f },
        .orientation = { .w = 0.64706f, .x = 0.67740f, .y = 0.22114f, .z = -0.27117f }
    },
    vr::VRBoneTransform_t { // Bone 29
        .position = { 0.01787f, -0.02324f, -0.14224f },
        .orientation = { .w = 0.72163f, .x = 0.59503f, .y = 0.23741f, .z = -0.26235f }
    },
    vr::VRBoneTransform_t { // Bone 30
        .position = { 0.01601f, -0.04565f, -0.11928f },
        .orientation = { .w = 0.73903f, .x = 0.51142f, .y = 0.34900f, .z = -0.26548f }
    },
};

const BoneArray leftOpenHandPose {
    vr::VRBoneTransform_t { // Bone 0
        .position = { 0.00000f, 0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 1
        .position = { -0.03404f, 0.03650f, 0.16472f },
        .orientation = { .w = -0.05515f, .x = -0.07861f, .y = -0.92028f, .z = 0.37930f }
    },
    vr::VRBoneTransform_t { // Bone 2
        .position = { -0.01208f, 0.02807f, 0.02505f },
        .orientation = { .w = 0.46411f, .x = 0.56742f, .y = 0.27211f, .z = 0.62337f }
    },
    vr::VRBoneTransform_t { // Bone 3
        .position = { 0.04041f, 0.00000f, -0.00000f },
        .orientation = { .w = 0.99484f, .x = 0.08294f, .y = 0.01945f, .z = 0.05513f }
    },
    vr::VRBoneTransform_t { // Bone 4
        .position = { 0.03252f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.97479f, .x = -0.00321f, .y = 0.02187f, .z = -0.22201f }
    },
    vr::VRBoneTransform_t { // Bone 5
        .position = { 0.03046f, -0.00000f, -0.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 6
        .position = { 0.00063f, 0.02687f, 0.01500f },
        .orientation = { .w = 0.64425f, .x = 0.42198f, .y = -0.47820f, .z = 0.42213f }
    },
    vr::VRBoneTransform_t { // Bone 7
        .position = { 0.07420f, -0.00500f, 0.00023f },
        .orientation = { .w = 0.99533f, .x = 0.00701f, .y = -0.03912f, .z = 0.08795f }
    },
    vr::VRBoneTransform_t { // Bone 8
        .position = { 0.04393f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.99789f, .x = 0.04581f, .y = 0.00214f, .z = -0.04594f }
    },
    vr::VRBoneTransform_t { // Bone 9
        .position = { 0.02870f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.99965f, .x = 0.00185f, .y = -0.02278f, .z = -0.01341f }
    },
    vr::VRBoneTransform_t { // Bone 10
        .position = { 0.02282f, 0.00000f, -0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 11
        .position = { 0.00218f, 0.00712f, 0.01632f },
        .orientation = { .w = 0.54672f, .x = 0.54128f, .y = -0.44252f, .z = 0.46075f }
    },
    vr::VRBoneTransform_t { // Bone 12
        .position = { 0.07095f, 0.00078f, 0.00100f },
        .orientation = { .w = 0.98029f, .x = -0.16726f, .y = -0.07896f, .z = 0.06937f }
    },
    vr::VRBoneTransform_t { // Bone 13
        .position = { 0.04311f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.99795f, .x = 0.01849f, .y = 0.01319f, .z = 0.05989f }
    },
    vr::VRBoneTransform_t { // Bone 14
        .position = { 0.03327f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.99739f, .x = -0.00333f, .y = -0.02823f, .z = -0.06632f }
    },
    vr::VRBoneTransform_t { // Bone 15
        .position = { 0.02589f, -0.00000f, 0.00000f },
        .orientation = { .w = 0.99919f, .x = 0.00000f, .y = 0.00000f, .z = 0.04013f }
    },
    vr::VRBoneTransform_t { // Bone 16
        .position = { 0.00051f, -0.00655f, 0.01635f },
        .orientation = { .w = 0.51669f, .x = 0.55014f, .y = -0.49555f, .z = 0.42989f }
    },
    vr::VRBoneTransform_t { // Bone 17
        .position = { 0.06588f, 0.00179f, 0.00069f },
        .orientation = { .w = 0.99042f, .x = -0.05870f, .y = -0.10182f, .z = 0.07250f }
    },
    vr::VRBoneTransform_t { // Bone 18
        .position = { 0.04070f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.99954f, .x = -0.00224f, .y = 0.00000f, .z = 0.03008f }
    },
    vr::VRBoneTransform_t { // Bone 19
        .position = { 0.02875f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.99910f, .x = -0.00072f, .y = -0.01269f, .z = 0.04042f }
    },
    vr::VRBoneTransform_t { // Bone 20
        .position = { 0.02243f, -0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 21
        .position = { -0.00248f, -0.01898f, 0.01521f },
        .orientation = { .w = 0.52692f, .x = 0.52394f, .y = -0.58402f, .z = 0.32674f }
    },
    vr::VRBoneTransform_t { // Bone 22
        .position = { 0.06288f, 0.00284f, 0.00033f },
        .orientation = { .w = 0.98661f, .x = -0.05961f, .y = -0.13516f, .z = 0.06913f }
    },
    vr::VRBoneTransform_t { // Bone 23
        .position = { 0.03022f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.99432f, .x = 0.00190f, .y = -0.00013f, .z = 0.10645f }
    },
    vr::VRBoneTransform_t { // Bone 24
        .position = { 0.01819f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.99593f, .x = -0.00201f, .y = -0.05208f, .z = -0.07353f }
    },
    vr::VRBoneTransform_t { // Bone 25
        .position = { 0.01802f, 0.00000f, -0.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = -0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 26
        .position = { -0.00606f, 0.05629f, 0.06006f },
        .orientation = { .w = 0.73724f, .x = 0.20275f, .y = 0.59427f, .z = 0.24944f }
    },
    vr::VRBoneTransform_t { // Bone 27
        .position = { -0.04042f, -0.04302f, 0.01934f },
        .orientation = { .w = -0.29033f, .x = 0.62353f, .y = -0.66381f, .z = -0.29373f }
    },
    vr::VRBoneTransform_t { // Bone 28
        .position = { -0.03935f, -0.07567f, 0.04705f },
        .orientation = { .w = -0.18705f, .x = 0.67806f, .y = -0.65929f, .z = -0.26568f }
    },
    vr::VRBoneTransform_t { // Bone 29
        .position = { -0.03834f, -0.09099f, 0.08258f },
        .orientation = { .w = -0.18304f, .x = 0.73679f, .y = -0.63476f, .z = -0.14394f }
    },
    vr::VRBoneTransform_t { // Bone 30
        .position = { -0.03181f, -0.08721f, 0.12102f },
        .orientation = { .w = -0.00366f, .x = 0.75841f, .y = -0.63934f, .z = -0.12668f }
    },
};

const BoneArray leftFistPose {
    vr::VRBoneTransform_t { // Bone 0
        .position = { 0.00000f, 0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 1
        .position = { -0.03404f, 0.03650f, 0.16472f },
        .orientation = { .w = -0.05515f, .x = -0.07861f, .y = -0.92028f, .z = 0.37930f }
    },
    vr::VRBoneTransform_t { // Bone 2
        .position = { -0.01631f, 0.02753f, 0.01780f },
        .orientation = { .w = 0.22570f, .x = 0.48333f, .y = 0.12641f, .z = 0.83634f }
    },
    vr::VRBoneTransform_t { // Bone 3
        .position = { 0.04041f, 0.00000f, -0.00000f },
        .orientation = { .w = 0.89434f, .x = -0.01330f, .y = -0.08290f, .z = 0.43945f }
    },
    vr::VRBoneTransform_t { // Bone 4
        .position = { 0.03252f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.84243f, .x = 0.00065f, .y = 0.00124f, .z = 0.53881f }
    },
    vr::VRBoneTransform_t { // Bone 5
        .position = { 0.03046f, -0.00000f, -0.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 6
        .position = { 0.00380f, 0.02151f, 0.01280f },
        .orientation = { .w = 0.61731f, .x = 0.39517f, .y = -0.51087f, .z = 0.44919f }
    },
    vr::VRBoneTransform_t { // Bone 7
        .position = { 0.07420f, -0.00500f, 0.00023f },
        .orientation = { .w = 0.73729f, .x = -0.03201f, .y = -0.11501f, .z = 0.66494f }
    },
    vr::VRBoneTransform_t { // Bone 8
        .position = { 0.04329f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.61138f, .x = 0.00329f, .y = 0.00382f, .z = 0.79132f }
    },
    vr::VRBoneTransform_t { // Bone 9
        .position = { 0.02828f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.74539f, .x = -0.00068f, .y = -0.00095f, .z = 0.66663f }
    },
    vr::VRBoneTransform_t { // Bone 10
        .position = { 0.02282f, 0.00000f, -0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 11
        .position = { 0.00579f, 0.00681f, 0.01653f },
        .orientation = { .w = 0.51420f, .x = 0.52232f, .y = -0.47835f, .z = 0.48370f }
    },
    vr::VRBoneTransform_t { // Bone 12
        .position = { 0.07095f, 0.00078f, 0.00100f },
        .orientation = { .w = 0.72365f, .x = -0.09790f, .y = 0.04855f, .z = 0.68146f }
    },
    vr::VRBoneTransform_t { // Bone 13
        .position = { 0.04311f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.63746f, .x = -0.00237f, .y = -0.00283f, .z = 0.77047f }
    },
    vr::VRBoneTransform_t { // Bone 14
        .position = { 0.03327f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.65801f, .x = 0.00261f, .y = 0.00320f, .z = 0.75300f }
    },
    vr::VRBoneTransform_t { // Bone 15
        .position = { 0.02589f, -0.00000f, 0.00000f },
        .orientation = { .w = 0.99919f, .x = 0.00000f, .y = 0.00000f, .z = 0.04013f }
    },
    vr::VRBoneTransform_t { // Bone 16
        .position = { 0.00412f, -0.00686f, 0.01656f },
        .orientation = { .w = 0.48961f, .x = 0.52337f, .y = -0.52064f, .z = 0.46400f }
    },
    vr::VRBoneTransform_t { // Bone 17
        .position = { 0.06588f, 0.00179f, 0.00069f },
        .orientation = { .w = 0.75997f, .x = -0.05561f, .y = 0.01157f, .z = 0.64747f }
    },
    vr::VRBoneTransform_t { // Bone 18
        .position = { 0.04033f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.66431f, .x = 0.00159f, .y = 0.00197f, .z = 0.74745f }
    },
    vr::VRBoneTransform_t { // Bone 19
        .position = { 0.02849f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.62696f, .x = -0.00278f, .y = -0.00323f, .z = 0.77904f }
    },
    vr::VRBoneTransform_t { // Bone 20
        .position = { 0.02243f, -0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 21
        .position = { 0.00113f, -0.01929f, 0.01543f },
        .orientation = { .w = 0.47977f, .x = 0.47783f, .y = -0.63020f, .z = 0.37993f }
    },
    vr::VRBoneTransform_t { // Bone 22
        .position = { 0.06288f, 0.00284f, 0.00033f },
        .orientation = { .w = 0.82700f, .x = 0.03428f, .y = 0.00344f, .z = 0.56114f }
    },
    vr::VRBoneTransform_t { // Bone 23
        .position = { 0.02987f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.70218f, .x = -0.00672f, .y = -0.00929f, .z = 0.71190f }
    },
    vr::VRBoneTransform_t { // Bone 24
        .position = { 0.01798f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.67685f, .x = 0.00796f, .y = 0.00992f, .z = 0.73601f }
    },
    vr::VRBoneTransform_t { // Bone 25
        .position = { 0.01802f, 0.00000f, -0.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = -0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 26
        .position = { 0.01972f, 0.00280f, 0.09394f },
        .orientation = { .w = 0.37729f, .x = -0.54083f, .y = 0.15045f, .z = -0.73656f }
    },
    vr::VRBoneTransform_t { // Bone 27
        .position = { 0.00017f, 0.01647f, 0.09652f },
        .orientation = { .w = -0.00646f, .x = 0.02275f, .y = -0.93293f, .z = -0.35929f }
    },
    vr::VRBoneTransform_t { // Bone 28
        .position = { 0.00045f, 0.00154f, 0.11654f },
        .orientation = { .w = -0.03936f, .x = 0.10514f, .y = -0.92883f, .z = -0.35308f }
    },
    vr::VRBoneTransform_t { // Bone 29
        .position = { 0.00395f, -0.01487f, 0.13061f },
        .orientation = { .w = -0.05507f, .x = 0.06870f, .y = -0.94402f, .z = -0.31793f }
    },
    vr::VRBoneTransform_t { // Bone 30
        .position = { 0.00326f, -0.03469f, 0.13993f },
        .orientation = { .w = 0.01969f, .x = -0.10074f, .y = -0.95733f, .z = -0.27015f }
    },
};

const BoneArray leftGripLimitPose {
    vr::VRBoneTransform_t { // Bone 0
        .position = { 0.00000f, 0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 1
        .position = { -0.04273f, 0.03744f, 0.16297f },
        .orientation = { .w = -0.07540f, .x = -0.08407f, .y = -0.91727f, .z = 0.38192f }
    },
    vr::VRBoneTransform_t { // Bone 2
        .position = { -0.01791f, 0.02918f, 0.02530f },
        .orientation = { .w = 0.44897f, .x = 0.54188f, .y = 0.04745f, .z = 0.70890f }
    },
    vr::VRBoneTransform_t { // Bone 3
        .position = { 0.04126f, 0.00000f, -0.00000f },
        .orientation = { .w = 0.99687f, .x = 0.07906f, .y = -0.00001f, .z = -0.00001f }
    },
    vr::VRBoneTransform_t { // Bone 4
        .position = { 0.03321f, -0.00000f, 0.00000f },
        .orientation = { .w = 0.99966f, .x = 0.01202f, .y = 0.00646f, .z = -0.02225f }
    },
    vr::VRBoneTransform_t { // Bone 5
        .position = { 0.03046f, -0.00000f, -0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = -0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 6
        .position = { -0.00393f, 0.02717f, 0.01464f },
        .orientation = { .w = 0.66784f, .x = 0.43168f, .y = -0.45441f, .z = 0.40143f }
    },
    vr::VRBoneTransform_t { // Bone 7
        .position = { 0.07602f, -0.00512f, 0.00024f },
        .orientation = { .w = 0.95834f, .x = 0.01474f, .y = -0.15100f, .z = 0.24199f }
    },
    vr::VRBoneTransform_t { // Bone 8
        .position = { 0.04393f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.89354f, .x = 0.04103f, .y = -0.02047f, .z = 0.44663f }
    },
    vr::VRBoneTransform_t { // Bone 9
        .position = { 0.02870f, -0.00000f, 0.00000f },
        .orientation = { .w = 0.92972f, .x = -0.06558f, .y = -0.10143f, .z = 0.34789f }
    },
    vr::VRBoneTransform_t { // Bone 10
        .position = { 0.02282f, 0.00000f, -0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = -0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 11
        .position = { 0.00218f, 0.00712f, 0.01632f },
        .orientation = { .w = 0.52936f, .x = 0.54051f, .y = -0.46378f, .z = 0.46101f }
    },
    vr::VRBoneTransform_t { // Bone 12
        .position = { 0.07095f, 0.00078f, 0.00100f },
        .orientation = { .w = 0.83245f, .x = -0.14589f, .y = -0.09844f, .z = 0.52541f }
    },
    vr::VRBoneTransform_t { // Bone 13
        .position = { 0.04311f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.82400f, .x = 0.00363f, .y = 0.02949f, .z = 0.56580f }
    },
    vr::VRBoneTransform_t { // Bone 14
        .position = { 0.03327f, -0.00000f, 0.00000f },
        .orientation = { .w = 0.96113f, .x = 0.00662f, .y = -0.01084f, .z = 0.27580f }
    },
    vr::VRBoneTransform_t { // Bone 15
        .position = { 0.02589f, -0.00000f, 0.00000f },
        .orientation = { .w = 0.99919f, .x = 0.00000f, .y = -0.00000f, .z = 0.04013f }
    },
    vr::VRBoneTransform_t { // Bone 16
        .position = { 0.00051f, -0.00655f, 0.01635f },
        .orientation = { .w = 0.50024f, .x = 0.53078f, .y = -0.51622f, .z = 0.44894f }
    },
    vr::VRBoneTransform_t { // Bone 17
        .position = { 0.06588f, 0.00179f, 0.00069f },
        .orientation = { .w = 0.81094f, .x = -0.13737f, .y = -0.11937f, .z = 0.55612f }
    },
    vr::VRBoneTransform_t { // Bone 18
        .position = { 0.04070f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.80612f, .x = -0.00183f, .y = 0.00126f, .z = 0.59175f }
    },
    vr::VRBoneTransform_t { // Bone 19
        .position = { 0.02875f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.99345f, .x = -0.01181f, .y = -0.01673f, .z = 0.11242f }
    },
    vr::VRBoneTransform_t { // Bone 20
        .position = { 0.02243f, -0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = -0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 21
        .position = { -0.00248f, -0.01898f, 0.01521f },
        .orientation = { .w = 0.47467f, .x = 0.43467f, .y = -0.65321f, .z = 0.39883f }
    },
    vr::VRBoneTransform_t { // Bone 22
        .position = { 0.06288f, 0.00284f, 0.00033f },
        .orientation = { .w = 0.79597f, .x = -0.06975f, .y = -0.15089f, .z = 0.58207f }
    },
    vr::VRBoneTransform_t { // Bone 23
        .position = { 0.03022f, 0.00000f, -0.00000f },
        .orientation = { .w = 0.83985f, .x = 0.00162f, .y = -0.00095f, .z = 0.54281f }
    },
    vr::VRBoneTransform_t { // Bone 24
        .position = { 0.01819f, -0.00000f, 0.00000f },
        .orientation = { .w = 0.99862f, .x = -0.00715f, .y = -0.03107f, .z = 0.04168f }
    },
    vr::VRBoneTransform_t { // Bone 25
        .position = { 0.01802f, 0.00000f, -0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = -0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 26
        .position = { -0.00167f, 0.03486f, 0.05613f },
        .orientation = { .w = 0.76507f, .x = -0.13211f, .y = 0.62048f, .z = 0.11054f }
    },
    vr::VRBoneTransform_t { // Bone 27
        .position = { -0.00660f, -0.03617f, 0.04355f },
        .orientation = { .w = -0.25766f, .x = 0.93566f, .y = 0.14735f, .z = 0.19088f }
    },
    vr::VRBoneTransform_t { // Bone 28
        .position = { 0.01649f, -0.02293f, 0.09679f },
        .orientation = { .w = -0.20391f, .x = 0.79901f, .y = 0.54935f, .z = 0.13497f }
    },
    vr::VRBoneTransform_t { // Bone 29
        .position = { 0.01367f, -0.02704f, 0.12021f },
        .orientation = { .w = -0.11925f, .x = 0.85137f, .y = 0.49273f, .z = 0.13474f }
    },
    vr::VRBoneTransform_t { // Bone 30
        .position = { 0.01126f, -0.02786f, 0.14022f },
        .orientation = { .w = -0.08589f, .x = 0.84083f, .y = 0.51969f, .z = 0.12467f }
    },
};

const BoneArray rightBindPose {
    vr::VRBoneTransform_t { // Bone 0
        .position = { 0.00000f, 0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 1
        .position = { 0.00016f, -0.00003f, 0.00063f },
        .orientation = { .w = 0.00000f, .x = 0.00000f, .y = 1.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 2
        .position = { 0.01791f, 0.02918f, 0.02530f },
        .orientation = { .w = 0.54119f, .x = -0.27639f, .y = 0.77304f, .z = -0.18203f }
    },
    vr::VRBoneTransform_t { // Bone 3
        .position = { -0.04041f, -0.00000f, 0.00000f },
        .orientation = { .w = 0.96917f, .x = 0.00006f, .y = -0.00137f, .z = 0.24638f }
    },
    vr::VRBoneTransform_t { // Bone 4
        .position = { -0.03252f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.98817f, .x = 0.00010f, .y = 0.00140f, .z = 0.15334f }
    },
    vr::VRBoneTransform_t { // Bone 5
        .position = { -0.03046f, 0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 6
        .position = { 0.00156f, 0.02107f, 0.01479f },
        .orientation = { .w = 0.53106f, .x = -0.55075f, .y = 0.53958f, .z = 0.35143f }
    },
    vr::VRBoneTransform_t { // Bone 7
        .position = { -0.07380f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.96898f, .x = 0.00162f, .y = -0.05289f, .z = 0.24140f }
    },
    vr::VRBoneTransform_t { // Bone 8
        .position = { -0.04329f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.98277f, .x = -0.00009f, .y = 0.00504f, .z = 0.18476f }
    },
    vr::VRBoneTransform_t { // Bone 9
        .position = { -0.02828f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.99707f, .x = 0.00003f, .y = -0.00117f, .z = 0.07646f }
    },
    vr::VRBoneTransform_t { // Bone 10
        .position = { -0.02282f, -0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = -0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 11
        .position = { -0.00218f, 0.00712f, 0.01632f },
        .orientation = { .w = 0.56175f, .x = -0.53342f, .y = 0.47299f, .z = 0.41974f }
    },
    vr::VRBoneTransform_t { // Bone 12
        .position = { -0.07089f, 0.00000f, -0.00000f },
        .orientation = { .w = 0.97339f, .x = -0.00000f, .y = -0.00019f, .z = 0.22916f }
    },
    vr::VRBoneTransform_t { // Bone 13
        .position = { -0.04311f, -0.00000f, 0.00000f },
        .orientation = { .w = 0.98753f, .x = 0.00009f, .y = -0.00369f, .z = 0.15740f }
    },
    vr::VRBoneTransform_t { // Bone 14
        .position = { -0.03327f, 0.00000f, -0.00000f },
        .orientation = { .w = 0.98996f, .x = -0.00011f, .y = 0.00413f, .z = 0.14128f }
    },
    vr::VRBoneTransform_t { // Bone 15
        .position = { -0.02589f, 0.00000f, -0.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = -0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 16
        .position = { -0.00051f, -0.00655f, 0.01635f },
        .orientation = { .w = 0.55014f, .x = -0.51669f, .y = 0.42989f, .z = 0.49555f }
    },
    vr::VRBoneTransform_t { // Bone 17
        .position = { -0.06597f, -0.00000f, 0.00000f },
        .orientation = { .w = 0.97456f, .x = -0.00090f, .y = -0.04096f, .z = 0.22037f }
    },
    vr::VRBoneTransform_t { // Bone 18
        .position = { -0.04033f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.99100f, .x = -0.00007f, .y = 0.00253f, .z = 0.13383f }
    },
    vr::VRBoneTransform_t { // Bone 19
        .position = { -0.02849f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.99079f, .x = 0.00020f, .y = -0.00426f, .z = 0.13535f }
    },
    vr::VRBoneTransform_t { // Bone 20
        .position = { -0.02243f, 0.00000f, -0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = -0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 21
        .position = { 0.00248f, -0.01898f, 0.01521f },
        .orientation = { .w = 0.51533f, .x = -0.48576f, .y = 0.34675f, .z = 0.61502f }
    },
    vr::VRBoneTransform_t { // Bone 22
        .position = { -0.06286f, 0.00000f, -0.00000f },
        .orientation = { .w = 0.99349f, .x = 0.00394f, .y = 0.02816f, .z = 0.11031f }
    },
    vr::VRBoneTransform_t { // Bone 23
        .position = { -0.02987f, -0.00000f, 0.00000f },
        .orientation = { .w = 0.99111f, .x = 0.00038f, .y = -0.01146f, .z = 0.13252f }
    },
    vr::VRBoneTransform_t { // Bone 24
        .position = { -0.01798f, 0.00000f, -0.00000f },
        .orientation = { .w = 0.99401f, .x = -0.00054f, .y = 0.01270f, .z = 0.10858f }
    },
    vr::VRBoneTransform_t { // Bone 25
        .position = { -0.01802f, -0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = -0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 26
        .position = { -0.03928f, 0.06008f, -0.08449f },
        .orientation = { .w = 0.81959f, .x = -0.04504f, .y = -0.56911f, .z = 0.04861f }
    },
    vr::VRBoneTransform_t { // Bone 27
        .position = { -0.01823f, 0.03728f, -0.14896f },
        .orientation = { .w = 0.70842f, .x = -0.59723f, .y = -0.31233f, .z = -0.20956f }
    },
    vr::VRBoneTransform_t { // Bone 28
        .position = { -0.01256f, 0.00787f, -0.15469f },
        .orientation = { .w = 0.67740f, .x = -0.64706f, .y = -0.27117f, .z = -0.22114f }
    },
    vr::VRBoneTransform_t { // Bone 29
        .position = { -0.01787f, -0.02324f, -0.14224f },
        .orientation = { .w = 0.59503f, .x = -0.72163f, .y = -0.26235f, .z = -0.23741f }
    },
    vr::VRBoneTransform_t { // Bone 30
        .position = { -0.01601f, -0.04565f, -0.11928f },
        .orientation = { .w = 0.51142f, .x = -0.73903f, .y = -0.26548f, .z = -0.34900f }
    },
};

const BoneArray rightOpenHandPose {
    vr::VRBoneTransform_t { // Bone 0
        .position = { 0.00000f, 0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 1
        .position = { 0.03404f, 0.03650f, 0.16472f },
        .orientation = { .w = -0.05515f, .x = -0.07861f, .y = 0.92028f, .z = -0.37930f }
    },
    vr::VRBoneTransform_t { // Bone 2
        .position = { 0.01208f, 0.02807f, 0.02505f },
        .orientation = { .w = 0.56742f, .x = -0.46411f, .y = 0.62337f, .z = -0.27211f }
    },
    vr::VRBoneTransform_t { // Bone 3
        .position = { -0.04041f, -0.00000f, 0.00000f },
        .orientation = { .w = 0.99484f, .x = 0.08294f, .y = 0.01945f, .z = 0.05513f }
    },
    vr::VRBoneTransform_t { // Bone 4
        .position = { -0.03252f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.97479f, .x = -0.00321f, .y = 0.02187f, .z = -0.22201f }
    },
    vr::VRBoneTransform_t { // Bone 5
        .position = { -0.03046f, 0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 6
        .position = { -0.00063f, 0.02687f, 0.01500f },
        .orientation = { .w = 0.42198f, .x = -0.64425f, .y = 0.42213f, .z = 0.47820f }
    },
    vr::VRBoneTransform_t { // Bone 7
        .position = { -0.07420f, 0.00500f, -0.00023f },
        .orientation = { .w = 0.99533f, .x = 0.00701f, .y = -0.03912f, .z = 0.08795f }
    },
    vr::VRBoneTransform_t { // Bone 8
        .position = { -0.04393f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.99789f, .x = 0.04581f, .y = 0.00214f, .z = -0.04594f }
    },
    vr::VRBoneTransform_t { // Bone 9
        .position = { -0.02870f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.99965f, .x = 0.00185f, .y = -0.02278f, .z = -0.01341f }
    },
    vr::VRBoneTransform_t { // Bone 10
        .position = { -0.02282f, -0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 11
        .position = { -0.00218f, 0.00712f, 0.01632f },
        .orientation = { .w = 0.54128f, .x = -0.54672f, .y = 0.46075f, .z = 0.44252f }
    },
    vr::VRBoneTransform_t { // Bone 12
        .position = { -0.07095f, -0.00078f, -0.00100f },
        .orientation = { .w = 0.98029f, .x = -0.16726f, .y = -0.07896f, .z = 0.06937f }
    },
    vr::VRBoneTransform_t { // Bone 13
        .position = { -0.04311f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.99795f, .x = 0.01849f, .y = 0.01319f, .z = 0.05989f }
    },
    vr::VRBoneTransform_t { // Bone 14
        .position = { -0.03327f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.99739f, .x = -0.00333f, .y = -0.02823f, .z = -0.06632f }
    },
    vr::VRBoneTransform_t { // Bone 15
        .position = { -0.02589f, 0.00000f, -0.00000f },
        .orientation = { .w = 0.99919f, .x = 0.00000f, .y = 0.00000f, .z = 0.04013f }
    },
    vr::VRBoneTransform_t { // Bone 16
        .position = { -0.00051f, -0.00655f, 0.01635f },
        .orientation = { .w = 0.55014f, .x = -0.51669f, .y = 0.42989f, .z = 0.49555f }
    },
    vr::VRBoneTransform_t { // Bone 17
        .position = { -0.06588f, -0.00179f, -0.00069f },
        .orientation = { .w = 0.99042f, .x = -0.05870f, .y = -0.10182f, .z = 0.07250f }
    },
    vr::VRBoneTransform_t { // Bone 18
        .position = { -0.04070f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.99954f, .x = -0.00224f, .y = 0.00000f, .z = 0.03008f }
    },
    vr::VRBoneTransform_t { // Bone 19
        .position = { -0.02875f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.99910f, .x = -0.00072f, .y = -0.01269f, .z = 0.04042f }
    },
    vr::VRBoneTransform_t { // Bone 20
        .position = { -0.02243f, 0.00000f, -0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 21
        .position = { 0.00248f, -0.01898f, 0.01521f },
        .orientation = { .w = 0.52394f, .x = -0.52692f, .y = 0.32674f, .z = 0.58402f }
    },
    vr::VRBoneTransform_t { // Bone 22
        .position = { -0.06288f, -0.00284f, -0.00033f },
        .orientation = { .w = 0.98661f, .x = -0.05961f, .y = -0.13516f, .z = 0.06913f }
    },
    vr::VRBoneTransform_t { // Bone 23
        .position = { -0.03022f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.99432f, .x = 0.00190f, .y = -0.00013f, .z = 0.10645f }
    },
    vr::VRBoneTransform_t { // Bone 24
        .position = { -0.01819f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.99593f, .x = -0.00201f, .y = -0.05208f, .z = -0.07353f }
    },
    vr::VRBoneTransform_t { // Bone 25
        .position = { -0.01802f, -0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = -0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 26
        .position = { 0.00606f, 0.05629f, 0.06006f },
        .orientation = { .w = 0.73724f, .x = 0.20275f, .y = -0.59427f, .z = -0.24944f }
    },
    vr::VRBoneTransform_t { // Bone 27
        .position = { 0.04042f, -0.04302f, 0.01934f },
        .orientation = { .w = -0.29033f, .x = 0.62353f, .y = 0.66381f, .z = 0.29373f }
    },
    vr::VRBoneTransform_t { // Bone 28
        .position = { 0.03935f, -0.07567f, 0.04705f },
        .orientation = { .w = -0.18705f, .x = 0.67806f, .y = 0.65929f, .z = 0.26568f }
    },
    vr::VRBoneTransform_t { // Bone 29
        .position = { 0.03834f, -0.09099f, 0.08258f },
        .orientation = { .w = -0.18304f, .x = 0.73679f, .y = 0.63476f, .z = 0.14394f }
    },
    vr::VRBoneTransform_t { // Bone 30
        .position = { 0.03181f, -0.08721f, 0.12102f },
        .orientation = { .w = -0.00366f, .x = 0.75841f, .y = 0.63934f, .z = 0.12668f }
    },
};

const BoneArray rightFistPose {
    vr::VRBoneTransform_t { // Bone 0
        .position = { 0.00000f, 0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 1
        .position = { 0.03404f, 0.03650f, 0.16472f },
        .orientation = { .w = -0.05515f, .x = -0.07861f, .y = 0.92028f, .z = -0.37930f }
    },
    vr::VRBoneTransform_t { // Bone 2
        .position = { 0.01631f, 0.02753f, 0.01780f },
        .orientation = { .w = 0.48333f, .x = -0.22570f, .y = 0.83634f, .z = -0.12641f }
    },
    vr::VRBoneTransform_t { // Bone 3
        .position = { -0.04041f, -0.00000f, 0.00000f },
        .orientation = { .w = 0.89434f, .x = -0.01330f, .y = -0.08290f, .z = 0.43945f }
    },
    vr::VRBoneTransform_t { // Bone 4
        .position = { -0.03252f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.84243f, .x = 0.00065f, .y = 0.00124f, .z = 0.53881f }
    },
    vr::VRBoneTransform_t { // Bone 5
        .position = { -0.03046f, 0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 6
        .position = { -0.00380f, 0.02151f, 0.01280f },
        .orientation = { .w = 0.39517f, .x = -0.61731f, .y = 0.44919f, .z = 0.51087f }
    },
    vr::VRBoneTransform_t { // Bone 7
        .position = { -0.07420f, 0.00500f, -0.00023f },
        .orientation = { .w = 0.73729f, .x = -0.03201f, .y = -0.11501f, .z = 0.66494f }
    },
    vr::VRBoneTransform_t { // Bone 8
        .position = { -0.04329f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.61138f, .x = 0.00329f, .y = 0.00382f, .z = 0.79132f }
    },
    vr::VRBoneTransform_t { // Bone 9
        .position = { -0.02828f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.74539f, .x = -0.00068f, .y = -0.00095f, .z = 0.66663f }
    },
    vr::VRBoneTransform_t { // Bone 10
        .position = { -0.02282f, -0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 11
        .position = { -0.00579f, 0.00681f, 0.01653f },
        .orientation = { .w = 0.52232f, .x = -0.51420f, .y = 0.48370f, .z = 0.47835f }
    },
    vr::VRBoneTransform_t { // Bone 12
        .position = { -0.07095f, -0.00078f, -0.00100f },
        .orientation = { .w = 0.72365f, .x = -0.09790f, .y = 0.04855f, .z = 0.68146f }
    },
    vr::VRBoneTransform_t { // Bone 13
        .position = { -0.04311f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.63746f, .x = -0.00237f, .y = -0.00283f, .z = 0.77047f }
    },
    vr::VRBoneTransform_t { // Bone 14
        .position = { -0.03327f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.65801f, .x = 0.00261f, .y = 0.00320f, .z = 0.75300f }
    },
    vr::VRBoneTransform_t { // Bone 15
        .position = { -0.02589f, 0.00000f, -0.00000f },
        .orientation = { .w = 0.99919f, .x = 0.00000f, .y = 0.00000f, .z = 0.04013f }
    },
    vr::VRBoneTransform_t { // Bone 16
        .position = { -0.00412f, -0.00686f, 0.01656f },
        .orientation = { .w = 0.52337f, .x = -0.48961f, .y = 0.46400f, .z = 0.52064f }
    },
    vr::VRBoneTransform_t { // Bone 17
        .position = { -0.06588f, -0.00179f, -0.00069f },
        .orientation = { .w = 0.75997f, .x = -0.05561f, .y = 0.01157f, .z = 0.64747f }
    },
    vr::VRBoneTransform_t { // Bone 18
        .position = { -0.04033f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.66431f, .x = 0.00159f, .y = 0.00197f, .z = 0.74745f }
    },
    vr::VRBoneTransform_t { // Bone 19
        .position = { -0.02849f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.62696f, .x = -0.00278f, .y = -0.00323f, .z = 0.77904f }
    },
    vr::VRBoneTransform_t { // Bone 20
        .position = { -0.02243f, 0.00000f, -0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 21
        .position = { -0.00113f, -0.01929f, 0.01543f },
        .orientation = { .w = 0.47783f, .x = -0.47977f, .y = 0.37993f, .z = 0.63020f }
    },
    vr::VRBoneTransform_t { // Bone 22
        .position = { -0.06288f, -0.00284f, -0.00033f },
        .orientation = { .w = 0.82700f, .x = 0.03428f, .y = 0.00344f, .z = 0.56114f }
    },
    vr::VRBoneTransform_t { // Bone 23
        .position = { -0.02987f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.70218f, .x = -0.00672f, .y = -0.00929f, .z = 0.71190f }
    },
    vr::VRBoneTransform_t { // Bone 24
        .position = { -0.01798f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.67685f, .x = 0.00796f, .y = 0.00992f, .z = 0.73601f }
    },
    vr::VRBoneTransform_t { // Bone 25
        .position = { -0.01802f, -0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = -0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 26
        .position = { -0.01972f, 0.00280f, 0.09394f },
        .orientation = { .w = 0.37729f, .x = -0.54083f, .y = -0.15045f, .z = 0.73656f }
    },
    vr::VRBoneTransform_t { // Bone 27
        .position = { -0.00017f, 0.01647f, 0.09652f },
        .orientation = { .w = -0.00646f, .x = 0.02275f, .y = 0.93293f, .z = 0.35929f }
    },
    vr::VRBoneTransform_t { // Bone 28
        .position = { -0.00045f, 0.00154f, 0.11654f },
        .orientation = { .w = -0.03936f, .x = 0.10514f, .y = 0.92883f, .z = 0.35308f }
    },
    vr::VRBoneTransform_t { // Bone 29
        .position = { -0.00395f, -0.01487f, 0.13061f },
        .orientation = { .w = -0.05507f, .x = 0.06870f, .y = 0.94402f, .z = 0.31793f }
    },
    vr::VRBoneTransform_t { // Bone 30
        .position = { -0.00326f, -0.03469f, 0.13993f },
        .orientation = { .w = 0.01969f, .x = -0.10074f, .y = 0.95733f, .z = 0.27015f }
    },
};

const BoneArray rightGripLimitPose {
    vr::VRBoneTransform_t { // Bone 0
        .position = { 0.00000f, 0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = 0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 1
        .position = { 0.04273f, 0.03744f, 0.16297f },
        .orientation = { .w = -0.07540f, .x = -0.08407f, .y = 0.91727f, .z = -0.38192f }
    },
    vr::VRBoneTransform_t { // Bone 2
        .position = { 0.01791f, 0.02918f, 0.02530f },
        .orientation = { .w = 0.54188f, .x = -0.44897f, .y = 0.70890f, .z = -0.04745f }
    },
    vr::VRBoneTransform_t { // Bone 3
        .position = { -0.04126f, -0.00000f, 0.00000f },
        .orientation = { .w = 0.99687f, .x = 0.07906f, .y = -0.00001f, .z = -0.00001f }
    },
    vr::VRBoneTransform_t { // Bone 4
        .position = { -0.03321f, 0.00000f, -0.00000f },
        .orientation = { .w = 0.99966f, .x = 0.01202f, .y = 0.00646f, .z = -0.02225f }
    },
    vr::VRBoneTransform_t { // Bone 5
        .position = { -0.03046f, 0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = -0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 6
        .position = { 0.00393f, 0.02717f, 0.01464f },
        .orientation = { .w = 0.43168f, .x = -0.66784f, .y = 0.40143f, .z = 0.45441f }
    },
    vr::VRBoneTransform_t { // Bone 7
        .position = { -0.07602f, 0.00512f, -0.00024f },
        .orientation = { .w = 0.95834f, .x = 0.01474f, .y = -0.15100f, .z = 0.24199f }
    },
    vr::VRBoneTransform_t { // Bone 8
        .position = { -0.04393f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.89354f, .x = 0.04103f, .y = -0.02047f, .z = 0.44663f }
    },
    vr::VRBoneTransform_t { // Bone 9
        .position = { -0.02870f, 0.00000f, -0.00000f },
        .orientation = { .w = 0.92972f, .x = -0.06558f, .y = -0.10143f, .z = 0.34789f }
    },
    vr::VRBoneTransform_t { // Bone 10
        .position = { -0.02282f, -0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = -0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 11
        .position = { -0.00218f, 0.00712f, 0.01632f },
        .orientation = { .w = 0.54051f, .x = -0.52936f, .y = 0.46101f, .z = 0.46378f }
    },
    vr::VRBoneTransform_t { // Bone 12
        .position = { -0.07095f, -0.00078f, -0.00100f },
        .orientation = { .w = 0.83245f, .x = -0.14589f, .y = -0.09844f, .z = 0.52541f }
    },
    vr::VRBoneTransform_t { // Bone 13
        .position = { -0.04311f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.82400f, .x = 0.00363f, .y = 0.02949f, .z = 0.56580f }
    },
    vr::VRBoneTransform_t { // Bone 14
        .position = { -0.03327f, 0.00000f, -0.00000f },
        .orientation = { .w = 0.96113f, .x = 0.00662f, .y = -0.01084f, .z = 0.27580f }
    },
    vr::VRBoneTransform_t { // Bone 15
        .position = { -0.02589f, 0.00000f, -0.00000f },
        .orientation = { .w = 0.99919f, .x = 0.00000f, .y = -0.00000f, .z = 0.04013f }
    },
    vr::VRBoneTransform_t { // Bone 16
        .position = { -0.00051f, -0.00655f, 0.01635f },
        .orientation = { .w = 0.53078f, .x = -0.50024f, .y = 0.44894f, .z = 0.51622f }
    },
    vr::VRBoneTransform_t { // Bone 17
        .position = { -0.06588f, -0.00179f, -0.00069f },
        .orientation = { .w = 0.81094f, .x = -0.13737f, .y = -0.11937f, .z = 0.55612f }
    },
    vr::VRBoneTransform_t { // Bone 18
        .position = { -0.04070f, -0.00000f, -0.00000f },
        .orientation = { .w = 0.80612f, .x = -0.00183f, .y = 0.00126f, .z = 0.59175f }
    },
    vr::VRBoneTransform_t { // Bone 19
        .position = { -0.02875f, 0.00000f, 0.00000f },
        .orientation = { .w = 0.99345f, .x = -0.01181f, .y = -0.01673f, .z = 0.11242f }
    },
    vr::VRBoneTransform_t { // Bone 20
        .position = { -0.02243f, 0.00000f, -0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = -0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 21
        .position = { 0.00248f, -0.01898f, 0.01521f },
        .orientation = { .w = 0.43467f, .x = -0.47467f, .y = 0.39883f, .z = 0.65321f }
    },
    vr::VRBoneTransform_t { // Bone 22
        .position = { -0.06288f, -0.00284f, -0.00033f },
        .orientation = { .w = 0.79597f, .x = -0.06975f, .y = -0.15089f, .z = 0.58207f }
    },
    vr::VRBoneTransform_t { // Bone 23
        .position = { -0.03022f, -0.00000f, 0.00000f },
        .orientation = { .w = 0.83985f, .x = 0.00162f, .y = -0.00095f, .z = 0.54281f }
    },
    vr::VRBoneTransform_t { // Bone 24
        .position = { -0.01819f, 0.00000f, -0.00000f },
        .orientation = { .w = 0.99862f, .x = -0.00715f, .y = -0.03107f, .z = 0.04168f }
    },
    vr::VRBoneTransform_t { // Bone 25
        .position = { -0.01802f, -0.00000f, 0.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = -0.00000f }
    },
    vr::VRBoneTransform_t { // Bone 26
        .position = { 0.00167f, 0.03486f, 0.05613f },
        .orientation = { .w = 0.76507f, .x = -0.13211f, .y = -0.62048f, .z = -0.11054f }
    },
    vr::VRBoneTransform_t { // Bone 27
        .position = { 0.00660f, -0.03617f, 0.04355f },
        .orientation = { .w = -0.25766f, .x = 0.93566f, .y = -0.14735f, .z = -0.19088f }
    },
    vr::VRBoneTransform_t { // Bone 28
        .position = { -0.01649f, -0.02293f, 0.09679f },
        .orientation = { .w = -0.20391f, .x = 0.79901f, .y = -0.54935f, .z = -0.13497f }
    },
    vr::VRBoneTransform_t { // Bone 29
        .position = { -0.01367f, -0.02704f, 0.12021f },
        .orientation = { .w = -0.11925f, .x = 0.85137f, .y = -0.49273f, .z = -0.13474f }
    },
    vr::VRBoneTransform_t { // Bone 30
        .position = { -0.01126f, -0.02786f, 0.14022f },
        .orientation = { .w = -0.08589f, .x = 0.84083f, .y = -0.51969f, .z = -0.12467f }
    },
};

}
