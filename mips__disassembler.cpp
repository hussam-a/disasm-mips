/*
	Authors
	Bishoy Boshra
	Hussam El-Araby
*/

#include <iostream>	//Library for input streams
#include <fstream>	//Library for output streams
#include <string>	//Library for strings
#include <map>	//Library for maps data structures
#include <sstream> //Library for string streams
using namespace std;	//Standard namespace region used

//Function prototypes
void decodeInst(unsigned int);
unsigned int negate(unsigned int);
unsigned int getFromMemory(unsigned int, int, bool);
void storeToMemory(unsigned int, int, unsigned int);
void viewFile();
void appendLabels(unsigned int);
string intToString(int);

/*
	Variables declared:
	- Array representing register values
	- Array represnting register names
	- Streams and names of input/output files
	- Array representing memory (8KB)
	- Maps representing adddresses and their labels (jumped/branched to and from)
	- Strings for labels
	- Generic iterator for maps
*/

unsigned int regs[32] = { 0 };
string reg[32] = { "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0",
"t1", "t2", "t3", "t4", "t5", "t6", "t7", "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0",
"k1", "gp", "sp", "fp", "ra" };
ifstream inFile;
ofstream outFile;
string inFileName = "";
string outFileName = "";
unsigned char memory[8 * 1024] = { 0 };
map < unsigned int, string > labels;	//Addresses jumped/branched to
map < unsigned int, string > fromLabels;	//Addresses jumped/branched from
int labelCount = 1;
string found_label = "";
string from_label = "";
map < unsigned int, string >::iterator pointer;

//Main function
int main()
{
	/*
		Variables declared:
		- Variable to hold single instruction
	*/

	unsigned int instruction = 0;

	//Getting file names from user and opening them

	cout << "Enter the binary input filename (e.g. sample1.bin): ";
	cin >> inFileName;
	cout << "Enter the assembly output filename (e.g. sample1.txt): ";
	cin >> outFileName;
	cout << endl;

	inFile.open(inFileName.c_str(), ios::in | ios::binary);

	//Looping to append labels to certain addresses

	if (inFile.is_open())
	{
		while (true)
		{
			// Read 4 bytes (an instruction) from the file
			if (!inFile.read((char *)&instruction, 4))
				break; // If 4 bytes cannot be read from the file (i.e. end of file has been reached), break the loop.
			appendLabels(instruction);
		}

		inFile.close();
	}
	else
		cout << "Cannot access input/output file" << endl;

	//Looping to translate instructions

	inFile.open(inFileName.c_str(), ios::in | ios::binary);
	outFile.open(outFileName.c_str(), ios::out);

	if (inFile.is_open() && outFile.is_open())
	{
		
		while (true)
		{
			// Read 4 bytes (an instruction) from the file
			if (!inFile.read((char *)&instruction, 4))
				break; // If 4 bytes cannot be read from the file (i.e. end of file has been reached), break the loop.

			// Decode the instruction read from the file
			decodeInst(instruction);
		}

		inFile.close();
		outFile.close();

		viewFile();
	}
	else
		cout << "Cannot access input/output file" << endl;


	cout << endl;
	system("pause");
	return 0;
}


