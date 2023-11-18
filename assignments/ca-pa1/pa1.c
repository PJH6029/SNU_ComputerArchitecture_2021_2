//---------------------------------------------------------------
//
//  4190.308 Computer Architecture (Fall 2021)
//
//  Project #1: Run-Length Encoding
//
//  September 14, 2021
//
//  Jaehoon Shim (mattjs@snu.ac.kr)
//  Ikjoon Son (ikjoon.son@snu.ac.kr)
//  Seongyeop Jeong (seongyeop.jeong@snu.ac.kr)
//  Systems Software & Architecture Laboratory
//  Dept. of Computer Science and Engineering
//  Seoul National University
//
//---------------------------------------------------------------



void setBit(const int bitIdx, char* const dst, const int dstlen) {
    /*
     * consider dst as binary array
     * set (bitIdx)-th bit to 1
     */

    int elementBitSize = 8;

    if (bitIdx >= dstlen * elementBitSize) {
        return;
    }

    int arrIdx = bitIdx / elementBitSize;
    int innerIdx = elementBitSize - bitIdx % elementBitSize - 1;

    unsigned int mask = 1 << innerIdx;  // 000010000

    *(dst + arrIdx) = *(dst + arrIdx) | mask;
}

void unsetBit(const int bitIdx, char* const dst, const int dstlen) {
    /*
     * consider dst as binary array
     * unset (bitIdx)-th bit to 0
     */

    int elementBitSize = 8;

    if (bitIdx >= dstlen * elementBitSize) {
        return;
    }

    int arrIdx = bitIdx / elementBitSize;
    int innerIdx = elementBitSize - bitIdx % elementBitSize - 1;

    unsigned int mask = ~(1 << innerIdx);

    *(dst + arrIdx) = *(dst + arrIdx) & mask;
}

void setMultipleBits(const int startBitIdx, const int endBitIdx, char* const dst, const int dstlen) {
    /*
     * consider dst as binary array
     * set (startBitIdx)-th bit ~ (endBitIdx - 1)-th bit to 1
     */

    int elementBitSize = 8;

    if (endBitIdx <= startBitIdx) {
        return;
    }

    if (endBitIdx >= dstlen * elementBitSize) {
        return;
    }

    int startArrIdx = startBitIdx / elementBitSize;
    int startInnerIdx = elementBitSize - startBitIdx % elementBitSize - 1;


    int endArrIdx = endBitIdx / elementBitSize;
    int endInnerIdx = elementBitSize - endBitIdx % elementBitSize - 1;


    if (startArrIdx == endArrIdx) {
        // 한 element 안에서 해결 가능
        int numBits = endBitIdx - startBitIdx;
        unsigned int mask = (1 << numBits) - 1;
        mask <<= (endInnerIdx + 1);
        *(dst + startArrIdx) = *(dst + startArrIdx) | mask;
    } else {
        // element를 건너뛰어아함
        for (int idx = startArrIdx + 1; idx < endArrIdx; idx++) {
            // 사이에 걸쳐있는 element들 처리 (endBitIdx - startBitIdx > 8인 경우)
            *(dst + idx) = *(dst + idx) | ((1 << elementBitSize) - 1);
        }

        // start element 처리
        int startNumBits = startInnerIdx + 1;
        unsigned int startMask = (1 << startNumBits) - 1;
        *(dst + startArrIdx) = *(dst + startArrIdx) | startMask;

        // end element 처리
        int endNumBits = elementBitSize - endInnerIdx - 1;
        unsigned int endMask = (1 << endNumBits) - 1;
        endMask <<= (endInnerIdx + 1);
        *(dst + endArrIdx) = *(dst + endArrIdx) | endMask;
    }
}

void unsetMultipleBits(const int startBitIdx, const int endBitIdx, char* const dst, const int dstlen) {
    /*
     * consider dst as binary array
     * unset (startBitIdx)-th bit ~ (endBitIdx - 1)-th bit to 0
     */

    int elementBitSize = 8;

    if (endBitIdx <= startBitIdx) {
        return;
    }

    if (endBitIdx >= dstlen * elementBitSize) {
        return;
    }


    int startArrIdx = startBitIdx / elementBitSize;
    int startInnerIdx = elementBitSize - startBitIdx % elementBitSize - 1;

    int endArrIdx = endBitIdx / elementBitSize;
    int endInnerIdx = elementBitSize - endBitIdx % elementBitSize - 1;


    if (startArrIdx == endArrIdx) {
        // 한 element 안에서 해결 가능
        int numBits = endBitIdx - startBitIdx;
        unsigned int mask = (1 << numBits) - 1;
        mask <<= (endInnerIdx + 1);
        mask = ~mask;
        *(dst + startArrIdx) = *(dst + startArrIdx) & mask;
    } else {
        // element를 건너뛰어아함
        for (int idx = startArrIdx + 1; idx < endArrIdx; idx++) {
            // 사이에 걸쳐있는 element들 처리 (endBitIdx - startBitIdx > 8인 경우)
            *(dst + idx) = *(dst + idx) & 0;
        }

        // start element 처리
        int startNumBits = startInnerIdx + 1;
        unsigned int startMask = (1 << startNumBits) - 1;
        startMask = ~startMask;
        *(dst + startArrIdx) = *(dst + startArrIdx) & startMask;

        // end element 처리
        int endNumBits = elementBitSize - endInnerIdx - 1;
        unsigned int endMask = (1 << endNumBits) - 1;
        endMask <<= (endInnerIdx + 1);
        endMask = ~endMask;
        *(dst + endArrIdx) = *(dst + endArrIdx) & endMask;
    }
}

