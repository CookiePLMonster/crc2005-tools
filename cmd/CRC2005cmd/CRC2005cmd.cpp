#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#define WINVER 0x0502
#define _WIN32_WINNT 0x0502

#include <windows.h>
#include <shellapi.h>
#include <io.h>
#include <Shlwapi.h>

#include "MemoryMgr.h"

#pragma comment(lib, "shlwapi.lib")

namespace LZWTools
{
	static constexpr int LZW_VERSION = 11;

	void UnpackLZW( const wchar_t* file )
	{
		auto LZW_decompress = (int(*)(const uint8_t* input, uint32_t inputSize, uint8_t* output, uint32_t outputSize, int version))0x403530;
		auto LZW_expand_size = (int(*)(const uint8_t* input, uint32_t inputSize))0x402DB0;

		FILE* hFile = nullptr;
		if ( _wfopen_s( &hFile, file, L"rb") == 0 )
		{
			int inputSize = _filelength( _fileno(hFile) );
			uint8_t* inputBuf = new uint8_t[inputSize];

			fread( inputBuf, 1, inputSize, hFile );
			fclose(hFile);

			int outputSize = LZW_expand_size( inputBuf, inputSize );
			uint8_t* outputBuf = new uint8_t[outputSize];

			LZW_decompress( inputBuf, inputSize, outputBuf, outputSize, LZW_VERSION );

			wchar_t outPath[MAX_PATH];
			wcscpy_s( outPath, file );
			PathRenameExtension( outPath, L".ishd" );

			FILE* outFile = nullptr;
			if ( _wfopen_s( &outFile, outPath, L"wb") == 0 )
			{
				fwrite( outputBuf, 1, outputSize, outFile );
				fclose( outFile );		
			}

			delete[] inputBuf;
			delete[] outputBuf;
		}
	}

	void PackLZW( const wchar_t* file )
	{
		auto LZW_shrink_size = (int(*)(const uint8_t* input, uint32_t inputSize))0x403230;
		auto LZW_compress = (int(*)(const uint8_t* input, uint32_t inputSize, uint8_t* output, uint32_t outputSize, int version))0x402E30;

		FILE* hFile = nullptr;
		if ( _wfopen_s( &hFile, file, L"rb") == 0 )
		{
			int inputSize = _filelength( _fileno(hFile) );
			uint8_t* inputBuf = new uint8_t[inputSize];

			fread( inputBuf, 1, inputSize, hFile );
			fclose(hFile);

			int outputSize = LZW_shrink_size( inputBuf, inputSize );
			uint8_t* outputBuf = new uint8_t[outputSize];

			LZW_compress( inputBuf, inputSize, outputBuf, outputSize, LZW_VERSION );

			wchar_t outPath[MAX_PATH];
			wcscpy_s( outPath, file );
			PathRenameExtension( outPath, L".dat" );

			FILE* outFile = nullptr;
			if ( _wfopen_s( &outFile, outPath, L"wb") == 0 )
			{
				fwrite( outputBuf, 1, outputSize, outFile );
				fclose( outFile );		
			}

			delete[] inputBuf;
			delete[] outputBuf;
		}
	}

	bool ProcessCommandLineArguments( LPWSTR* cmdLine, int numArgs )
	{
		for ( int i = 2; i < numArgs; i++ )
		{
			// Decompress LZW archive
			if ( _wcsicmp( cmdLine[i], L"-u" ) == 0 )
			{
				if ( i + 1 < numArgs )
				{
					UnpackLZW( cmdLine[i + 1] );
					return true;
				}
			}

			// Compress LZW archive
			else if ( _wcsicmp( cmdLine[i], L"-p" ) == 0 )
			{
				if ( i + 1 < numArgs )
				{
					PackLZW( cmdLine[i + 1] );
					return true;
				}
			}
		}
		return false;
	}

	void InitLZW_CommandLine()
	{
		bool exit = false;

		int numArgs = 0;
		LPWSTR* cmdLine = CommandLineToArgvW( GetCommandLineW(), &numArgs );
		if ( cmdLine != nullptr )
		{
			// If there are enough arguments, the first one should always be --cmd to indicate
			// that we want the game to start in "tool" mode
			if ( numArgs >= 2 )
			{
				if ( _wcsicmp( cmdLine[1], L"--cmd" ) == 0 )
				{
					if ( ProcessCommandLineArguments( cmdLine, numArgs ) )
					{
						exit = true;
					}
				}
			}
			LocalFree( cmdLine );
		}

		if ( exit )
		{
			ExitProcess(0);
		}
	}

	static void (*orgInitLZW)();
	void InitLZW_Hook()
	{
		orgInitLZW();
		InitLZW_CommandLine();
	}
}

void InitASI()
{
	using namespace LZWTools;
	using namespace Memory::VP;

	ReadCall( 0x5F0E2A, orgInitLZW );
	InjectHook( 0x5F0E2A, InitLZW_Hook );
}

extern "C"
{
	static LONG InitCount = 0;
	__declspec(dllexport) void InitializeASI()
	{
		if ( _InterlockedCompareExchange( &InitCount, 1, 0 ) != 0 ) return;
		InitASI();
	}
}