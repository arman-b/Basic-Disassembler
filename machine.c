#include <stdio.h>
#include "machine.h"


static int valid_instruction(unsigned short opcode, unsigned short reg1,
				unsigned short reg2, unsigned short reg3,
				unsigned int addr_or_constant);


void print_instruction(Hardware_word instruction) {
  /* By shifting the insturction word to the right by 28,
     we get the first 4 bits which would give us the opcode */
  unsigned int opcode = instruction >> 28;
  unsigned int temp;
  int i;
  
  /* Determining the opcode by corresponding decimal value */
  switch (opcode) {
  case 0:
    printf("halt\t");
    break;
  case 1:
    printf("add\t");
    break;
  case 2:
    printf("sub\t");
    break;
  case 3:
    printf("mul\t");
    break;
  case 4:
    printf("div\t");
    break;
  case 5:
    printf("rem\t");
    break;
  case 6:
    printf("inv\t");
    break;
  case 7:
    printf("and\t");
    break;
  case 8:
    printf("or\t");
    break;
  case 9:
    printf("not\t");
    break;
  case 10:
    printf("cmp\t");
    break;
  case 11:
    printf("mv\t");
    break;
  case 12:
    printf("li\t");
    break;
  case 13:
    printf("load\t");
    break;
  case 14:
    printf("store\t");
    break;
  }

  /* Prints the relevant registers of this instruction dependent upon the
     certain opcode. We iterate through this loop 3 times, and dependent
     upon which iteration we are on, if the certain opcode includes that
     register as an operand, we print the (i+1)st register number/value
     through bitshifting/bitmasking to retrieve the 5 bits of the
     instruction that signify the register. */
  if (opcode != 0) {
    for (i = 0; i < 3; i++) {
      if (!((i == 1 && (opcode == 12 || opcode == 13 || opcode == 14)) ||
	    (i == 2 && (opcode == 6 || opcode == 9 || opcode == 11 ||
			opcode == 12 || opcode == 13 || opcode == 14))))
	printf("R%d\t", (instruction >> (23 - (5 * i))) & 31);
    }
  }

  temp = instruction & 8191;
  if (opcode == 12)
    printf("%d", temp);
  else if (opcode == 10 || opcode == 13 || opcode == 14)
    printf("%04d", temp);
}


/* If the given parameters represent a valid opcode +  Hardware_word pointer,
   and possibly valid registers, memory address, and constant depending on
   the opcode, an instruction word is built dependent upon these parameters */
unsigned int encode_instruction(unsigned short opcode, unsigned short reg1,
				unsigned short reg2, unsigned short reg3,
				unsigned int addr_or_constant,
				Hardware_word *const hw_word) {
  unsigned int instruction;
  if (hw_word == NULL ||
      valid_instruction(opcode, reg1, reg2, reg3, addr_or_constant) == 0)
    return 0;
  instruction = opcode << 28;
  *hw_word = (((instruction | (reg1 << 23)) | (reg2 << 18))
	      | (reg3 << 13)) | addr_or_constant;
  return 1;
}


/* In this function, we disassemble the set of instructions and
machine/hardware word data points given to us into their seperate
assembly components to display both their assembly
components/instructions and their respective location in memory,
calling upon the print_instruction() function */ 
unsigned int disassemble(const Hardware_word memory[],
			 unsigned int memory_size, unsigned int num_instrs) {
  int i;

  if (memory == NULL || memory_size > 512 || memory_size == 0 ||
      num_instrs == 0 || num_instrs > memory_size)
    return 0;
  
  for (i = 0; i < memory_size; i++) {
    if (i >= num_instrs)
      printf("%03x: %08x\n", i * 4, memory[i]);
    else {
      unsigned short opcode = memory[i] >> 28;
      unsigned short reg1 = (memory[i] >> 23) & 31; /* 31 signifies 11111 */
      unsigned short reg2 = (memory[i] >> 18) & 31;
      unsigned short reg3 = (memory[i] >> 13) & 31;
      unsigned int addr_or_constant = memory[i] & 8191; /* 8191 signifies 13 1's
							   in binary*/
      
      if (valid_instruction(opcode, reg1, reg2, reg3, addr_or_constant) == 0)
	return 0;
      
      /* Printing  mathlon memory address of instruction in hex */
      printf("%03x: ", i * 4);
      /* Printing instruction word through print_instruction() */
      print_instruction(memory[i]);
      printf("\n");
    }
  }
  return 1;
}