void writeRunLength(const int runLength, const int startIdx, const int writelen, char* const dst, const int dstlen) {
    if (runLength >= (1 << writelen)) {
        return;
    }

    if (runLength == 7) {
        setMultipleBits(startIdx, startIdx + 3, dst, dstlen);
    } else {
        for (int i = 0; i < writelen; i++) {
            int bit = (runLength >> (writelen - i - 1)) & 1;
            if (bit) {
                setBit(startIdx + i, dst, dstlen);
            } else {
                unsetBit(startIdx + i, dst, dstlen);
            }
        }
    }
}

void addPadding(const int paddingBitsNum, const int startIdx, char* const dst, const int dstlen) {
    if (startIdx + paddingBitsNum >= dstlen - 1) {
        return;
    }

    unsetMultipleBits(startIdx, startIdx + paddingBitsNum, dst, dstlen);
}


int encode(const char* const src, const int srclen, char* const dst, const int dstlen) {
    /*
     * src: input1, input2, ...
     * srclen: byte 단위 길이
     */

    /*
     * element idx : -------0-------   -------1-------   -------2-------
     * bit idx     : 7 6 5 4 3 2 1 0   7 6 5 4 3 2 1 0   7 6 5 4 3 2 1 0
     * value (ex)  : 0 1 0 0 1 0 1 0   0 1 0 1 1 0 1 1   1 0 0 1 0 0 0 0
     */

    /*
     * write 순서:
     * dst[0][7], dst[0][6], ..., dst[0][0], dst[1][7], ...
     */

    int cntTarget = 0;
    int runLength = 0;
    int writeBitIdx = 0;
    int resultSize;

    int elementSize = 8;
    int writelen = 3;

    if (srclen <= 0) {
        return 0;
    }

    for (int i = 0; i < srclen; i++) {
        for (int j = elementSize - 1; j >= 0; j--) {
            int currBit = (src[i] >> j) & 1;

            if (currBit == cntTarget) {
                runLength++;
            } else {
                writeRunLength(runLength, writeBitIdx, writelen, dst, dstlen);
                writeBitIdx += writelen;
                runLength = 1;
                cntTarget = cntTarget ^ 1;
            }

            if (runLength >= 7) {
                writeRunLength(runLength, writeBitIdx, writelen, dst, dstlen);
                writeBitIdx += writelen;
                runLength = 0;  // 카운트 과정에서 바뀐게 아니므로 0으로
                cntTarget = cntTarget ^ 1;
            }
        }
    }

    if (runLength > 0) {
        // 남은 runLength 처리
        writeRunLength(runLength, writeBitIdx, writelen, dst, dstlen);
        writeBitIdx += writelen;
    }


    resultSize = (writeBitIdx - 1) / elementSize + 1;

    int paddingBitsNum = elementSize - writeBitIdx % elementSize;
    if (paddingBitsNum != elementSize) {
        addPadding(paddingBitsNum, writeBitIdx, dst, dstlen);
    }


    if (dstlen < resultSize) {
        return -1;
    }

    return resultSize;
}
int readRunLength(const int readBitIdx, const int readlen, const char* const src, const int srclen) {
    int elementBitSize = 8;

    if (readBitIdx + readlen > srclen * elementBitSize) {
        return -1;
    }

    int runLength = 0;
    for (int i = readBitIdx; i < readBitIdx + readlen; i++) {
        int arrIdx = i / elementBitSize;
        int innerIdx = elementBitSize - i % elementBitSize - 1;
        int currBit = (src[arrIdx] >> innerIdx) & 1;
        runLength <<= 1;
        runLength += currBit;
    }

    return runLength;
}

void writeDecodeResult(const int runLength, const int writeValue, const int writeBitIdx, char* const dst, const int dstlen) {
    if (writeValue == 0) {
        unsetMultipleBits(writeBitIdx, writeBitIdx + runLength, dst, dstlen);
    } else {
        setMultipleBits(writeBitIdx, writeBitIdx + runLength, dst, dstlen);
    }
}

int decode(const char* const src, const int srclen, char* const dst, const int dstlen)
{
    int cntTarget = 0;

    int writeBitIdx = 0;

    int resultSize;

    int elementBitSize = 8;
    int readlen = 3;

    if (srclen <= 0) {
        return 0;
    }


    for (int readBitIdx = 0; readBitIdx < srclen * elementBitSize; readBitIdx += readlen) {
        int runLength = readRunLength(readBitIdx, readlen, src, srclen);

        writeDecodeResult(runLength, cntTarget, writeBitIdx, dst, dstlen);
        writeBitIdx += runLength;
        cntTarget = cntTarget ^ 1;
    }

    resultSize = (writeBitIdx - 1) / elementBitSize + 1;

    if (dstlen < resultSize) {
        return -1;
    }

    return resultSize;
}
