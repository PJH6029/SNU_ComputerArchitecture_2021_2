#include "stdio.h"
#include "stdint.h"
#include "pa2.h"

#define FP10_BIAS 15
#define FP10_BIAS_SHIFT 0x00f0
#define FP10_MINUS_ZERO 0xfe00 // 1111 1110 0000 0000
#define FP10_INF 0x01f0  // 0000 0001 1111 0000
#define FP10_MINUS_INF 0xfff0
#define FP10_EXP_MASK 0x01f0
#define FP10_FRAC_MASK 0x000f
#define FP10_NAN 0x01f1
#define FP10_MINUS_NAN 0xfff1
#define FP10_POS_MAX 0x01ef
#define FP10_POS_MIN 0x0001
#define FP10_NEG_MAX 0xffef

#define FLOAT_BIAS 127
#define FLOAT_BIAS_SHIFT 0x3f800000
#define FLOAT_MINUS_ZERO 0x80000000
#define FLOAT_INF 0x7f800000
#define FLOAT_MINUS_INF 0xff800000
#define FLOAT_EXP_MASK 0x7f800000
#define FLOAT_FRAC_MASK 0x007fffff
#define FLOAT_FP10_S_MASK 0x0003ffff
#define FLOAT_NAN 0x7f800001
#define FLOAT_MINUS_NAN 0xff800001
#define FLOAT_FP10_POS_MAX 0x42780000
#define FLOAT_FP10_NEG_MAX 0xc2780000
#define FLOAT_MAX_VAL_ROUND_ZERO 0x36000000
#define FLOAT_MAX_VAL_ROUND_MINUS_ZERO 0xb6000000

#define INT_MIN 0x80000000


fp10 float_fp10(float f)
{
    // fp10: denormalized smallest: 0 00000 0001 -> 0.0001 * 2^(-14)
    // fp10: denormalized largest: 0 00000 1111 -> 0.1111 * 2^(-14)
    // fp10: normalized smallest: 0 00001 0000 -> 1.0000 * 2^(-14)
    // fp10 1: 0 01111 0000
    // fp10 largest: 0 11110 1111 -> 1.1111 * 2^15

    fp10 result_fp10 = 0;
    register unsigned int bits = *((uint32_t *)&f);
    if ((bits >> 31) & 1) {
        // sign
        result_fp10 = FP10_MINUS_ZERO;

        //nan check
        if (bits > FLOAT_MINUS_INF) {
            return FP10_MINUS_NAN;
        }

        // inf check
        else if (bits >= 0xc77c0000) {
            return FP10_MINUS_INF;
        }

        // zero check
        else if (bits <= FLOAT_MAX_VAL_ROUND_MINUS_ZERO) {
            return FP10_MINUS_ZERO;
        }
    } else {
        //nan check
        if (bits > FLOAT_INF) {
            return FP10_NAN;
        }

        // inf check
        else if (bits >= 0x477c0000) {
            return FP10_INF;
        }

        // zero check
        else if (bits <= FLOAT_MAX_VAL_ROUND_ZERO) {
            return 0;
        }
    }
/*

    unsigned char sign = (bits >> 31) & 1;
    if (sign) {
        result_fp10 = FP10_MINUS_ZERO;
    }

    // nan check
    if (bits > FLOAT_INF && !sign) {
        return FP10_NAN;
    }
    if (bits > FLOAT_MINUS_INF && sign) {
        return FP10_MINUS_NAN;
    }

    // inf check
    if (bits >= 0x477c0000 && !sign) {
        return FP10_INF;
    }
    if (bits >= 0xc77c0000 && sign) {
        return FP10_MINUS_INF;
    }

    // zero check
    // exponent = ((bits & FLOAT_EXP_MASK) >> 23);
    if (bits <= FLOAT_MAX_VAL_ROUND_ZERO && !sign) {
        return 0;
    }
    if (bits <= FLOAT_MAX_VAL_ROUND_MINUS_ZERO && sign) {
        return FP10_MINUS_ZERO;
    }*/

    // now f is normalized value, which can be normalized fp10 or denormalized one that is rounded to 0

    // bits &= 0x7fffffff;  // TODO optional?
    if ((bits & FLOAT_EXP_MASK) < FLOAT_BIAS_SHIFT - (14 << 23)) {
        // 얘는 79라인이랑 상관 x

        // denorm
        unsigned int val = 4 + FLOAT_BIAS - ((bits & FLOAT_EXP_MASK) >> 23);
        bits &= FLOAT_FRAC_MASK;
        bits |= 0x00800000;  // get frac + 1 더해줌
        // round = (bits & (1 << val)) && (bits & ((3 << val) - 1));  // r = 1 or at lest of bits except r is 1 // TODO 미리 저장
        bits += ((bits & (1 << val)) && (bits & ((3 << val) - 1))) << (val + 1);
        bits >>= (val + 1);
        return result_fp10 | bits;  // TODO sign을 extend?
    } else {
        // norm
        bits -= FLOAT_BIAS_SHIFT;

        // round = (bits & 0x00040000) && (bits & 0x000bffff);  // val = 18
        bits >>= 19;
        bits += (bits & 0x00040000) && (bits & 0x000bffff);;  // 얘도 79라인이랑 상관 없는 것 같은데?
        return (result_fp10 | bits) + FP10_BIAS_SHIFT;
    }
}
