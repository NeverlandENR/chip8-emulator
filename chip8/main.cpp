#include <iostream>
#include <cstdint>
#include <fstream>
#include <chrono>
#include <random>
#include <cstdlib>
#include <cstring>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>


const unsigned int START_ADDRESS = 0x200; // Because rom Memory Starts from 0x200 and ends at 0xFFF
const unsigned int FONTSET_START_ADDRESS = 0x50; // Because fontset Memory Starts from 0x50 and ends at 0xA0
const unsigned int FONTSET_SIZE = 80;
const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;


class Chip8
{

	
	
public:
	Chip8()
		: randGen(std::chrono::system_clock::now().time_since_epoch().count())
	{

		pc = START_ADDRESS;
		for (unsigned int i = 0; i < FONTSET_SIZE; i++)
		{
			memory[FONTSET_START_ADDRESS + i] = fontset[i];

		}

		// Initialize RNG
		randByte = std::uniform_int_distribution<unsigned int>(0, 255u);


		table[0x0] = &Chip8::Table0;
		table[0x1] = &Chip8::OP_1nnn;
		table[0X2] = &Chip8::OP_2nnn;
		table[0X3] = &Chip8::OP_3xkk;
		table[0X4] = &Chip8::OP_4xkk;
		table[0X5] = &Chip8::OP_5xy0;
		table[0X6] = &Chip8::OP_6xkk;
		table[0x7] = &Chip8::OP_7xkk;
		table[0x8] = &Chip8::Table8;
		table[0x9] = &Chip8::OP_9xy0;
		table[0xA] = &Chip8::OP_Annn;
		table[0xB] = &Chip8::OP_Bnnn;
		table[0xC] = &Chip8::OP_Cxkk;
		table[0xD] = &Chip8::OP_Dxyn;
		table[0xE] = &Chip8::TableE;
		table[0xF] = &Chip8::TableF;

		table0[0x0] = &Chip8::OP_00E0;
		table0[0xE] = &Chip8::OP_00EE;

		table8[0x0] = &Chip8::OP_8xy0;
		table8[0x1] = &Chip8::OP_8xy1;
		table8[0x2] = &Chip8::OP_8xy2;
		table8[0x3] = &Chip8::OP_8xy3;
		table8[0x4] = &Chip8::OP_8xy4;
		table8[0x5] = &Chip8::OP_8xy5;
		table8[0x6] = &Chip8::OP_8xy6;
		table8[0x7] = &Chip8::OP_8xy7;
		table8[0xE] = &Chip8::OP_8xyE;

		tableE[0x1] = &Chip8::OP_ExA1;
		tableE[0xE] = &Chip8::OP_Ex9E;

		tableF[0x07] = &Chip8::OP_Fx07;
		tableF[0x0A] = &Chip8::OP_Fx0A;
		tableF[0x15] = &Chip8::OP_Fx15;
		tableF[0x18] = &Chip8::OP_Fx18;
		tableF[0x1E] = &Chip8::OP_Fx1E;
		tableF[0x29] = &Chip8::OP_Fx29;
		tableF[0x33] = &Chip8::OP_Fx33;
		tableF[0x55] = &Chip8::OP_Fx55;
		tableF[0x65] = &Chip8::OP_Fx65;

	}

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

	void LoadROM(char const* filename)
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


	uint8_t registers[16]{};
	uint8_t memory[4096];
	uint16_t index{};
	uint16_t pc{};
	uint16_t stack[16]{};
	uint8_t stackPointer{};
	uint8_t delayTimer{};
	uint8_t soundTimer{};
	uint8_t inputKeys[16]{};
	uint32_t graphics[VIDEO_WIDTH * VIDEO_HEIGHT]{};
	uint16_t opcode;

	void Cycle();
	

	std::default_random_engine randGen;
	std::uniform_int_distribution<unsigned int> randByte;


	void Table0()
	{
		((*this).*(table0[opcode & 0x000Fu]))();
	}
	void Table8()
	{
		((*this).*(table8[opcode & 0x000Fu]))();
	}

	void TableE()
	{
		((*this).*(tableE[opcode & 0x000Fu]))();
	}

	void TableF()
	{
		((*this).*(tableF[opcode & 0x00FFu]))();
	}
	void OP_NULL() {}


	typedef void (Chip8::* Chip8Func)();
	Chip8Func table[0xF + 1]{ &Chip8::OP_NULL };
	Chip8Func table0[0xE + 1]{ &Chip8::OP_NULL };
	Chip8Func table8[0xE + 1]{ &Chip8::OP_NULL };
	Chip8Func tableE[0xE + 1]{ &Chip8::OP_NULL };
	Chip8Func tableF[0x65 + 1]{ &Chip8::OP_NULL };


