#include "Chip8.h"
#include "Console.h"
#include <fstream>
#include <iomanip>
#include <vector>

Chip8::Chip8(uint8_t pixel_size) :
	m_distribution(0, 255),
	m_pixel_size{ pixel_size }
{
	m_screen_width = 64 * m_pixel_size;
	m_screen_height = 32 * m_pixel_size;
	m_sprite_width = 8 * m_pixel_size;
	m_pc = &m_memory[512];


	CreateConsole();

	InitSDL();

	LoadFontData("font_data.txt");
}

Chip8::~Chip8()
{
	if (m_renderer)SDL_DestroyRenderer(m_renderer);
	if (m_client)SDL_DestroyWindow(m_client);
	SDL_Quit();
}

void Chip8::InitSDL()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		ConOut("Failed to initialize SDL", '\n');
		return;
	}

	m_client = SDL_CreateWindow("Chip-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_screen_width, m_screen_height, 0);
	if (!m_client) {
		ConOut("Failed to create Window", '\n');
		return;
	}

	m_renderer = SDL_CreateRenderer(m_client, -1, SDL_RENDERER_ACCELERATED);
	if (!m_renderer) {
		ConOut("Failed to create Renderer", '\n');
		return;
	}
	m_is_init = true;
}

void Chip8::LoadFontData(const char* path)
{

	std::array<std::array<char, 4>, 5> missing_font{
		'1','0','1','0',
		'0','1','0','1',
		'1','0','1','0',
		'0','1','0','1',
		'1','0','1','0'
	};

	const int symbols = 16;
	const int font_height = 5;
	const int font_width = 4;
	char ch = ' ';
	byte line;
	std::ifstream font;
	font.open(path);

	for (size_t i = 0; i < symbols; i++)
	{
		for (size_t j = 0; j < font_height; j++)
		{
			line = 0;
			for (size_t k = 0; k < font_width; k++)
			{
				do
				{
					if (!font.get(ch))
					{
						ConOut("malformed font data");
						ch = missing_font[j][k];
					}
				} while (ch == '\n');

				if (ch == '1')
				{
					line |= 1 << (7 - k);
				}
			}
			m_memory[i * 5 + j] = line;
		}
	}

	font.close();
}

void Chip8::Load(const char* path)
{
	std::ifstream rom;
	rom.open(path, std::ios_base::in | std::ios_base::binary);
	if (rom.fail())
	{
		ConOut("Failed to Load rom");
		return;
	}

	auto it = m_memory.begin() + 512;
	uint8_t b;
	while (!rom.eof())
	{
		if (rom.get((char&)(b)))
		{
			*it++ = b;
		}
	}

	rom.close();
}

void Chip8::Run()
{
	m_is_running = m_is_init;

	SDL_Event evnt{};
	while (m_is_running)
	{
		//TODO:Replace timer
		if (m_delay_timer > 0) {
			m_delay_timer--;
		}

		while (SDL_PollEvent(&evnt)) {
			ProcessEvent(&evnt);
		}

		m_is_running = EmulateCycle();
		RenderPresent();
	}
}

void Chip8::RenderPresent()const
{
	SDL_RenderClear(m_renderer);
	SetRenderDrawColor(m_renderer, m_color_white);


	for (size_t row = 0; row < m_pixels.size(); row++)
	{
		for (size_t col = 0; col < m_pixels[0].size(); col++)
		{
			if (m_pixels[row][col])
			{
				SDL_Rect pixel{
					col * m_pixel_size, //x
					row * m_pixel_size, //y
					m_pixel_size,       //w
					m_pixel_size        //h
				};

				SDL_RenderFillRect(m_renderer, &pixel);
			}
		}
	}

	SDL_RenderPresent(m_renderer);
	SetRenderDrawColor(m_renderer, m_color_black);
}

std::array<byte, 4> Chip8::FetchInstruction()
{
	uint16_t instruction = (*m_pc << 8) | *(m_pc + 1);
	byte A = (instruction & 0xF000) >> 12;
	byte B = (instruction & 0x0F00) >> 8;
	byte C = (instruction & 0x00F0) >> 4;
	byte D = (instruction & 0x000F) >> 0;
	m_pc += 2;

	return std::array<byte, 4> { A, B, C, D };
}

