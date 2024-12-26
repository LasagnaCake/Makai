#include "file.hpp"

#include "../tool/archive/archive.hpp"

#include <fstream>
#include <sstream>
#include <filesystem>

#define IMPL_ARCHIVE_

#if !(defined(MAKAILIB_DEBUG) || defined(MAKAILIB_ARCHIVE_DISABLED))
#define IMPL_ARCHIVE_
#endif

using namespace Makai;
using Makai::Tool::Arch::FileArchive;

#ifdef IMPL_ARCHIVE_
enum class ArchiveState {
	FAS_CLOSED,
	FAS_LOADING,
	FAS_OPEN
};

ArchiveState& state() {
	static ArchiveState s = ArchiveState::FAS_CLOSED;
	return s;
}

FileArchive& archive() {
	static FileArchive arc;
	return arc;
}
#endif

[[noreturn]] void fileLoadError(String const& path, String const& reason) {
	throw Makai::File::FileLoadError(
		"Could not load file '" + path + "'!",
		reason,
		"none",
		CTL_CPP_UNKNOWN_SOURCE
	);
}

[[noreturn]] void fileSaveError(String const& path, String const& reason) {
	throw Makai::File::FileLoadError(
		"Could not save file '" + path + "'!",
		reason,
		"none",
		CTL_CPP_UNKNOWN_SOURCE
	);
}

void assertFileExists(String const& path) {
	if (!OS::FS::exists(path))
		fileLoadError(path, toString("File or directory '", path, "' does not exist!"));
}

void Makai::File::attachArchive(String const& path, String const& password) try {
	assertFileExists(path);
	static FileBuffer buffer;
	if (buffer.is_open()) buffer.close();
	buffer.open(path.cstr(), std::ios::in | std::ios::binary);
	Makai::File::attachArchive(buffer, password);
} catch (std::runtime_error const& e) {
	fileLoadError(path, e.what());
}

void Makai::File::attachArchive(DataBuffer& buffer, String const& password) {
	#ifdef IMPL_ARCHIVE_
	DEBUGLN("Attaching archive...");
	if (state() == ArchiveState::FAS_LOADING)
		throw Error::FailedAction("Other archive is being loaded!", CTL_CPP_PRETTY_SOURCE);
	try {
		state() = ArchiveState::FAS_LOADING;
		archive().close();
		archive().open(buffer, password);
		state() = ArchiveState::FAS_OPEN;
		DEBUGLN("Archive Attached!");
	} catch (...) {
		DEBUGLN("Archive attachment failed!");
	}
	#endif
}

bool Makai::File::isArchiveAttached() {
	#ifdef IMPL_ARCHIVE_
	return state() == ArchiveState::FAS_OPEN;
	#else
	return false;
	#endif
}

[[gnu::destructor]] void Makai::File::detachArchive() {
	#ifdef IMPL_ARCHIVE_
	DEBUGLN("Detaching archive...");
	archive().close();
	state() = ArchiveState::FAS_CLOSED;
	DEBUGLN("Archive detached!");
	#endif
}

#ifdef IMPL_ARCHIVE_
void assertArchive(String const& path) {
	if (!Makai::File::isArchiveAttached())
		fileLoadError(path, "Archive is not attached!");
}

[[noreturn]] void fileGetError(String const& path, String const& fe, String const& ae) {
	fileLoadError(
		path,
		toString(
			"\nMultiple possibilities!\n\n",
			"[[ FOLDER ]]\n", fe, "\n",
			"[[ ARCHIVE ]]\n", ae, "\n"
		)
	);
}
#endif

void setExceptionMask(std::ios& stream) {
	#ifndef MAKAILIB_FILE_GET_NO_EXCEPTIONS
	stream.exceptions(std::ios::failbit | std::ios::badbit);
	//stream.exceptions(std::ios::badbit);
	//stream.exceptions(std::ios::failbit);
	#endif
}

/*
// If all else fails, the horrors
template<class T>
void readFile(String const& path, T& buf) {
	FILE* file = fopen(path.cstr(), "rb");
	if (!file)							fileLoadError(path, "File open error");
	if (fseek(file, 0, SEEK_END))		fileLoadError(path, "File seek error");
	ssize sz;
	if ((sz = ftell(file)) == -1)		fileLoadError(path, "File tell error");
	if (fseek(file, 0, SEEK_SET))		fileLoadError(path, "File rewind error");
	buf.reserve(sz);
	if (fread(buf.data(), sz, 1, file))	fileLoadError(path, "File read error");
	if (fclose(file))					fileLoadError(path, "File close error");
}
*/

String Makai::File::loadText(String const& path) {
	// Ensure directory exists
	assertFileExists(path);
	try {
		// The file and its contents
		String content;
		std::ifstream file;		// This line SEGFAULTS on init() for -o1 and above
		// Ensure ifstream object can throw exceptions
		setExceptionMask(file);
		// Open file
		file.open(path.cstr());
		std::stringstream stream;
		// Read file's buffer contents into stringstream
		stream << file.rdbuf();
		// Convert stream into string
		content = String(stream.str());
		// Close file handler
		file.close();
		// Return contents
		return content;
	} catch (std::exception const& e) {
		fileLoadError(path, e.what());
	}
	// Return contents
	return "";
}

