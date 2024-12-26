#ifndef CTL_IO_CONSOLE_H
#define CTL_IO_CONSOLE_H

#include "../namespace.hpp"
#include "../meta/fold.hpp"
#include "../cpperror.hpp"
#include "stream.hpp"

#ifdef CTL_CONSOLE_OUT
#include <iostream>
#endif // CTL_CONSOLE_OUT

CTL_NAMESPACE_BEGIN

/// @brief Console I/O.
struct Console {
	#ifdef CTL_CONSOLE_OUT
	/// @brief Console input stream.
	inline static InputStream<char>		in	= InputStream<char>(std::cin.rdbuf());
	/// @brief Console output stream.
	inline static OutputStream<char>	out	= OutputStream<char>(std::cout.rdbuf());
	#else
	/// @brief Console input stream.
	inline static InputStream<char>		in	= InputStream<char>(nullptr);
	/// @brief Console output stream.
	inline static OutputStream<char>	out	= OutputStream<char>(nullptr);
	#endif // CTL_CONSOLE_OUT

	/// @brief Prints to the output stream.
	/// @tparam ...Args Argument types.
	/// @param ...args Values to print.
	template<typename... Args>
	static void print(Args... args) {
		Fold::strins(out, args...);
	}

	/// @brief Prints to the output stream, followed by a new line.
	/// @tparam ...Args Argument types.
	/// @param ...args Values to print.
	template<typename... Args>
	static void println(Args... args) {
		print(args..., "\n");
	}
	
	/// @brief Synchronous version of `print`.
	/// @tparam ...Args Argument types.
	/// @param ...args Values to print.
	template<typename... Args>
	static void syncprint(Args... args) {
		Fold::strins(std::osyncstream(out), args...);
	}

	/// @brief Synchronous version of `println`.
	/// @tparam ...Args Argument types.
	/// @param ...args Values to print.
	template<typename... Args>
	static void syncprintln(Args... args) {
		syncprint(args..., "\n");
	}

	/// @brief Flushes the output stream.
	static void flush() {
		out.flush();
	}

	/// @brief Gets a value from the input stream.
	/// @tparam T Value type.
	/// @return Value.
	/// @throw MissingStreamException if no stream is attached.
	template<class T>
	static T get() {
		if (!in)
			throw MissingStreamException("No console input/output stream was attached!");
		T val;
		in >> val;
		return val;
	}

	/// @brief Synchronous version of `get`.
	/// @tparam T Value type.
	/// @return Value.
	/// @throw MissingStreamException if no stream is attached.
	template<class T>
	static T syncget() {
		if (!in)
			throw MissingStreamException("No console input/output stream was attached!");
		T val;
		in.sync();
		in >> val;
		return val;
	}
};

CTL_NAMESPACE_END

#ifdef CTL_CONSOLE_OUT
/// @brief Prints to the output stream.
#define DEBUG(...)			CTL::Console::print(__VA_ARGS__)
/// @brief Prints to the output stream, followed by a new line.
#define DEBUGLN(...)		CTL::Console::println(__VA_ARGS__)
	/// @brief Synchronous version of `print`.
#define SYNCDEBUG(...)		CTL::Console::syncprint(__VA_ARGS__)
	/// @brief Synchronous version of `println`.
#define SYNCDEBUGLN(...)	CTL::Console::syncprintln(__VA_ARGS__)
#else
/// @brief Does nothing. Only enabled when the `CTL_CONSOLE_OUT` macro is set.
#define DEBUG(...)
/// @brief Does nothing. Only enabled when the `CTL_CONSOLE_OUT` macro is set.
#define DEBUGLN(...)
/// @brief Does nothing. Only enabled when the `CTL_CONSOLE_OUT` macro is set.
#define SYNCDEBUG(...)
/// @brief Does nothing. Only enabled when the `CTL_CONSOLE_OUT` macro is set.
#define SYNCDEBUGLN(...)
#endif // CTL_CONSOLE_OUT

#endif // CTL_IO_CONSOLE_H