// This function is used to decode a single instruction
void decodeInst(unsigned int instruction)
{
	/*
		Variables declared:
		- Register numbers (target, destination, source)
		- Op-code, function, shift amount, immediate, signed immediate, address
		- Program counter
		- Temporary op-code and immediate (supports pseudo-instruction translation)
	*/
	
	unsigned int rd = 0, rs = 0, rt = 0, func = 0, shamt = 0, imm = 0, sImm = 0, opcode = 0, address = 0;
	static unsigned int pc = 0x00400000, oTemp = 0, iTemp = 0;

	//Linking current instruction (through PC) to any labels that may have been appended to

	found_label = "";
	if (oTemp == 0)
		pointer = labels.find(pc);
	else
		pointer = labels.find(pc - 4);

	if (pointer != labels.end())
		found_label = pointer->second;

	from_label = "";
	pointer = fromLabels.find(pc);
	if (pointer != fromLabels.end())
		from_label = pointer->second;

	//Translation of instruction follows

	opcode = (instruction >> 26);

	if (0 == opcode)
	{
		//R-Format
		func = instruction & 0x3F;
		shamt = (instruction >> 6) & 0x1f;
		rd = (instruction >> 11) & 0x1f;
		rt = (instruction >> 16) & 0x1f;
		rs = (instruction >> 21) & 0x1f;

		switch (func)
		{
		case 0x20: // ADD
			outFile << "0x" << hex << pc << "\t" << found_label << "\tADD\t$" << dec << reg[rd] << ", $" << reg[rs] << ", $" << reg[rt] << endl;
			regs[rd] = regs[rs] + regs[rt];
			break;

		case 0x21: // ADDU, MOVE
			if (rs == 0)
				outFile << "0x" << hex << pc << "\t" << found_label << "\tMOVE\t$" << dec << reg[rd] << ", $" << reg[rt] << endl;
			else if (rt == 0)
				outFile << "0x" << hex << pc << "\t" << found_label << "\tMOVE\t$" << dec << reg[rd] << ", $" << reg[rs] << endl;
			else
				outFile << "0x" << hex << pc << "\t" << found_label << "\tADDU\t$" << dec << reg[rd] << ", $" << reg[rs] << ", $" << reg[rt] << endl;

			regs[rd] = regs[rs] + regs[rt];
			break;

		case 0x22: //SUB
			if (oTemp == 0x08)
			{
				outFile << "0x" << hex << pc - 4 << "\tSUBI\t$" << dec << reg[rd] << ", $" << reg[rs] << ", " << int(iTemp) << endl;
				oTemp = 0;
			}
			else
				outFile << "0x" << hex << pc << "\t" << found_label << "\tSUB\t$" << dec << reg[rd] << ", $" << reg[rs] << ", $" << reg[rt] << endl;

			regs[rd] = regs[rs] - regs[rt];
			break;

		case 0x24: //AND
			outFile << "0x" << hex << pc << "\t" << found_label << "\tAND\t$" << dec << reg[rd] << ", $" << reg[rs] << ", $" << reg[rt] << endl;
			regs[rd] = regs[rs] & regs[rt];
			break;

		case 0x25: //OR
			outFile << "0x" << hex << pc << "\t" << found_label << "\tOR\t$" << dec << reg[rd] << ", $" << reg[rs] << ", $" << reg[rt] << endl;
			regs[rd] = regs[rs] | regs[rt];
			break;

		case 0x26: //XOR
			outFile << "0x" << hex << pc << "\t" << found_label << "\tXOR\t$" << dec << reg[rd] << ", $" << reg[rs] << ", $" << reg[rt] << endl;
			regs[rd] = regs[rs] ^ regs[rt];
			break;

		case 0x2A: //SLT
			outFile << "0x" << hex << pc << "\t" << found_label << "\tSLT\t$" << dec << reg[rd] << ", $" << reg[rs] << ", $" << reg[rt] << endl;
			if (regs[rs] < regs[rt])
				regs[rd] = 1;
			else
				regs[rd] = 0;
			break;

		case 0x02: //SRL
			outFile << "0x" << hex << pc << "\t" << found_label << "\tSRL\t$" << dec << reg[rd] << ", $" << reg[rt] << ", " << shamt << endl;
			regs[rd] = regs[rt] >> shamt;
			break;

		case 0x00: //SLL
			outFile << "0x" << hex << pc << "\t" << found_label << "\tSLL\t$" << dec << reg[rd] << ", $" << reg[rt] << ", " << shamt << endl;
			regs[rd] = regs[rt] << shamt;
			break;

		case 0x08: //JR
			outFile << "0x" << hex << pc << "\t" << found_label << "\tJR\t$" << dec << reg[rs] << endl;
			pc = regs[rs];
			break;

		case 0x0C: //syscall
			outFile << "0x" << hex << pc << "\t" << found_label << "\tsyscall" << endl;
			break;

		default:
			outFile << "0x" << hex << pc << "\t\tOpcode: " << dec << opcode << " - Unkown R - Instruction" << endl;
		}
	}

	else if (0 != opcode && 2 != opcode && 3 != opcode && 16 != opcode && 17 != opcode && 18 != opcode && 19 != opcode)
	{
		// I-Format
		rt = (instruction >> 16) & 0x1f;
		rs = (instruction >> 21) & 0x1f;
		imm = (instruction & 0x0000FFFF);
		sImm = (imm & 0x8000) ? (0xFFFF0000 | imm) : imm;	// sign extending the immediate field
		address = regs[rs] + sImm;

		switch (opcode)
		{
		case 0x23:	// LW
			outFile << "0x" << hex << pc << "\t" << found_label << "\tLW\t$" << dec << reg[rt] << ", " << int(sImm) << " ($" << reg[rs] << ")" << endl;
			regs[rt] = getFromMemory(address, 4, true);
			break;

		case 0x2B:	// SW
			outFile << "0x" << hex << pc << "\t" << found_label << "\tSW\t$" << dec << reg[rt] << ", " << int(sImm) << " ($" << reg[rs] << ")" << endl;
			storeToMemory(address, 4, regs[rt]);
			break;

		case 0x20:	// LB
			outFile << "0x" << hex << pc << "\t" << found_label << "\tLB\t$" << dec << reg[rt] << ", " << int(sImm) << " ($" << reg[rs] << ")" << endl;
			regs[rt] = getFromMemory(address, 1, true);
			break;

		case 0x28:	// SB
			outFile << "0x" << hex << pc << "\t" << found_label << "\tSB\t$" << dec << reg[rt] << ", " << int(sImm) << " ($" << reg[rs] << ")" << endl;
			storeToMemory(address, 1, regs[rt]);
			break;

		case 0x21:	// LH
			outFile << "0x" << hex << pc << "\t" << found_label << "\tLH\t$" << dec << reg[rt] << ", " << int(sImm) << " ($" << reg[rs] << ")" << endl;
			regs[rt] = getFromMemory(address, 2, true);
			break;

		case 0x29:	// SH
			outFile << "0x" << hex << pc << "\t" << found_label << "\tSH\t$" << dec << reg[rt] << ", " << int(sImm) << " ($" << reg[rs] << ")" << endl;
			storeToMemory(address, 2, regs[rt]);
			break;

		case 0x08:	// ADDI
			if (rt == 1)
			{
				oTemp = 0x08;
				iTemp = sImm;
			}
			else
				outFile << "0x" << hex << pc << "\t" << found_label << "\tADDI\t$" << dec << reg[rt] << ", $" << reg[rs] << ", " << int(sImm) << endl;

			regs[rt] = regs[rs] + sImm;
			break;

		case 0x09:	// ADDIU
			if (rs == 0)
				outFile << "0x" << hex << pc << "\t" << found_label << "\tLI\t$" << dec << reg[rt] << ", " << int(sImm) << endl;
			else
				outFile << "0x" << hex << pc << "\t" << found_label << "\tADDIU\t$" << dec << reg[rt] << ", $" << reg[rs] << ", " << int(sImm) << endl;

			regs[rt] = regs[rs] + imm;
			break;

		case 0x0C:	//ANDI
			outFile << "0x" << hex << pc << "\t" << found_label << "\tANDI\t$" << dec << reg[rt] << ", $" << reg[rs] << ", " << int(sImm) << endl;
			regs[rt] = regs[rs] & imm;
			break;

		case 0x0D:	// ORI
			if (oTemp == 0x0F)
			{
				iTemp = (iTemp | imm);
				outFile << "0x" << hex << pc - 4 << "\t" << found_label << "\tLI/LA\t$" << dec << reg[rt] << ", " << int(iTemp) << endl;
				oTemp = 0;
			}
			else
				outFile << "0x" << hex << pc << "\t" << found_label << "\tORI\t$" << dec << reg[rt] << ", $" << reg[rs] << ", " << int(sImm) << endl;

			regs[rt] = regs[rs] | imm;
			break;

		case 0x0E:	//	XORI
			outFile << "0x" << hex << pc << "\t" << found_label << "\tXORI\t$" << dec << reg[rt] << ", $" << reg[rs] << ", " << int(sImm) << endl;
			regs[rt] = regs[rs] ^ imm;
			break;

		case 0x0A: // SLTI
			outFile << "0x" << hex << pc << "\t" << found_label << "\tSLTI\t$" << dec << reg[rt] << ", $" << reg[rs] << ", " << int(sImm) << endl;
			if (regs[rs] < sImm)
				regs[rt] = 1;
			else
				regs[rt] = 0;
			break;

		case 0x0F: //LUI
			if (rt == 1)
			{
				iTemp = (imm << 16);
				oTemp = 0x0F;
			}
			else outFile << "0x" << hex << pc << "\t" << found_label << "\tLUI\t$" << dec << reg[rt] << ", " << int(sImm) << endl;

			regs[rt] = (imm << 16);
			break;

		case 0x04: //BEQ
			outFile << "0x" << hex << pc << "\t" << found_label << "\tBEQ\t$" << dec << reg[rs] << ", $" << reg[rt] << ", " << from_label << endl;
			break;

		case 0x05: //BNE
			outFile << "0x" << hex << pc << "\t" << found_label << "\tBNE\t$" << dec << reg[rs] << ", $" << reg[rt] << ", " << from_label << endl;
			break;

		default:
			outFile << "0x" << hex << pc << "\t\tOpcode: " << dec << opcode << " - Unkown I - Instruction" << endl;

		}
	}
	else if (opcode == 2 || opcode == 3)
	{
		imm = instruction & 0x03FFFFFF;
		imm = imm << 2;
		address = pc & 0xF0000000;
		address = address | imm;

		if (opcode == 2)
			outFile << "0x" << hex << pc << "\t" << found_label << "\tJ " << from_label << endl;
		else
		{
			outFile << "0x" << hex << pc << "\t" << found_label << "\tJAL " << from_label << endl;
			regs[31] = pc + 4;
		}
	}
	else
	{
		// The opcode is not known
		outFile << "0x" << hex << pc << "\t\tOpcode: " << dec << opcode << " - Unkown Instruction" << endl;
	}


	// Increment the PC by 4
	pc = pc + 4;
}