	// opcodes
	void OP_00E0(); // CLS                1
	void OP_00EE(); // RTN                2
	void OP_1nnn(); // JP addr		      3
	void OP_2nnn(); // CALL addr		  4 
	void OP_3xkk(); // SE Vx, byte		  5
	void OP_4xkk(); // SNE Vx, byte		  6
	void OP_5xy0(); //  SE Vx, Vy         7
	void OP_6xkk(); // LD Vx, byte        8
	void OP_7xkk(); // ADD Vx, byte       9
	void OP_8xy0(); // LD Vx, Vy          10
	void OP_8xy1(); // OR Vx, Vy          11
	void OP_8xy2(); // AND Vx, Vy         12
	void OP_8xy3(); // XOR Vx, Vy         13
	void OP_8xy4(); // ADD Vx, Vy         14
	void OP_8xy5(); // SUB Vx, Vy         15
	void OP_8xy6(); // SHR Vx {, Vy}      16
	void OP_8xy7(); // SUBN Vx, Vy        17
	void OP_8xyE(); // SHL Vx {, Vy}      18
	void OP_9xy0(); // SNE Vx, Vy         19
	void OP_Annn(); // LD I, addr         20
	void OP_Bnnn(); // JP V0, addr        21
	void OP_Cxkk(); // RND Vx, byte       22
	void OP_Dxyn(); // DRW Vx, Vy, nibble 23
	void OP_Ex9E(); // SKP Vx             24
	void OP_ExA1(); // SKNP Vx            25
	void OP_Fx07(); // LD Vx, DT          26
	void OP_Fx0A(); // LD Vx, K           27
	void OP_Fx15(); // LD DT, Vx          28
	void OP_Fx1E(); // ADD I, Vx          29
	void OP_Fx18(); // LD ST, Vx          30
	void OP_Fx29(); // LD F, Vx           31
	void OP_Fx33(); // LD B, Vx           32
	void OP_Fx55(); // LD [I], Vx         33
	void OP_Fx65(); // LD Vx, [I]         34


};















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

	if ((registers[Vx] & 0x1u) == 1)
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	
	registers[Vx] /= 2;
}


void Chip8::OP_8xy7()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x00F0) >> 4u;

	if (registers[Vy] > registers[Vx])
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}


	registers[Vx] -= registers[Vy];
}


void Chip8::OP_8xyE()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;

	if ((registers[Vx] & 0x8u) == 1)
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}


	registers[Vx] *= 2;
}

void Chip8::OP_9xy0()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x00F0) >> 4u;

	if (registers[Vx] != registers[Vy])
	{
		pc += 2;
	}

}

void Chip8::OP_Annn() 
{
	uint16_t address = opcode & 0xFFF;
	index = address;
}


void Chip8::OP_Bnnn() 
{
	uint16_t address = opcode & 0xFFF;

	pc = address + registers[0];
}

void Chip8::OP_Cxkk() 
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t byte = opcode & 0x00FF;
	

	registers[Vx] = randByte(randGen) & byte;


}

void Chip8::OP_Dxyn() 
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t Vy = (opcode & 0x00F0) >> 4u;
	uint8_t height = opcode & 0x000Fu ;



	uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
	uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;
	


	registers[0x0F] = 0;

	for (unsigned int row = 0; row < height; ++row)
	{
		uint8_t spriteByte = memory[index + row];

		for (unsigned int col = 0; col < 8; ++col)
		{
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &graphics[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

			// Sprite pixel is on
			if (spritePixel)
			{
				// Screen pixel also on - collision
				if (*screenPixel == 0xFFFFFFFF)
				{
					registers[0xF] = 1;
				}

				// Effectively XOR with the sprite pixel
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}




}

void Chip8::OP_Ex9E() 
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;

	uint8_t key = registers[Vx];

	if (inputKeys[key]) 
	{
		pc += 2;
	}

}

void Chip8::OP_ExA1()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;

	uint8_t key = registers[Vx];

	if (!inputKeys[key])
	{
		pc += 2;
	}

}

void Chip8::OP_Fx07()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;

	registers[Vx] = delayTimer;
}

void Chip8::OP_Fx0A() 
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	

	if (inputKeys[0])
	{
		registers[Vx] = 0;
	}
	else if (inputKeys[1])
	{
		registers[Vx] = 1;
	}
	else if (inputKeys[2])
	{
		registers[Vx] = 2;
	}
	else if (inputKeys[3])
	{
		registers[Vx] = 3;
	}
	else if (inputKeys[4])
	{
		registers[Vx] = 4;
	}
	else if (inputKeys[5])
	{
		registers[Vx] = 5;
	}
	else if (inputKeys[6])
	{
		registers[Vx] = 6;
	}
	else if (inputKeys[7])
	{
		registers[Vx] = 7;
	}
	else if (inputKeys[8])
	{
		registers[Vx] = 8;
	}
	else if (inputKeys[9])
	{
		registers[Vx] = 9;
	}
	else if (inputKeys[10])
	{
		registers[Vx] = 10;
	}
	else if (inputKeys[11])
	{
		registers[Vx] = 11;
	}
	else if (inputKeys[12])
	{
		registers[Vx] = 12;
	}
	else if (inputKeys[13])
	{
		registers[Vx] = 13;
	}
	else if (inputKeys[14])
	{
		registers[Vx] = 14;
	}
	else if (inputKeys[15])
	{
		registers[Vx] = 15;
	}
	else 
	{
		pc -= 2;
	}

}
void Chip8::OP_Fx15()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;

	delayTimer = registers[Vx];
}
void Chip8::OP_Fx18()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;

	soundTimer = registers[Vx];
}

