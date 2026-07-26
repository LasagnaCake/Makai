#ifndef CTL_STREAM_FILESTREAM_H
#define CTL_STREAM_FILESTREAM_H

#include "core.hpp"
#include "../container/strings/strings.hpp"
#include "../container/lists/lists.hpp"
#include "../container/nullable.hpp"
#include <cstdio>

CTL_NAMESPACE_BEGIN

struct File {
	constexpr File() {}

	constexpr File(String const& path, String const& mode) {open(path, mode);}

	constexpr File(File&&)		= delete;
	constexpr File(File const&)	= delete;

	constexpr File& operator=(File&&)		= delete;
	constexpr File& operator=(File const&)	= delete;

	constexpr ~File() {close();}

	constexpr bool isOpen() const {
		return file;
	}

	constexpr File& open(String const& path, String const& mode) {
		if (isOpen()) return *this;
		file = fopen(path.cstr(), mode.cstr());
		return *this;
	}

	constexpr File& close() {
		if (!isOpen()) return *this;
		if (file) fclose(file);
		file = nullptr;
		return *this;
	}

	constexpr ref<FILE> handle() const {return file;}

private:
	ref<FILE> file = nullptr;
};

template <Type::OneOf<String, Bytes<>> T>
struct InputFileStream: IInputStream<T> {
	constexpr static bool const BINARY = Type::Equal<T, Bytes<>>;

	InputFileStream() {}

	InputFileStream(String const& path) {open(path);}

	constexpr InputFileStream& open(String const& path) {
		file.open(path, BINARY ? "rb" : "r");
	}

	constexpr InputFileStream& close() {
		file.close();
		return *this;
	}

	constexpr virtual Nullable<T> tryRead(usize const count) override {
		if (!isOpen()) return null;
		T out;
		out.resize(count, 0);
		auto const total = fread(out.data(), 1, count, file.handle());
		if (ferror(file.handle()))
			return null;
		return out.resize(total);
	}

	constexpr Nullable<T> tryReadUntil(typename T::DataType const& match, usize const chunk = 1024, usize const max = -1) {
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
		fseek(pos, file.handle(), SEEK_SET);
	}

	constexpr virtual usize position() const override {
		if (!isOpen()) return -1;
		return ftell(file.handle());
	}

	constexpr virtual bool isOpen() const override {
		return file.isOpen();
	}

private:
	File file;
};

template <Type::OneOf<String, Bytes<>> T>
struct OutputFileStream: IInputStream<T> {
	constexpr static bool const BINARY = Type::Equal<T, Bytes<>>;

	OutputFileStream() {}

	OutputFileStream(String const& path, bool const append = false) {open(path, append);}

	constexpr OutputFileStream& open(String const& path, bool const append = false) {
		if (isOpen()) return *this;
		if (append)
			file.open(path.cstr(), BINARY ? "ab" : "a");
		else file.open(path.cstr(), BINARY ? "wb" : "w");
	}

	constexpr OutputFileStream& close() {
		file.close();
		return *this;
	}

	constexpr void write(T const& value) override {
		if (!isOpen()) return;
		fwrite(value.data(), 1, value.size(), file);
	}

	constexpr virtual void go(usize const pos = 0) override {
		if (!isOpen()) return;
		fseek(pos, file.handle(), SEEK_SET);
	}

	constexpr virtual usize position() const override {
		if (!file.isOpen()) return -1;
		return ftell(file.handle());
	}

	constexpr virtual bool isOpen() const override {
		return file.isOpen();
	}

private:
	File file;
};

CTL_NAMESPACE_END

#endif