// Function to return a word/half word/byte from memory
unsigned int getFromMemory(unsigned int address, int num, bool state)
{
	/*
		Variables declared:
		-Address to start to load from
		- Number of consecutive bytes to load
		- Boolean to indicate whether loading signed or unsinged
	*/

	unsigned int fullLoad = 0;
	unsigned int singleLoad;

	for (int i = 0; i < num; i++)
	{
		singleLoad = memory[address - 0x10010000 + i];
		fullLoad = fullLoad | (singleLoad << (i * 8));
	}

	//Sign extending the loading
	if (state)
		fullLoad = (fullLoad & (1 << (num)* 8)) ? (0xFFFFFFFF << (num * 8) | fullLoad) : fullLoad;

	return fullLoad;
}

//Function to write a word/half word/byte to memory
void storeToMemory(unsigned int address, int num, unsigned int x)
{
	/*
		Variables declared:
		- Address to start to load from
		- Number of consecutive bytes to store
		- Register value to store
	*/
	
	for (int i = 0; i < num; i++)
		memory[address - 0x10010000 + i] = 0x000000FF & (x >> (i * 8));
}

//The redundant negation function
unsigned int negate(unsigned int original)
{
	return ((original ^ 0xFFFFFFFF) + 1);
}


//Function that takes a text file and output its content on-screen
void viewFile()
{
	inFile.open(outFileName.c_str(), ios::in);
	while (!(inFile.eof()))		//Validates that it is not the end of the input file
		cout << char(inFile.get());
	inFile.close();
}