void Chip8::OP_Fx1E() 
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;

	index += registers[Vx];
}

void Chip8::OP_Fx29() 
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t digits = registers[Vx];


	index = FONTSET_START_ADDRESS + (5 * digits);
}
void Chip8::OP_Fx33()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;
	uint8_t value = registers[Vx];


	// Ones-place
	memory[index + 2] = value % 10;
	value /= 10;

	// Tens-place
	memory[index + 1] = value % 10;
	value /= 10;

	// Hundreds-place
	memory[index] = value % 10;
}

void Chip8::OP_Fx55()
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		memory[index + i] = registers[i];
	}

}
void Chip8::OP_Fx65() 
{
	uint8_t Vx = (opcode & 0x0F00) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		registers[i] = memory[index + i];
	}
	
}


void Chip8::Cycle() 
{
	opcode = (memory[pc] << 8u) | memory[pc + 1];

	pc += 2;

	((*this).*(table[(opcode & 0xF000) >> 12u]))();

	if (delayTimer > 0)
	{
		--delayTimer;
	}
	if (soundTimer > 0) 
	{
		--soundTimer;
	}


}

class Platform
{
public:
	Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight)
	{
		SDL_Init(SDL_INIT_VIDEO);

		window = SDL_CreateWindow(title, 0, 0, windowWidth, windowHeight, SDL_WINDOW_SHOWN);

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, textureWidth, textureHeight);
	}


	~Platform()
	{
		SDL_DestroyTexture(texture);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	void update(void const* buffer, int pitch) 
	{
		SDL_UpdateTexture(texture, nullptr, buffer, pitch);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		SDL_RenderPresent(renderer);
	}

	bool ProcessInput(uint8_t* keys) 
	{
		bool quit = false;

		SDL_Event event;

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT: 
			{
				quit = true;
			} break;


			case SDL_KEYDOWN: 
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
				{
					quit = true;
				} break;
				case SDLK_x:
				{
					keys[0] = 1;
				} break;
				case SDLK_1:
				{
					keys[1] = 1;
				} break;
				case SDLK_2:
				{
					keys[2] = 1;
				} break;
				case SDLK_3:
				{
					keys[3] = 1;
				} break;
				case SDLK_q:
				{
					keys[4] = 1;
				} break;
				case SDLK_w:
				{
					keys[5] = 1;
				} break;
				case SDLK_e:
				{
					keys[6] = 1;
				} break;
				case SDLK_a:
				{
					keys[7] = 1;
				} break;
				case SDLK_s:
				{
					keys[8] = 1;
				} break;
				case SDLK_d:
				{
					keys[9] = 1;
				} break;
				case SDLK_z:
				{
					keys[0xA] = 1;
				} break;
				case SDLK_c:
				{
					keys[0xB] = 1;
				} break;
				case SDLK_4:
				{
					keys[0xC] = 1;
				} break;
				case SDLK_r:
				{
					keys[0xD] = 1;
				} break;
				case SDLK_f:
				{
					keys[0xE] = 1;
				} break;
				case SDLK_v:
				{
					keys[0xF] = 1;
				} break;

				}
			} break;

			case SDL_KEYUP:
				switch (event.key.keysym.sym)
				{
				case SDLK_x:
				{
					keys[0] = 0;
				} break;
				case SDLK_1:
				{
					keys[1] = 0;
				} break;
				case SDLK_2:
				{
					keys[2] = 0;
				} break;
				case SDLK_3:
				{
					keys[3] = 0;
				} break;
				case SDLK_q:
				{
					keys[4] = 0;
				} break;
				case SDLK_w:
				{
					keys[5] = 0;
				} break;
				case SDLK_e:
				{
					keys[6] = 0;
				} break;
				case SDLK_a:
				{
					keys[7] = 0;
				} break;
				case SDLK_s:
				{
					keys[8] = 0;
				} break;
				case SDLK_d:
				{
					keys[9] = 0;
				} break;
				case SDLK_z:
				{
					keys[0xA] = 0;
				} break;
				case SDLK_c:
				{
					keys[0xB] = 0;
				} break;
				case SDLK_4:
				{
					keys[0xC] = 0;
				} break;
				case SDLK_r:
				{
					keys[0xD] = 0;
				} break;
				case SDLK_f:
				{
					keys[0xE] = 0;
				} break;
				case SDLK_v:
				{
					keys[0xF] = 0;

				} break;
			}
		} break;
	} 
	


private:
	SDL_Window* window{};
	SDL_Renderer* renderer{};
	SDL_Texture* texture{};

};
int main(int argc, char* argv[])
{

	while (true)
	{

	}

    SDL_Quit();

	return 0;
}
