# This file makes sure that the signed and unsigned version of slt work
# differently.
addi  $t1, $0, -10
slti  $t2, $t1, 10   # $t2 = 1 if -10 < 10
sltiu $t3, $t1, 10   # $t3 = 1 if twos-complement(-10) < 10
sw $t2, 0($0)
sw $t3, 4($0)
