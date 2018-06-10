#include <memory>
#include "Chip8.h"

int main(int argc, char** argv)
{
	const uint8_t pixel_size = 14;
	auto Emu = std::make_unique<Chip8>(pixel_size);

	Emu->Load("pong.rom");

	Emu->Run();

	SDL_Delay(3000);
	return 0;
}
