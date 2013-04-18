
addi $t1, $0, 5
addi $t2, $0, 10
and $t3, $0, $0

# Computes $t3 = $t1 * $t2, and then stores $t3 at memory address 0.
loop:
    beq $t1, $0, end
    add $t3, $t3, $t2
    addi $t1, $t1, -1
    j loop

end:
    sw $t3, 0($0)

