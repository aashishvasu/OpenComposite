#pragma once

#include "generated/interfaces/public_vrtypes.h"
#include <array>

using BoneArray = std::array<vr::VRBoneTransform_t, 31>;
namespace left_hand {
const BoneArray bindPoseParentSpace {
    vr::VRBoneTransform_t { // Bone 0
        .position = { 0.00000f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 1
        .position = { -0.03404f, 0.03650f, 0.16472f, 1.00000f },
        .orientation = { .w = -0.05515f, .x = -0.07861f, .y = -0.92028f, .z = 0.37930f } },
    vr::VRBoneTransform_t { // Bone 2
        .position = { -0.01208f, 0.02807f, 0.02505f, 1.00000f },
        .orientation = { .w = 0.46411f, .x = 0.56742f, .y = 0.27211f, .z = 0.62337f } },
    vr::VRBoneTransform_t { // Bone 3
        .position = { 0.04041f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99484f, .x = 0.08294f, .y = 0.01945f, .z = 0.05513f } },
    vr::VRBoneTransform_t { // Bone 4
        .position = { 0.03252f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.97479f, .x = -0.00321f, .y = 0.02187f, .z = -0.22201f } },
    vr::VRBoneTransform_t { // Bone 5
        .position = { 0.03046f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 6
        .position = { 0.00063f, 0.02687f, 0.01500f, 1.00000f },
        .orientation = { .w = 0.64425f, .x = 0.42198f, .y = -0.47820f, .z = 0.42213f } },
    vr::VRBoneTransform_t { // Bone 7
        .position = { 0.07420f, -0.00500f, 0.00023f, 1.00000f },
        .orientation = { .w = 0.99533f, .x = 0.00701f, .y = -0.03912f, .z = 0.08795f } },
    vr::VRBoneTransform_t { // Bone 8
        .position = { 0.04393f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99789f, .x = 0.04581f, .y = 0.00214f, .z = -0.04594f } },
    vr::VRBoneTransform_t { // Bone 9
        .position = { 0.02870f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99965f, .x = 0.00185f, .y = -0.02278f, .z = -0.01341f } },
    vr::VRBoneTransform_t { // Bone 10
        .position = { 0.02282f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 11
        .position = { 0.00218f, 0.00712f, 0.01632f, 1.00000f },
        .orientation = { .w = 0.54672f, .x = 0.54128f, .y = -0.44252f, .z = 0.46075f } },
    vr::VRBoneTransform_t { // Bone 12
        .position = { 0.07095f, 0.00078f, 0.00100f, 1.00000f },
        .orientation = { .w = 0.98029f, .x = -0.16726f, .y = -0.07896f, .z = 0.06937f } },
    vr::VRBoneTransform_t { // Bone 13
        .position = { 0.04311f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99795f, .x = 0.01849f, .y = 0.01319f, .z = 0.05989f } },
    vr::VRBoneTransform_t { // Bone 14
        .position = { 0.03327f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99739f, .x = -0.00333f, .y = -0.02822f, .z = -0.06632f } },
    vr::VRBoneTransform_t { // Bone 15
        .position = { 0.02589f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99919f, .x = 0.00000f, .y = 0.00000f, .z = 0.04013f } },
    vr::VRBoneTransform_t { // Bone 16
        .position = { 0.00051f, -0.00654f, 0.01635f, 1.00000f },
        .orientation = { .w = 0.51669f, .x = 0.55014f, .y = -0.49555f, .z = 0.42989f } },
    vr::VRBoneTransform_t { // Bone 17
        .position = { 0.06588f, 0.00179f, 0.00069f, 1.00000f },
        .orientation = { .w = 0.99042f, .x = -0.05870f, .y = -0.10182f, .z = 0.07249f } },
    vr::VRBoneTransform_t { // Bone 18
        .position = { 0.04070f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99954f, .x = -0.00224f, .y = 0.00000f, .z = 0.03008f } },
    vr::VRBoneTransform_t { // Bone 19
        .position = { 0.02875f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99910f, .x = -0.00072f, .y = -0.01269f, .z = 0.04042f } },
    vr::VRBoneTransform_t { // Bone 20
        .position = { 0.02243f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 21
        .position = { -0.00248f, -0.01898f, 0.01521f, 1.00000f },
        .orientation = { .w = 0.52692f, .x = 0.52394f, .y = -0.58403f, .z = 0.32674f } },
    vr::VRBoneTransform_t { // Bone 22
        .position = { 0.06288f, 0.00284f, 0.00033f, 1.00000f },
        .orientation = { .w = 0.98661f, .x = -0.05962f, .y = -0.13516f, .z = 0.06913f } },
    vr::VRBoneTransform_t { // Bone 23
        .position = { 0.03022f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99432f, .x = 0.00190f, .y = -0.00013f, .z = 0.10645f } },
    vr::VRBoneTransform_t { // Bone 24
        .position = { 0.01819f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99593f, .x = -0.00201f, .y = -0.05208f, .z = -0.07353f } },
    vr::VRBoneTransform_t { // Bone 25
        .position = { 0.01802f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // Bone 26
        .position = { -0.00606f, 0.05629f, 0.06006f, 1.00000f },
        .orientation = { .w = 0.73724f, .x = 0.20275f, .y = 0.59427f, .z = 0.24944f } },
    vr::VRBoneTransform_t { // Bone 27
        .position = { -0.04042f, -0.04302f, 0.01935f, 1.00000f },
        .orientation = { .w = -0.29033f, .x = 0.62353f, .y = -0.66381f, .z = -0.29373f } },
    vr::VRBoneTransform_t { // Bone 28
        .position = { -0.03935f, -0.07567f, 0.04705f, 1.00000f },
        .orientation = { .w = -0.18705f, .x = 0.67806f, .y = -0.65929f, .z = -0.26568f } },
    vr::VRBoneTransform_t { // Bone 29
        .position = { -0.03834f, -0.09099f, 0.08258f, 1.00000f },
        .orientation = { .w = -0.18304f, .x = 0.73679f, .y = -0.63476f, .z = -0.14394f } },
    vr::VRBoneTransform_t { // Bone 30
        .position = { -0.03181f, -0.08721f, 0.12101f, 1.00000f },
        .orientation = { .w = -0.00366f, .x = 0.75841f, .y = -0.63934f, .z = -0.12668f } },
};
const BoneArray openHandParentSpace {
    vr::VRBoneTransform_t { // Bone 0
        .position = { 0.00000f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 1
        .position = { -0.03404f, 0.03650f, 0.16472f, 1.00000f },
        .orientation = { .w = -0.05515f, .x = -0.07861f, .y = -0.92028f, .z = 0.37930f } },
    vr::VRBoneTransform_t { // Bone 2
        .position = { -0.01208f, 0.02807f, 0.02505f, 1.00000f },
        .orientation = { .w = 0.46411f, .x = 0.56742f, .y = 0.27211f, .z = 0.62337f } },
    vr::VRBoneTransform_t { // Bone 3
        .position = { 0.04041f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99484f, .x = 0.08294f, .y = 0.01945f, .z = 0.05513f } },
    vr::VRBoneTransform_t { // Bone 4
        .position = { 0.03252f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.97479f, .x = -0.00321f, .y = 0.02187f, .z = -0.22201f } },
    vr::VRBoneTransform_t { // Bone 5
        .position = { 0.03046f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 6
        .position = { 0.00063f, 0.02687f, 0.01500f, 1.00000f },
        .orientation = { .w = 0.64425f, .x = 0.42198f, .y = -0.47820f, .z = 0.42213f } },
    vr::VRBoneTransform_t { // Bone 7
        .position = { 0.07420f, -0.00500f, 0.00023f, 1.00000f },
        .orientation = { .w = 0.99533f, .x = 0.00701f, .y = -0.03912f, .z = 0.08795f } },
    vr::VRBoneTransform_t { // Bone 8
        .position = { 0.04393f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99789f, .x = 0.04581f, .y = 0.00214f, .z = -0.04594f } },
    vr::VRBoneTransform_t { // Bone 9
        .position = { 0.02870f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99965f, .x = 0.00185f, .y = -0.02278f, .z = -0.01341f } },
    vr::VRBoneTransform_t { // Bone 10
        .position = { 0.02282f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 11
        .position = { 0.00218f, 0.00712f, 0.01632f, 1.00000f },
        .orientation = { .w = 0.54672f, .x = 0.54128f, .y = -0.44252f, .z = 0.46075f } },
    vr::VRBoneTransform_t { // Bone 12
        .position = { 0.07095f, 0.00078f, 0.00100f, 1.00000f },
        .orientation = { .w = 0.98029f, .x = -0.16726f, .y = -0.07896f, .z = 0.06937f } },
    vr::VRBoneTransform_t { // Bone 13
        .position = { 0.04311f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99795f, .x = 0.01849f, .y = 0.01319f, .z = 0.05989f } },
    vr::VRBoneTransform_t { // Bone 14
        .position = { 0.03327f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99739f, .x = -0.00333f, .y = -0.02822f, .z = -0.06632f } },
    vr::VRBoneTransform_t { // Bone 15
        .position = { 0.02589f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99919f, .x = 0.00000f, .y = 0.00000f, .z = 0.04013f } },
    vr::VRBoneTransform_t { // Bone 16
        .position = { 0.00051f, -0.00654f, 0.01635f, 1.00000f },
        .orientation = { .w = 0.51669f, .x = 0.55014f, .y = -0.49555f, .z = 0.42989f } },
    vr::VRBoneTransform_t { // Bone 17
        .position = { 0.06588f, 0.00179f, 0.00069f, 1.00000f },
        .orientation = { .w = 0.99042f, .x = -0.05870f, .y = -0.10182f, .z = 0.07249f } },
    vr::VRBoneTransform_t { // Bone 18
        .position = { 0.04070f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99954f, .x = -0.00224f, .y = 0.00000f, .z = 0.03008f } },
    vr::VRBoneTransform_t { // Bone 19
        .position = { 0.02875f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99910f, .x = -0.00072f, .y = -0.01269f, .z = 0.04042f } },
    vr::VRBoneTransform_t { // Bone 20
        .position = { 0.02243f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 21
        .position = { -0.00248f, -0.01898f, 0.01521f, 1.00000f },
        .orientation = { .w = 0.52692f, .x = 0.52394f, .y = -0.58403f, .z = 0.32674f } },
    vr::VRBoneTransform_t { // Bone 22
        .position = { 0.06288f, 0.00284f, 0.00033f, 1.00000f },
        .orientation = { .w = 0.98661f, .x = -0.05962f, .y = -0.13516f, .z = 0.06913f } },
    vr::VRBoneTransform_t { // Bone 23
        .position = { 0.03022f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99432f, .x = 0.00190f, .y = -0.00013f, .z = 0.10645f } },
    vr::VRBoneTransform_t { // Bone 24
        .position = { 0.01819f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99593f, .x = -0.00201f, .y = -0.05208f, .z = -0.07353f } },
    vr::VRBoneTransform_t { // Bone 25
        .position = { 0.01802f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // Bone 26
        .position = { -0.00606f, 0.05629f, 0.06006f, 1.00000f },
        .orientation = { .w = 0.73724f, .x = 0.20275f, .y = 0.59427f, .z = 0.24944f } },
    vr::VRBoneTransform_t { // Bone 27
        .position = { -0.04042f, -0.04302f, 0.01935f, 1.00000f },
        .orientation = { .w = -0.29033f, .x = 0.62353f, .y = -0.66381f, .z = -0.29373f } },
    vr::VRBoneTransform_t { // Bone 28
        .position = { -0.03935f, -0.07567f, 0.04705f, 1.00000f },
        .orientation = { .w = -0.18705f, .x = 0.67806f, .y = -0.65929f, .z = -0.26568f } },
    vr::VRBoneTransform_t { // Bone 29
        .position = { -0.03834f, -0.09099f, 0.08258f, 1.00000f },
        .orientation = { .w = -0.18304f, .x = 0.73679f, .y = -0.63476f, .z = -0.14394f } },
    vr::VRBoneTransform_t { // Bone 30
        .position = { -0.03181f, -0.08721f, 0.12101f, 1.00000f },
        .orientation = { .w = -0.00366f, .x = 0.75841f, .y = -0.63934f, .z = -0.12668f } },
};
const BoneArray squeezeParentSpace {
    vr::VRBoneTransform_t { // Bone 0
        .position = { 0.00000f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 1
        .position = { -0.03404f, 0.03650f, 0.16472f, 1.00000f },
        .orientation = { .w = -0.05515f, .x = -0.07861f, .y = -0.92028f, .z = 0.37930f } },
    vr::VRBoneTransform_t { // Bone 2
        .position = { -0.01643f, 0.03087f, 0.02512f, 1.00000f },
        .orientation = { .w = 0.40385f, .x = 0.59570f, .y = 0.08245f, .z = 0.68938f } },
    vr::VRBoneTransform_t { // Bone 3
        .position = { 0.04041f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.98966f, .x = -0.09043f, .y = 0.02846f, .z = 0.10769f } },
    vr::VRBoneTransform_t { // Bone 4
        .position = { 0.03252f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.98859f, .x = 0.14398f, .y = 0.04152f, .z = 0.01536f } },
    vr::VRBoneTransform_t { // Bone 5
        .position = { 0.03046f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 6
        .position = { 0.00380f, 0.02151f, 0.01280f, 1.00000f },
        .orientation = { .w = 0.61731f, .x = 0.39518f, .y = -0.51087f, .z = 0.44919f } },
    vr::VRBoneTransform_t { // Bone 7
        .position = { 0.07420f, -0.00500f, 0.00023f, 1.00000f },
        .orientation = { .w = 0.73729f, .x = -0.03201f, .y = -0.11501f, .z = 0.66494f } },
    vr::VRBoneTransform_t { // Bone 8
        .position = { 0.04329f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.61138f, .x = 0.00329f, .y = 0.00382f, .z = 0.79132f } },
    vr::VRBoneTransform_t { // Bone 9
        .position = { 0.02827f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.74539f, .x = -0.00068f, .y = -0.00094f, .z = 0.66663f } },
    vr::VRBoneTransform_t { // Bone 10
        .position = { 0.02282f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 11
        .position = { 0.00579f, 0.00681f, 0.01653f, 1.00000f },
        .orientation = { .w = 0.51420f, .x = 0.52231f, .y = -0.47835f, .z = 0.48370f } },
    vr::VRBoneTransform_t { // Bone 12
        .position = { 0.07095f, 0.00078f, 0.00100f, 1.00000f },
        .orientation = { .w = 0.72365f, .x = -0.09790f, .y = 0.04855f, .z = 0.68146f } },
    vr::VRBoneTransform_t { // Bone 13
        .position = { 0.04311f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.63746f, .x = -0.00237f, .y = -0.00283f, .z = 0.77047f } },
    vr::VRBoneTransform_t { // Bone 14
        .position = { 0.03327f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.65801f, .x = 0.00261f, .y = 0.00320f, .z = 0.75300f } },
    vr::VRBoneTransform_t { // Bone 15
        .position = { 0.02589f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99919f, .x = 0.00000f, .y = 0.00000f, .z = 0.04013f } },
    vr::VRBoneTransform_t { // Bone 16
        .position = { 0.00412f, -0.00686f, 0.01656f, 1.00000f },
        .orientation = { .w = 0.48961f, .x = 0.52337f, .y = -0.52064f, .z = 0.46400f } },
    vr::VRBoneTransform_t { // Bone 17
        .position = { 0.06588f, 0.00179f, 0.00069f, 1.00000f },
        .orientation = { .w = 0.75997f, .x = -0.05561f, .y = 0.01157f, .z = 0.64747f } },
    vr::VRBoneTransform_t { // Bone 18
        .position = { 0.04033f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.66431f, .x = 0.00159f, .y = 0.00197f, .z = 0.74745f } },
    vr::VRBoneTransform_t { // Bone 19
        .position = { 0.02849f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.62696f, .x = -0.00278f, .y = -0.00323f, .z = 0.77904f } },
    vr::VRBoneTransform_t { // Bone 20
        .position = { 0.02243f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 21
        .position = { 0.00113f, -0.01929f, 0.01543f, 1.00000f },
        .orientation = { .w = 0.47977f, .x = 0.47783f, .y = -0.63020f, .z = 0.37993f } },
    vr::VRBoneTransform_t { // Bone 22
        .position = { 0.06288f, 0.00284f, 0.00033f, 1.00000f },
        .orientation = { .w = 0.82700f, .x = 0.03428f, .y = 0.00344f, .z = 0.56114f } },
    vr::VRBoneTransform_t { // Bone 23
        .position = { 0.02987f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.70218f, .x = -0.00672f, .y = -0.00929f, .z = 0.71190f } },
    vr::VRBoneTransform_t { // Bone 24
        .position = { 0.01798f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.67685f, .x = 0.00796f, .y = 0.00992f, .z = 0.73601f } },
    vr::VRBoneTransform_t { // Bone 25
        .position = { 0.01802f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // Bone 26
        .position = { -0.00519f, 0.05419f, 0.06003f, 1.00000f },
        .orientation = { .w = 0.74737f, .x = 0.18239f, .y = 0.59962f, .z = 0.22052f } },
    vr::VRBoneTransform_t { // Bone 27
        .position = { 0.00017f, 0.01647f, 0.09651f, 1.00000f },
        .orientation = { .w = -0.00646f, .x = 0.02275f, .y = -0.93293f, .z = -0.35929f } },
    vr::VRBoneTransform_t { // Bone 28
        .position = { 0.00045f, 0.00154f, 0.11654f, 1.00000f },
        .orientation = { .w = -0.03936f, .x = 0.10514f, .y = -0.92883f, .z = -0.35308f } },
    vr::VRBoneTransform_t { // Bone 29
        .position = { 0.00395f, -0.01487f, 0.13061f, 1.00000f },
        .orientation = { .w = -0.05507f, .x = 0.06870f, .y = -0.94402f, .z = -0.31793f } },
    vr::VRBoneTransform_t { // Bone 30
        .position = { 0.00326f, -0.03469f, 0.13993f, 1.00000f },
        .orientation = { .w = 0.01969f, .x = -0.10074f, .y = -0.95733f, .z = -0.27015f } },
};
const BoneArray gripLimitParentSpace {
    vr::VRBoneTransform_t { // Bone 0
        .position = { 0.00000f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 1
        .position = { -0.03404f, 0.03650f, 0.16472f, 1.00000f },
        .orientation = { .w = -0.05515f, .x = -0.07861f, .y = -0.92028f, .z = 0.37930f } },
    vr::VRBoneTransform_t { // Bone 2
        .position = { -0.01730f, 0.03257f, 0.02528f, 1.00000f },
        .orientation = { .w = 0.31761f, .x = 0.52834f, .y = 0.21313f, .z = 0.75799f } },
    vr::VRBoneTransform_t { // Bone 3
        .position = { 0.04041f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99174f, .x = 0.08532f, .y = 0.01942f, .z = 0.09376f } },
    vr::VRBoneTransform_t { // Bone 4
        .position = { 0.03252f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.95938f, .x = -0.01220f, .y = -0.03105f, .z = 0.28012f } },
    vr::VRBoneTransform_t { // Bone 5
        .position = { 0.03046f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 6
        .position = { -0.00392f, 0.02717f, 0.01464f, 1.00000f },
        .orientation = { .w = 0.66645f, .x = 0.43003f, .y = -0.45595f, .z = 0.40377f } },
    vr::VRBoneTransform_t { // Bone 7
        .position = { 0.07602f, -0.00512f, 0.00024f, 1.00000f },
        .orientation = { .w = -0.95601f, .x = -0.00003f, .y = 0.15836f, .z = -0.24691f } },
    vr::VRBoneTransform_t { // Bone 8
        .position = { 0.04393f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = -0.94414f, .x = -0.04335f, .y = 0.01495f, .z = -0.32634f } },
    vr::VRBoneTransform_t { // Bone 9
        .position = { 0.02870f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = -0.91215f, .x = 0.00363f, .y = 0.03989f, .z = -0.40790f } },
    vr::VRBoneTransform_t { // Bone 10
        .position = { 0.02282f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 11
        .position = { 0.00218f, 0.00712f, 0.01632f, 1.00000f },
        .orientation = { .w = 0.52936f, .x = 0.54051f, .y = -0.46378f, .z = 0.46101f } },
    vr::VRBoneTransform_t { // Bone 12
        .position = { 0.07095f, 0.00078f, 0.00100f, 1.00000f },
        .orientation = { .w = -0.83173f, .x = 0.27093f, .y = 0.17565f, .z = -0.45164f } },
    vr::VRBoneTransform_t { // Bone 13
        .position = { 0.04311f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = -0.85488f, .x = -0.00823f, .y = -0.02811f, .z = -0.51799f } },
    vr::VRBoneTransform_t { // Bone 14
        .position = { 0.03327f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = -0.82576f, .x = 0.08521f, .y = 0.08646f, .z = -0.55081f } },
    vr::VRBoneTransform_t { // Bone 15
        .position = { 0.02589f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99919f, .x = -0.00000f, .y = 0.00000f, .z = 0.04013f } },
    vr::VRBoneTransform_t { // Bone 16
        .position = { 0.00051f, -0.00654f, 0.01635f, 1.00000f },
        .orientation = { .w = 0.50024f, .x = 0.53078f, .y = -0.51622f, .z = 0.44894f } },
    vr::VRBoneTransform_t { // Bone 17
        .position = { 0.06588f, 0.00179f, 0.00069f, 1.00000f },
        .orientation = { .w = 0.83162f, .x = -0.24293f, .y = -0.13970f, .z = 0.47946f } },
    vr::VRBoneTransform_t { // Bone 18
        .position = { 0.04070f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.76916f, .x = -0.00175f, .y = 0.00136f, .z = 0.63905f } },
    vr::VRBoneTransform_t { // Bone 19
        .position = { 0.02875f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.96862f, .x = -0.06454f, .y = -0.04659f, .z = 0.23548f } },
    vr::VRBoneTransform_t { // Bone 20
        .position = { 0.02243f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // Bone 21
        .position = { -0.00248f, -0.01898f, 0.01521f, 1.00000f },
        .orientation = { .w = 0.47467f, .x = 0.43467f, .y = -0.65321f, .z = 0.39883f } },
    vr::VRBoneTransform_t { // Bone 22
        .position = { 0.06288f, 0.00284f, 0.00033f, 1.00000f },
        .orientation = { .w = 0.79879f, .x = -0.19958f, .y = -0.09442f, .z = 0.55964f } },
    vr::VRBoneTransform_t { // Bone 23
        .position = { 0.03022f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.85309f, .x = 0.00164f, .y = -0.00091f, .z = 0.52176f } },
    vr::VRBoneTransform_t { // Bone 24
        .position = { 0.01819f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.97425f, .x = 0.05249f, .y = 0.00359f, .z = 0.21925f } },
    vr::VRBoneTransform_t { // Bone 25
        .position = { 0.01802f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 26
        .position = { 0.00663f, 0.02669f, 0.06187f, 1.00000f },
        .orientation = { .w = 0.80508f, .x = -0.01837f, .y = 0.58479f, .z = -0.09760f } },
    vr::VRBoneTransform_t { // Bone 27
        .position = { -0.00788f, -0.04048f, 0.03934f, 1.00000f },
        .orientation = { .w = -0.32249f, .x = 0.93209f, .y = 0.12186f, .z = 0.11114f } },
    vr::VRBoneTransform_t { // Bone 28
        .position = { 0.01664f, -0.02999f, 0.08320f, 1.00000f },
        .orientation = { .w = -0.09458f, .x = 0.69455f, .y = 0.70284f, .z = 0.12110f } },
    vr::VRBoneTransform_t { // Bone 29
        .position = { 0.01114f, -0.02873f, 0.10837f, 1.00000f },
        .orientation = { .w = -0.07633f, .x = 0.78828f, .y = 0.60510f, .z = 0.08153f } },
    vr::VRBoneTransform_t { // Bone 30
        .position = { 0.01133f, -0.02604f, 0.12858f, 1.00000f },
        .orientation = { .w = -0.14479f, .x = 0.73745f, .y = 0.65696f, .z = -0.06007f } },
};
} // namespace left_hand

namespace right_hand {
const BoneArray bindPoseParentSpace {
    vr::VRBoneTransform_t { // Bone 0
        .position = { 0.00000f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 1
        .position = { 0.03404f, 0.03650f, 0.16472f, 1.00000f },
        .orientation = { .w = -0.05515f, .x = -0.07861f, .y = 0.92028f, .z = -0.37930f } },
    vr::VRBoneTransform_t { // Bone 2
        .position = { 0.01233f, 0.02866f, 0.02505f, 1.00000f },
        .orientation = { .w = 0.57106f, .x = -0.45128f, .y = 0.63006f, .z = -0.27068f } },
    vr::VRBoneTransform_t { // Bone 3
        .position = { -0.04041f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99457f, .x = 0.07828f, .y = 0.01828f, .z = 0.06618f } },
    vr::VRBoneTransform_t { // Bone 4
        .position = { -0.03252f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.97766f, .x = -0.00304f, .y = 0.02072f, .z = -0.20916f } },
    vr::VRBoneTransform_t { // Bone 5
        .position = { -0.03046f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 6
        .position = { -0.00063f, 0.02687f, 0.01500f, 1.00000f },
        .orientation = { .w = 0.42183f, .x = -0.64379f, .y = 0.42246f, .z = 0.47866f } },
    vr::VRBoneTransform_t { // Bone 7
        .position = { -0.07420f, 0.00500f, -0.00023f, 1.00000f },
        .orientation = { .w = 0.99478f, .x = 0.00705f, .y = -0.04129f, .z = 0.09301f } },
    vr::VRBoneTransform_t { // Bone 8
        .position = { -0.04393f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99840f, .x = 0.04591f, .y = 0.00278f, .z = -0.03277f } },
    vr::VRBoneTransform_t { // Bone 9
        .position = { -0.02870f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99970f, .x = 0.00195f, .y = -0.02277f, .z = -0.00828f } },
    vr::VRBoneTransform_t { // Bone 10
        .position = { -0.02282f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 11
        .position = { -0.00218f, 0.00712f, 0.01632f, 1.00000f },
        .orientation = { .w = 0.54187f, .x = -0.54743f, .y = 0.46000f, .z = 0.44170f } },
    vr::VRBoneTransform_t { // Bone 12
        .position = { -0.07095f, -0.00078f, -0.00100f, 1.00000f },
        .orientation = { .w = 0.97984f, .x = -0.16806f, .y = -0.07591f, .z = 0.07690f } },
    vr::VRBoneTransform_t { // Bone 13
        .position = { -0.04311f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99727f, .x = 0.01828f, .y = 0.01338f, .z = 0.07027f } },
    vr::VRBoneTransform_t { // Bone 14
        .position = { -0.03327f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99840f, .x = -0.00314f, .y = -0.02642f, .z = -0.04985f } },
    vr::VRBoneTransform_t { // Bone 15
        .position = { -0.02589f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99919f, .x = 0.00000f, .y = 0.00000f, .z = 0.04013f } },
    vr::VRBoneTransform_t { // Bone 16
        .position = { -0.00051f, -0.00654f, 0.01635f, 1.00000f },
        .orientation = { .w = 0.54898f, .x = -0.51907f, .y = 0.42691f, .z = 0.49692f } },
    vr::VRBoneTransform_t { // Bone 17
        .position = { -0.06588f, -0.00179f, -0.00069f, 1.00000f },
        .orientation = { .w = 0.98979f, .x = -0.06588f, .y = -0.09642f, .z = 0.08172f } },
    vr::VRBoneTransform_t { // Bone 18
        .position = { -0.04070f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99910f, .x = -0.00217f, .y = -0.00002f, .z = 0.04232f } },
    vr::VRBoneTransform_t { // Bone 19
        .position = { -0.02875f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99858f, .x = -0.00067f, .y = -0.01271f, .z = 0.05165f } },
    vr::VRBoneTransform_t { // Bone 20
        .position = { -0.02243f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 21
        .position = { 0.00248f, -0.01898f, 0.01521f, 1.00000f },
        .orientation = { .w = 0.51860f, .x = -0.52730f, .y = 0.32826f, .z = 0.58758f } },
    vr::VRBoneTransform_t { // Bone 22
        .position = { -0.06288f, -0.00284f, -0.00033f, 1.00000f },
        .orientation = { .w = 0.98729f, .x = -0.06336f, .y = -0.12596f, .z = 0.07327f } },
    vr::VRBoneTransform_t { // Bone 23
        .position = { -0.03022f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99341f, .x = 0.00157f, .y = -0.00015f, .z = 0.11458f } },
    vr::VRBoneTransform_t { // Bone 24
        .position = { -0.01819f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99705f, .x = -0.00069f, .y = -0.05201f, .z = -0.05649f } },
    vr::VRBoneTransform_t { // Bone 25
        .position = { -0.01802f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // Bone 26
        .position = { 0.00520f, 0.05420f, 0.06003f, 1.00000f },
        .orientation = { .w = 0.74732f, .x = 0.18251f, .y = -0.59959f, .z = -0.22069f } },
    vr::VRBoneTransform_t { // Bone 27
        .position = { 0.03878f, -0.04297f, 0.01982f, 1.00000f },
        .orientation = { .w = -0.29744f, .x = 0.63937f, .y = 0.64891f, .z = 0.28573f } },
    vr::VRBoneTransform_t { // Bone 28
        .position = { 0.03803f, -0.07484f, 0.04694f, 1.00000f },
        .orientation = { .w = -0.19990f, .x = 0.69822f, .y = 0.63577f, .z = 0.26141f } },
    vr::VRBoneTransform_t { // Bone 29
        .position = { 0.03684f, -0.08978f, 0.08197f, 1.00000f },
        .orientation = { .w = -0.19096f, .x = 0.75647f, .y = 0.60759f, .z = 0.14873f } },
    vr::VRBoneTransform_t { // Bone 30
        .position = { 0.03025f, -0.08606f, 0.11989f, 1.00000f },
        .orientation = { .w = -0.01895f, .x = 0.77925f, .y = 0.61218f, .z = 0.13285f } },
};
const BoneArray openHandParentSpace {
    vr::VRBoneTransform_t { // Bone 0
        .position = { 0.00000f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 1
        .position = { 0.03404f, 0.03650f, 0.16472f, 1.00000f },
        .orientation = { .w = -0.05515f, .x = -0.07861f, .y = 0.92028f, .z = -0.37930f } },
    vr::VRBoneTransform_t { // Bone 2
        .position = { 0.01233f, 0.02866f, 0.02505f, 1.00000f },
        .orientation = { .w = 0.57106f, .x = -0.45128f, .y = 0.63006f, .z = -0.27068f } },
    vr::VRBoneTransform_t { // Bone 3
        .position = { -0.04041f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99457f, .x = 0.07828f, .y = 0.01828f, .z = 0.06618f } },
    vr::VRBoneTransform_t { // Bone 4
        .position = { -0.03252f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.97766f, .x = -0.00304f, .y = 0.02072f, .z = -0.20916f } },
    vr::VRBoneTransform_t { // Bone 5
        .position = { -0.03046f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 6
        .position = { -0.00063f, 0.02687f, 0.01500f, 1.00000f },
        .orientation = { .w = 0.42183f, .x = -0.64379f, .y = 0.42246f, .z = 0.47866f } },
    vr::VRBoneTransform_t { // Bone 7
        .position = { -0.07420f, 0.00500f, -0.00023f, 1.00000f },
        .orientation = { .w = 0.99478f, .x = 0.00705f, .y = -0.04129f, .z = 0.09301f } },
    vr::VRBoneTransform_t { // Bone 8
        .position = { -0.04393f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99840f, .x = 0.04591f, .y = 0.00278f, .z = -0.03277f } },
    vr::VRBoneTransform_t { // Bone 9
        .position = { -0.02870f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99970f, .x = 0.00195f, .y = -0.02277f, .z = -0.00828f } },
    vr::VRBoneTransform_t { // Bone 10
        .position = { -0.02282f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 11
        .position = { -0.00218f, 0.00712f, 0.01632f, 1.00000f },
        .orientation = { .w = 0.54187f, .x = -0.54743f, .y = 0.46000f, .z = 0.44170f } },
    vr::VRBoneTransform_t { // Bone 12
        .position = { -0.07095f, -0.00078f, -0.00100f, 1.00000f },
        .orientation = { .w = 0.97984f, .x = -0.16806f, .y = -0.07591f, .z = 0.07690f } },
    vr::VRBoneTransform_t { // Bone 13
        .position = { -0.04311f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99727f, .x = 0.01828f, .y = 0.01338f, .z = 0.07027f } },
    vr::VRBoneTransform_t { // Bone 14
        .position = { -0.03327f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99840f, .x = -0.00314f, .y = -0.02642f, .z = -0.04985f } },
    vr::VRBoneTransform_t { // Bone 15
        .position = { -0.02589f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99919f, .x = 0.00000f, .y = 0.00000f, .z = 0.04013f } },
    vr::VRBoneTransform_t { // Bone 16
        .position = { -0.00051f, -0.00654f, 0.01635f, 1.00000f },
        .orientation = { .w = 0.54898f, .x = -0.51907f, .y = 0.42691f, .z = 0.49692f } },
    vr::VRBoneTransform_t { // Bone 17
        .position = { -0.06588f, -0.00179f, -0.00069f, 1.00000f },
        .orientation = { .w = 0.98979f, .x = -0.06588f, .y = -0.09642f, .z = 0.08172f } },
    vr::VRBoneTransform_t { // Bone 18
        .position = { -0.04070f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99910f, .x = -0.00217f, .y = -0.00002f, .z = 0.04232f } },
    vr::VRBoneTransform_t { // Bone 19
        .position = { -0.02875f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99858f, .x = -0.00067f, .y = -0.01271f, .z = 0.05165f } },
    vr::VRBoneTransform_t { // Bone 20
        .position = { -0.02243f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 21
        .position = { 0.00248f, -0.01898f, 0.01521f, 1.00000f },
        .orientation = { .w = 0.51860f, .x = -0.52730f, .y = 0.32826f, .z = 0.58758f } },
    vr::VRBoneTransform_t { // Bone 22
        .position = { -0.06288f, -0.00284f, -0.00033f, 1.00000f },
        .orientation = { .w = 0.98729f, .x = -0.06336f, .y = -0.12596f, .z = 0.07327f } },
    vr::VRBoneTransform_t { // Bone 23
        .position = { -0.03022f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99341f, .x = 0.00157f, .y = -0.00015f, .z = 0.11458f } },
    vr::VRBoneTransform_t { // Bone 24
        .position = { -0.01819f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99705f, .x = -0.00069f, .y = -0.05201f, .z = -0.05649f } },
    vr::VRBoneTransform_t { // Bone 25
        .position = { -0.01802f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // Bone 26
        .position = { 0.00520f, 0.05420f, 0.06003f, 1.00000f },
        .orientation = { .w = 0.74732f, .x = 0.18251f, .y = -0.59959f, .z = -0.22069f } },
    vr::VRBoneTransform_t { // Bone 27
        .position = { 0.03878f, -0.04297f, 0.01982f, 1.00000f },
        .orientation = { .w = -0.29744f, .x = 0.63937f, .y = 0.64891f, .z = 0.28573f } },
    vr::VRBoneTransform_t { // Bone 28
        .position = { 0.03803f, -0.07484f, 0.04694f, 1.00000f },
        .orientation = { .w = -0.19990f, .x = 0.69822f, .y = 0.63577f, .z = 0.26141f } },
    vr::VRBoneTransform_t { // Bone 29
        .position = { 0.03684f, -0.08978f, 0.08197f, 1.00000f },
        .orientation = { .w = -0.19096f, .x = 0.75647f, .y = 0.60759f, .z = 0.14873f } },
    vr::VRBoneTransform_t { // Bone 30
        .position = { 0.03025f, -0.08606f, 0.11989f, 1.00000f },
        .orientation = { .w = -0.01895f, .x = 0.77925f, .y = 0.61218f, .z = 0.13285f } },
};
const BoneArray squeezeParentSpace {
    vr::VRBoneTransform_t { // Bone 0
        .position = { 0.00000f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 1
        .position = { 0.03404f, 0.03650f, 0.16472f, 1.00000f },
        .orientation = { .w = -0.05515f, .x = -0.07861f, .y = 0.92028f, .z = -0.37930f } },
    vr::VRBoneTransform_t { // Bone 2
        .position = { 0.01643f, 0.03087f, 0.02512f, 1.00000f },
        .orientation = { .w = 0.59570f, .x = -0.40385f, .y = 0.68938f, .z = -0.08245f } },
    vr::VRBoneTransform_t { // Bone 3
        .position = { -0.04041f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.98966f, .x = -0.09043f, .y = 0.02846f, .z = 0.10769f } },
    vr::VRBoneTransform_t { // Bone 4
        .position = { -0.03252f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.98859f, .x = 0.14398f, .y = 0.04152f, .z = 0.01536f } },
    vr::VRBoneTransform_t { // Bone 5
        .position = { -0.03046f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 6
        .position = { -0.00380f, 0.02151f, 0.01280f, 1.00000f },
        .orientation = { .w = 0.39517f, .x = -0.61731f, .y = 0.44919f, .z = 0.51087f } },
    vr::VRBoneTransform_t { // Bone 7
        .position = { -0.07420f, 0.00500f, -0.00023f, 1.00000f },
        .orientation = { .w = 0.73729f, .x = -0.03201f, .y = -0.11501f, .z = 0.66494f } },
    vr::VRBoneTransform_t { // Bone 8
        .position = { -0.04329f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.61138f, .x = 0.00329f, .y = 0.00382f, .z = 0.79132f } },
    vr::VRBoneTransform_t { // Bone 9
        .position = { -0.02827f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.74539f, .x = -0.00068f, .y = -0.00094f, .z = 0.66663f } },
    vr::VRBoneTransform_t { // Bone 10
        .position = { -0.02282f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 11
        .position = { -0.00579f, 0.00681f, 0.01653f, 1.00000f },
        .orientation = { .w = 0.52231f, .x = -0.51420f, .y = 0.48370f, .z = 0.47835f } },
    vr::VRBoneTransform_t { // Bone 12
        .position = { -0.07095f, -0.00078f, -0.00100f, 1.00000f },
        .orientation = { .w = 0.72365f, .x = -0.09790f, .y = 0.04855f, .z = 0.68146f } },
    vr::VRBoneTransform_t { // Bone 13
        .position = { -0.04311f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.63746f, .x = -0.00237f, .y = -0.00283f, .z = 0.77047f } },
    vr::VRBoneTransform_t { // Bone 14
        .position = { -0.03327f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.65801f, .x = 0.00261f, .y = 0.00320f, .z = 0.75300f } },
    vr::VRBoneTransform_t { // Bone 15
        .position = { -0.02589f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.99919f, .x = 0.00000f, .y = 0.00000f, .z = 0.04013f } },
    vr::VRBoneTransform_t { // Bone 16
        .position = { -0.00412f, -0.00686f, 0.01656f, 1.00000f },
        .orientation = { .w = 0.52337f, .x = -0.48961f, .y = 0.46400f, .z = 0.52064f } },
    vr::VRBoneTransform_t { // Bone 17
        .position = { -0.06588f, -0.00179f, -0.00069f, 1.00000f },
        .orientation = { .w = 0.75997f, .x = -0.05561f, .y = 0.01157f, .z = 0.64747f } },
    vr::VRBoneTransform_t { // Bone 18
        .position = { -0.04033f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.66431f, .x = 0.00159f, .y = 0.00197f, .z = 0.74745f } },
    vr::VRBoneTransform_t { // Bone 19
        .position = { -0.02849f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.62696f, .x = -0.00278f, .y = -0.00323f, .z = 0.77904f } },
    vr::VRBoneTransform_t { // Bone 20
        .position = { -0.02243f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 21
        .position = { -0.00113f, -0.01929f, 0.01543f, 1.00000f },
        .orientation = { .w = 0.47783f, .x = -0.47977f, .y = 0.37994f, .z = 0.63020f } },
    vr::VRBoneTransform_t { // Bone 22
        .position = { -0.06288f, -0.00284f, -0.00033f, 1.00000f },
        .orientation = { .w = 0.82700f, .x = 0.03428f, .y = 0.00344f, .z = 0.56114f } },
    vr::VRBoneTransform_t { // Bone 23
        .position = { -0.02987f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.70218f, .x = -0.00672f, .y = -0.00929f, .z = 0.71190f } },
    vr::VRBoneTransform_t { // Bone 24
        .position = { -0.01798f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.67685f, .x = 0.00796f, .y = 0.00992f, .z = 0.73601f } },
    vr::VRBoneTransform_t { // Bone 25
        .position = { -0.01802f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // Bone 26
        .position = { 0.00439f, 0.05551f, 0.06025f, 1.00000f },
        .orientation = { .w = 0.74592f, .x = 0.15676f, .y = -0.59795f, .z = -0.24795f } },
    vr::VRBoneTransform_t { // Bone 27
        .position = { -0.00017f, 0.01647f, 0.09651f, 1.00000f },
        .orientation = { .w = -0.00646f, .x = 0.02275f, .y = 0.93293f, .z = 0.35929f } },
    vr::VRBoneTransform_t { // Bone 28
        .position = { -0.00045f, 0.00154f, 0.11654f, 1.00000f },
        .orientation = { .w = -0.03936f, .x = 0.10514f, .y = 0.92883f, .z = 0.35308f } },
    vr::VRBoneTransform_t { // Bone 29
        .position = { -0.00395f, -0.01487f, 0.13061f, 1.00000f },
        .orientation = { .w = -0.05507f, .x = 0.06870f, .y = 0.94402f, .z = 0.31793f } },
    vr::VRBoneTransform_t { // Bone 30
        .position = { -0.00326f, -0.03469f, 0.13993f, 1.00000f },
        .orientation = { .w = 0.01969f, .x = -0.10074f, .y = 0.95733f, .z = 0.27015f } },
};
const BoneArray gripLimitParentSpace {
    vr::VRBoneTransform_t { // Bone 0
        .position = { 0.00000f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 1
        .position = { 0.03404f, 0.03650f, 0.16472f, 1.00000f },
        .orientation = { .w = -0.05515f, .x = -0.07861f, .y = 0.92028f, .z = -0.37930f } },
    vr::VRBoneTransform_t { // Bone 2
        .position = { 0.01730f, 0.03257f, 0.02528f, 1.00000f },
        .orientation = { .w = 0.52834f, .x = -0.31761f, .y = 0.75799f, .z = -0.21313f } },
    vr::VRBoneTransform_t { // Bone 3
        .position = { -0.04041f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99174f, .x = 0.08532f, .y = 0.01942f, .z = 0.09376f } },
    vr::VRBoneTransform_t { // Bone 4
        .position = { -0.03252f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.95938f, .x = -0.01220f, .y = -0.03105f, .z = 0.28012f } },
    vr::VRBoneTransform_t { // Bone 5
        .position = { -0.03046f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 6
        .position = { -0.00392f, 0.02717f, 0.01464f, 1.00000f },
        .orientation = { .w = 0.66645f, .x = 0.43003f, .y = -0.45595f, .z = 0.40377f } },
    vr::VRBoneTransform_t { // Bone 7
        .position = { 0.07602f, -0.00512f, 0.00024f, 1.00000f },
        .orientation = { .w = -0.95601f, .x = -0.00003f, .y = 0.15836f, .z = -0.24691f } },
    vr::VRBoneTransform_t { // Bone 8
        .position = { 0.04393f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = -0.94414f, .x = -0.04335f, .y = 0.01495f, .z = -0.32634f } },
    vr::VRBoneTransform_t { // Bone 9
        .position = { 0.02870f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = -0.91215f, .x = 0.00363f, .y = 0.03989f, .z = -0.40790f } },
    vr::VRBoneTransform_t { // Bone 10
        .position = { 0.02282f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 11
        .position = { 0.00218f, 0.00712f, 0.01632f, 1.00000f },
        .orientation = { .w = 0.52936f, .x = 0.54051f, .y = -0.46378f, .z = 0.46101f } },
    vr::VRBoneTransform_t { // Bone 12
        .position = { 0.07095f, 0.00078f, 0.00100f, 1.00000f },
        .orientation = { .w = -0.83173f, .x = 0.27093f, .y = 0.17565f, .z = -0.45164f } },
    vr::VRBoneTransform_t { // Bone 13
        .position = { 0.04311f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = -0.85488f, .x = -0.00823f, .y = -0.02811f, .z = -0.51799f } },
    vr::VRBoneTransform_t { // Bone 14
        .position = { 0.03327f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = -0.82576f, .x = 0.08521f, .y = 0.08646f, .z = -0.55081f } },
    vr::VRBoneTransform_t { // Bone 15
        .position = { 0.02589f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.99919f, .x = -0.00000f, .y = 0.00000f, .z = 0.04013f } },
    vr::VRBoneTransform_t { // Bone 16
        .position = { 0.00051f, -0.00654f, 0.01635f, 1.00000f },
        .orientation = { .w = 0.50024f, .x = 0.53078f, .y = -0.51622f, .z = 0.44894f } },
    vr::VRBoneTransform_t { // Bone 17
        .position = { 0.06588f, 0.00179f, 0.00069f, 1.00000f },
        .orientation = { .w = 0.83162f, .x = -0.24293f, .y = -0.13970f, .z = 0.47946f } },
    vr::VRBoneTransform_t { // Bone 18
        .position = { 0.04070f, 0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.76916f, .x = -0.00175f, .y = 0.00136f, .z = 0.63905f } },
    vr::VRBoneTransform_t { // Bone 19
        .position = { 0.02875f, -0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.96862f, .x = -0.06454f, .y = -0.04659f, .z = 0.23548f } },
    vr::VRBoneTransform_t { // Bone 20
        .position = { 0.02243f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // Bone 21
        .position = { -0.00248f, -0.01898f, 0.01521f, 1.00000f },
        .orientation = { .w = 0.47467f, .x = 0.43467f, .y = -0.65321f, .z = 0.39883f } },
    vr::VRBoneTransform_t { // Bone 22
        .position = { 0.06288f, 0.00284f, 0.00033f, 1.00000f },
        .orientation = { .w = 0.79879f, .x = -0.19958f, .y = -0.09442f, .z = 0.55964f } },
    vr::VRBoneTransform_t { // Bone 23
        .position = { 0.03022f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 0.85309f, .x = 0.00164f, .y = -0.00091f, .z = 0.52176f } },
    vr::VRBoneTransform_t { // Bone 24
        .position = { 0.01819f, -0.00000f, 0.00000f, 1.00000f },
        .orientation = { .w = 0.97425f, .x = 0.05249f, .y = 0.00359f, .z = 0.21925f } },
    vr::VRBoneTransform_t { // Bone 25
        .position = { 0.01802f, 0.00000f, -0.00000f, 1.00000f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // Bone 26
        .position = { 0.00663f, 0.02669f, 0.06187f, 1.00000f },
        .orientation = { .w = 0.80508f, .x = -0.01837f, .y = 0.58479f, .z = -0.09760f } },
    vr::VRBoneTransform_t { // Bone 27
        .position = { -0.00788f, -0.04048f, 0.03934f, 1.00000f },
        .orientation = { .w = -0.32249f, .x = 0.93209f, .y = 0.12186f, .z = 0.11114f } },
    vr::VRBoneTransform_t { // Bone 28
        .position = { 0.01664f, -0.02999f, 0.08320f, 1.00000f },
        .orientation = { .w = -0.09458f, .x = 0.69455f, .y = 0.70284f, .z = 0.12110f } },
    vr::VRBoneTransform_t { // Bone 29
        .position = { 0.01114f, -0.02873f, 0.10837f, 1.00000f },
        .orientation = { .w = -0.07633f, .x = 0.78828f, .y = 0.60510f, .z = 0.08153f } },
    vr::VRBoneTransform_t { // Bone 30
        .position = { 0.01133f, -0.02604f, 0.12858f, 1.00000f },
        .orientation = { .w = -0.14479f, .x = 0.73745f, .y = 0.65696f, .z = -0.06007f } },
};
} // namespace right_hand
