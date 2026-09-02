#ifndef CTL_STREAM_FILESTREAM_H
#define CTL_STREAM_FILESTREAM_H

#include "core.hpp"
#include "../os/cfile.hpp"
#include "../container/strings/strings.hpp"
#include "../container/lists/lists.hpp"
#include "../container/nullable.hpp"
#include <cstdio>

CTL_NAMESPACE_BEGIN

template <Type::OneOf<String, Bytes<>> T>
struct InputFileStream: IInputStream<T> {
	constexpr static bool const BINARY = Type::Equal<T, Bytes<>>;

	using IInputStream<T>::read;

	virtual ~InputFileStream() {}

	InputFileStream() {}

	InputFileStream(String const& path) {open(path);}

	constexpr InputFileStream& open(String const& path) {
		file.open(path.cstr(), BINARY ? "rb" : "r");
		return *this;
	}

	constexpr InputFileStream& close() {
		file.close();
		return *this;
	}

	constexpr Nullable<T> tryRead(usize const count) override {
		if (!isOpen()) return null;
		T out;
		out.resize(count, 0);
		if (auto const total = file.tryRead(out.data(), count))
			return out.resize(*total);
		return null;
	}

	constexpr void go(usize const pos = 0) override {
		if (!isOpen()) return;
		file.go(pos);
	}

	constexpr usize position() const override {
		if (!isOpen()) return -1;
		return ftell(file.handle());
	}

	constexpr bool isOpen() const override {
		return file.isOpen();
	}

	constexpr bool atEnd() const override {
		return file.atEnd();
	}

private:
	CFile file;
};

template <Type::OneOf<String, Bytes<>> T>
struct OutputFileStream: IOutputStream<T> {
	virtual ~OutputFileStream() {}

	constexpr static bool const BINARY = Type::Equal<T, Bytes<>>;

	OutputFileStream() {}

	OutputFileStream(String const& path, bool const append = false) {open(path, append);}

	constexpr OutputFileStream& open(String const& path, bool const append = false) {
		if (isOpen()) return *this;
		if (append)
			file.open(path.cstr(), BINARY ? "ab" : "a");
		else file.open(path.cstr(), BINARY ? "wb" : "w");
		return *this;
	}

	constexpr OutputFileStream& close() {
		file.close();
		return *this;
	}

	constexpr void write(T const& value) override {
		if (!isOpen()) return;
		file.tryWrite(value.data(), value.size());
	}

	constexpr void go(usize const pos = 0) override {
		if (!isOpen()) return;
		file.go(pos);
	}

	constexpr usize position() const override {
		if (!file.isOpen()) return -1;
		return ftell(file.handle());
	}

	constexpr bool isOpen() const override {
		return file.isOpen();
	}

	constexpr bool atEnd() const override {
		return file.atEnd();
	}

private:
	CFile file;
};

using InputByteFileStream = InputFileStream<Bytes<>>;
using InputTextFileStream = InputFileStream<String>;

using OutputByteFileStream = OutputFileStream<Bytes<>>;
using OutputTextFileStream = OutputFileStream<String>;

CTL_NAMESPACE_END

#endif