void Chip8::PrintMemory(uint8_t columns) const
{
	ConOut(std::hex);
	for (size_t i = 0; i < m_memory.size(); i++)
	{
		ConOut(
			std::setw(2),
			m_memory[i] >> 4,
			m_memory[i] & 0x0F,
			" "
		);

		if ((i + 1) % 8 == 0) {
			ConOut('\n');
		}
	}
	ConOut(std::dec, '\n');
}

void Chip8::ProcessEvent(SDL_Event* evnt)
{
	switch (evnt->type)
	{

		case SDL_KEYDOWN:
		{
			switch (evnt->key.keysym.sym)
			{
				case SDLK_KP_0: {  m_key |= 1 << 0;  }break;
				case SDLK_KP_1: {  m_key |= 1 << 7;  }break;
				case SDLK_KP_2: {  m_key |= 1 << 8;  }break;
				case SDLK_KP_3: {  m_key |= 1 << 9;  }break;
				case SDLK_KP_4: {  m_key |= 1 << 4;  }break;
				case SDLK_KP_5: {  m_key |= 1 << 5;  }break;
				case SDLK_KP_6: {  m_key |= 1 << 6;  }break;
				case SDLK_KP_7: {  m_key |= 1 << 1;  }break;
				case SDLK_KP_8: {  m_key |= 1 << 2;  }break;
				case SDLK_KP_9: {  m_key |= 1 << 3;  }break;
				case SDLK_a: {  m_key |= 1 << 10; }break;
				case SDLK_b: {  m_key |= 1 << 11; }break;
				case SDLK_c: {  m_key |= 1 << 12; }break;
				case SDLK_d: {  m_key |= 1 << 13; }break;
				case SDLK_e: {  m_key |= 1 << 14; }break;
				case SDLK_f: {  m_key |= 1 << 15; }break;
			}
			break;
		}

		case SDL_KEYUP:
		{
			switch (evnt->key.keysym.sym)
			{
				case SDLK_KP_0: {  m_key &= 0xFFFE;  }break;
				case SDLK_KP_1: {  m_key &= 0xFF7F;  }break;
				case SDLK_KP_2: {  m_key &= 0xFEFF;  }break;
				case SDLK_KP_3: {  m_key &= 0xFDFF;  }break;
				case SDLK_KP_4: {  m_key &= 0xFFEF;  }break;
				case SDLK_KP_5: {  m_key &= 0xFFDF;  }break;
				case SDLK_KP_6: {  m_key &= 0xFFBF;  }break;
				case SDLK_KP_7: {  m_key &= 0xFFFD;  }break;
				case SDLK_KP_8: {  m_key &= 0xFFFB;  }break;
				case SDLK_KP_9: {  m_key &= 0xFFF7;  }break;
				case SDLK_a: {  m_key &= 0xFBFF;  }break;
				case SDLK_b: {  m_key &= 0xF7FF;  }break;
				case SDLK_c: {  m_key &= 0xEFFF;  }break;
				case SDLK_d: {  m_key &= 0xDFFF;  }break;
				case SDLK_e: {  m_key &= 0xBFFF;  }break;
				case SDLK_f: {  m_key &= 0x7FFF;  }break;
			}
			break;
		}

		case SDL_QUIT:
		{
			m_is_running = false;
			break;
		}
	}
}

