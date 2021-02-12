# MIPS Disassembler

This is a dissassembler of binary images of MIPS compatible code. Binary image (0s and 1s) is translated into assembly instructions.

## Inputs and Outputs

- Program input: MIPS binary image (.bin)
- Program output: Decoded instructions on-screen and in an output text file (.txt)

## Features

- Partial simulation is provided in terms of:
  a. Address resolution in jumps and branches
  b. Loading 32 bit immediates

- This code is complete in terms of:
  a. Correctly translating the required instructions
  b. Correctly recognizing the required pseudo-instructions and producing it
  c. Correctly fulfilling all the required and bonus points in the project assignment sheet.

- The "load immediate" and "load address" pseudo-instructions are inrecognizable when
  loading 32-bit immediates so we decided to output that as (LI/LA).
  (Both translate into "load upper immediate" and a following "or immediate")

- The program counter has been adjusted occasionally to adjust to pseudo-instructions.

- A "negation function" has been implemented but was found to be redundant
  It was only kept to be available for the code writers for future reference.

## Authors

- Bishoy Boshra
- Hussam El-Araby
