#ifndef CTL_STREAM_MEMORYSTREAM_H
#define CTL_STREAM_MEMORYSTREAM_H

#include "core.hpp"
#include "../container/lists/lists.hpp"
#include "../container/span.hpp"
#include "../container/nullable.hpp"

CTL_NAMESPACE_BEGIN

struct InputMemoryStream: IInputStream<Bytes<>> {
	virtual ~InputMemoryStream() {}

	InputMemoryStream() {}

	InputMemoryStream(ByteSpan<> const& buffer): buffer(buffer) {}

	constexpr Nullable<Bytes<>> tryRead(usize const count) override {
		if (!isOpen()) return null;
		Bytes<> out;
		if (pointer < buffer.size()) {
			auto const span = buffer.sliced(pointer, ((pointer + count) < buffer.size()) ? pointer + count : buffer.size());
			out = Bytes<>(span.begin(), span.end());
			pointer = ((pointer + count) < buffer.size()) ? pointer + count : buffer.size();
		}
		return out;
	}

	constexpr void go(usize const pos = 0) override {
		if (!isOpen()) return;
		pointer = (pos < buffer.size()) ? pos : buffer.size();
	}

	constexpr usize position() const override {
		if (!isOpen()) return -1;
		return pointer;
	}

	constexpr bool isOpen() const override {
		return buffer.size();
	}

private:
	usize		pointer = 0;
	ByteSpan<>	buffer;
};

struct OutputMemoryStream: IOutputStream<Bytes<>> {
	virtual ~OutputMemoryStream() {}

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

	constexpr void go(usize const pos = 0) override {
		if (!isOpen()) return;
		pointer = (pos < buffer.size()) ? pos : buffer.size();
	}

	constexpr usize position() const override {
		if (!isOpen()) return -1;
		return pointer;
	}

	constexpr bool isOpen() const override {
		return buffer.size();
	}

private:
	usize		pointer = 0;
	ByteSpan<>	buffer;
};

CTL_NAMESPACE_END

#endif
