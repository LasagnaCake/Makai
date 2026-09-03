#ifndef CTL_STREAM_SUBSTREAM_H
#define CTL_STREAM_SUBSTREAM_H

#include "core.hpp"

CTL_NAMESPACE_BEGIN

template <class T>
struct InputSubstream: IInputStream<T> {
	virtual ~InputSubstream() {}

	InputSubstream() {}

	InputSubstream(IInputStream<T>& stream, usize const begin, usize const end): stream(stream), pointer(begin), start(begin), stop(end) {
		stream.go(start);
	}

	constexpr Nullable<T> tryRead(usize const count) override {
		if (!isOpen()) return null;
		Nullable<T> out;
		if (pointer < stop) {
			stream.go(pointer);
			out = stream.tryRead(((pointer + count) < stop) ? count : (stop - pointer));
			pointer = stream.position();
		}
		return out;
	}

	constexpr void go(usize const pos = 0) override {
		if (!isOpen()) return;
		pointer = (pos < stop) ? (pos >= start ? pos : start) : stop;
		stream.go(pointer);
	}

	constexpr usize position() const override {
		if (!isOpen()) return -1;
		return pointer;
	}

	constexpr bool isOpen() const override {
		return stream.isOpen();
	}

	constexpr bool atEnd() const override {
		return stream.atEnd();
	}

private:
	IInputStream<T>&	stream;
	usize				pointer = 0;
	usize const			start, stop;
};

CTL_NAMESPACE_END

#endif
