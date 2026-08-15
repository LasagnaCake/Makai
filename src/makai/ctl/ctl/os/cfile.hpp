#ifndef CTL_STREAM_CFILE_H
#define CTL_STREAM_CFILE_H

#include "core.hpp"
#include "../container/nullable.hpp"
#include <cstdio>

CTL_NAMESPACE_BEGIN

// C-Style file wrapper.
struct CFile {
	constexpr CFile() {}

	constexpr CFile(cstring const path, cstring const mode) {open(path, mode);}

	constexpr CFile(CFile&&)		= delete;
	constexpr CFile(CFile const&)	= delete;

	constexpr CFile& operator=(CFile&&)			= delete;
	constexpr CFile& operator=(CFile const&)	= delete;

	constexpr ~CFile() {close();}

	constexpr bool isOpen() const {
		return file;
	}

	constexpr CFile& open(cstring const path, cstring const mode) {
		if (isOpen()) return *this;
		file = fopen(path, mode);
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

	constexpr usize position() const {
		if (!isOpen()) return -1;
		return ftell(file);
	}

	template <class T>
	constexpr Nullable<usize> tryRead(ref<T> const out, usize const count = 1) {
		if (!isOpen()) return null;
		auto const total = fread(out, 1, count * sizeof(T), file);
		if (ferror(file))
			return null;
		return total;
	}

	template <class T>
	constexpr void tryWrite(ref<T> const data, usize const count = 1) {
		if (!isOpen()) return;
		fwrite(data, 1, count * sizeof(T), file);
		return !ferror(file);
	}

	constexpr ref<FILE> handle() const {return file;}

private:
	ref<FILE> file = nullptr;
};
