#include <iostream>
#include <bitset>
#include <stdio.h>
#include<stdlib.h>
#include <string>
using namespace std;

const int OPCODE_LENGTH = 7;
const int REG_LENGTH = 5;
const int FUNCT3_LENGTH = 3;
const int FUNCT7_LENGTH = 7;
const int IMMEDIATE_LENGTH = REG_LENGTH;

const int ALU_OPS_ADD = 0b010; 
const int ALU_OPS_SUB = 0b001;
const int ALU_OPS_AND = 0b000;
const int ALU_OPS_OR  = 0b011;
const int ALU_OPS_XOR = 0b100;
const int ALU_OPS_SHIFT_RIGHT = 0b101;
const int ALU_OPS_BLT = 0b110;


class instruction {	
public:
	bitset<32> instr;//instruction
	instruction(bitset<32> fetch); // constructor

};

class ControlSignals {


public:
    bitset<1> isBranching;
    bitset<1> readMem;
    bitset<1> isMemToReg;
    bitset<3> ALUOperation;
    bitset<1> writeMem;
    bitset<1> isALUSource;
    bitset<1> writeReg;
    int ALUControlSignal;
	ControlSignals() :
	isBranching(0), readMem(0), isMemToReg(0),
	ALUOperation(0), writeMem(0), isALUSource(0),
	writeReg(0), ALUControlSignal(0) { }

    // Getter and Setter methods for each control signal can be added here
    void setBranching(bitset<1> value) { isBranching = value; }
    bitset<1> getBranching() const { return isBranching; }

    // Getter and Setter methods for each control signal can be added here

    // ... Similarly for other signals ...
};

class CPU {
public:
    enum InstrType {
        R_TYPE = 0b0110011,
        I_TYPE = 0b0010011,
        LW = 0b0000011,
        JALR = 0b1100111,
        S_TYPE = 0b0100011,
        B_TYPE = 0b1100011,
        TERMINATE = 0b0000000
    };

    enum InstrOps {
        INVALID,
        ADD_OP, SUB_OP, XOR_OP, SRA_OP,
        ANDI_OP, ADDI_OP, LOAD_OP,
        JALR_OP, STORE_OP, BLT_OP, END_OP
    };
	CPU();
	unsigned long readPC();
	bitset<32> Fetch(bitset<8> *instmem);
	bool Decode(instruction* instr);
    bool execute(int op1, int aluOp);
	int32_t getRegisterDestination() const { return registerDestination; }
    int32_t getData2() const { return data2; }
	int32_t getData1() const { return data1; }
	int getALUOperation() const { return static_cast<int>(controlSignals.ALUOperation.to_ulong()); }
    bool Memory(int alu_output, int reg2);
	int32_t getALUResult() const { return aluResult; }
	int32_t getMemoryOutput() const { return memoryOutput; }
    void setALUResult(int32_t value) { aluResult = value; }
    bool WriteBack(int ALUResult, int memoryOutput, int rd);
    int getRTypeInstructionCount();
	int getImmediateValue() const { return immediateValue; }
	bitset<1> getNegativeALUFlag() const { return negativeALU; }
	void updatePC(int immediateValue, bitset<1> aluNeg);
    int32_t accessRegister(int index);
private:
	int dmemory[4096]; //data memory byte addressable in little endian fashion;
	unsigned long PC; //pc 
	unsigned long prevPC;
    int32_t registers[32];
    int32_t data1, data2;
    int registerDestination, immediateValue, memoryOutput;
    int rTypeInstructionCount = 0;
	int previousPC;
    int32_t aluResult;
    bitset<1> negativeALU;
	InstrOps currentOperation;

    ControlSignals controlSignals;

};

// add other functions and objects here