BinaryData<> Makai::File::loadBinary(String const& path) {
	// Ensure directory exists
	assertFileExists(path);
	try {
		// Try and open the file
		std::ifstream file;
		// Ensure ifstream object can throw exceptions
		setExceptionMask(file);
		// Open file
		file.open(path.cstr(), /*std::ios::ate | */std::ios::binary);
		// Get file size
		usize const fsize = std::filesystem::file_size(path.cstr());
		//file.seekg(0);
		// Preallocate data
		BinaryData<> data;
		data.resize(fsize, 0);
		// Read & close file
		if(static_cast<std::streamsize>(fsize) != file.readsome((char*)data.data(), fsize))
			fileLoadError(path, "Failure to read entire file!");
		file.close();
		// Return data
		return data;
	} catch (std::exception const& e) {
		fileLoadError(path, e.what());
	}
	return BinaryData<>();
}

Makai::File::CSVData Makai::File::loadCSV(String const& path, char const delimiter) {
	// The file and its contents
	String content = Makai::File::loadText(path);
	// Get values
	return content.split(delimiter);
}

void Makai::File::saveBinary(String const& path, CTL::ByteSpan<> const& data) {
	OS::FS::makeDirectory(OS::FS::directoryFromPath(path));
	// Try and save data
	try {
		std::ofstream file(path.cstr(), std::ios::binary);
		if (!file) fileLoadError(path, "Mysterious error");
		// Ensure ofstream object can throw exceptions
		setExceptionMask(file);
		// Write data to file
		file.write((char*)data.data(), data.size());
		file.flush();
		file.close();
	} catch (std::exception const& e) {
		fileSaveError(path, e.what());
	}
}

void Makai::File::saveBinary(String const& path, BinaryData<> const& data) {
	Makai::File::saveBinary(path, ByteSpan<>((ubyte*)data.data(), data.size()));
}

void Makai::File::saveText(String const& path, String const& text) {
	OS::FS::makeDirectory(OS::FS::directoryFromPath(path));
	// Try and save data
	try {
		std::ofstream file(path.cstr(), std::ios::trunc);
		if (!file) fileLoadError(path, "Mysterious error");
		// Ensure ofstream object can throw exceptions
		setExceptionMask(file);
		// Write data to file
		file.write(text.data(), text.size());
		file.flush();
		file.close();
	} catch (std::exception const& e) {
		fileSaveError(path, e.what());
	}
}

String Makai::File::loadTextFromArchive(String const& path) {
	#ifdef IMPL_ARCHIVE_
	assertArchive(path);
	return archive().getTextFile(OS::FS::childPath(path));
	#else
	fileLoadError(path, "Archive functionality disabled!");
	#endif
}

BinaryData<> Makai::File::loadBinaryFromArchive(String const& path) {
	#ifdef IMPL_ARCHIVE_
	assertArchive(path);
	return archive().getBinaryFile(OS::FS::childPath(path));
	#else
	fileLoadError(path, "Archive functionality disabled!");
	#endif
}

Makai::File::CSVData Makai::File::loadCSVFromArchive(String const& path, char const delimiter) {
	#ifdef IMPL_ARCHIVE_
	assertArchive(path);
	return loadTextFromArchive(path).split(delimiter);
	#else
	fileLoadError(path, "Archive functionality disabled!");
	#endif
}

String Makai::File::getText(String const& path) {
	#ifdef IMPL_ARCHIVE_
	String res;
	if (isArchiveAttached())
		try {
			DEBUGLN("[ARC] Loading text file...");
			res = Makai::File::loadTextFromArchive(path);
		} catch (FileLoadError const& ae) {
			try {
				DEBUGLN("[FLD-2] Loading text file...");
				res = Makai::File::loadText(path);
			} catch (FileLoadError const& fe) {
				fileGetError(path, fe.summary(), ae.summary());
			}
		}
	else try {
		DEBUGLN("[FLD-1] Loading text file...");
		res = Makai::File::loadText(path);
	} catch (FileLoadError const& e) {
		fileGetError(path, e.summary(), "Archive not attached!");
	}
	return res;
	#else
	return Makai::File::loadText(path);
	#endif
}

BinaryData<> Makai::File::getBinary(String const& path) {
	#ifdef IMPL_ARCHIVE_
	BinaryData<> res;
	if (isArchiveAttached())
		try {
			DEBUGLN("[ARC] Loading binary file...");
			res = Makai::File::loadBinaryFromArchive(path);
		} catch (FileLoadError const& ae) {
			try {
				DEBUGLN("[FLD-2] Loading binary file...");
				res = Makai::File::loadBinary(path);
			} catch (FileLoadError const& fe) {
				fileGetError(path, fe.summary(), ae.summary());
			}
		}
	else try {
		DEBUGLN("[FLD-1] Loading binary file...");
		res = Makai::File::loadBinary(path);
	} catch (FileLoadError const& e) {
		fileGetError(path, e.summary(), "Archive not attached!");
	}
	return res;
	#else
	return Makai::File::loadBinary(path);
	#endif
}

Makai::File::CSVData Makai::File::getCSV(String const& path, char const delimiter) {
	#ifdef IMPL_ARCHIVE_
	CSVData res;
	if (isArchiveAttached())
		try {
			DEBUGLN("[ARC] Loading CSV file...");
			res = Makai::File::loadCSVFromArchive(path);
		} catch (FileLoadError const& ae) {
			try {
				DEBUGLN("[FLD-2] Loading CSV file...");
				res = Makai::File::loadCSV(path);
			} catch (FileLoadError const& fe) {
				fileGetError(path, fe.summary(), ae.summary());
			}
		}
	else try {
		DEBUGLN("[FLD-1] Loading CSV file...");
		res = Makai::File::loadCSV(path);
	} catch (FileLoadError const& e) {
		fileGetError(path, e.summary(), "Archive not attached!");
	}
	return res;
	#else
	return Makai::File::loadCSV(path);
	#endif
}
