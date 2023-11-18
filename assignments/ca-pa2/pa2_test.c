//---------------------------------------------------------------
//
//  4190.308 Computer Architecture (Fall 2021)
//
//  Project #2: FP10 (10-bit floating point) Representation
//
//  October 5, 2021
//
//  Jaehoon Shim (mattjs@snu.ac.kr)
//  Ikjoon Son (ikjoon.son@snu.ac.kr)
//  Seongyeop Jeong (seongyeop.jeong@snu.ac.kr)
//  Systems Software & Architecture Laboratory
//  Dept. of Computer Science and Engineering
//  Seoul National University
//
//---------------------------------------------------------------

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


#define BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BINARY(BYTE)			\
	(BYTE & 0x80 ? '1' : '0'),	\
	(BYTE & 0x40 ? '1' : '0'),	\
	(BYTE & 0x20 ? '1' : '0'),	\
	(BYTE & 0x10 ? '1' : '0'),	\
	(BYTE & 0x08 ? '1' : '0'),	\
	(BYTE & 0x04 ? '1' : '0'),	\
	(BYTE & 0x02 ? '1' : '0'),	\
	(BYTE & 0x01 ? '1' : '0')

#define PRINT_BYTE(BYTE) printf(BINARY_PATTERN, BINARY(BYTE))
#define PRINT(DATATYPE, TYPENAME, NUM)				\
	do {							\
		size_t typesize = sizeof(DATATYPE);		\
		DATATYPE data = NUM;				\
		uint8_t *ptr = (uint8_t *)&data;		\
								\
		printf("%s(", TYPENAME);			\
		PRINT_BYTE(*(ptr + typesize - 1));		\
		for (ssize_t i = typesize - 2; i >= 0; i--) {	\
			printf(" ");				\
			PRINT_BYTE(*(ptr + i));			\
		}						\
		printf(")");					\
	} while (0)


typedef union float_bit {
    float f;
    unsigned int bits;
} float_bit;

static inline int clz(unsigned int x) {
    // ref: https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
    int y = __builtin_clz(x);
    return y;
}

/* Convert 32-bit signed integer to 10-bit floating point */
fp10 int_fp10(int n)
{
    register unsigned int frac, flag, bits;
    register int exponent;
    register fp10 result_fp10 = 0;
    unsigned int l, r, s;

    if (n == 0) {
        return 0;
    } else if (n < 0) {
        result_fp10 = FP10_MINUS_ZERO;
        bits = -n;
    } else {
        bits = n;
    }

    exponent = 31 - clz(bits);  // flag: bin_point

    if (exponent > 15) {
        return result_fp10 | FP10_INF;
    }

    // 표현 가능한 숫자들:
    // exponent(-> 1), exponent-1, exponent-2, exponent-3, exponent-4(=L) / exponent-5(=R)
    if (exponent - 4 <= 0) {
        // need to pad
        return result_fp10 | ((exponent + FP10_BIAS) << 4) | (n << (4 - exponent));
    }

    // exponent - 4 > 0
    // round up
    unsigned int val = exponent - 5;
    unsigned int round = (bits & (1 << val)) && (bits & ((3 << val) - 1));
    bits >>= ++val;
    bits &= FP10_FRAC_MASK;
    bits |= exponent << 4;
    bits += round;

    if (bits >> 4 > 0xf) {
        return result_fp10 | FP10_INF;
    }

    return result_fp10 | bits + FP10_BIAS_SHIFT;
}

/* Convert 10-bit floating point to 32-bit signed integer */
int fp10_int(fp10 x)
{
    register int exponent;
    unsigned int sign;
    int result_int;

    if (x == FP10_MINUS_ZERO || x == 0) {
        return 0;
    }

    if ((x & FP10_EXP_MASK) == FP10_EXP_MASK) {
        return INT_MIN;
    }

    sign = (x >> 9) & 1;
    exponent = ((x & FP10_EXP_MASK) >> 4) - FP10_BIAS;
    if (exponent < 0) {
        return 0;
    }

    if (exponent >= 4) {
        result_int = ((x & FP10_FRAC_MASK) + 16) << (exponent - 4);
    } else {
        result_int = ((x & FP10_FRAC_MASK) + 16) >> (4 - exponent);
    }

	return (sign) ? -result_int : result_int;
}

