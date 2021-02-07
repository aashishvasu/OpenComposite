#include "stdafx.h"
#include "debug_helper.h"

#if defined(_DEBUG)

//******************************************************************************
static const int MaxNameLen = 256;

#include <Windows.h>
#include "dbghelp.h"
#include <sstream>
#pragma comment(lib,"Dbghelp.lib")

static HMODULE globalhModule;
void DbgSetModule(HMODULE hModule) {
	globalhModule = hModule;
	::SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_INCLUDE_32BIT_MODULES | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES);
	//if (!::SymInitialize(::GetCurrentProcess(), "http://msdl.microsoft.com/download/symbols", TRUE)) return false;
	::SymInitialize(hModule, NULL, TRUE);
}

void GetStackWalk() {
	std::string outWalk;

	// Set up the symbol options so that we can gather information from the current
	// executable's PDB files, as well as the Microsoft symbol servers.  We also want
	// to undecorate the symbol names we're returned.  If you want, you can add other
	// symbol servers or paths via a semi-colon separated list in SymInitialized.
	static bool inited = false;
	if (!inited) {
		// See DLLMain
		//::SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_INCLUDE_32BIT_MODULES | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES);
		//if (!::SymInitialize(::GetCurrentProcess(), "http://msdl.microsoft.com/download/symbols", TRUE)) return false;
		//if (!::SymInitialize(::GetCurrentProcess(), NULL, TRUE)) return;
		inited = true;
	}

	// Capture up to 25 stack frames from the current call stack.  We're going to
	// skip the first stack frame returned because that's the GetStackWalk function
	// itself, which we don't care about.
	PVOID addrs[25] = { 0 };
	USHORT frames = CaptureStackBackTrace(1, 25, addrs, NULL);

	for (USHORT i = 0; i < frames; i++) {
		char buffer[sizeof(IMAGEHLP_SYMBOL) + MAX_SYM_NAME * sizeof(TCHAR)] = { 0 };
#ifdef _WIN64
		DWORD64 dis = 0;
#else
		DWORD dis = 0;
#endif
		IMAGEHLP_SYMBOL *pSym = NULL;
		BOOL res;

		pSym = (IMAGEHLP_SYMBOL *)buffer;
		pSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
		pSym->MaxNameLength = MAX_PATH;

		res = SymGetSymFromAddr(globalhModule /*GetCurrentProcess()*/, (DWORD)addrs[i], &dis, pSym);

		std::stringstream stream;
		stream << std::hex << (DWORD64)addrs[i];
		if (res) {
			stream << " " << pSym->Name;
		}
		std::string result(stream.str());
		outWalk.append(stream.str());
		outWalk.append("\n");
	}

	//::SymCleanup(::GetCurrentProcess());

	OOVR_LOG(outWalk.c_str());
	OOVR_MESSAGE(outWalk.c_str(), "OOVR Stack Trace");
}

//******************************************************************************

#endif
