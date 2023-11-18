#----------------------------------------------------------------
#
#  4190.308 Computer Architecture (Fall 2021)
#
#  Project #3: Image Convolution in RISC-V Assembly
#
#  October 25, 2021
#
#  Jaehoon Shim (mattjs@snu.ac.kr)
#  Ikjoon Son (ikjoon.son@snu.ac.kr)
#  Seongyeop Jeong (seongyeop.jeong@snu.ac.kr)
#  Systems Software & Architecture Laboratory
#  Dept. of Computer Science and Engineering
#  Seoul National University
#
#----------------------------------------------------------------

####################
# void bmpconv(unsigned char *imgptr, int h, int w, unsigned char *k, unsigned char *outptr)
####################

/*
    a0: img ptr
    a1: h
    a2: w
    a3: kernel ptr
    a4: out ptr
*/
# available: a0~4, t0~4, x0, ra, sp

	.globl bmpconv
bmpconv:
    addi sp, sp, -128
    sw a4, 124(sp)
    sw a0, 0(sp) # img ptr

    # a1 = h - 2
    addi a1, a1, -2
    sw a1, 4(sp) # h - 2


    # img row byte num
    mv t0, a2 # t0 = 3w
    slli t0, t0, 1
    add t0, t0, a2
    addi t1, t0, 3 # t1 = (3w + r) // 4
    srli t1, t1, 2
    slli t1, t1, 2
    sw t1, 12(sp) # img row byte num
    
    addi a2, a2, -2
    sw a2, 8(sp) # w - 2

    # ret img row byte num
    addi t1, t0, -3 # t1 = (3(w-2) + r) // 4
    srli t1, t1, 2
    slli t1, t1, 2
    sw t1, 16(sp) # ret img row byte num

    # compute rirbn * (h - 2)
    li t0, 0 # ret
    li t2, 0 # i
    compute_rirwn_loop:
        add t0, t0, t1

        addi t2, t2, 1
        blt t2, a1, compute_rirwn_loop
    add t0, t0, a4
    sw t0, 20(sp) # outptr + rirbn * (h - 2)
    
    sw a3, 24(sp) # kernel
    sw a4, 28(sp) # out ptr
init:
    # init
    # in use: a0, a2, a3, a4
    # loop var: t0, t3 / t2
    # condition: a1 / t1
    # t1 = ret_img_row_byte_num
    li t0, 0 # i
    li t3, 0 # W*i
    init_outer_loop:
        li t2, 0 # j
        init_inner_loop:
            add t4, t3, t2 # t4 = W * i + j
            add t4, t4, a4 # t4 = outptr + W*i + j
            sw x0, 0(t4) # init to 0
            
            addi t2, t2, 4
            blt t2, t1, init_inner_loop
        addi t0, t0, 1  # i++
        add t3, t3, t1 # t3 += W
        blt t0, a1, init_outer_loop
parse_kernel:
    # parse kernel
    # var: t4

    # 32(sp), 36(sp), 40(sp)
    # 44(sp), 48(sp), 52(sp)
    # 56(sp), 60(sp), 64(sp) -> parsed kernels
    # first word
    lw t1, 0(a3) # t1 = first word of kernel

    andi t0, t1, 0xff
    sw t0, 32(sp)
    
    srli t1, t1, 8
    andi t0, t1, 0xff
    sw t0, 36(sp)

    srli t1, t1, 8
    andi t0, t1, 0xff
    sw t0, 40(sp)

    srli t1, t1, 8
    andi t0, t1, 0xff
    sw t0, 44(sp)

    # second word
    lw t1, 4(a3)

    andi t0, t1, 0xff
    sw t0, 48(sp)
    
    srli t1, t1, 8
    andi t0, t1, 0xff
    sw t0, 52(sp)

    srli t1, t1, 8
    andi t0, t1, 0xff
    sw t0, 56(sp)

    srli t1, t1, 8
    andi t0, t1, 0xff
    sw t0, 60(sp)

    # third word
    lw t1, 8(a3)

    andi t0, t1, 0xff
    sw t0, 64(sp)
