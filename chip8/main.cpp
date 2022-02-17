#include <iostream>
#include <cstdint>
#include <fstream>
#include <chrono>
#include <random>
#include <cstdlib>
#include <cstring>

const unsigned int START_ADDRESS = 0x200; // Because rom Memory Starts from 0x200 and ends at 0xFFF
const unsigned int FONTSET_START_ADDRESS = 0x50; // Because fontset Memory Starts from 0x50 and ends at 0xA0
const unsigned int FONTSET_SIZE = 80;



class Chip8
{
	
public:
	Chip8();
	uint8_t registers[16]{};
	uint8_t memory[4096];
	uint16_t index{};
	uint16_t pc{};
	uint16_t stack[16]{};
	uint8_t stackPointer{};
	uint8_t delayTimer{};
	uint8_t soundTimer{};
	uint8_t inputKeys[16]{};
	uint32_t graphics[64 * 32]{};
	uint16_t opcode;
	void LoadROM(char const* filename); // not sure
	std::default_random_engine randGen;
	std::uniform_int_distribution<unsigned int> randByte;


	// opcodes
	void OP_00E0(); // CLS             1
	void OP_00EE(); // RTN             2
	void OP_1nnn(); // JP addr         3
	void OP_2nnn(); // CALL addr       4 
	void OP_3xkk(); // SE Vx, byte     5
	void OP_4xkk(); // SNE Vx, byte    6
	void OP_5xy0(); //  SE Vx, Vy      7
	void OP_6xkk(); // LD Vx, byte     8
	void OP_7xkk(); // ADD Vx, byte    9
	void OP_8xy0(); // LD Vx, Vy       10
	void OP_8xy1(); // OR Vx, Vy       11
	void OP_8xy2(); // AND Vx, Vy      12
	void OP_8xy3(); // XOR Vx, Vy      13
	void OP_8xy4(); // ADD Vx, Vy      14
	void OP_8xy5(); // SUB Vx, Vy      15
	void OP_8xy6(); // SHR Vx, Vy      16
	void OP_8xy7(); // SUBN Vx, Vy     17

};




uint8_t fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


Chip8::Chip8() 
	: randGen(std::chrono::system_clock::now().time_since_epoch().count())
{

	pc = START_ADDRESS;
	for (unsigned int i = 0; i < FONTSET_SIZE; i++)
	{
		memory[FONTSET_START_ADDRESS + i] = fontset[i];

	}

	// Initialize RNG
	randByte = std::uniform_int_distribution<unsigned int>(0, 255u);

}


void Chip8::LoadROM(char const* filename)
{
	//open file and stream it as binary and move the file pointer to the end
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file.is_open())
	{
		//get the size of file and allocate buffer  to hold its content
		std::streampos size = file.tellg();
		char* buffer = new char[size];

		//get back to the beginning of file and fill the buffer
		file.seekg(0, std::ios::beg);
		file.read(buffer, size);
		file.close();

		// load the rom content in memory starting from 0x200
		for (long i = 0; size > i; i++)
		{
			memory[START_ADDRESS + i] = buffer[i];
		}


		delete[] buffer;
	}
}





void Chip8::OP_00E0() 
{
	memset(graphics, 0, sizeof(graphics));
}

void Chip8::OP_00EE()
{
	--stackPointer;
	pc = stack[stackPointer];
}

void Chip8::OP_1nnn() 
{
	uint16_t address = opcode & 0x0FFF;
	pc = address;
}

void Chip8::OP_2nnn() 
{
	uint16_t address = opcode & 0x0FFF;

	stack[stackPointer] = pc;
	++stackPointer;
	pc = address;
}

void Chip8::OP_3xkk()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t byte = (opcode & 0x0F00);

	if (registers[Vx] == byte)
	{
		pc += 2;
	}
}

void Chip8::OP_4xkk() 
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t byte = opcode & 0x00FF;

	if (registers[Vx] != byte)
	{
		pc += 2;
	}
}

void Chip8::OP_5xy0() 
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x0F00) >> 4u;

	if (Vx == Vy) 
	{
		pc += 2;
	}
}

void Chip8::OP_6xkk() 
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t byte = opcode & 0x00FF;
	registers[Vx] = byte;
}

void Chip8::OP_7xkk() 
{

	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t byte = opcode & 0x00FF;

	registers[Vx] += byte;
}

void Chip8::OP_8xy0() 
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x0F00) >> 4u;

	registers[Vx] += registers[Vy];
}

void Chip8::OP_8xy1() 
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x00F0) >> 4u;


	registers[Vx] |= registers[Vy];
}


void Chip8::OP_8xy2() 
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x00F0) >> 4u;


	registers[Vx] &= registers[Vy];
}

void Chip8::OP_8xy3()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x00F0) >> 4u;


	registers[Vx] ^= registers[Vy];
}



void Chip8::OP_8xy4()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x00F0) >> 4u;

	uint16_t VxWithVy = registers[Vx] + registers[Vy];
	if (VxWithVy > 0xFF) 
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[Vx] = VxWithVy & 0xFFu;

}


void Chip8::OP_8xy5()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x00F0) >> 4u;

	if (registers[Vx] > registers[Vy]) 
	{
		registers[0xF] = 1;
	}
	else 
	{
		registers[0xF] = 0;
	}

	registers[Vx] -= registers[Vy];

}


void Chip8::OP_8xy6()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;

	if (registers[Vx] & 0x1u == 1)
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	
	registers[Vx] /= 2;
}


int main() 
{
}