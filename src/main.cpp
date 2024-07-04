#include <iostream>
#include <string>
#include <cstdint>

#include <Windows.h>

int main( int argc, const char** argv ) {
	if ( argc <= 1 ) {
		std::string executable_name{ argv[ 0 ] };
		executable_name = executable_name.substr( executable_name.find_last_of( "\\" ) + 1 );

		std::cout << "Usage: " << executable_name << " process_id dll_path" << '\n';
		return -1;
	}

	HANDLE	    process_handle{ };
	HANDLE	   file_handle{ };
	std::byte* file_buffer{ };

	do {
		const auto process_id = std::stoi( argv[ 1 ] );
		process_handle = OpenProcess(
			PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
			FALSE,
			process_id
		);

		if ( process_handle == INVALID_HANDLE_VALUE ) {
			std::cout << "Failed to open a handle to the process (has it been blocked via ObRegisterObjectCallback?)" << '\n';
			break;
		}

		const auto dll_path = std::string( argv[ 2 ] );
		std::cout << dll_path << '\n';
		file_handle = CreateFileA(
			dll_path.c_str( ),
			GENERIC_READ | GENERIC_WRITE,
			NULL,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);

		if ( file_handle == INVALID_HANDLE_VALUE ) {
			std::cout << "Failed to open a handle to the requested file." << '\n';
			break;
		}

		const auto file_size = GetFileSize( file_handle, NULL );
		if ( file_size == INVALID_FILE_SIZE || !file_size ) {
			std::cout << "Failed to retrieve the file file size." << '\n';
			break;
		}

		file_buffer = new std::byte[ file_size ]{ };
		DWORD bytes_read{ };
		if ( !ReadFile( file_handle, file_buffer, file_size, &bytes_read, NULL ) ) {
			std::cout << "Failed to read the file into the allocated buffer." << '\n';
			break;
		}

		const auto module_handle = reflective_loading( process_handle, file_buffer, file_size, nullptr );
		if ( !module_handle ) {
			std::cout << "Reflective loading failed." << '\n';
			break;
		}

		break;
	} while (true);
	
	if (process_handle) {
		CloseHandle(process_handle);
	}

	if (file_handle)
		CloseHandle(file_handle);

	if (file_buffer)
		delete[] file_buffer;

	return 0;


}