/* Convert 32-bit single-precision floating point to 10-bit floating point */
/*fp10 float_fp10(float f)
{
    // fp10: denormalized smallest: 0 00000 0001 -> 0.0001 * 2^(-14)
    // fp10: denormalized largest: 0 00000 1111 -> 0.1111 * 2^(-14)
    // fp10: normalized smallest: 0 00001 0000 -> 1.0000 * 2^(-14)
    // fp10 1: 0 01111 0000
    // fp10 largest: 0 11110 1111 -> 1.1111 * 2^15

    register fp10 result_fp10 = 0;
    register unsigned int bits;
    register int exponent;
    float_bit fb;

    fb.f = f;
    bits = fb.bits;

    unsigned int sign = (bits >> 31) & 1;
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
    if (bits == FLOAT_INF) {
    // if (bits >= 0x477c0000 && !sign) {
        return FP10_INF;
    }
    if (bits == FLOAT_MINUS_INF) {
    // if (bits >= 0xc77c0000 && sign) {
        return FP10_MINUS_INF;
    }

    // zero check
    // exponent = ((bits & FLOAT_EXP_MASK) >> 23);
    if (bits <= FLOAT_MAX_VAL_ROUND_ZERO && !sign) {
        return 0;
    }
    if (bits <= FLOAT_MAX_VAL_ROUND_MINUS_ZERO && sign) {
        return FP10_MINUS_ZERO;
    }

    // now f is normalized value, which can be normalized fp10 or denormalized one that is rounded to 0

    exponent = bits & FLOAT_EXP_MASK;

    unsigned int round;
    bits &= 0x7fffffff;
    if (exponent < FLOAT_BIAS_SHIFT - (14 << 23)) {
        // denorm
        unsigned int val = 4 + FLOAT_BIAS - (exponent >> 23);
        bits &= FLOAT_FRAC_MASK;
        bits |= 0x00800000;  // get frac
        round = (bits & (1 << val)) && (bits & ((3 << val) - 1));  // r = 1 or at lest of bits except r is 1
        bits >>= ++val;
        bits += round;
        return result_fp10 | bits;
    } else {
        // norm
        bits -= FLOAT_BIAS_SHIFT;

        round = (bits & 0x00040000) && (bits & 0x000bffff);  // 1 << 18,
        bits >>= 19;
        bits += round;
        if (bits >> 4 > 0xf) {  // inf check  // TODO 뒤로 넘기기?
            return result_fp10 | FP10_INF;
        }
        return (result_fp10 | bits) + FP10_BIAS_SHIFT;
    }
}*/
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

/* Convert 10-bit floating point to 32-bit single-precision floating point */
float fp10_float(fp10 x)
{
    // 모두 float에서 normalized value

    register unsigned int frac, bits = 0;
    register int exponent;
    float_bit fb;
    int clzz;

    unsigned int sign = (x >> 15) & 1;  // 1: neg 0: pos
    if (sign) {
        // TODO bits = 0xfffff000;
        bits = FLOAT_MINUS_ZERO;
    }

    // nan check
    if (x > FP10_INF && !sign) {
        fb.bits = FLOAT_NAN;
        return fb.f;
    }
    if (x > FP10_MINUS_INF && sign) {
        fb.bits = FLOAT_MINUS_NAN;
        return fb.f;
    }

    // inf check
    if (x == FP10_INF) {
        fb.bits = FLOAT_INF;
        return fb.f;
    }
    if (x == FP10_MINUS_INF) {
        fb.bits = FLOAT_MINUS_INF;
        return fb.f;
    }

    // zero check
    if (x == 0) {
        return 0;
    }
    if (x == FP10_MINUS_ZERO) {
        fb.bits = FLOAT_MINUS_ZERO;
        return fb.f;
    }

    exponent = (x & FP10_EXP_MASK) >> 4;
    frac = x & FP10_FRAC_MASK;
    if (!exponent) {
        // denormalized value
        exponent = -14;

        clzz = clz(frac); // 31일 때 4번 shift, 28일 때 1번 shift
        frac <<= (clzz - 27);
        exponent -= (clzz - 28);

        frac = frac & FP10_FRAC_MASK;
        exponent = exponent - 1 + FLOAT_BIAS;
    } else {
        exponent = exponent - FP10_BIAS + FLOAT_BIAS;
    }
    bits = bits | (exponent << 4 | frac) << 19;

    fb.bits = bits;
    return fb.f;
}
/*

int main() {
    uint32_t float_fp[] = {0x477bffff, 0x477c0000};
    uint16_t fp_float[] = {0x01ef};
    PRINT(uint16_t, "fp", fp_float[0]);
    printf("\n");
    float f = fp10_float(fp_float[0]);
    uint32_t *p = (uint32_t *)&f;
    uint32_t result = *p;
    PRINT(uint32_t, "fl", result);
    printf("\n\n");

    for (int i = 0; i < 2; i++) {
        PRINT(uint32_t, "fl", float_fp[i]);
        printf("\n");
        float *pp = (float *)&float_fp[i];
        float ff = *pp;
        fp10 x = float_fp10(ff);
        PRINT(uint16_t, "fp", x);
        printf("\n");
    }


}
*/
