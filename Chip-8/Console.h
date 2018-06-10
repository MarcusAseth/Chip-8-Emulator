#pragma once
#include <Windows.h>
#include <iostream>

void CreateConsole()
{
	AllocConsole();
	FILE* stream;
	freopen_s(&stream, "CONIN$", "r", stdin);
	freopen_s(&stream, "CONOUT$", "w", stdout);
	freopen_s(&stream, "CONOUT$", "w", stderr);
}

void ConOut()
{
}

template <typename first, typename... rest>
void ConOut(first arg1, rest... args)
{
	std::cout << arg1;
	ConOut(args...);
}