do_conv:

    # 32(sp), 36(sp), 40(sp)
    # 44(sp), 48(sp), 52(sp)
    # 56(sp), 60(sp), 64(sp) -> parsed kernels

    /*
        a0: img ptr
        a1: h - 2
        a2: w - 2
        a3: kernel ptr
        a4: out ptr
    */

    /*
        ok a0 = img_ptr // a0: first_row_addr
        ok for (a4 = outptr; a4 < outptr + 4 * riwrn * (h-2); a4 += 4 * riwrn) // a4: write_row_addr
            ok for (j = 0; j < w-2; j++)
                type = j % 4
                ok word_j_idx = (3 * j + 4) / 4 - 1

                first_word_addr = a0 + 4 * word_j_jdx

                ok for (k_row = 0; k_row < 3; k_row++)
                    ...
                    ok ker_addr = sp + 32 + 12 * k_row
                    ok for (k_col = 0; k_col < 3; k_col++) -> 4배로?
                        ker = *(ker_addr)
                        ...
                        ok ker_addr += 4
                    ...
                    ok first_word_addr += irwn -> next row

                write_addr = a4 + word_j_idx
                ...
            ok a0 += 4 * irwn
    */


    # row loop var: a0, a4(img, out)

    # col loop var: t1

    # condition: t0 (also first_word_addr)

    # a0: first_row_addr
    
    conv_row_loop: 
        li t1, 0 # j
        conv_col_loop:
            # in use: a0(first_row_addr), t1(j), a4(write_row_addr)
            # temp available: t0
            # available: t2, t3, t4, a1, a2
            # don't know: a3(ker)

            
            # word_j_idx
            # t4 = ((3 * j + 4) // 4 - 1) * 4
            mv t4, t1
            slli t4, t4, 1
            add t4, t4, t1
            addi t4, t4, 4
            srli t4, t4, 2
            addi t4, t4, -1 
            slli t4, t4, 2
            sw t4, 68(sp) # word_j_idx

            # conv_res bgr
            sw x0, 72(sp) # conv_res[b]
            sw x0, 76(sp) # conv_res[g]
            sw x0, 80(sp) # conv_res[r]

            # t0 = first_word_addr (a0 + word_j_idx)
            add t0, a0, t4

            li t2, 0
            # condition: a1
            k_row_loop:          
                # in use: a0, t1, t2, t0,
                # available: t4, t3, a1, a2, (a3)

                sw x0, 84(sp) # word1
                sw x0, 88(sp) # word2
                sw x0, 92(sp) # word3

                andi t3, t1, 3 # type
                beq t3, x0, type0
                addi t3, t3, -1
                beq t3, x0, type1
                addi t3, t3, -1
                beq t3, x0, type2
                j type3


                # use t3, a1, a2
                # load words
                type0:
                    lw t3, 8(t0) # word3
                    andi t3, t3, 0x000000ff
                    slli t3, t3, 16

                    lw a1, 4(t0) # word2
                    li a2, 0xff
                    slli a2, a2, 8
                    addi a2, a2, 0xff
                    not a2, a2 # a2: 0xffff0000

                    and a2, a1, a2
                    srli a2, a2, 16
                    or t3, t3, a2
                    sw t3, 92(sp) # store word3

                    li a2, 0xff
                    slli a2, a2, 8
                    addi a2, a2, 0xff # a2: 0x0000ffff
                    and a1, a1, a2
                    slli a1, a1, 8

                    lw t3, 0(t0) # word1

                    li a2, 0xff
                    slli a2, a2, 24 # a2: 0xff000000
                    and a2, t3, a2 
                    srli a2, a2, 24
                    or a1, a1, a2
                    sw a1, 88(sp) # store word2

                    li a2, 0xff
                    slli a2, a2, 24
                    not a2, a2 # a2: 0x00ffffff
                    and t3, t3, a2
                    sw t3, 84(sp) # store word1
                    j type_exit
                type1:
                    lw t3, 0(t0) # word1
                    srli t3, t3, 24

                    lw a1, 4(t0) # word2
                    li a2, 0xff
                    slli a2, a2, 8
                    addi a2, a2, 0xff # a2: 0x0000ffff
                    and a2, a1, a2
                    slli a2, a2, 8
                    or t3, t3, a2
                    sw t3, 84(sp) # store word1

                    li a2, 0xff
                    slli a2, a2, 8
                    addi a2, a2, 0xff
                    not a2, a2
                    and a1, a1, a2
                    srli a1, a1, 16

                    lw t3, 8(t0) # word3
                    andi a2, t3, 0x000000ff
                    slli a2, a2, 16
                    or a1, a1, a2
                    sw a1, 88(sp) # store word2

                    srli t3, t3, 8
                    sw t3, 92(sp) # store word3
                
                    j type_exit
                type2:
                    lw t3, 0(t0) # word1

                    srli t3, t3, 16

                    lw a1, 4(t0) # word2
                    andi a2, a1, 0x000000ff
                    slli a2, a2, 16
                    or t3, t3, a2
                    sw t3, 84(sp) # store word1

                    li t3, 0xff
                    not t3, t3
                    and a1, a1, t3
                    srli a1, a1, 8
                    sw a1, 88(sp)

                    lw t3, 8(t0) # word3
                    li a1, 0xff
                    slli a1, a1, 24
                    not a1, a1
                    and t3, t3, a1
                    sw t3, 92(sp)

                    j type_exit
                type3:
                    lw t3, 8(t0) # word3
                    li a1, 0xff
                    slli a1, a1, 8
                    addi a1, a1, 0xff
                    and t3, t3, a1
                    slli t3, t3, 8

                    lw a1, 4(t0) # word2
                    li a2, 0xff
                    slli a2, a2, 24
                    and a2, a1, a2
                    srli a2, a2, 24
                    or t3, t3, a2
                    sw t3, 92(sp)

                    li t3, 0xff
                    slli t3, t3, 24
                    not t3, t3
                    and a1, a1, t3
                    sw a1, 88(sp)

                    lw t3, 0(t0) # word1
                    srli t3, t3, 8
                    sw t3, 84(sp)
                    
                type_exit:

                    # in use: a0, t1, t2, t0,
                    # available: t4, t3, a1, a2, (a3)

                    # now, 
                    # word1: 0RGB 84(sp)
                    # word2: 0RGB 88(sp)
                    # word3: 0RGB 92(sp)
                    # -> word addr: 84 + sp + 4 * k_col
                    # kernel_addr = 32 + sp + 12 * k_row +  4 * k_col

                    # compute conv
                    # conv_res[b]: 72(sp)
                    # conv_res[g]: 76(sp)
                    # conv_res[r]: 80(sp)

                    # ker start: 32(sp)

                    # t3: ker_addr
                    slli t3, t2, 3
                    slli t4, t2, 2
                    add t3, t4, t3 # t3 = 12 * k_row + sp + 32
                    add t3, t3, sp
                    addi t3, t3, 32
                    
                    li t4, 0 # k_col
                    k_col_loop:
                        # available: a1, a2, a3
                        # a1: ker, a2: target_word

                        beq t4, x0, col_0
                        addi t4, t4, -1
                        beq t4, x0, col_1
                        addi t4, t4, -1
                        beq t4, x0, col_2
                        addi t4, t4, 2
                        j load_target_word_exit

                        col_0:
                            lw a2, 84(sp)
                            j load_target_word_exit
                        col_1:
                            addi t4, t4, 1
                            lw a2, 88(sp)
                            j load_target_word_exit
                        col_2:
                            addi t4, t4, 2
                            lw a2, 92(sp)
                        load_target_word_exit:
                            # a2: target word
                            # jal ra, test2
                            # test2:
                            # ebreak
                            lw a1, 0(t3) # ker
                            addi a1, a1, -1
                            beq a1, x0, ker_1
                            addi a1, a1, -0xfe
                            beq a1, x0, ker_ff
                            j conv_col_exit

                            ker_1:
                                # b
                                andi a3, a2, 0x000000ff
                                lw a1, 72(sp)
                                add a1, a1, a3
                                sw a1, 72(sp)

                                # g
                                srli a2, a2, 8
                                andi a3, a2, 0x000000ff
                                lw a1, 76(sp)
                                add a1, a1, a3
                                sw a1, 76(sp)

                                # r
                                srli a2, a2, 8
                                andi a3, a2, 0x000000ff
                                lw a1, 80(sp)
                                add a1, a1, a3
                                sw a1, 80(sp)

                                j conv_col_exit
                            ker_ff:   
                                # b
                                andi a3, a2, 0x000000ff
                                neg a3, a3
                                lw a1, 72(sp)
                                add a1, a1, a3
                                sw a1, 72(sp)

                                # g
                                srli a2, a2, 8
                                andi a3, a2, 0x000000ff
                                neg a3, a3                                
                                lw a1, 76(sp)
                                add a1, a1, a3
                                sw a1, 76(sp)

                                # r
                                srli a2, a2, 8
                                andi a3, a2, 0x000000ff
                                neg a3, a3                                
                                lw a1, 80(sp)
                                add a1, a1, a3
                                sw a1, 80(sp)

                                j conv_col_exit
                            conv_col_exit:
  
                        # update k_col
                        addi t3, t3, 4
                        addi t4, t4, 1
                        li a1, 3 # k_col condition(3)
                        blt t4, a1, k_col_loop

                # update k_row
                
                # li a2, 1
                # bne t2, a2, test
                # lw t3, 72(sp)
                # lw t4, 76(sp)
                # lw a1, 80(sp)
                # ebreak
                # test:  

                lw a1, 12(sp) # irbn
                add t0, t0, a1 # first_word_addr += irbn
                
                li a1, 3 # k_row condition(3)
                addi t2, t2, 1
                blt t2, a1, k_row_loop

            # after k_row. saturation
            # b
            lw t4, 72(sp)
            blt t4, x0, under_sat_b
            li t0, 255
            bge t4, t0, over_sat_b
            j sat_exit_b

            under_sat_b:
                sw x0, 72(sp)
                j sat_exit_b
            over_sat_b:
                sw t0, 72(sp)
            sat_exit_b:

            # g
            lw t4, 76(sp)
            blt t4, x0, under_sat_g
            li t0, 255
            bge t4, t0, over_sat_g
            j sat_exit_g

            under_sat_g:
                sw x0, 76(sp)
                j sat_exit_g
            over_sat_g:
                sw t0, 76(sp)
            sat_exit_g:

            # r
            lw t4, 80(sp)
            blt t4, x0, under_sat_r
            li t0, 255
            bge t4, t0, over_sat_r
            j sat_exit_r

            under_sat_r:
                sw x0, 80(sp)
                j sat_exit_r
            over_sat_r:
                sw t0, 80(sp)
            sat_exit_r:
            

            # write res

            # in use: a0(first_row_addr), t1(j)
            # temp available: t0
            # available: t2, t3, t4, a1, a2
            # don't know: a3(ker)  

            lw t2, 68(sp) # t2 = word_j_idx
            add t2, t2, a4 # t2 = word_j_idx + write_row_addr: write_addr

            # t0: tmp
            andi t3, t1, 3 # type
            
            beq t3, x0, write_type0
            addi t3, t3, -1
            beq t3, x0, write_type1
            addi t3, t3, -1
            beq t3, x0, write_type2
            j write_type3

            write_type0:
                lw t0, 80(sp) # tmp = cr[2] << 16
                slli t0, t0, 16

                lw t3, 76(sp)
                slli t3, t3, 8
                or t0, t0, t3

                lw t3, 72(sp)
                or t0, t0, t3
                
                lw t4, 0(t2) # t4 = *(write_addr)
                or t4, t4, t0
                sw t4, 0(t2)
                
                j write_exit
            write_type1:
                lw t0, 72(sp) # tmp = cr[0] << 24
                slli t0, t0, 24

                lw t4, 0(t2) # t4 = *(write_addr)
                or t4, t4, t0
                sw t4, 0(t2)

                lw t0, 80(sp)
                slli t0, t0, 8

                lw t3, 76(sp)
                or t0, t0, t3

                lw t4, 4(t2)
                or t4, t4, t0
                sw t4, 4(t2)

                j write_exit
            write_type2:
                lw t0, 76(sp)
                slli t0, t0, 8

                lw t3, 72(sp)
                or t0, t0, t3
                slli t0, t0, 16

                lw t4, 0(t2) # t4 = *(write_addr)
                or t4, t4, t0
                sw t4, 0(t2)

                lw t4, 4(t2)
                lw t0, 80(sp)
                or t4, t4, t0
                sw t4, 4(t2)

                j write_exit
            write_type3:
                lw t0, 80(sp)
                slli t0, t0, 16
                
                lw t3, 76(sp)
                slli t3, t3, 8
                or t0, t0, t3

                lw t3, 72(sp)
                or t0, t0, t3

                slli t0, t0, 8

                lw t4, 0(t2)
                or t4, t4, t0
                sw t4, 0(t2)

            write_exit:

            # update col
            addi t1, t1, 1
            lw t0, 8(sp) # condition(w-2)
            blt t1, t0, conv_col_loop
        
        # update row
        lw t0, 12(sp) # irbn
        add a0, a0, t0
        lw t0, 16(sp) # rirbn
        add a4, a4, t0
        lw t0, 20(sp) # condition(outptr + riwbn*(h-2))
        blt a4, t0, conv_row_loop
        

    addi sp, sp, 128
    ret
