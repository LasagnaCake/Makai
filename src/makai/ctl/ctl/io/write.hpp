#ifndef CTL_IO_WRITE_H
#define CTL_IO_WRITE_H

#include "console.hpp"
#include "ansi.hpp"

CTL_NAMESPACE_BEGIN

namespace IO {
	template <class... TArgs>	inline void	write(TArgs const&... args)		{Console::print(toString(args)...);		}
	template <class... TArgs>	inline void	writeLine(TArgs const&... args)	{Console::println(toString(args)...);	}
	template <class T>			inline T	read()							{return Console::get<T>();				}
}

CTL_NAMESPACE_END

#endif
