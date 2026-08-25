#ifndef CTL_OS_SYSTEM_H
#define CTL_OS_SYSTEM_H

#include "../namespace.hpp"
#include "../container/strings/string.hpp"
#include "../container/pointer/shared.hpp"
#include "../container/error.hpp"
#include "../regex/core.hpp"
#include "filesystem.hpp"

#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
#include <windows.h>
#include <winapifamily.h>
#include <commdlg.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <spawn.h>
#endif

CTL_NAMESPACE_BEGIN

/// @brief Operating system (and related) facilities.
namespace OS {
	/// @brief Process identifier.
	struct Process {
		#ifdef CTL_ON_WINDOWS
		pointer handle;
		#else
		pid_t id;
		#endif
	};

	namespace {
		inline String sanitizedArgument(String arg) {
			#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
			return "\"" + arg + "\"";
			#else
			return "'" + arg + "'";
			#endif
		}
	}

	/// @brief Returns the given path with the appropriate executable file extension for the current operating system.
	/// @return Executable path.
	constexpr String executable(String const& path = "") {
		if constexpr (CTL_TARGET_OS == CTL_OS_WINDOWS)
			return path + ".exe";
		else return path;
	}

	/// @brief Launches an executable in a different process.
	/// @param path Path to executable.
	/// @param directory Directory to run in. By default, it is the same directory as the executable.
	/// @param args Arguments to pass to executable. By default, it is empty.
	/// @return Process object associated with the executable.
	/// @note Running the program in a separate directory is currently only supported on Windows.
	inline Process launchAsync(String const& path, String const& directory = "", StringList args = StringList()) {
		if (!FS::exists(path))
			throw Error::InvalidValue("File [" + path + "] does not exist!", CTL_CPP_PRETTY_SOURCE);
		#if (CTL_TARGET_OS == CTL_OS_WINDOWS)
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
		if (!proc) throw Error::FailedAction(toString("could not run '", path,"!"), CTL_CPP_PRETTY_SOURCE);
		CloseHandle(pInfo.hThread);
		return {(pointer)pInfo.hProcess};
		#else
		List<const char*> prgArgs;
		auto const fname = FS::fileName(path);
		prgArgs.pushBack(fname.cstr());
		for (String& arg: args)
			prgArgs.pushBack(arg.cstr());
		prgArgs.pushBack(NULL);
		pid_t pid;
		posix_spawnp(&pid, path.cstr(), NULL, NULL, Cast::mutate<ref<ref<char>>>(prgArgs.data()), NULL);
		return {pid};
		#endif
	}

	/// @brief Awaits a given process to return a result.
	/// @param process Process to wait for.
	/// @return Exit code of the process.
	/// @caution On windows, the handle to the process also gets closed!
	inline int awaitResult(Process& process) {
		#ifdef CTL_ON_WINDOWS
		if (!process.handle) return 0;
		proc = WaitForSingleObject(process.handle, INFINITE);
		DWORD res;
		GetExitCodeProcess(process.handle, &res);
		CloseHandle(process.handle);
		process.handle = nullptr;
		return (int)res;
		#else
		if (!process.id) return 0;
		int result;
		waitpid(process.id, &result, WUNTRACED | WCONTINUED);
		return result;
		#endif
	}

	/// @brief Awaits a given process to return a result.
	/// @param process Process to wait for.
	/// @return Exit code of the process.
	/// @caution On windows, the handle to the process also gets closed!
	inline int awaitResult(Process&& process) {
		return awaitResult(process);
	}

	/// @brief Runs an executable in the same thread.
	/// @param path Path to executable.
	/// @param directory Directory to run in. By default, it is the same directory as the executable.
	/// @param args Arguments to pass to executable. By default, it is empty.
	/// @return Exit code of the executable.
	/// @note Running the program in a separate directory is currently only supported on Windows.
	inline int launch(String const& path, String const& directory = "", StringList args = StringList()) {
		return awaitResult(launchAsync(path, directory, args));
	}
}

CTL_NAMESPACE_END

#endif // CTL_OS_SYSTEM_H