//Funciton to append labels to certain instructions
void appendLabels(unsigned int instruction)
{
	/*
		Variables declared:
		- Register number (source)
		- Immediate, signed immediate, op-code
		- Jump/branch address (i.e "to")
		- Program Counter
	*/
	
	static unsigned int pc = 0x00400000;
	unsigned int rs = 0, imm = 0, sImm = 0, opcode = 0, to = 0;
	opcode = (instruction >> 26);

	//In case it is branching (BEQ and BNE)
	if (opcode == 0x04 || opcode == 0x05)
	{
		imm = (instruction & 0x0000FFFF);
		sImm = (imm & 0x8000) ? (0xFFFF0000 | imm) : imm;	// sign extending the immediate field
		to = pc + 4 + (sImm << 2);

		string label = "L" + intToString(labelCount);
		labelCount++;
		labels[to] = label;
		fromLabels[pc] = label;
	}

	//In case it is a jump (J and JAL)
	else if (opcode == 2 || opcode == 3)
	{
		imm = instruction & 0x03FFFFFF;
		imm = imm << 2;
		to = pc & 0xF0000000;
		to = to | imm;

		string label = "L" + intToString(labelCount);
		labelCount++;
		labels[to] = label;
		fromLabels[pc] = label;
	}


	//Increment program counter
	pc = pc + 4;
}


//This function returns a string from a number
string intToString(int number)
{
	stringstream s;
	s << number;
	return s.str();
}
