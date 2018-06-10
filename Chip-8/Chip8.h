#pragma once
#include "SDL2\SDL.h"
#include <cstdint>
#include <stack>
#include <array>
#include <random>
#include <string>
using byte = unsigned char;

class Chip8
{
public:
	Chip8(uint8_t pixel_size);
	~Chip8();
	Chip8(const Chip8&) = delete;
	Chip8(Chip8&&) = delete;
	Chip8& operator=(const Chip8&) = delete;
	Chip8& operator=(Chip8&&) = delete;


	void Load(const char* path);
	void PrintMemory(uint8_t columns) const;
	void Run();

private:

	void InitSDL();
	void LoadFontData(const char* path);
	void ProcessEvent(SDL_Event* evnt);
	void RenderPresent()const;
	bool EmulateCycle();
	std::array<byte, 4> FetchInstruction();

	//Display
	uint8_t m_pixel_size{};
	uint32_t m_screen_width{};
	uint32_t m_screen_height{};
	uint32_t m_sprite_width{};
	SDL_Color m_color_black{ 0,0,0,255 };
	SDL_Color m_color_white{ 255,255,255,255 };

	//Render
	SDL_Window* m_client{ nullptr };
	SDL_Renderer* m_renderer{ nullptr };
	std::array<std::array<byte, 64>, 32> m_pixels;

	//State
	bool m_is_init{};
	bool m_is_running{};

	//Input
	uint16_t m_key{};

	//Memory
	std::array<byte, 4096> m_memory;
	std::stack<uint16_t> m_stack;
	std::array<byte, 16> m_reg;
	uint16_t m_i{};
	byte* m_pc{ nullptr };

	//Random
	std::mt19937 m_generator;
	std::uniform_int_distribution<int> m_distribution;

	//Timer
	byte m_delay_timer{};
	byte m_sound_timer{};
};


inline void SetRenderDrawColor(SDL_Renderer* renderer, SDL_Color col)
{
	SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
}

inline std::string InstructionToString(const std::array<byte, 4>& instruction)
{
	std::string ret;
	for (auto& val : instruction) {
		ret += val < 10 ? '0' + val : 'A' + val - 10;
	}
	return ret;
}

inline uint16_t InstructionToAddress(const std::array<byte, 4>& val)
{
	return static_cast<uint16_t>((val[1] << 8) | (val[2] << 4) | (val[3]));
}