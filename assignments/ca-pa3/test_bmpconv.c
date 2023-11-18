#include <stdio.h>
typedef unsigned char uc;
typedef unsigned int ui;

void bmpconv(unsigned int *imgptr, int h, int w, 
            unsigned int *k, unsigned int *outptr) {
    printf("%d %d\n", h, w);
    unsigned int img_row_sz = 3 * w + 3;
    img_row_sz &= 0xfffffffc;

    unsigned int img_row_word_num = img_row_sz >> 2;

    unsigned int ret_img_row_sz = 3 * (w - 2) + 3;
    ret_img_row_sz &= 0xfffffffc;

    printf("%d %d %d\n", img_row_sz, img_row_word_num, ret_img_row_sz);

    for (int i = 0; i < h; i++) {
        unsigned int kernel_row = i % 3;
        char kernel_cir, kernel_tri, kernel_rect;
        if (kernel_row == 0) {
            unsigned int kernel_word = *k;
            kernel_cir = kernel_word & 0xff;
            kernel_tri = (kernel_word >> 8) & 0xff;
            kernel_rect = (kernel_word >> 16) & 0xff;
        } else if (kernel_row == 1) {
            unsigned int kernel_word1 = *k;
            unsigned int kernel_word2 = *(k + 1);
            kernel_cir = (kernel_word1 >> 24) & 0xff;
            kernel_tri = kernel_word2 & 0xff;
            kernel_rect = (kernel_word2 >> 8) & 0xff;
        } else {
            ui kernel_word1 = *(k + 1);
            ui kernel_word2 = *(k + 2);
            kernel_cir = (kernel_word1 >> 16) & 0xff;
            kernel_tri = (kernel_word1 >> 24) & 0xff;
            kernel_rect = (kernel_word2) & 0xff;
        }
        printf("row %d kernel: %x %x %x\n", kernel_row, kernel_cir, kernel_tri, kernel_rect);


        // kernel_cir = *(k + 3 * kernel_row);
        // kernel_tri = *(k + 3 * kernel_row + 1);
        // kernel_row = *(k + 3 * kernel_row + 2);
        for (int j = 0; j < img_row_word_num; j++) {
            unsigned int loaded_word = *(imgptr + img_row_word_num * i + j);
            printf("loaded word: %x\n", loaded_word);
            for (int u = 0; u < 4; u++) {
                unsigned int mask = 0xff << (u << 3);
                unsigned char target_byte = (loaded_word & mask) >> (u << 3);
                printf("%x\n", target_byte);


                for (int v = 0; v < 3; v++) {
                    // c, t, r order
                    char byte_kernel, conv_res;
                    if (v == 0) {
                        byte_kernel = kernel_cir;
                    } else if (v == 1) {
                        byte_kernel = kernel_tri;
                    } else {
                        byte_kernel = kernel_rect;
                    }

                    if (byte_kernel == 0) {
                        conv_res = 0;
                    } else if (byte_kernel == 1) {
                        conv_res = target_byte;
                    } else {
                        conv_res = -target_byte;
                    }
                    
                    
                }

            }
        }
    }

}

int test1_kernerl[] = {
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

int main() {
    int output[10];

    bmpconv(test1_bitmap, test1_h, test1_w, test1_kernerl, output);

    return 0;
}