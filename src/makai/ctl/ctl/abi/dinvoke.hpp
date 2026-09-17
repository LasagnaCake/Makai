#ifndef CTL_ABI_DINVOKE_HPP
#define CTL_ABI_DINVOKE_HPP

#include "../namespace.hpp"
#include "../typetraits/traits.hpp"
#include "../memory/memoryslice.hpp"
#include "../memory/core.hpp"

CTL_NAMESPACE_BEGIN

namespace ABI::Dyn {
	struct Arguments {
		template <class T>
		void add(T const& value) {
			MemorySlice<char> buf;
			buf.invoke(args.size() + alignedSize<T>());
			MX::memmove(buf.data(), args.data(), args.size());
			MX::memmove(buf.data() + args.size(), &value, sizeof(T));
			swap(buf, args);
		}

		ref<void> data()				{return args.data();}
		ref<void const> data() const	{return args.data();}

		usize size() const {return args.byteSize();}

		template <class T>
		consteval static usize alignedSize() {
			usize align = 1;
			usize size = sizeof(T);
			if (size < sizeof(int))
				size = sizeof(int);
			return (size / align) * align;
		}

	private:
		MemorySlice<char> args;
	};

	template <class TReturn>
	inline TReturn invoke(ref<void(...)> const fn, Arguments& args) {
		__builtin_return(__builtin_apply(fn, args.data(), args.size()));
	}
}

CTL_NAMESPACE_END

#endif
