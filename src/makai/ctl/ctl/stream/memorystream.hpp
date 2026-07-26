#ifndef CTL_STREAM_MEMORYSTREAM_H
#define CTL_STREAM_MEMORYSTREAM_H

#include "core.hpp"
#include "../container/lists/lists.hpp"
#include "../container/span.hpp"
#include "../container/nullable.hpp"

CTL_NAMESPACE_BEGIN

struct InputMemoryStream: IInputStream<Bytes<>> {
	InputMemoryStream() {}

	InputMemoryStream(ByteSpan<> const& buffer): buffer(buffer) {}

	constexpr Nullable<Bytes<>> tryRead(usize const count) override {
		if (!isOpen()) return null;
		Bytes<> out;
		if (pointer < buffer.size()) {
			out = buffer.sliced(pointer, ((pointer + count) < buffer.size()) ? pointer + count : buffer.size());
			pointer = ((pointer + count) < buffer.size()) ? pointer + count : buffer.size();
		}
		return out;
	}

	constexpr Nullable<T> tryReadAll(usize const chunk = 1024) {
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

	constexpr Nullable<T> tryReadUntil(byte const match, usize const chunk = 1024, usize const max = -1) {
		if (!isOpen()) return null;
		T out, buf;
		ssize pos = -1;
		while (true) {
			auto const v = read(chunk);
			if (!v) return null;
			buf = v.value();
			if ((pos = buf.rfind(match)) != -1) {
				out += buf.sliced(0, pos);
				break;
			}
			out += buf;
			if (buf.size() < chunk) break;
			if (max < out.size()) {
				out.resize(max);
				break;
			}
		}
		return out;
	}

	constexpr virtual void go(usize const pos = 0) override {
		if (!isOpen()) return;
		buffer = (pos < buffer.size()) ? pos : buffer.size();
	}

	constexpr virtual usize position() const override {
		if (!isOpen()) return -1;
		return pointer;
	}

	constexpr virtual bool isOpen() const override {
		return buffer.size();
	}

private:
	usize		pointer = 0;
	ByteSpan<>	buffer;
};

struct OutputMemoryStream: IOutputStream<Bytes<>> {
	OutputMemoryStream() {}

	OutputMemoryStream(ByteSpan<> const& buffer): buffer(buffer) {}

	constexpr void write(Bytes<> const& value) override {
		if (!isOpen()) return;
		if (!(pointer < buffer.size())) return;
		MX::memcpy(
			buffer.data() + pointer,
			value.data(),
			((pointer + value.size()) < buffer.size()) ? value.size() : (buffer.size() - pointer)
		);
		pointer += value.size();
		if (buffer.size() < pointer)
			pointer = buffer.size();
	}

	constexpr virtual void go(usize const pos = 0) override {
		if (!isOpen()) return;
		buffer = (pos < buffer.size()) ? pos : buffer.size();
	}

	constexpr virtual usize position() const override {
		if (!isOpen()) return -1;
		return pointer;
	}

	constexpr virtual bool isOpen() const override {
		return buffer.size();
	}

private:
	usize		pointer = 0;
	ByteSpan<>	buffer;
};

CTL_NAMESPACE_END

#endif
