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
    ui words[9]; // b g r
    ui first_words[6]; // b g r


    // first load
    // BGRB GRBG Rxxx -> BGRx
    // BRGB GBRG xxxR -> xRGB
    ui word1, word2, word3;

    // first load
    ui* first_load_ptr = imgptr;
    for (int j = 0; j < 3; j++) {
        word3 = *(first_load_ptr + 2);
        word3 &= 0x000000ff;
        word3 <<= (2 << 3);

        word2 = *(first_load_ptr + 1);
        tmp = word2 & 0xffff0000;
        tmp >>= (2 << 3);
        word3 |= tmp;
        words[3 * j + 2] = word3;

        word2 &= 0x0000ffff;
        word2 <<= (1 << 3);
        
        word1 = *(first_load_ptr);
        tmp = word1 & 0xff000000;
        tmp >>= (3 << 3);
        word2 |= tmp;
        words[3 * j + 1] = word2;

        word1 &= 0x00ffffff;   
        words[3 * j] = word1;

        first_load_ptr += img_row_word_num;   
    } 
    for (int j = 0; j < 6; j++)  {
        first_words[j] = words[3 + j];
    }


    int* first_row_addr = imgptr;
    unsigned int* max_addr = outptr + ret_img_row_word_num * (h - 2);
    int first = 1;
    for (; outptr < max_addr; outptr += ret_img_row_word_num) {
        if (first) {
            first = 0;
            goto col_loop;
        }
        
        for (int j = 0; j < 6; j++) {
            words[j] = first_words[j];
        }

        // first load
        ui* first_load_ptr = first_row_addr + img_row_word_num + img_row_word_num;
        word3 = *(first_load_ptr + 2);
        word3 &= 0x000000ff;
        word3 <<= (2 << 3);

        word2 = *(first_load_ptr + 1);
        tmp = word2 & 0xffff0000;
        tmp >>= (2 << 3);
        word3 |= tmp;
        words[8] = word3;

        word2 &= 0x0000ffff;
        word2 <<= (1 << 3);
        
        word1 = *(first_load_ptr);
        tmp = word1 & 0xff000000;
        tmp >>= (3 << 3);
        word2 |= tmp;
        words[7] = word2;

        word1 &= 0x00ffffff;   
        words[6] = word1;
        for (int j = 0; j < 6; j++)  {
            first_words[j] = words[3 + j];
        }
        
        col_loop:
        printf("first load\n");
        for (int j = 0; j < 3; j++) {
            printf("%x %x %x\n", words[3*j], words[3*j+1], words[3*j+2]);
        }

        for (int j = 0; j < w - 2; j++) {
            ui word_j_idx = (3 * j + 4) / 4 - 1;
            int* first_word_addr = first_row_addr + word_j_idx;
            unsigned char type = j % 4;    

            int conv_res[3]; // bgr
            for (int i = 0; i < 3; i++) {
                conv_res[i] = 0;
            }

            // j==0: skip load
            if (j == 0) {
                printf("skip!\n");
                goto calculate_conv;
            }
            // printf("type: %d\n", type);


            // load words
            ui tmp1, tmp2;
            ui* new_word_addr = first_word_addr;
            for (int u = 0; u < 3; u++) {
                if (type == 0) {
                    tmp1 = *(new_word_addr + 1);
                    tmp1 >>= 16;
                    tmp2 = *(new_word_addr + 2) & 0xff;
                    tmp2 <<= 16;
                    tmp1 |= tmp2;
                } else if (type == 1) {
                    tmp1 = *(new_word_addr + 2);
                    tmp1 >>= 8;
                } else if (type == 2) {
                    tmp1 = *(new_word_addr + 2) & 0xffffff;
                } else {
                    tmp1 = *(new_word_addr + 2) & 0xffff;
                    tmp1 <<= 8;
                    tmp2 = *(new_word_addr + 1);
                    tmp2 >>= 24;
                    tmp1 |= tmp2;
                }

                words[3 * u] = words[3 * u + 1];
                words[3 * u + 1] = words[3 * u + 2];
                words[3 * u + 2] = tmp1;
                new_word_addr += img_row_word_num;
            }


            calculate_conv:

            printf("type %d col %d %d\n", type, j, word_j_idx);
            printf("%x %x %x\n", *(first_word_addr), *(first_word_addr+1), *(first_word_addr+2));
            printf("conv target\n");
            for (int j = 0; j < 3; j++) {
                printf("%x %x %x\n", words[3*j], words[3*j+1], words[3*j+2]);
            }
            // calculate conv
            for (int ker_idx = 0; ker_idx < 9; ker_idx++) {
                ui ker = kers[ker_idx];
                ui target_word = *(words + ker_idx);

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
            }

            for (int i = 0; i < 3; i++) {
                if (conv_res[i] > 255) {
                    conv_res[i] = 255;
                } else if (conv_res[i] < 0) {
                    conv_res[i] = 0;
                }
            }
            //printf("%d: %x %x %x\n", j, conv_res[2], conv_res[1], conv_res[0]);

            // write conv_res: 0RGB
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
        // outptr += ret_img_row_word_num;
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