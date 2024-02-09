#include "CPU.h"

instruction::instruction(bitset<32> fetch)
{
	// cout << fetch << endl;
	instr = fetch;
	// cout << instr << endl;
}

CPU::CPU()
{
	//cout << "herer in the CPU() constructor" << endl;
	PC = 0; //set PC to 0
	for (int i = 0; i < 4096; i++) //copy instrMEM
	{
		dmemory[i] = (0);
	}
}

bitset<32> CPU::Fetch(bitset<8> *instmem) {
	bitset<32> instr = ((((instmem[PC + 3].to_ulong()) << 24)) + ((instmem[PC + 2].to_ulong()) << 16) + ((instmem[PC + 1].to_ulong()) << 8) + (instmem[PC + 0].to_ulong()));  //get 32 bit instruction
	PC += 4;//increment PC
	//cout<<"what is the current PC"<<PC<<endl;
	return instr;
}


bool CPU::Decode(instruction* curr)
{
	//cout << "here in the Decode" << endl;
    InstrType opcode = (InstrType)(curr->instr.to_ulong() & 0x7F);
	//cout << "opcode in the Decode is " << opcode << endl;
	//cout << opcode << endl;
    switch(opcode)
    {
        case R_TYPE:
		{
			bitset<32> instrBits = curr->instr;
			//cout<<"now go into the R_type"<<endl;
			//cout << "instrBits in R-type is  " << instrBits << endl;
			// Extracting fields directly
			bitset<REG_LENGTH> rd((instrBits.to_ulong() >> 7) & 0x1F);
			//cout<<"rd in R_type"<<rd<<endl;
			bitset<FUNCT3_LENGTH> funct3((instrBits.to_ulong() >> 12) & 0x7);
			//cout<<"funct3 in R_type"<<funct3<<endl;
			bitset<REG_LENGTH> rs1((instrBits.to_ulong() >> 15) & 0x1F);
			//cout<<"rs1 in R_type"<<rs1<<endl;
			bitset<REG_LENGTH> rs2((instrBits.to_ulong() >> 20) & 0x1F);
			//cout<<"rs2 in R_type"<<rs2<<endl;
			bitset<FUNCT7_LENGTH> funct7((instrBits.to_ulong() >> 25) & 0x7F);
			//cout<<"rd in funct7"<<funct7<<endl;

			// Read source registers
			data1 = registers[rs1.to_ulong()];
			data2 = registers[rs2.to_ulong()];
			//cout<<"what is data1 now in R_type" <<data1<<endl;
			//cout<<"what is data2 now in R_type" <<data2<<endl;
			// Determine destination register
			registerDestination = rd.to_ulong();
			//cout<<"what rd in int now" <<registerDestination<<endl;
			// Set control signals for R-type instructions
			controlSignals.isBranching = 0;
			controlSignals.readMem = 0;
			controlSignals.isMemToReg = 0;
			controlSignals.writeMem = 0;
			controlSignals.isALUSource = 0;
			controlSignals.writeReg = 1;

			// Determine the exact ALU operation based on funct3 and funct7
			if (funct3 == 0b000) {
				if (funct7 == 0b0000000) {
					controlSignals.ALUOperation = ALU_OPS_ADD;
					currentOperation = ADD_OP;
				} else if (funct7 == 0b0100000) {
					controlSignals.ALUOperation = ALU_OPS_SUB;
					currentOperation = SUB_OP;
				}
			} else if (funct3 == 0b100 && funct7 == 0b0000000) {
				controlSignals.ALUOperation = ALU_OPS_XOR;
				currentOperation = XOR_OP;
			} else if (funct3 == 0b101 && funct7 == 0b0100000) {
				controlSignals.ALUOperation = ALU_OPS_SHIFT_RIGHT;
				currentOperation = SRA_OP;
			}
			// ... Add more cases for other R-type operations based on funct3 and funct7 ...
			//cout<<"what is data2 now in R_type" <<data2<<endl;
			//cout<<"what is the ALUOperation now"<<controlSignals.ALUOperation<<endl;
			rTypeInstructionCount++;
			break;
		}
			// ... Other R-type operations can be decoded similarly using funct3 and funct7
        
		case I_TYPE:
		{
			//cout << "now get into I_Type in decode statge" << endl;
			bitset<32> instrBits = curr->instr; // Extract the instruction bits
			//cout << "instrBits in I-type is  " << instrBits << endl;
			// Extracting fields directly
			bitset<FUNCT3_LENGTH> funct3((instrBits.to_ulong() >> 12) & 0x7);
			//cout << "funct3 is now " << funct3 << endl;
			bitset<REG_LENGTH> rs1((instrBits.to_ulong() >> 15) & 0x1F);
			//cout << "rs1 is " << rs1 << endl;
			bitset<REG_LENGTH> rd((instrBits.to_ulong() >> 7) & 0x1F);
			//cout << "rd is " << rd << endl;
			bitset<12> immediate(instrBits.to_ulong()>>20 & 0xFFF); // Mask the lowest 12 bits for immediate value
			//cout << "immediate is " << immediate << endl;
			// Reading the value of the source register
			data1 = registers[rs1.to_ulong()];
			//cout << "data1 is " << data1 << endl;
			// Converting the bitset to an integer value
			immediateValue = static_cast<int>(immediate.to_ulong());
			//cout << "immediateValue is now " << immediateValue << endl;

			// Adjusting for two's complement if the immediate is negative
			if (immediate[11] == 1) // Check if the MSB of the immediate value is 1
			{
				// cout << "in...." << endl;
				immediateValue = immediateValue | 0xFFFFF000; // Extend the sign bit
			}

			// Destination register
			registerDestination = rd.to_ulong();
			//cout << "registerDestination is now " << registerDestination << endl;
			// Set the common control signals for I-type instructions
			controlSignals.isBranching = 0;
			controlSignals.readMem = 0;
			controlSignals.isMemToReg = 0;
			controlSignals.writeMem = 0;
			controlSignals.isALUSource = 1; // For I-type, second ALU operand is an immediate value
			controlSignals.writeReg = 1;

			// Decoding the exact operation
			if (funct3 == 0b000) {
				// This is ADDI
				controlSignals.ALUOperation = ALU_OPS_ADD;
				
				currentOperation = ADDI_OP;
			} else if (funct3 == 0b111) {
				// This is ANDI
				controlSignals.ALUOperation = ALU_OPS_AND;
				currentOperation = ANDI_OP;
			}
			// ... Add more cases for other I-type operations based on funct3 ...
			//cout<<"This is ALUOperation"<<controlSignals.ALUOperation<<endl;
			break;
		}
        
        
		case LW:
		{
			//cout<<"go into 'LW_type'"<<endl;
			bitset<32> instrBits = curr->instr; // Extract the instruction bits
			//cout<<"the Lw_type instrBits" <<instrBits << endl;
			// Extracting fields directly
			bitset<FUNCT3_LENGTH> funct3((instrBits.to_ulong() >> 12) & 0x7);
			//cout<<" the Lw_type funct3" <<funct3 << endl;
			bitset<REG_LENGTH> rs1((instrBits.to_ulong() >> 15) & 0x1F);
			//cout<<"the Lw_type rs1" <<rs1 << endl;
			bitset<REG_LENGTH> rd((instrBits.to_ulong() >> 7) & 0x1F);
			//cout<<"the Lw_type rd" <<rd << endl;
			bitset<12> immediate(instrBits.to_ulong()>>20 & 0xFFF); // Mask the lowest 12 bits for immediate value
			//cout<<"the Lw_type immediate" <<immediate << endl;
			// Reading the value of the source register which will be used as a base address
			data1 = registers[rs1.to_ulong()];
			//cout<<"the Lw_type data1" <<data1 << endl;
			// Converting the bitset to an integer value
			immediateValue = static_cast<int>(immediate.to_ulong());
			//cout<<"the Lw_type immediateValue" <<immediateValue << endl;
			// Adjusting for two's complement if the immediate is negative
			if (immediate[11] == 1) // Check if the MSB of the immediate value is 1
			{
				immediateValue = immediateValue | 0xFFFFF000; // Extend the sign bit
			}

			// Destination register where the loaded word will be stored
			registerDestination = rd.to_ulong();

			// Set the control signals for LW instruction
			controlSignals.isBranching = 0;
			controlSignals.readMem = 1;        // We will read memory for loading
			controlSignals.isMemToReg = 1;     // Data from memory will go to register
			controlSignals.writeMem = 0;
			controlSignals.isALUSource = 1;    // For LW, second ALU operand is an immediate value
			controlSignals.writeReg = 1;

			// Ensure the funct3 is correct for LW
			if (funct3 != 0b010) {
				// Error handling for unexpected funct3 value
				cerr << "Error: unexpected funct3 value for LW instruction." << endl;
				return false;
			}
			controlSignals.ALUOperation = ALU_OPS_ADD;
			currentOperation = LOAD_OP; // Setting the operation to LOAD_OP

			break;
		}
        
		case JALR:
		{
			//cout<<"go into JALR type"<<endl;
			bitset<32> instrBits = curr->instr; // Extract the instruction bits
			//cout<<"JALR type instrBits"<<instrBits<<endl;	
			// Extracting fields directly
			bitset<FUNCT3_LENGTH> funct3((instrBits.to_ulong() >> 12) & 0x7);
			//cout<<"JALR type funct3"<<funct3<<endl;	
			bitset<REG_LENGTH> rs1((instrBits.to_ulong() >> 15) & 0x1F);
			//cout<<"JALR type rs1"<<rs1<<endl;	
			bitset<REG_LENGTH> rd((instrBits.to_ulong() >> 7) & 0x1F);
			//cout<<"JALR type rd"<<rd<<endl;	

			bitset<12> immediate(instrBits.to_ulong()>> 20& 0xFFF); // Mask the lowest 12 bits for immediate value
			//cout<<"JALR type immediate"<<immediate<<endl;	
			// Reading the value of the source register which will be used as a base for the jump
			data1 = registers[rs1.to_ulong()];
			//cout<<"JALR type data1 "<<data1<<endl;	
			// Converting the bitset to an integer value for the immediate offset
			immediateValue = static_cast<int>(immediate.to_ulong());
			//cout<<"JALR type immediateValue "<<immediateValue<<endl;	
			// Adjusting for two's complement if the immediate is negative
			if (immediate[11] == 1) // Check if the MSB of the immediate value is 1
			{
				immediateValue = immediateValue | 0xFFFFF000; // Extend the sign bit
			}

			// The destination register where the return address will be stored
			registerDestination = rd.to_ulong();

			// Set the control signals for JALR instruction
			controlSignals.isBranching = 1;   // We will perform a jump
			controlSignals.readMem = 0;
			controlSignals.isMemToReg = 0;
			controlSignals.writeMem = 0;
			controlSignals.isALUSource = 1;    // The ALU will compute the target address
			controlSignals.writeReg = 1;       // The return address will be written to a register

			// Ensure the funct3 is correct for JALR
			if (funct3 != 0b000) {
				// Error handling for unexpected funct3 value
				cerr << "Error: unexpected funct3 value for JALR instruction." << endl;
				return false;
			}

			// The ALU will calculate the effective jump address: base address (`rs1`) + immediate value
			// The PC will then be set to this value in a later stage (e.g., during the execute or memory access stage)
			// The return address (PC + 4) will be placed in the destination register (`rd`)
			controlSignals.ALUOperation = ALU_OPS_ADD;
			currentOperation = JALR_OP; // Setting the operation to JALR_OP
			break;
		}
        
		case S_TYPE:
		{
			//cout<<"Go into the S_type"<<endl;
			bitset<32> instrBits = curr->instr;
            //cout<<"S_type instruction"<<instrBits<<endl;
			// Extract the different fields from the instruction
			bitset<FUNCT3_LENGTH> funct3((instrBits.to_ulong() >> 12) & 0x7);
			//cout<<"S_type funct3"<<funct3<<endl;
			bitset<REG_LENGTH> rs1((instrBits.to_ulong() >> 15) & 0x1F);
			//cout<<"S_type rs1"<<rs1<<endl;
			bitset<REG_LENGTH> rs2((instrBits.to_ulong() >> 20) & 0x1F);
			//cout<<"S_type rs2"<<rs2<<endl;

			// Extract and reconstruct the 12-bit immediate for S-type instructions
			bitset<7> immUpper((instrBits.to_ulong() >> 25) & 0x7F);
			bitset<5> immLower((instrBits.to_ulong() >> 7) & 0x1F);
			bitset<12> immediate((immUpper.to_ulong() << 5) | immLower.to_ulong());
			//cout<<"S_type immediate"<<immediate<<endl;
			// Check for negative immediate and sign-extend if needed
			if (immediate[11] == 1) // MSB is the sign bit
			{
				immediateValue = immediate.to_ulong() | 0xFFFFF000; // Sign-extend
			}
			else
			{
				immediateValue = immediate.to_ulong();
			}

			// Read the source registers
			data1 = registers[rs1.to_ulong()];  // Base address
			data2 = registers[rs2.to_ulong()];  // Data to be stored
			//cout<<"S_type data1"<<data1<<endl;
			//cout<<"S_type data2"<<data2<<endl;
			// Control signals for store operations
			controlSignals.isBranching = 0;
			controlSignals.readMem = 0;
			controlSignals.isMemToReg = 0;
			controlSignals.writeMem = 1;    // For 'sw', we need to write to memory
			controlSignals.isALUSource = 1; // The ALU source will be immediate for calculating effective address
			controlSignals.writeReg = 0;    // No writing to registers for 'sw'

			// Decoding the exact operation based on funct3
			if (funct3 == 0b010) {
				// This is 'sw'
				currentOperation = STORE_OP;
				controlSignals.ALUOperation = ALU_OPS_ADD; // Effective address is calculated as rs1 + immediate
			}
			// ... potentially other S-type instructions ...

			break;
		}
        
		case B_TYPE:
		{
			//cout<<"go into the B_type"<<endl;
			bitset<32> instrBits = curr->instr; // Extract the instruction bits
			//cout<<"B_type instruction"<<instrBits<<endl;
			// Extracting fields directly
			bitset<FUNCT3_LENGTH> funct3((instrBits.to_ulong() >> 12) & 0x7);
			//cout<<"B_type funct3"<<funct3<<endl;
			bitset<REG_LENGTH> rs1((instrBits.to_ulong() >> 15) & 0x1F);
			//cout<<"B_type rs1"<<rs1<<endl;
			bitset<REG_LENGTH> rs2((instrBits.to_ulong() >> 20) & 0x1F);
			//cout<<"B_type rs2"<<rs2<<endl;

			// Forming the 12-bit immediate value for branches, bits are scattered in B-type format
			
			bitset<13> immediate;
			immediate[11] = instrBits[7];
			immediate[12] = instrBits[31];
			immediate[10] = instrBits[30];
			immediate[9] = instrBits[29];
			immediate[8] = instrBits[28];
			immediate[7] = instrBits[27];
			immediate[6] = instrBits[26];
			immediate[5] = instrBits[25];
			immediate[4] = instrBits[11];
			immediate[3] = instrBits[10];
			immediate[2] = instrBits[9];
			immediate[1] = instrBits[8];
			immediate[0] = 0;
			//cout<<"B_type immediate "<<immediate<<endl;
			immediateValue = static_cast<int>(immediate.to_ulong());
			//cout<<"B_type immediate_value "<<immediateValue<<endl;
			// Adjusting for two's complement if the immediate is negative
			if (immediate[11] == 1) // Check if the MSB of the immediate value is 1
			{
				immediateValue = immediateValue | 0xFFFFF000; // Extend the sign bit
			}

			// Reading the values of the source registers
			data1 = registers[rs1.to_ulong()];
			data2 = registers[rs2.to_ulong()];
			//cout<<"B_type data1 "<<data1<<endl;
			//cout<<"B_type data2 "<<data2<<endl;
			// Set common control signals for B-type instructions
			controlSignals.isBranching = 1;
			controlSignals.readMem = 0;
			controlSignals.isMemToReg = 0;
			controlSignals.writeMem = 0;
			controlSignals.isALUSource = 0;    // For branches, both ALU operands are from registers
			controlSignals.writeReg = 0;

			// Now, based on funct3, we decode the exact branch operation.
			if (funct3 == 0b100) {
				controlSignals.ALUOperation = ALU_OPS_SUB;
				currentOperation = BLT_OP; // Setting the operation to BLT_OP
			} else {
				cerr << "Error: unexpected funct3 value for B_TYPE instruction." << endl;
				return false;
			}
			//cout<<"B_type ALuOperation "<<controlSignals.ALUOperation <<endl;
			break;
		}
        
        case TERMINATE:
        {
            // Decode logic for EXIT instruction
            break;
        }
        
        default:
            // Invalid or unknown opcode
            return false;
    }
    
    return true;
}
bool CPU::execute(int op1, int ALUOperation) {
    int op2;
	//cout<<"This is ALU operation:"<<ALUOperation<<endl;
    // Decide on the second operand to ALU based on isALUSource control signal
    if (controlSignals.isALUSource.to_ulong()) {
        op2 = immediateValue; // Use the immediate value for operations like ADDI, SW etc.
    } else {
        op2 = data2; // Use the data2 (typically from a second source register) for operations like ADD, SUB etc.
    }
	//cout << "what is op1 "<<op1<<"op2"<<op2<<endl;
    // Perform ALU operation based on the provided ALUOperation
    switch (ALUOperation) {
        case ALU_OPS_ADD:
			//cout << "go into the add operation"<<endl;
            aluResult = op1 + op2;
            break;
        case ALU_OPS_SUB:
			//cout << "go into the sub operation"<<endl;
            aluResult = op1 - op2;
            break;
        case ALU_OPS_AND:
            aluResult = op1 & op2;
            break;
        case ALU_OPS_OR:
            aluResult = op1 | op2;
            break;
		// case ALU_OPS_BLT:
		// 	cout << "Handling BLT comparison" << endl;
		// 	aluResult = op2 - op1;
        // break;
        case ALU_OPS_XOR:
            aluResult = op1 ^ op2;
            break;
        case ALU_OPS_SHIFT_RIGHT:
            aluResult = op1 >> op2;
            break;
        // ... Add other ALU operations as needed
        default:
            // Handle invalid ALU operations or operations not implemented
            cerr << "Invalid or unimplemented ALU operation!" << endl;
            return false;
    }

    // Set the negative flag if the ALU result is negative (can be useful for branch decisions later)
    negativeALU = (aluResult < 0) ? 1 : 0;
	//cout<<"what is the ALUresult" << aluResult<<endl;
	//cout<<"what is nega value "<<negativeALU<<endl;
	updatePC(immediateValue,negativeALU);
    return true; // Indicating the execution was successful
}
bool CPU::Memory(int alu_result, int data_to_write) {
    const size_t MEMORY_SIZE = sizeof(dmemory)/sizeof(dmemory[0]); // Assuming dmemory is an array
    if (controlSignals.writeMem.to_ulong()) {  // Handle Store operations
		//cout<<"go into writeMEm"<<endl;
        if (alu_result < 0 || alu_result >= MEMORY_SIZE) {
            cerr << "Memory access out of bounds!" << endl;
            return false;
        }
		//cout<<"data to write"<< data_to_write << endl;
        dmemory[alu_result] = data_to_write;
		//cout<<"what is the store memory location and value: "<<dmemory[alu_result]<<endl;
        #if defined(LOG)
        cout << "Memory Write (SW): Address[" << alu_result << "] = " << data_to_write << endl;
        #endif
    } else if (controlSignals.readMem.to_ulong()) {  // Handle Load operations
        if (alu_result < 0 || alu_result >= MEMORY_SIZE) {
            cerr << "Memory access out of bounds!" << endl;
            return false;
        }
        memoryOutput = dmemory[alu_result];
		//cout<<"what is memory Output"<<memoryOutput<<endl;
        #if defined(LOG)
        cout << "Memory Read (LW): Address[" << alu_result << "] = " << memoryOutput << endl;
        #endif
    } else {
        #if defined(LOG)
        cout << "Memory Stage: No operation" << endl;
        #endif
    }
    return true;
}
bool CPU::WriteBack(int alu_result, int memory_data, int destination_register) {
	//cout<<"go in to the write Back"<<endl;
    if (controlSignals.isMemToReg.to_ulong()&&controlSignals.writeReg.to_ulong()) {
        // If the instruction is of type that needs to write data from memory to register
        registers[destination_register] = memory_data;
		//cout<<"whta is the r[d] store from memory finally"<<registers[destination_register]<<endl;
        #if defined(LOG)
        cout << "WriteBack: Register[" << destination_register << "] = " << memory_data << " (from memory)" << endl;
        #endif
    } else if (controlSignals.writeReg.to_ulong()) {
        // If the instruction is of type that needs to write ALU result to register
		if(currentOperation == JALR_OP)
		{
			registers[destination_register]=previousPC;
			//cout<<"whta is the r[d] store finally"<<registers[destination_register]<<endl;
		}
		else
		{
			registers[destination_register] = alu_result;
			//cout<<"whta is the r[d] store finally "<<registers[destination_register]<<endl;
		}
        #if defined(LOG)
        cout << "WriteBack: Register[" << destination_register << "] = " << alu_result << " (from ALU)" << endl;
        #endif
    } else {
        #if defined(LOG)
        cout << "WriteBack: No operation" << endl;
        #endif
    }

    return true;
}
int CPU::getRTypeInstructionCount() {
    return rTypeInstructionCount;
}
int32_t CPU::accessRegister(int index) {
    if (index < 0 || index >= 32) {
        throw std::out_of_range("Invalid register index");
    }
    return registers[index];
}
void CPU::updatePC(int immediateValue, bitset<1> negativeALU) {
	 
    if (controlSignals.isBranching.to_ulong()) {
        if (currentOperation == BLT_OP && negativeALU.to_ulong()) {
			
            PC = PC - 4 + immediateValue;  // Adjust for already incremented PC
        } else if (currentOperation == JALR_OP) {
            previousPC = PC;  // Save the return address
            PC = aluResult;  // Adjust for already incremented PC
        } 
        // Add any other branch or jump conditions if necessary
    } 
    
    //cout << "PC after update " << PC << endl;
}
unsigned long CPU::readPC()
{
	return PC;
}
