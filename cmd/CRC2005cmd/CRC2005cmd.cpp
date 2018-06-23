#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#define WINVER 0x0502
#define _WIN32_WINNT 0x0502

#include <windows.h>
#include <shellapi.h>
#include <io.h>
#include <Shlwapi.h>
#include <algorithm>

#include "MemoryMgr.h"

#pragma comment(lib, "shlwapi.lib")

namespace LZWTools
{
	static constexpr int LZW_VERSION = 11;

	void UnpackLZW( const wchar_t* file, const wchar_t* outName )
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
			if ( outName == nullptr )
			{
				wcscpy_s( outPath, file );
				PathRenameExtension( outPath, L".ishd" );
				outName = outPath;
			}

			FILE* outFile = nullptr;
			if ( _wfopen_s( &outFile, outName, L"wb") == 0 )
			{
				fwrite( outputBuf, 1, outputSize, outFile );
				fclose( outFile );		
			}

			delete[] inputBuf;
			delete[] outputBuf;
		}
	}

	void PackLZW( const wchar_t* file, const wchar_t* outName )
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
			if ( outName == nullptr )
			{
				wcscpy_s( outPath, file );
				PathRenameExtension( outPath, L".dat" );
				outName = outPath;
			}

			FILE* outFile = nullptr;
			if ( _wfopen_s( &outFile, outName, L"wb") == 0 )
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
		// A simple helper for managing commandline arguments
		class CMDLine
		{
		public:
			CMDLine( LPWSTR* cmdLine, int numArgs )
				: begin(cmdLine), end(cmdLine+numArgs)
			{
			}

			bool hasOption( LPCWSTR opt ) const
			{
				return std::find_if( begin, end, [&]( LPCWSTR e ) {
					return _wcsicmp( e, opt ) == 0;
				} ) != end;
			}

			LPCWSTR getOptionArgument( LPCWSTR opt ) const
			{
				LPWSTR* foundOpt = std::find_if( begin, end, [&]( LPCWSTR e ) {
					return _wcsicmp( e, opt ) == 0;
				} );		
				return foundOpt != end && (foundOpt+1) != end ? *(foundOpt+1) : nullptr;
			}

		private:
			LPWSTR* const begin;
			LPWSTR* const end;

		};
		const CMDLine commands( cmdLine, numArgs );


		// Decompress LZW archive
		if ( commands.hasOption( L"-u" ) )
		{
			if ( LPCWSTR arg = commands.getOptionArgument( L"-u") )
			{
				UnpackLZW( arg, commands.getOptionArgument( L"-o") );
				return true;
			}
		}

		// Compress LZW archive
		if ( commands.hasOption( L"-p" ) )
		{
			if ( LPCWSTR arg = commands.getOptionArgument( L"-p") )
			{
				PackLZW( arg, commands.getOptionArgument( L"-o") );
				return true;
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
			for ( int i = 1; i < numArgs; i++ )
			{
				// If there are enough arguments, the first one should always be --cmd to indicate
				// that we want the game to start in "tool" mode
				if ( _wcsicmp( cmdLine[i], L"--cmd" ) == 0 )
				{
					if ( ProcessCommandLineArguments( cmdLine+(i+1), numArgs-(i+1) ) )
					{
						exit = true;
					}
					break;
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