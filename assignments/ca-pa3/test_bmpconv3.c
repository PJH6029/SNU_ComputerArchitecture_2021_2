#include <stdio.h>
typedef unsigned char uc;
typedef unsigned int ui;


void bmpconv(unsigned int *imgptr, int h, int w, 
            unsigned int *k, unsigned int *outptr) {
    
    ui img_row_byte_num = 3 * w + 3;
    img_row_byte_num &= 0xfffffffc;

    ui img_row_word_num = img_row_byte_num >> 2;

    ui ret_img_row_byte_num = 3 * (w - 2) + 3;
    ret_img_row_byte_num &= 0xfffffffc;

    ui ret_img_row_word_num = ret_img_row_byte_num >> 2;

    unsigned int kers[9];

    // init
    for (int i = 0; i < h - 2; i++) {
        for (int j = 0; j < ret_img_row_word_num; j++) {
            *(outptr + ret_img_row_word_num * i + j) = 0;
        }
    }

    // parse kernel
    ui ker_load = *k;
    ui tmp;
    for (int i = 0; i < 3; i++) {
        ker_load = *(k + i);
        if (i != 2) {
            for (int j = 0; j < 4; j++) {
                kers[4 * i + j] = (ker_load >> (j << 3)) & 0x000000ff;
            }
        } else {
            kers[8] = (ker_load) & 0x000000ff;
        }
    }

    // for (int i = 0; i < h - 2; i++) {
    int* first_row_addr = imgptr;
    unsigned int* max_addr = outptr + ret_img_row_word_num * (h - 2);
    for (; outptr < max_addr; outptr += ret_img_row_word_num) {
        for (int j = 0; j < w - 2; j++) {

            ui word_j_idx = (3 * j + 4) / 4 - 1;

            // printf("type: %d\n", type);

            int conv_res[3]; // bgr
            for (int i = 0; i < 3; i++) {
                conv_res[i] = 0;
            }

            int* first_word_addr = first_row_addr + word_j_idx;

            for (int k_row = 0; k_row < 3; k_row++) {
                ui word1, word2, word3;

                // ui* first_word_addr = imgptr + img_row_word_num * (i + k_row) + word_j_idx;

                ui mask, tmp;
                // parse words
                // word1, word2, word3: BGR0 BGR0 BGR0 (0RGB 0RGB 0RGB)
                unsigned char type = j % 4;

                if (type == 0) {
                    // BGRB GRBG Rxxx -> BGRx
                    // BRGB GBRG xxxR -> xRGB
                    word3 = *(first_word_addr + 2);
                    word3 &= 0x000000ff;
                    word3 <<= (2 << 3);

                    word2 = *(first_word_addr + 1);
                    tmp = word2 & 0xffff0000;
                    tmp >>= (2 << 3);
                    word3 |= tmp;

                    word2 &= 0x0000ffff;
                    word2 <<= (1 << 3);
                    
                    word1 = *(first_word_addr);
                    tmp = word1 & 0xff000000;
                    tmp >>= (3 << 3);
                    word2 |= tmp;

                    word1 &= 0x00ffffff;
                } else if (type == 1) {
                    // xxxB GRBG RBGR -> xxxB GRxx
                    // Bxxx GBRG RGBR -> Bxxx xxRG
                    word1 = *(first_word_addr);
                    word1 >>= (3 << 3);

                    word2 = *(first_word_addr + 1);
                    tmp = word2 & 0x0000ffff;
                    tmp <<= (1 << 3);
                    word1 |= tmp;

                    word2 &= 0xffff0000;
                    word2 >>= (2 << 3);

                    word3 = *(first_word_addr + 2);
                    tmp = word3 & 0x000000ff;
                    tmp <<= (2 << 3);
                    word2 |= tmp;

                    word3 >>= (1 << 3);
                    // if (j == 1) {
                    //     printf("%d %x\n", j, word3);
                    //     return;
                    // }
                } else if (type == 2) {
                    // xxBG RBGR BGRx -> xxBG Rxxx
                    // GBxx RGBR xRGB -> GBxx xxxR
                    word1 = *(first_word_addr);
                    word1 >>= (2 << 3);

                    word2 = *(first_word_addr + 1);
                    tmp = word2 & 0x000000ff;
                    tmp <<= (2 << 3);
                    word1 |= tmp;

                    word2 &= 0xffffff00;
                    word2 >>= (1 << 3);

                    word3 = *(first_word_addr + 2);
                    word3 &= 0x00ffffff;
                } else {
                    // xBGR BGRB GRxx -> xBGR
                    // RGBx BRGB xxRG -> RGBx
                    word3 = *(first_word_addr + 2);
                    word3 &= 0x0000ffff;
                    word3 <<= (1 << 3);

                    word2 = *(first_word_addr + 1);
                    tmp = word2 &= 0xff000000;
                    tmp >>= (3 << 3);
                    word3 |= tmp;

                    word2 &= 0x00ffffff;

                    word1 = *(first_word_addr);
                    word1 >>= (1 << 3);
                }
                // now
                // word1, word2, word3: BGR0 BGR0 BGR0 (0RGB 0RGB 0RGB)
                // TODO store
                // printf("%x %x %x\n", word1, word2, word3);
                unsigned int words[3];
                words[0] = word1;
                words[1] = word2;
                words[2] = word3;

                // compute conv
                int ker_addr = 3 * k_row;
                for (int k_col = 0; k_col < 3; k_col++) {
                    ui ker = kers[ker_addr];
                    ui target_word = words[k_col];    

                    if (ker == 1) {
                        for (int color = 0; color < 3; color++) {
                            // bgr order
                            int byte = (target_word >> (color << 3)) & 0x000000ff;
                            conv_res[color] += byte;
                        }
                    }

                    if (ker == 0xff) {
                        for (int color = 0; color < 3; color++) {
                            // bgr order
                            int byte = (target_word >> (color << 3)) & 0x000000ff;
                            // byte = -byte;
                            // printf("neg byte %x %x\n", -byte, conv_res[i]);
                            conv_res[color] += -byte;
                        }
                    }
                    // printf("%d %d %d %d col end: %x %x %x\n", i, j, k_row, k_col, conv_res[0], conv_res[1], conv_res[2]);
                    ker_addr += 1;
                }
                // printf("%d %d %d row end: %x %x %x\n", i, j, k_row, conv_res[0], conv_res[1], conv_res[2]);
                first_word_addr += img_row_word_num;
            }
            // printf("%d %d pixel end: %x %x %x\n", i, j, conv_res[0], conv_res[1], conv_res[2]);


            for (int i = 0; i < 3; i++) {
                if (conv_res[i] > 255) {
                    conv_res[i] = 255;
                } else if (conv_res[i] < 0) {
                    conv_res[i] = 0;
                }
            }
            //printf("%d: %x %x %x\n", j, conv_res[2], conv_res[1], conv_res[0]);

            // conv_res: 0RGB
            unsigned char type = j % 4;
            ui* write_addr = outptr + word_j_idx;
            ui tmp;
            if (type == 0) {
                tmp = (conv_res[2] << (2 << 3));
                tmp |= (conv_res[1] << (1 << 3));
                tmp |= conv_res[0];
                *(write_addr) |= tmp;
            } else if (type == 1) {
                *(write_addr) |= (conv_res[0] << (3 << 3));

                tmp = (conv_res[2] << (1 << 3));
                tmp |= conv_res[1];

                *(write_addr + 1) |= tmp;
            } else if (type == 2) {
                tmp = (conv_res[1] << (1 << 3));
                tmp |= conv_res[0];
                tmp <<= (2 << 3);

                *(write_addr) |= tmp;

                *(write_addr + 1) |= conv_res[2];
            } else {
                tmp = (conv_res[2] << (2 << 3));
                tmp |= (conv_res[1] << (1 << 3));
                tmp |= conv_res[0];
                tmp <<= (1 << 3);

                *(write_addr) |= tmp;
            }
        }
        first_row_addr += img_row_word_num;
    }

}

