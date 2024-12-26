#ifndef CTL_OS_SYSTEM_H
#define CTL_OS_SYSTEM_H

#include "../namespace.hpp"
#include "../container/string.hpp"
#include "../container/error.hpp"
#include "../regex/core.hpp"
#include "filesystem.hpp"

#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__) && !defined(CTL_NO_WINDOWS_PLEASE)
#include <windows.h>
#include <winapifamily.h>
#include <commdlg.h>
#else

#endif

CTL_NAMESPACE_BEGIN

/// @brief Operating system (and related) facilities.
namespace OS {
	namespace {
		inline String sanitizedArgument(String arg) {
			arg = Regex::replace(arg, "\\\\+", "\\\\");
			#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__) && !defined(CTL_NO_WINDOWS_PLEASE)
			arg = Regex::replace(arg, "\\\\+\"", "\\\"");
			return "\"" + arg + "\"";
			#else
			arg = Regex::replace(arg, "\\\\+'", "\\'");
			arg = Regex::replace(arg, "'", "\\'");
			return "'" + arg + "'";
			#endif
		}
	}

	/// @brief Runs an executable in the same thread.
	/// @param path Path to executable.
	/// @param directory Directory to run in. By default, it is the same directory as the executable.
	/// @param args Arguments to pass to executable. By default, it is empty.
	/// @return Exit code of the executable.
	/// @note Running the program in a separate directory is currently only supported on Windows.
	inline int launch(String const& path, String const& directory = "", StringList args = StringList()) {
		if (!FS::exists(path))
			throw Error::InvalidValue("File [" + path + "] does not exist!", CTL_CPP_PRETTY_SOURCE);
		#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__) && !defined(CTL_NO_WINDOWS_PLEASE)
		String prgArgs = "";
		if (!args.empty())
			for (String const& arg: args)
				prgArgs += sanitizedArgument(arg) + " ";
		prgArgs = sanitizedArgument(path) + (args.empty() ? "" : (" " + prgArgs));
		// This is a nightmare, but the other option pops up a command prompt.
		STARTUPINFOA sInfo;
		PROCESS_INFORMATION pInfo;
		memset(&sInfo, 0, sizeof(sInfo));
		sInfo.cb = sizeof(sInfo);
		memset(&pInfo, 0, sizeof(pInfo));
		auto proc = CreateProcessA(
			(LPCSTR)path.cstr(),
			(LPSTR)prgArgs.cstr(),
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			(LPCSTR)directory.empty() ? NULL : directory.cstr(),
			(LPSTARTUPINFOA)&sInfo,
			&pInfo
		);
		if (!proc) throw Error::FailedAction(toString("could not find '", path,"!"), CTL_CPP_PRETTY_SOURCE);
		proc = WaitForSingleObject(pInfo.hProcess, INFINITE);
		DWORD res;
		GetExitCodeProcess(pInfo.hProcess, &res);
		CloseHandle(pInfo.hProcess);
		CloseHandle(pInfo.hThread);
		return (int)res;
		#else
		List<const char*> prgArgs;
		prgArgs.pushBack(FS::fileName(path));
		for (String& arg: args)
			prgArgs.pushBack(arg.cstr());
		prgArgs.pushBack(NULL);
		return execv(path.cstr(), prgArgs.data());
		#endif
	}
}

CTL_NAMESPACE_END

#endif // CTL_OS_SYSTEM_H