/* Compares the useful/relevant fields of these two instructions
   dependent on their opcode value to see if they
   carry out the same instruction */
unsigned int compare_instructions(Hardware_word instr1, Hardware_word instr2) {
  unsigned int opcode = instr1 >> 28;
  unsigned int opcode2 = instr2 >> 28;

  unsigned int first_reg1 = (instr1 >> 23) & 31;
  unsigned int first_reg2 = (instr1 >> 18) & 31;
  unsigned int first_reg3 = (instr1 >> 13) & 31;
  unsigned int first_addr_or_constant = instr1 & 8191;
	
  unsigned int second_reg1 = (instr2 >> 23) & 31;
  unsigned int second_reg2 = (instr2 >> 18) & 31;
  unsigned int second_reg3 = (instr2 >> 13) & 31;
  unsigned int second_addr_or_constant = instr2 & 8191;

  if (opcode != opcode2)
    return 0;
  
  if (opcode == 1 || opcode == 2 || opcode == 3 || opcode == 4 ||
      opcode == 5 || opcode == 7 || opcode == 8) {
    if (first_reg1 == second_reg1 && first_reg2 == second_reg2 &&
	first_reg3 == second_reg3)
      return 1;
  }
  else if (opcode == 6 || opcode == 9 || opcode == 11) {
    if (first_reg1 == second_reg1 && first_reg2 == second_reg2)
      return 1;
  }
  else if (opcode == 12 || opcode == 13 || opcode == 14) {
    if (first_reg1 == second_reg1 &&
	first_addr_or_constant == second_addr_or_constant)
      return 1;
  }
  else if (opcode == 10) {
    if (first_reg1 == second_reg1 && first_reg2 == second_reg2 &&
	first_reg3 == second_reg3 &&
	first_addr_or_constant == second_addr_or_constant)
      return 1;
  }
  return 0;
}


static int valid_instruction(unsigned short opcode, unsigned short reg1,
				unsigned short reg2, unsigned short reg3,
				unsigned int addr_or_constant) {
  if (!(opcode >= 0 && opcode < 15))
    return 0;
  
  if (opcode == 1 || opcode == 2 || opcode == 3 || opcode == 4 ||
      opcode == 5 || opcode == 7 || opcode == 8) {
    if (!((reg1 > 1 && reg1 < 20) && (reg2 >= 0 && reg2 < 20) &&
	  (reg3 >= 0 && reg3 < 20)))
      return 0;
  }
  else if (opcode == 6 || opcode == 9 || opcode == 11) {
    if (!((reg1 > 1 && reg1 < 20) && (reg2 >= 0 && reg2 < 20)))
      return 0;
  }
  else if (opcode == 12) {
    if (!(reg1 > 1 && reg1 < 20 && addr_or_constant >= 0 &&
	  addr_or_constant < 8192))
      return 0;
  }
  else if (opcode == 10) {
    if (!((reg1 >= 0 && reg1 < 20) && (reg2 >= 0 && reg2 < 20) &&
	  (reg3 >= 0 && reg3 < 20) &&
	  (addr_or_constant >= 0 && addr_or_constant < 2048 &&
	   (addr_or_constant % 4) == 0)))
      return 0;
  }
  else if (opcode == 13) {
    if (!((reg1 > 1 && reg1 < 20) &&
	  (addr_or_constant >= 0 && addr_or_constant < 2048 &&
	   (addr_or_constant % 4) == 0)))
      return 0;
  }
  else if (opcode == 14) {
    if (!((reg1 >= 0 && reg1 < 20) &&
	  (addr_or_constant >= 0 && addr_or_constant < 2048 &&
	   (addr_or_constant % 4) == 0)))
      return 0;
  }
  return 1;
}
