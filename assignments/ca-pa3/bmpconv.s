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
    addi sp, sp, -148
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

    # first load
    mv t4, sp # word store addr
    add t4, t4, 84   

    mv t0, a0 # first_load_ptr
    li t2, 0 # j
    init_load_loop:
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
        sw t3, 8(t4) # store word3

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
        sw a1, 4(t4) # store word2

        li a2, 0xff
        slli a2, a2, 24
        not a2, a2 # a2: 0x00ffffff
        and t3, t3, a2
        sw t3, 0(t4) # store word1

        add t4, t4, 12
        addi t2, t2, 1
        lw t3, 12(sp)
        add t0, t0, t3
        li t3, 3
        blt t2, t3, init_load_loop
    
    mv t4, sp
    add t4, t4, 96
    li t2, 0 # j
    copy_loop:

        lw a2, 0(t4)
        sw a2, 24(t4)

        add t4, t4, 4
        addi t2, t2, 1
        li t3, 6
        blt t2, t3, copy_loop
    # lw a0, 120(sp)
    # lw a1, 124(sp)
    # lw a2, 128(sp)
    # lw a3, 132(sp)
    # lw a4, 136(sp)
    # lw t0, 140(sp)
    # lw t1, 108(sp)
    # lw t2, 112(sp)
    # lw t3, 116(sp)
    # ebreak

    # row loop var: a0, a4(img, out)

    # col loop var: t1

    # condition: t0 (also first_word_addr)

    # a0: first_row_addr
    li t4, 0
    sw t4, 144(sp)
    conv_row_loop: 
        lw t4, 144(sp)
        bne t4, x0, not_first
        # first
        li t4, 1
        sw t4, 144(sp)
        j col_loop_start

        not_first:
            # move prev data
            mv t4, sp
            add t4, t4, 84
            li t2, 0 # j
            update_loop:

                lw a2, 36(t4)
                sw a2, 0(t4)

                add t4, t4, 4
                addi t2, t2, 1
                li t3, 6
                blt t2, t3, update_loop         

            # update words
            mv t0, a0
            lw t4, 12(sp)
            add t0, t0, t4
            add t0, t0, t4 # first_load_ptr

            mv t4, sp
            addi t4, t4, 108

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
            sw t3, 8(t4) # store word3

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
            sw a1, 4(t4) # store word2

            li a2, 0xff
            slli a2, a2, 24
            not a2, a2 # a2: 0x00ffffff
            and t3, t3, a2
            sw t3, 0(t4) # store word1

            mv t4, sp
            add t4, t4, 96
            li t2, 0 # j
            copy_loop_inner:

                lw a2, 0(t4)
                sw a2, 24(t4)

                add t4, t4, 4
                addi t2, t2, 1
                li t3, 6
                blt t2, t3, copy_loop_inner



        col_loop_start:

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

            # t0 = first_word_addr (a0 + word_j_idx)
            add t0, a0, t4

            # conv_res bgr
            sw x0, 72(sp) # conv_res[b]
            sw x0, 76(sp) # conv_res[g]
            sw x0, 80(sp) # conv_res[r]
            
            beq t1, x0, calcuate_conv
            
            # load words
            mv t2, t0 # t2: new_word_addr
            li a2, 84 # 84 + sp + 12*i
            add a2, a2, sp
            load_loop:
            
                and t4, t1, 3 # type
                beq t4, x0, load_type0
                addi t4, t4, -1
                beq t4, x0, load_type1
                addi t4, t4, -1
                beq t4, x0, load_type2
                j load_type3

                # t3, t4
                load_type0:
                    lw t3, 4(t2)
                    srli t3, t3, 16
                    lw t4, 8(t2)
                    andi t4, t4, 0xff
                    slli t4, t4, 16
                    or t3, t3, t4
                    j load_type_exit
                load_type1:
                    lw t3, 8(t2)
                    srli t3, t3, 8
                    j load_type_exit
                load_type2:
                    lw t3, 8(t2)
                    li t4, 0xff
                    slli t4, t4, 24
                    not t4, t4
                    and t3, t3, t4
                    j load_type_exit
                load_type3:
                    li t4, 0xff
                    slli t4, t4, 8
                    addi t4, t4, 0xff
                    lw t3, 8(t2)
                    and t3, t3, t4
                    slli t3, t3, 8
                    lw t4, 4(t2)
                    srli t4, t4, 24
                    or t3, t3, t4
                load_type_exit:
                    # t3: tmp1
                    lw a1, 4(a2)
                    sw a1, 0(a2)
                    
                    lw a1, 8(a2)
                    
                    sw a1, 4(a2)

                    sw t3, 8(a2)

                lw a1, 12(sp)
                add t2, t2, a1
                addi a2, a2, 12
                li a1, 120
                add a1, a1, sp
                blt a2, a1, load_loop


                # li a1, 108
                # add a1, a1, sp
                # bne a2, a1, test
                # lw a0, 84(sp)
                # lw a1, 88(sp)
                # lw a2, 92(sp)
                # lw a3, 96(sp)
                # lw a4, 100(sp)
                # lw t0, 104(sp)
                # lw t1, 108(sp)
                # lw t2, 112(sp)
                # lw t3, 116(sp)
                # ebreak
                # test:

            calcuate_conv:

            li t2, 0 # ker_idx. condition: a1
            # a2: target_word
            # a1: ker
            ker_loop:
                addi a2, t2, 84
                add a2, a2, sp
                lw a2, 0(a2)

                addi a1, t2, 32
                add a1, a1, sp
                lw a1, 0(a1)  # ker

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
                conv_col_exit:

                addi t2, t2, 4
                li a1, 36
                blt t2, a1, ker_loop

            saturation:
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
        

    addi sp, sp, 148
    ret
