#ifndef CTL_STREAM_FILESTREAM_H
#define CTL_STREAM_FILESTREAM_H

#include "core.hpp"
#include "../container/strings/strings.hpp"
#include "../container/lists/lists.hpp"
#include "../container/nullable.hpp"
#include <cstdio>

CTL_NAMESPACE_BEGIN

struct CFile {
	constexpr CFile() {}

	constexpr CFile(String const& path, String const& mode) {open(path, mode);}

	constexpr CFile(CFile&&)		= delete;
	constexpr CFile(CFile const&)	= delete;

	constexpr CFile& operator=(CFile&&)			= delete;
	constexpr CFile& operator=(CFile const&)	= delete;

	constexpr ~CFile() {close();}

	constexpr bool isOpen() const {
		return file;
	}

	constexpr CFile& open(String const& path, String const& mode) {
		if (isOpen()) return *this;
		file = fopen(path.cstr(), mode.cstr());
		return *this;
	}

	constexpr CFile& close() {
		if (!isOpen()) return *this;
		if (file) fclose(file);
		file = nullptr;
		return *this;
	}

	constexpr CFile& go(usize const pos) {
		if (!isOpen()) return *this;
		fseek(file, pos, SEEK_SET);
		return *this;
	}

	constexpr ref<FILE> handle() const {return file;}

private:
	ref<FILE> file = nullptr;
};

template <Type::OneOf<String, Bytes<>> T>
struct InputFileStream: IInputStream<T> {
	constexpr static bool const BINARY = Type::Equal<T, Bytes<>>;

	using IInputStream<T>::read;

	virtual ~InputFileStream() {}

	InputFileStream() {}

	InputFileStream(String const& path) {open(path);}

	constexpr InputFileStream& open(String const& path) {
		file.open(path, BINARY ? "rb" : "r");
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
		auto const total = fread(out.data(), 1, count, file.handle());
		if (ferror(file.handle()))
			return null;
		return out.resize(total);
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
		fwrite(value.data(), 1, value.size(), file.handle());
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

private:
	CFile file;
};

using InputByteFileStream = InputFileStream<Bytes<>>;
using InputTextFileStream = InputFileStream<String>;

using OutputByteFileStream = OutputFileStream<Bytes<>>;
using OutputTextFileStream = OutputFileStream<String>;

CTL_NAMESPACE_END

#endif
