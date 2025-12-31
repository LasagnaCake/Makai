#ifndef CTL_CPP_STACKTRACE_H
#define CTL_CPP_STACKTRACE_H

#include "../namespace.hpp"
#include "../ctypes.hpp"

#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__) && !defined(CTL_NO_WINDOWS_PLEASE)
#include <windows.h>
#include <dbghelp.h>
#include <process.h>
#else

#endif

CTL_NAMESPACE_BEGIN

namespace CPP::Stack {
	struct Frame {
		constexpr static usize SIZE = 1024;
		using Text = As<char[SIZE]>;
		Text	info		= "\0";
		Text	file		= "\0";
		ssize	line		= -1;
		uint64	address		= 0;
	};

	template <usize F = 256>
	struct Trace {
		constexpr static usize const	MAX_FRAMES = F;
		owner<Frame const> const		frames	= nullptr;
		usize const						count	= 0;

		Trace() {}

		Trace(Trace const& other):	Trace(other.frames, other.count, true) {}
		Trace(Trace&& other):		Trace(other.frames, other.count, true) {}

		~Trace() {
			delete[] frames;
		}
		
		#if (_WIN32 || _WIN64 || __WIN32__ || __WIN64__) && !defined(CTL_NO_WINDOWS_PLEASE)
		__declspec(noinline) static Trace capture() {
    		HANDLE proc = GetCurrentProcess();
			SymInitialize(proc, NULL, TRUE);
			As<pointer[MAX_FRAMES]> stack;
			auto const n = CaptureStackBackTrace(0, MAX_FRAMES, stack, NULL);
			As<char[(sizeof(SYMBOL_INFO) + Frame::SIZE + 1) * sizeof(TCHAR)]> buf;
			SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(buf);
			symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
			symbol->MaxNameLen = Frame::SIZE;
			DWORD offset;
			IMAGEHLP_LINE64 file;
			auto frames = new Frame[n]{};
			Trace trace{frames, n};
			for (usize i = 0; i < n; ++i) {
				DWORD64 address = reinterpret_cast<DWORD64>(stack[i]);
				SymFromAddr(proc, address, NULL, symbol);
				auto& frame = frames[i];
				if (SymGetLineFromAddr64(proc, address, &offset, &file)) {
					auto const len = __builtin_strlen(file.FileName);
					__builtin_memmove(frame.file, file.FileName, (len < Frame::SIZE ? len : Frame::SIZE));
					frame.line = file.LineNumber;
				}
				frame.address = symbol->Address;
				__builtin_memmove(frame.info, symbol->Name, (symbol->NameLen < Frame::SIZE ? symbol->NameLen : Frame::SIZE));
			}
			return trace;
		}
		#else
		[[gnu::unavailable("Unimplemented for non-Windows systems!")]]
		static Trace capture() {}
		#endif

	private:
		Trace(owner<Frame const> const frames, usize const count): frames(frames), count(count) {}
		
		Trace(owner<Frame const> const frames, usize const count, bool): Trace(new Frame[count]{}, frames, count) {}

		Trace(owner<Frame> const frames, owner<Frame const> const source, usize const count): Trace(frames, count) {
			__builtin_memmove(frames, source ,count);
		}
	};

	template <usize F = 256>
	struct Traceable {
		using TraceType = CPP::Stack::Trace<F>;
		TraceType trace;

		__declspec(noinline) Traceable(): trace(TraceType::capture()) {
		}
	};
}

CTL_NAMESPACE_END

#endif