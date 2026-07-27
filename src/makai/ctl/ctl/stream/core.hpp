#ifndef CTL_STREAM_CORE_H
#define CTL_STREAM_CORE_H

#include "../interface/core.hpp"
#include "../container/nullable.hpp"

CTL_NAMESPACE_BEGIN

struct IStream {
	virtual ~IStream() {}

	constexpr virtual bool isOpen() const		= 0;
	constexpr virtual usize position() const	= 0;
};

template <class T, class TData = typename T::DataType>
struct IInputStream: IStream, IInput<T> {
	virtual ~IInputStream() {}

	constexpr virtual Nullable<T> tryRead(usize const count) = 0;

	constexpr virtual Nullable<T> tryReadAll(usize const chunk = 1024) {
		if (!isOpen()) return null;
		T out, buf;
		while (true) {
			auto const v = tryRead(chunk);
			if (!v) return null;
			buf = v.value();
			out.appendBack(buf);
			if (buf.size() < chunk) break;
		}
		return out;
	}

	constexpr T read(usize const count) override {
		return tryRead(count).orElse(T());
	}

	constexpr virtual T readAll(usize const chunk = 1024) {
		return tryReadAll(chunk).orElse(T());
	}

	constexpr virtual Nullable<T> tryReadUntil(TData const match, usize const chunk = 1024, usize const max = -1) {
		if (!isOpen()) return null;
		T out, buf;
		ssize pos = -1;
		while (true) {
			auto const v = tryRead(chunk);
			if (!v) return null;
			buf = v.value();
			if ((pos = buf.rfind(match)) != -1) {
				out.appendBack(buf.sliced(0, pos));
				break;
			}
			out.appendBack(buf);
			if (buf.size() < chunk) break;
			if (max < out.size()) {
				out.resize(max);
				break;
			}
		}
		return out;
	}

	template <Type::Functional<T(T const&)> TFunc>
	constexpr T operator|(TFunc const& f) {
		return f(readAll());
	}

	constexpr virtual void readInto(ref<TData> const where, usize const count) {
		auto const v = read(count);
		MX::memcpy(where, v.data(), count);
	}
};

template <class T>
struct IOutputStream: IStream, IOutput<T> {
	virtual ~IOutputStream() {}

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
