#ifndef CTL_STREAM_CORE_H
#define CTL_STREAM_CORE_H

#include "../interface/core.hpp"
#include "../container/nullable.hpp"

CTL_NAMESPACE_BEGIN

struct IStream {
	constexpr virtual bool isOpen() const		= 0;
	constexpr virtual usize position() const	= 0;
};

template <class T>
struct IInputStream: IInput<T>, IStream {
	constexpr virtual Nullable<T> tryRead(usize const count) = 0;

	constexpr virtual Nullable<T> tryReadAll(usize const chunk = 1024) {
		if (!isOpen()) return null;
		T out, buf;
		while (true) {
			auto const v = read(chunk);
			if (!v) return null;
			buf = v.value();
			out += buf;
			if (buf.size() < chunk) break;
		}
		return out;
	}

	constexpr T read(usize const count) override {
		return tryRead(count).orElse(T());
	}

	constexpr T readAll(usize const chunk = 1024) {
		return tryReadAll(count).orElse(T());
	}

	template <Type::Functional<T(T const&)> TFunc>
	constexpr T operator|(TFunc const& f) {
		return f(readAll());
	}
};

template <class T>
struct IOutputStream: IOutput<T>, IStream {
	constexpr friend IOutputStream& operator|(T const& val, IOutputStream& self) {
		self.write(val);
	}

	constexpr friend IOutputStream& operator|(IInputStream<T>& val, IOutputStream& self) {
		self.write(val.readAll());
	}
};

template <class TInputStream, class TOutputStream>
struct Pipe {
	TInputStream	input;
	TOutputStream	output;

	constexpr bool isOpen() const {
		return input.isOpen() && output.isOpen();
	}
};

CTL_NAMESPACE_END

#endif
