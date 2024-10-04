#pragma once

#include "generated/interfaces/public_vrtypes.h"
#include <array>

using BoneArray = std::array<vr::VRBoneTransform_t, 31>;
namespace left_hand {
const BoneArray bindPoseParentSpace {
    vr::VRBoneTransform_t { // eBone_Root
        .position = { 0.00000f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_Wrist
        .position = { -0.00016f, -0.00003f, 0.00063f, 1.f },
        .orientation = { .w = 0.00000f, .x = -0.00000f, .y = -1.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_Thumb1
        .position = { -0.01791f, 0.02918f, 0.02530f, 1.f },
        .orientation = { .w = 0.27639f, .x = 0.54119f, .y = 0.18203f, .z = 0.77304f } },
    vr::VRBoneTransform_t { // eBone_Thumb2
        .position = { 0.04041f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.96917f, .x = 0.00006f, .y = -0.00137f, .z = 0.24638f } },
    vr::VRBoneTransform_t { // eBone_Thumb3
        .position = { 0.03252f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.98817f, .x = 0.00010f, .y = 0.00140f, .z = 0.15334f } },
    vr::VRBoneTransform_t { // eBone_Thumb3
        .position = { 0.03046f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger0
        .position = { -0.00156f, 0.02107f, 0.01479f, 1.f },
        .orientation = { .w = 0.55075f, .x = 0.53106f, .y = -0.35143f, .z = 0.53958f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger1
        .position = { 0.07380f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.96898f, .x = 0.00162f, .y = -0.05289f, .z = 0.24140f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger2
        .position = { 0.04329f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.98277f, .x = -0.00009f, .y = 0.00504f, .z = 0.18476f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger3
        .position = { 0.02828f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.99707f, .x = 0.00003f, .y = -0.00117f, .z = 0.07646f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger4
        .position = { 0.02282f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger0
        .position = { 0.00218f, 0.00712f, 0.01632f, 1.f },
        .orientation = { .w = 0.53342f, .x = 0.56175f, .y = -0.41974f, .z = 0.47299f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger1
        .position = { 0.07089f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.97339f, .x = 0.00000f, .y = -0.00019f, .z = 0.22916f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger2
        .position = { 0.04311f, -0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.98753f, .x = 0.00009f, .y = -0.00369f, .z = 0.15740f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger3
        .position = { 0.03327f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.98996f, .x = -0.00011f, .y = 0.00413f, .z = 0.14128f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger4
        .position = { 0.02589f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // eBone_RingFinger0
        .position = { 0.00051f, -0.00655f, 0.01635f, 1.f },
        .orientation = { .w = 0.51669f, .x = 0.55014f, .y = -0.49555f, .z = 0.42989f } },
    vr::VRBoneTransform_t { // eBone_RingFinger1
        .position = { 0.06597f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.97456f, .x = -0.00090f, .y = -0.04096f, .z = 0.22037f } },
    vr::VRBoneTransform_t { // eBone_RingFinger2
        .position = { 0.04033f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.99100f, .x = -0.00007f, .y = 0.00253f, .z = 0.13383f } },
    vr::VRBoneTransform_t { // eBone_RingFinger3
        .position = { 0.02849f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.99079f, .x = 0.00020f, .y = -0.00426f, .z = 0.13535f } },
    vr::VRBoneTransform_t { // eBone_RingFinger4
        .position = { 0.02243f, -0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger0
        .position = { -0.00248f, -0.01898f, 0.01521f, 1.f },
        .orientation = { .w = 0.48576f, .x = 0.51533f, .y = -0.61502f, .z = 0.34675f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger1
        .position = { 0.06286f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.99349f, .x = 0.00394f, .y = 0.02816f, .z = 0.11031f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger2
        .position = { 0.02987f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.99111f, .x = 0.00038f, .y = -0.01146f, .z = 0.13252f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger3
        .position = { 0.01798f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.99401f, .x = -0.00054f, .y = 0.01270f, .z = 0.10858f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger4
        .position = { 0.01802f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // eBone_Aux_Thumb
        .position = { -0.03928f, 0.06008f, 0.08449f, 1.f },
        .orientation = { .w = 0.04861f, .x = -0.56911f, .y = 0.04504f, .z = -0.81959f } },
    vr::VRBoneTransform_t { // eBone_Aux_IndexFinger
        .position = { -0.01823f, 0.03728f, 0.14896f, 1.f },
        .orientation = { .w = 0.20956f, .x = 0.31233f, .y = -0.59723f, .z = 0.70842f } },
    vr::VRBoneTransform_t { // eBone_Aux_MiddleFinger
        .position = { -0.01256f, 0.00787f, 0.15469f, 1.f },
        .orientation = { .w = 0.22114f, .x = 0.27117f, .y = -0.64706f, .z = 0.67740f } },
    vr::VRBoneTransform_t { // eBone_Aux_RingFinger
        .position = { -0.01787f, -0.02324f, 0.14224f, 1.f },
        .orientation = { .w = 0.23741f, .x = 0.26235f, .y = -0.72163f, .z = 0.59503f } },
    vr::VRBoneTransform_t { // eBone_Aux_PinkyFinger
        .position = { -0.01601f, -0.04565f, 0.11928f, 1.f },
        .orientation = { .w = 0.34900f, .x = 0.26548f, .y = -0.73903f, .z = 0.51142f } },
};
const BoneArray openHandParentSpace {
    vr::VRBoneTransform_t { // eBone_Root
        .position = { 0.00000f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_Wrist
        .position = { -0.00016f, -0.00003f, 0.00063f, 1.f },
        .orientation = { .w = 0.00000f, .x = -0.00000f, .y = -1.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_Thumb1
        .position = { -0.01791f, 0.02918f, 0.02530f, 1.f },
        .orientation = { .w = 0.43792f, .x = 0.56781f, .y = 0.11983f, .z = 0.68663f } },
    vr::VRBoneTransform_t { // eBone_Thumb2
        .position = { 0.04041f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.99031f, .x = 0.04887f, .y = 0.05609f, .z = 0.11728f } },
    vr::VRBoneTransform_t { // eBone_Thumb3
        .position = { 0.03252f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.99493f, .x = 0.08159f, .y = 0.04521f, .z = -0.03764f } },
    vr::VRBoneTransform_t { // eBone_Thumb3
        .position = { 0.03046f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger0
        .position = { -0.00156f, 0.02107f, 0.01479f, 1.f },
        .orientation = { .w = 0.55075f, .x = 0.53106f, .y = -0.35143f, .z = 0.53958f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger1
        .position = { 0.07380f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.99318f, .x = 0.06183f, .y = 0.04100f, .z = 0.08997f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger2
        .position = { 0.04329f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.98958f, .x = -0.14077f, .y = -0.01481f, .z = -0.02620f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger3
        .position = { 0.02828f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.99707f, .x = 0.00003f, .y = -0.00117f, .z = 0.07646f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger4
        .position = { 0.02282f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger0
        .position = { 0.00218f, 0.00712f, 0.01632f, 1.f },
        .orientation = { .w = 0.53342f, .x = 0.56175f, .y = -0.41974f, .z = 0.47299f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger1
        .position = { 0.07089f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.99087f, .x = -0.03929f, .y = -0.01545f, .z = 0.12805f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger2
        .position = { 0.04311f, -0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.99882f, .x = -0.04623f, .y = -0.01254f, .z = -0.00778f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger3
        .position = { 0.03327f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.99920f, .x = -0.03577f, .y = 0.00817f, .z = 0.01562f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger4
        .position = { 0.02589f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // eBone_RingFinger0
        .position = { 0.00051f, -0.00655f, 0.01635f, 1.f },
        .orientation = { .w = 0.51669f, .x = 0.55014f, .y = -0.49555f, .z = 0.42989f } },
    vr::VRBoneTransform_t { // eBone_RingFinger1
        .position = { 0.06597f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.98958f, .x = -0.04109f, .y = -0.08942f, .z = 0.10511f } },
    vr::VRBoneTransform_t { // eBone_RingFinger2
        .position = { 0.04033f, -0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.99475f, .x = -0.07026f, .y = 0.04928f, .z = -0.05571f } },
    vr::VRBoneTransform_t { // eBone_RingFinger3
        .position = { 0.02849f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.99079f, .x = 0.00020f, .y = -0.00426f, .z = 0.13535f } },
    vr::VRBoneTransform_t { // eBone_RingFinger4
        .position = { 0.02243f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger0
        .position = { -0.00248f, -0.01898f, 0.01521f, 1.f },
        .orientation = { .w = 0.48576f, .x = 0.51533f, .y = -0.61502f, .z = 0.34675f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger1
        .position = { 0.06286f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.99505f, .x = 0.00921f, .y = -0.09085f, .z = 0.03929f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger2
        .position = { 0.02987f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.99406f, .x = -0.09116f, .y = -0.01704f, .z = -0.05688f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger3
        .position = { 0.01798f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.99768f, .x = -0.02291f, .y = 0.01686f, .z = 0.06191f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger4
        .position = { 0.01802f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // eBone_Aux_Thumb
        .position = { -0.02218f, 0.07865f, 0.07720f, 1.f },
        .orientation = { .w = 0.29496f, .x = 0.54402f, .y = 0.20688f, .z = 0.75779f } },
    vr::VRBoneTransform_t { // eBone_Aux_IndexFinger
        .position = { 0.01112f, 0.05525f, 0.15428f, 1.f },
        .orientation = { .w = 0.51341f, .x = 0.42038f, .y = -0.45372f, .z = 0.59483f } },
    vr::VRBoneTransform_t { // eBone_Aux_MiddleFinger
        .position = { 0.01291f, 0.00792f, 0.16137f, 1.f },
        .orientation = { .w = 0.51675f, .x = 0.44600f, .y = -0.55996f, .z = 0.46957f } },
    vr::VRBoneTransform_t { // eBone_Aux_RingFinger
        .position = { 0.00763f, -0.02903f, 0.14747f, 1.f },
        .orientation = { .w = 0.47395f, .x = 0.40717f, .y = -0.65043f, .z = 0.43188f } },
    vr::VRBoneTransform_t { // eBone_Aux_PinkyFinger
        .position = { -0.00586f, -0.05994f, 0.11671f, 1.f },
        .orientation = { .w = 0.46974f, .x = 0.47051f, .y = -0.70428f, .z = 0.24892f } },
};
const BoneArray squeezeParentSpace {
    vr::VRBoneTransform_t { // eBone_Root
        .position = { 0.00000f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_Wrist
        .position = { -0.00016f, -0.00003f, 0.00063f, 1.f },
        .orientation = { .w = 0.00000f, .x = -0.00000f, .y = -1.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_Thumb1
        .position = { -0.01791f, 0.02918f, 0.02530f, 1.f },
        .orientation = { .w = 0.27639f, .x = 0.54119f, .y = 0.18203f, .z = 0.77304f } },
    vr::VRBoneTransform_t { // eBone_Thumb2
        .position = { 0.04041f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.99031f, .x = 0.04887f, .y = 0.05609f, .z = 0.11728f } },
    vr::VRBoneTransform_t { // eBone_Thumb3
        .position = { 0.03252f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.98817f, .x = 0.00010f, .y = 0.00140f, .z = 0.15334f } },
    vr::VRBoneTransform_t { // eBone_Thumb3
        .position = { 0.03046f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger0
        .position = { -0.00139f, 0.01484f, 0.01476f, 1.f },
        .orientation = { .w = 0.50360f, .x = 0.55558f, .y = -0.33906f, .z = 0.56812f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger1
        .position = { 0.07380f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.68856f, .x = -0.10477f, .y = 0.02622f, .z = 0.71709f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger2
        .position = { 0.04329f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.77070f, .x = -0.10731f, .y = -0.07406f, .z = 0.62371f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger3
        .position = { 0.02828f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.72833f, .x = 0.11408f, .y = 0.06229f, .z = 0.67279f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger4
        .position = { 0.02282f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger0
        .position = { 0.00239f, 0.00507f, 0.01631f, 1.f },
        .orientation = { .w = 0.47825f, .x = 0.57711f, .y = -0.38315f, .z = 0.53983f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger1
        .position = { 0.07089f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.70251f, .x = 0.00968f, .y = 0.08009f, .z = 0.70709f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger2
        .position = { 0.04311f, -0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.72563f, .x = -0.09510f, .y = 0.03877f, .z = 0.68038f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger3
        .position = { 0.03327f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.80974f, .x = 0.07168f, .y = 0.01950f, .z = 0.58207f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger4
        .position = { 0.02589f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // eBone_RingFinger0
        .position = { 0.00240f, -0.00604f, 0.01627f, 1.f },
        .orientation = { .w = 0.51773f, .x = 0.53846f, .y = -0.48437f, .z = 0.45541f } },
    vr::VRBoneTransform_t { // eBone_RingFinger1
        .position = { 0.06597f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.74485f, .x = 0.04915f, .y = -0.06633f, .z = 0.66210f } },
    vr::VRBoneTransform_t { // eBone_RingFinger2
        .position = { 0.04033f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.71214f, .x = 0.03370f, .y = -0.01893f, .z = 0.70097f } },
    vr::VRBoneTransform_t { // eBone_RingFinger3
        .position = { 0.02849f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.81247f, .x = 0.01195f, .y = 0.00995f, .z = 0.58280f } },
    vr::VRBoneTransform_t { // eBone_RingFinger4
        .position = { 0.02243f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger0
        .position = { 0.00370f, -0.01332f, 0.01647f, 1.f },
        .orientation = { .w = 0.50294f, .x = 0.52823f, .y = -0.54218f, .z = 0.41721f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger1
        .position = { 0.06286f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.72357f, .x = 0.25616f, .y = 0.01231f, .z = 0.64083f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger2
        .position = { 0.02987f, -0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.75307f, .x = -0.05787f, .y = 0.02975f, .z = 0.65472f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger3
        .position = { 0.01798f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.74162f, .x = -0.02136f, .y = 0.00875f, .z = 0.67042f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger4
        .position = { 0.01802f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // eBone_Aux_Thumb
        .position = { -0.04135f, 0.06801f, 0.08090f, 1.f },
        .orientation = { .w = 0.01872f, .x = 0.54615f, .y = 0.08747f, .z = 0.83290f } },
    vr::VRBoneTransform_t { // eBone_Aux_IndexFinger
        .position = { -0.04379f, 0.02325f, 0.06410f, 1.f },
        .orientation = { .w = 0.61286f, .x = 0.75056f, .y = 0.23429f, .z = -0.07859f } },
    vr::VRBoneTransform_t { // eBone_Aux_MiddleFinger
        .position = { -0.03729f, 0.00292f, 0.05840f, 1.f },
        .orientation = { .w = 0.65101f, .x = 0.70942f, .y = 0.23326f, .z = -0.13605f } },
    vr::VRBoneTransform_t { // eBone_Aux_RingFinger
        .position = { -0.03532f, -0.01631f, 0.06226f, 1.f },
        .orientation = { .w = 0.67249f, .x = 0.68173f, .y = 0.24639f, .z = -0.14932f } },
    vr::VRBoneTransform_t { // eBone_Aux_PinkyFinger
        .position = { -0.02603f, -0.03303f, 0.06707f, 1.f },
        .orientation = { .w = 0.72597f, .x = 0.63292f, .y = 0.26608f, .z = -0.03987f } },
};
} // namespace left_hand

namespace right_hand {
const BoneArray bindPoseParentSpace {
    vr::VRBoneTransform_t { // eBone_Root
        .position = { 0.00000f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_Wrist
        .position = { 0.00016f, -0.00003f, 0.00063f, 1.f },
        .orientation = { .w = 0.00000f, .x = -0.00000f, .y = -1.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_Thumb1
        .position = { 0.01791f, 0.02918f, 0.02530f, 1.f },
        .orientation = { .w = 0.54119f, .x = -0.27639f, .y = 0.77304f, .z = -0.18203f } },
    vr::VRBoneTransform_t { // eBone_Thumb2
        .position = { -0.04041f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.96917f, .x = 0.00006f, .y = -0.00137f, .z = 0.24638f } },
    vr::VRBoneTransform_t { // eBone_Thumb3
        .position = { -0.03252f, -0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.98817f, .x = 0.00010f, .y = 0.00140f, .z = 0.15334f } },
    vr::VRBoneTransform_t { // eBone_Thumb3
        .position = { -0.03046f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger0
        .position = { 0.00156f, 0.02107f, 0.01479f, 1.f },
        .orientation = { .w = 0.53106f, .x = -0.55075f, .y = 0.53958f, .z = 0.35143f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger1
        .position = { -0.07380f, -0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.96898f, .x = 0.00162f, .y = -0.05289f, .z = 0.24140f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger2
        .position = { -0.04329f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.98277f, .x = -0.00009f, .y = 0.00504f, .z = 0.18476f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger3
        .position = { -0.02828f, -0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.99707f, .x = 0.00003f, .y = -0.00117f, .z = 0.07646f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger4
        .position = { -0.02282f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger0
        .position = { -0.00218f, 0.00712f, 0.01632f, 1.f },
        .orientation = { .w = 0.56175f, .x = -0.53342f, .y = 0.47299f, .z = 0.41974f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger1
        .position = { -0.07089f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.97339f, .x = 0.00000f, .y = -0.00019f, .z = 0.22916f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger2
        .position = { -0.04311f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.98753f, .x = 0.00009f, .y = -0.00369f, .z = 0.15740f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger3
        .position = { -0.03327f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.98996f, .x = -0.00011f, .y = 0.00413f, .z = 0.14128f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger4
        .position = { -0.02589f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // eBone_RingFinger0
        .position = { -0.00051f, -0.00655f, 0.01635f, 1.f },
        .orientation = { .w = 0.55014f, .x = -0.51669f, .y = 0.42989f, .z = 0.49555f } },
    vr::VRBoneTransform_t { // eBone_RingFinger1
        .position = { -0.06597f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.97456f, .x = -0.00090f, .y = -0.04096f, .z = 0.22037f } },
    vr::VRBoneTransform_t { // eBone_RingFinger2
        .position = { -0.04033f, -0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.99100f, .x = -0.00007f, .y = 0.00253f, .z = 0.13383f } },
    vr::VRBoneTransform_t { // eBone_RingFinger3
        .position = { -0.02849f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.99079f, .x = 0.00020f, .y = -0.00426f, .z = 0.13535f } },
    vr::VRBoneTransform_t { // eBone_RingFinger4
        .position = { -0.02243f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger0
        .position = { 0.00248f, -0.01898f, 0.01521f, 1.f },
        .orientation = { .w = 0.51533f, .x = -0.48576f, .y = 0.34675f, .z = 0.61502f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger1
        .position = { -0.06286f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.99349f, .x = 0.00394f, .y = 0.02816f, .z = 0.11031f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger2
        .position = { -0.02987f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.99111f, .x = 0.00038f, .y = -0.01146f, .z = 0.13252f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger3
        .position = { -0.01798f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.99401f, .x = -0.00054f, .y = 0.01270f, .z = 0.10858f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger4
        .position = { -0.01802f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_Aux_Thumb
        .position = { 0.03928f, 0.06008f, 0.08449f, 1.f },
        .orientation = { .w = 0.56911f, .x = 0.04861f, .y = 0.81959f, .z = 0.04504f } },
    vr::VRBoneTransform_t { // eBone_Aux_IndexFinger
        .position = { 0.01823f, 0.03728f, 0.14896f, 1.f },
        .orientation = { .w = 0.31233f, .x = -0.20956f, .y = 0.70842f, .z = 0.59723f } },
    vr::VRBoneTransform_t { // eBone_Aux_MiddleFinger
        .position = { 0.01256f, 0.00787f, 0.15469f, 1.f },
        .orientation = { .w = 0.27117f, .x = -0.22114f, .y = 0.67740f, .z = 0.64706f } },
    vr::VRBoneTransform_t { // eBone_Aux_RingFinger
        .position = { 0.01787f, -0.02324f, 0.14224f, 1.f },
        .orientation = { .w = 0.26235f, .x = -0.23741f, .y = 0.59503f, .z = 0.72163f } },
    vr::VRBoneTransform_t { // eBone_Aux_PinkyFinger
        .position = { 0.01601f, -0.04565f, 0.11928f, 1.f },
        .orientation = { .w = 0.26548f, .x = -0.34900f, .y = 0.51142f, .z = 0.73903f } },
};
const BoneArray openHandParentSpace {
    vr::VRBoneTransform_t { // eBone_Root
        .position = { 0.00000f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // eBone_Wrist
        .position = { 0.00016f, -0.00003f, 0.00063f, 1.f },
        .orientation = { .w = 0.00000f, .x = -0.00000f, .y = 1.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // eBone_Thumb1
        .position = { 0.01791f, 0.02918f, 0.02530f, 1.f },
        .orientation = { .w = 0.56781f, .x = -0.43792f, .y = 0.68663f, .z = -0.11983f } },
    vr::VRBoneTransform_t { // eBone_Thumb2
        .position = { -0.04041f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.99031f, .x = 0.04887f, .y = 0.05609f, .z = 0.11728f } },
    vr::VRBoneTransform_t { // eBone_Thumb3
        .position = { -0.03252f, -0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.99493f, .x = 0.08159f, .y = 0.04521f, .z = -0.03764f } },
    vr::VRBoneTransform_t { // eBone_Thumb3
        .position = { -0.03046f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger0
        .position = { 0.00156f, 0.02107f, 0.01479f, 1.f },
        .orientation = { .w = 0.53106f, .x = -0.55075f, .y = 0.53958f, .z = 0.35143f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger1
        .position = { -0.07380f, -0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.99318f, .x = 0.06183f, .y = 0.04100f, .z = 0.08997f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger2
        .position = { -0.04329f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.98958f, .x = -0.14077f, .y = -0.01481f, .z = -0.02620f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger3
        .position = { -0.02828f, -0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.99707f, .x = 0.00003f, .y = -0.00117f, .z = 0.07646f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger4
        .position = { -0.02282f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger0
        .position = { -0.00218f, 0.00712f, 0.01632f, 1.f },
        .orientation = { .w = 0.56175f, .x = -0.53342f, .y = 0.47299f, .z = 0.41974f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger1
        .position = { -0.07089f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.99087f, .x = -0.03929f, .y = -0.01545f, .z = 0.12805f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger2
        .position = { -0.04311f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.99882f, .x = -0.04623f, .y = -0.01254f, .z = -0.00778f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger3
        .position = { -0.03327f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.99920f, .x = -0.03577f, .y = 0.00817f, .z = 0.01562f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger4
        .position = { -0.02589f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // eBone_RingFinger0
        .position = { -0.00051f, -0.00655f, 0.01635f, 1.f },
        .orientation = { .w = 0.55014f, .x = -0.51669f, .y = 0.42989f, .z = 0.49555f } },
    vr::VRBoneTransform_t { // eBone_RingFinger1
        .position = { -0.06597f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.98958f, .x = -0.04109f, .y = -0.08942f, .z = 0.10511f } },
    vr::VRBoneTransform_t { // eBone_RingFinger2
        .position = { -0.04033f, -0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.99475f, .x = -0.07026f, .y = 0.04928f, .z = -0.05571f } },
    vr::VRBoneTransform_t { // eBone_RingFinger3
        .position = { -0.02849f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.99079f, .x = 0.00020f, .y = -0.00426f, .z = 0.13535f } },
    vr::VRBoneTransform_t { // eBone_RingFinger4
        .position = { -0.02243f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger0
        .position = { 0.00248f, -0.01898f, 0.01521f, 1.f },
        .orientation = { .w = 0.51533f, .x = -0.48576f, .y = 0.34675f, .z = 0.61502f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger1
        .position = { -0.06286f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.99505f, .x = 0.00921f, .y = -0.09085f, .z = 0.03929f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger2
        .position = { -0.02987f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.99406f, .x = -0.09116f, .y = -0.01704f, .z = -0.05688f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger3
        .position = { -0.01798f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.99768f, .x = -0.02291f, .y = 0.01686f, .z = 0.06191f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger4
        .position = { -0.01802f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_Aux_Thumb
        .position = { 0.02218f, 0.07865f, 0.07720f, 1.f },
        .orientation = { .w = 0.54403f, .x = -0.29496f, .y = 0.75779f, .z = -0.20688f } },
    vr::VRBoneTransform_t { // eBone_Aux_IndexFinger
        .position = { -0.01112f, 0.05525f, 0.15428f, 1.f },
        .orientation = { .w = 0.42038f, .x = -0.51341f, .y = 0.59483f, .z = 0.45372f } },
    vr::VRBoneTransform_t { // eBone_Aux_MiddleFinger
        .position = { -0.01291f, 0.00792f, 0.16137f, 1.f },
        .orientation = { .w = 0.44600f, .x = -0.51675f, .y = 0.46957f, .z = 0.55996f } },
    vr::VRBoneTransform_t { // eBone_Aux_RingFinger
        .position = { -0.00763f, -0.02903f, 0.14747f, 1.f },
        .orientation = { .w = 0.40717f, .x = -0.47395f, .y = 0.43188f, .z = 0.65043f } },
    vr::VRBoneTransform_t { // eBone_Aux_PinkyFinger
        .position = { 0.00586f, -0.05994f, 0.11671f, 1.f },
        .orientation = { .w = 0.47050f, .x = -0.46974f, .y = 0.24892f, .z = 0.70428f } },
};
const BoneArray squeezeParentSpace {
    vr::VRBoneTransform_t { // eBone_Root
        .position = { 0.00000f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = -0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_Wrist
        .position = { 0.00016f, -0.00003f, 0.00063f, 1.f },
        .orientation = { .w = 0.00000f, .x = -0.00000f, .y = -1.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_Thumb1
        .position = { 0.01791f, 0.02918f, 0.02530f, 1.f },
        .orientation = { .w = 0.54119f, .x = -0.27639f, .y = 0.77304f, .z = -0.18203f } },
    vr::VRBoneTransform_t { // eBone_Thumb2
        .position = { -0.04041f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.96917f, .x = 0.00006f, .y = -0.00137f, .z = 0.24638f } },
    vr::VRBoneTransform_t { // eBone_Thumb3
        .position = { -0.03252f, -0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.98817f, .x = 0.00010f, .y = 0.00140f, .z = 0.15334f } },
    vr::VRBoneTransform_t { // eBone_Thumb3
        .position = { -0.03046f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger0
        .position = { 0.00218f, 0.01773f, 0.01625f, 1.f },
        .orientation = { .w = 0.57080f, .x = -0.51658f, .y = 0.53668f, .z = 0.34542f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger1
        .position = { -0.07407f, -0.00037f, -0.00460f, 1.f },
        .orientation = { .w = 0.64992f, .x = -0.14599f, .y = 0.06483f, .z = 0.74302f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger2
        .position = { -0.04329f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.76851f, .x = -0.05789f, .y = -0.01468f, .z = 0.63705f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger3
        .position = { -0.02828f, -0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.81120f, .x = -0.06082f, .y = -0.13770f, .z = 0.56505f } },
    vr::VRBoneTransform_t { // eBone_IndexFinger4
        .position = { -0.02282f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger0
        .position = { -0.00231f, 0.00559f, 0.01632f, 1.f },
        .orientation = { .w = 0.55347f, .x = -0.51293f, .y = 0.49363f, .z = 0.43232f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger1
        .position = { -0.07089f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.70241f, .x = -0.01896f, .y = 0.03782f, .z = 0.71051f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger2
        .position = { -0.04311f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.75555f, .x = -0.03905f, .y = 0.03900f, .z = 0.65276f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger3
        .position = { -0.03327f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.74411f, .x = -0.01136f, .y = -0.06222f, .z = 0.66506f } },
    vr::VRBoneTransform_t { // eBone_MiddleFinger4
        .position = { -0.02589f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = -0.00000f, .z = 0.00000f } },
    vr::VRBoneTransform_t { // eBone_RingFinger0
        .position = { -0.00051f, -0.00655f, 0.01635f, 1.f },
        .orientation = { .w = 0.55014f, .x = -0.51669f, .y = 0.42989f, .z = 0.49555f } },
    vr::VRBoneTransform_t { // eBone_RingFinger1
        .position = { -0.06537f, -0.00070f, 0.00205f, 1.f },
        .orientation = { .w = 0.70078f, .x = 0.07535f, .y = 0.01841f, .z = 0.70915f } },
    vr::VRBoneTransform_t { // eBone_RingFinger2
        .position = { -0.04033f, -0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.71382f, .x = 0.00833f, .y = 0.07700f, .z = 0.69603f } },
    vr::VRBoneTransform_t { // eBone_RingFinger3
        .position = { -0.02849f, 0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.78821f, .x = 0.05548f, .y = 0.13836f, .z = 0.59708f } },
    vr::VRBoneTransform_t { // eBone_RingFinger4
        .position = { -0.02243f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = 0.00000f, .y = 0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger0
        .position = { -0.00363f, -0.01205f, 0.01982f, 1.f },
        .orientation = { .w = 0.51533f, .x = -0.48576f, .y = 0.34675f, .z = 0.61502f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger1
        .position = { -0.06286f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.75876f, .x = 0.16471f, .y = 0.00588f, .z = 0.63017f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger2
        .position = { -0.02987f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 0.70376f, .x = -0.00134f, .y = -0.01480f, .z = 0.71028f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger3
        .position = { -0.01798f, 0.00000f, -0.00000f, 1.f },
        .orientation = { .w = 0.82872f, .x = -0.00344f, .y = 0.00593f, .z = 0.55962f } },
    vr::VRBoneTransform_t { // eBone_PinkyFinger4
        .position = { -0.01802f, -0.00000f, 0.00000f, 1.f },
        .orientation = { .w = 1.00000f, .x = -0.00000f, .y = 0.00000f, .z = -0.00000f } },
    vr::VRBoneTransform_t { // eBone_Aux_Thumb
        .position = { 0.03928f, 0.06008f, 0.08449f, 1.f },
        .orientation = { .w = 0.56911f, .x = 0.04861f, .y = 0.81959f, .z = 0.04504f } },
    vr::VRBoneTransform_t { // eBone_Aux_IndexFinger
        .position = { 0.03767f, 0.02825f, 0.06347f, 1.f },
        .orientation = { .w = 0.57495f, .x = -0.76383f, .y = -0.21599f, .z = -0.19834f } },
    vr::VRBoneTransform_t { // eBone_Aux_MiddleFinger
        .position = { 0.03942f, 0.00844f, 0.05936f, 1.f },
        .orientation = { .w = 0.67319f, .x = -0.71387f, .y = -0.15013f, .z = -0.12112f } },
    vr::VRBoneTransform_t { // eBone_Aux_RingFinger
        .position = { 0.03506f, -0.01146f, 0.05747f, 1.f },
        .orientation = { .w = 0.80517f, .x = -0.55694f, .y = -0.13109f, .z = -0.15599f } },
    vr::VRBoneTransform_t { // eBone_Aux_PinkyFinger
        .position = { 0.02885f, -0.03045f, 0.06792f, 1.f },
        .orientation = { .w = 0.69794f, .x = -0.65475f, .y = -0.18023f, .z = -0.22736f } },
};
} // namespace right_hand
