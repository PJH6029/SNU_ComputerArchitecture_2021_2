beq t0, t0, BEQ # nt -> t
add t1, t2, t3
addi t1, t1, 123
addi t1, x0, 1

BEQ:
sub t5, t6, t3
xori t5, t5, 1
add t6, t6, t5
addi t1, x0, 1
beq x0, t1, BEQ2 # nt -> nt
srli t1, t1, 1
slli t1, t1, 2

BEQ2:
ori t5, t5, 1
addi t0, t0, 1

BEQ3:
add t5, t6, x0
addi t0, t0, -1
bge t0, x0, BEQ3 # t -> t

ori t6, t6, 1
addi t6, t6, -1

BEQ4:
addi t1, t2, 1
addi t1, t2, 1
addi t1, x0, 1
beq t1, x0, BEQ4 # t -> nt
addi x0, x0, 0
ebreak
