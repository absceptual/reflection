#pragma once
#include <Windows.h>

auto reflective_loading( HANDLE process_handle, HANDLE file_handle, DWORD file_size, void* argument );