int test1_kernel[] = {
    0xff000101,
    0x0100ff00,
    0x00000001
};

int test1_w = 5, test1_h = 4;

int test1_bitmap[] = {
    0x00000000, 0xf2350000, 0xfbf235fb, 0x00000000,
    0x00000000, 0xf2350000, 0xfa6a0afb, 0x00fa6a0a,
    0x35fbf235, 0xf235fbf2, 0xfbf235fb, 0x00000000,
    0x35000000, 0xf235fbf2, 0x000000fb, 0x00000000
};

int test1_ans[] = {
    0x95fbf235, 0xff60ffff, 0x000000fc,
    0x00000000, 0x6a0a0000, 0x000000fa
};

int test2_kernel[] = {
    0xff000001,
	0x0101ff01,
	0x00000001
};

int test2_w = 5, test2_h = 8;

int test2_bitmap[] = {
    0x913e3d3c
	,0x33328e90
	,0x3e3d3c34
	,0x00434241
	,0x808e3d4d
	,0x9e8e3e92
	,0x001ebcaa
	,0x00871300
	,0x988234d3
	,0x11443122
	,0xeed32200
	,0x00e342d1
	,0x00000000
	,0x00000000
	,0x00000000
	,0x00000000
	,0x913e3d3c
	,0x33328e90
	,0x3e3d3c34
	,0x00434241
	,0x988234d3
	,0x11443122
	,0xeed32200
	,0x00e342d1
	,0x00000000
	,0x00000000
	,0x00000000
	,0x00000000
	,0x808e3d4d
	,0x9e8e3e92
	,0x001ebcaa
	,0x00871300
};

int test2_ans[] = {
    0xe1005bff
	,0xc6ffffff
	,0x000000d4
	,0x0a3d1a00
	,0xff000000
	,0x000000b5
	,0xffffffff
	,0xc3f3ffff
	,0x000000b5
	,0x63cf87ff
	,0xeeff876c
	,0x000000ff
	,0x1b001a00
	,0xb3000000
	,0x0000003f
	,0xffffffff
	,0xe0ffffff
	,0x000000ff
};

int main() {
    int output[50];
    for (int i = 0; i < 50; i++) {
        output[i] = 0;
    }
    printf("--before--\n");
    for (int i = 0; i < 6; i++ ) {
        printf("%x\n", output[i]);
    }
    bmpconv(test1_bitmap, test1_h, test1_w, test1_kernel, output);
    printf("--result--\n");
    for (int i = 0; i < 6; i++ ) {
        printf("%x\n", output[i]);
    }
    printf("--ans--\n");
    for (int i = 0; i < 6; i++ ) {
        printf("%x\n", test1_ans[i]);
    }   
    for (int i = 0; i < 6; i++ ) {
        printf("%d\n", test1_ans[i] == output[i]);
    }   

    return 0;
}