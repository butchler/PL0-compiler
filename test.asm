.ent main

main: # Comment
addi $8, $8, 1		
addi $9, $8, 2		
add $10, $8, $9		
sw $10, 0($29)        	
lw $11, 0($29) 	      	
slt $t4, $t3, $t2    	
sltu $t5, $t1, $t2  	
lui $14, 32		
j end		      	
# Because both the addi and sub are valid instructions, this will become an
# infinite loop if you run the compiled instructions.
label: addi $8, $8, 2929
#sub $8, $8, 3   # Leaving this invalid instruction will cause the assembler to
                 # quit with an error.
sub $8, $8, $3
end: beq $10, $11, label  
.end main
# The assembler just ignores the .ent and .end assembler directives, so the
# below instruction will still be compiled as if the .end line wasn't there.
addi $8, $8, -100