bool Chip8::EmulateCycle()
{
	auto val = FetchInstruction();
	byte& VX = m_reg[val[1]];
	byte& VY = m_reg[val[2]];
	byte& VF = m_reg[15];

	ConOut(std::hex, "Instruction ", InstructionToString(val), "\n");

	switch (val[0])
	{

		case 0x00:
		{

			switch ((val[1] << 8) | (val[2] << 4) | (val[3]))
			{

				//00E0	Clears the screen.
				case 0x0E0:
				{
					for (auto& row : m_pixels) {
						row.fill(0);
					}
					break;
				}

				//00EE  Returns from a subroutine.
				case 0x0EE:
				{
					m_pc = &m_memory[0] + m_stack.top();
					m_stack.pop();
					break;
				}

				default:
					// 0NNN	Calls RCA 1802 program at address NNN.Not necessary for most ROMs.
					ConOut(std::hex, "Instruction ", InstructionToString(val), " is not supported. Aborting process.\n");
					return false;
			}

			break;
		}

		//1NNN  Jumps to address NNN.
		case 0x01:
		{
			m_pc = &m_memory[0] + InstructionToAddress(val);
			break;
		}

		//2NNN  Calls subroutine at NNN.
		case 0x02:
		{
			m_stack.push(static_cast<uint16_t>(m_pc - &m_memory[0]));
			m_pc = &m_memory[0] + InstructionToAddress(val);
			break;
		}

		//3XNN	Skips the next instruction if VX equals NN.
		case 0x03:
		{
			if (VX == ((val[2] << 4) | val[3]))
			{
				m_pc += 2;
			}
			break;
		}

		//4XNN  Skips the next instruction if VX doesn't equal NN.
		case 0x04:
		{
			if (VX != ((val[2] << 4) | val[3]))
			{
				m_pc += 2;
			}
			break;
		}

		//5XY0  Skips the next instruction if VX equals VY.
		case 0x05:
		{
			if (VX == VY)
			{
				m_pc += 2;
			}
			break;
		}

		//6XNN  Sets VX to NN.
		case 0x06:
		{
			VX = (val[2] << 4) | val[3];
			break;
		}

		//7XNN	Adds NN to VX.
		case 0x07:
		{
			VX += (val[2] << 4) | val[3];
			break;
		}

		case 0x08:
		{
			switch (val[3])
			{
				//8XY0	Sets VX to the value of VY.
				case 0x00:
				{
					VX = VY;
					break;
				}

				//8XY1  Sets VX to VX or VY.
				case 0x01:
				{
					VX |= VY;
					break;
				}

				//8XY2  Sets VX to VX and VY. 
				case 0x02:
				{
					VX &= VY;
					break;
				}

				//8XY3  Sets VX to VX xor VY.
				case 0x03:
				{
					VX ^= VY;
					break;
				}

				//8XY4  Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
				case 0x04:
				{
					VF = (static_cast<int>(VX) + static_cast<int>(VY)) > 255 ? 1 : 0;
					VX += VY;
					break;
				}

				//8XY5  VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
				case 0x05:
				{
					VF = (VX < VY) ? 0 : 1;
					VX -= VY;
					break;
				}

				//8XY6	Shifts VY right by one and stores the result to VX (VY remains unchanged). 
				//      VF is set to the value of the least significant bit of VY before the shift.
				case 0x06:
				{
					VX = VY >> 1;
					VF = VY & 0x01;
					break;
				}

				//8XY7  Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
				case 0x07:
				{
					VF = (VY < VX) ? 0 : 1;
					VX = VY - VX;
					break;
				}

				//8XYE	Shifts VY left by one and copies the result to VX. 
				//		VF is set to the value of the most significant bit of VY before the shift.
				case 0x0E:
				{
					VX = VY << 1;
					VF = (VY & 0x80) >> 7;
					break;
				}

				default:
					ConOut(std::hex, "Instruction ", InstructionToString(val), " is not supported. Aborting process.\n");
					return false;
			}
			break;
		}

		//9XY0	Skips the next instruction if VX doesn't equal VY.
		case 0x09:
		{
			if (VX != VY)
			{
				m_pc += 2;
			}
			break;
		}

		//ANNN	Sets I to the address NNN.
		case 0x0A:
		{
			m_i = InstructionToAddress(val);
			break;
		}

		//BNNN  Jumps to the address NNN plus V0.
		case 0x0B:
		{
			m_pc = &m_memory[0] + m_reg[0] + InstructionToAddress(val);
			break;
		}

		//CXNN  Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
		case 0x0C:
		{
			VX = ((val[2] << 4) | val[3]) & m_distribution(m_generator);
			break;
		}

		//DXYN 	The interpreter reads n bytes from memory, starting at the address stored in I.
		//		These bytes are then displayed as sprites on screen at coordinates(Vx, Vy).Sprites are XORed onto the existing screen.
		//		If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0. 
		//		If the sprite is positioned so part of it is outside the coordinates of the display, it wraps around to the opposite side of 
		//		the screen.
		case 0x0D:
		{
			uint8_t x = VX;
			uint8_t y = VY;
			uint8_t height = val[3];
			uint8_t width = 8;
			VF = 0;
			auto sprite_address = m_memory.begin() + m_i;
			
			std::vector<byte> sprite(sprite_address, sprite_address + height);


			for (uint8_t sprite_row = 0; sprite_row < height; sprite_row++)
			{
				for (uint8_t sprite_col = 0; sprite_col < width; sprite_col++)
				{
					byte curr_bit = (1 << (7 - sprite_col)) & sprite[sprite_row];
					uint8_t pixel_Y = (y + sprite_row) % 32;
					uint8_t pixel_X = (x + sprite_col) % 64;

					if (m_pixels[pixel_Y][pixel_X] && curr_bit)
					{
							VF = 1;
					}

					m_pixels[pixel_Y][pixel_X] ^= curr_bit;
				}
			}
			break;
		}

		case 0x0E:
		{
			switch ((val[2] << 4) | val[3])
			{
				//EX9E	Skips the next instruction if the key stored in VX is pressed.
				case 0x9E:
				{
					if (m_key & (1 << VX))
					{
						m_pc += 2;
					}
					break;
				}

				//EXA1  Skips the next instruction if the key stored in VX isn't pressed.
				case 0xA1:
				{
					if (!(m_key & (1 << VX)))
					{
						m_pc += 2;
					}
					break;
				}

				default:
					ConOut(std::hex, "Instruction ", InstructionToString(val), " is not supported. Aborting process.\n");
					return false;
			}

			break;
		}

		case 0x0F:
		{
			switch ((val[2] << 4) | val[3])
			{
				//FX07  Sets VX to the value of the delay timer.
				case 0x07:
				{
					VX = m_delay_timer;
					break;
				}

				//FX0A	A key press is awaited, and then stored in VX. (Blocking Operation.All instruction halted until next key event)
				case 0x0A:
				{
					if (!m_key)
					{
						m_pc -= 2;
					}
					VX = m_key;
					break;
				}

				//FX15	Sets the delay timer to VX.
				case 0x15:
				{
					m_delay_timer = VX;
					break;
				}

				//FX18  Sets the sound timer to VX.
				case 0x18:
				{
					m_sound_timer = VX;
					break;
				}

				//FX1E	Adds VX to I.
				case 0x1E:
				{
					m_i += VX;
					break;
				}

				//FX29  Sets I to the location of the sprite for the character in VX.
				//		Characters 0 - F(in hexadecimal) are represented by a 4x5 font.
				case 0x29:
				{
					m_i = VX * 5;
					break;
				}

				//FX33  Stores the binary-coded decimal representation of VX, with the most significant of three digits 
				//		at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. 	
				case 0x33:
				{
					int num = VX;

					m_memory[m_i + 0] = num / 100;
					num -= m_memory[m_i + 0] * 100;

					m_memory[m_i + 1] = num / 10;
					num -= m_memory[m_i + 1] * 10;

					m_memory[m_i + 2] = num / 1;

					break;
				}

				//TODO
				//FX55	Stores V0 to VX(including VX) in m_memory starting at address I.I is increased by 1 for each value written.
				case 0x55:
				{
					for (size_t i = 0; i <= val[1]; i++)
					{
						m_memory[m_i] = m_reg[i];
						m_i++;
					}
					break;
				}

				//FX65  Fills V0 to VX (including VX) with values from memory starting at address I. I is increased by 1 for each value written.
				case 0x65:
				{
					for (size_t i = 0; i <= val[1]; i++)
					{
						m_reg[i] = m_memory[m_i];
						m_i++;
					}
					break;
				}

				default:
					ConOut(std::hex, "Instruction ", InstructionToString(val), " is not supported. Aborting process.\n");
					return false;
			}
			break;
		}



		default:
			ConOut(std::hex, "Instruction ", InstructionToString(val), " is not supported. Aborting process.\n");
			return false;
	}
	return true;
}