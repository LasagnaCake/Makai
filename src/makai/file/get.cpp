#include "file.hpp"

#include "../tool/archive/archive.hpp"

#include <exception>
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

static Atomic<ArchiveState>& state() {
	static Atomic<ArchiveState> s = ArchiveState::FAS_CLOSED;
	return s;
}

static FileArchive& archive() {
	static FileArchive arc;
	return arc;
}
#endif

[[noreturn]] static void emptyPathError() {
	throw Makai::Error::InvalidValue(
		"File path is empty!",
		CTL_CPP_UNKNOWN_SOURCE
	);
}

[[noreturn]] static void invalidPathError(String const& path, String const& sanitized) {
	throw Makai::Error::InvalidValue(
		"Path '"+path+"' contains invalid characters!",
		"('*' are where invalid characters are)\n"+sanitized,
		CTL_CPP_UNKNOWN_SOURCE
	);
}

/*
[[noreturn]] static void nullBoundPathError(String const& path) {
	throw Makai::Error::InvalidValue(
		"Path '" + path + "' contains null characters!",
		CTL_CPP_UNKNOWN_SOURCE
	);
}
*/

[[noreturn]] static void fileLoadError(String const& path, String const& reason) {
	throw Makai::File::FileLoadError(
		"Could not load file '" + path + "'!",
		reason,
		"none",
		CTL_CPP_UNKNOWN_SOURCE
	);
}

[[noreturn]] static void fileSaveError(String const& path, String const& reason) {
	throw Makai::File::FileLoadError(
		"Could not save file '" + path + "'!",
		reason,
		"none",
		CTL_CPP_UNKNOWN_SOURCE
	);
}

constexpr bool isOtherStuffChar(char const c) {
	return (
		c == '_'
	||	c == '-'
	||	c == '/'
	||	c == '\\'
	||	c == '.'
	||	c == ' '
	||	c == '('
	||	c == ')'
	||	c == '+'
	||	c == ':'
	);
}

constexpr bool isInvalidPathChar(char const c) {
	return (
		c == '|'
	||	c == '>'
	||	c == '<'
	||	c == '?'
	||	c == '*'
	||	c == '"'
	||	(c >= '\x00' && c <= '\x31')
	);
}

constexpr static bool isValidPathChar(char const c) {
	return (
		isAlphanumericChar(c)
	||	isOtherStuffChar(c)
	);
}

static String sanitizedForDisplay(String path) {
	for (char& c : path)
		if (!isValidPathChar(c))
			c = '*';
	return path;
}

static void assertPathIsValid(String const& path) {
	if (path.empty() || path.isNullOrSpaces())
		emptyPathError();
	if (!path.validate(isValidPathChar))
		invalidPathError(path, sanitizedForDisplay(path));
}

static void assertFileExists(String const& path) {
	assertPathIsValid(path);
	if (!OS::FS::exists(path))
		fileLoadError(path, toString("File or directory '", path, "' does not exist!"));
}

void Makai::File::attachArchive(String const& path, String const& password) {
	assertFileExists(path);
	Makai::File::attachArchive(Tool::Arch::FileArchive::Source::create<InputByteFileStream>(path), password);
}

void Makai::File::attachArchive(Tool::Arch::FileArchive::Source&& buffer, String const& password) {
	#ifdef IMPL_ARCHIVE_
	DEBUGLN("Attaching archive...");
	if (state() == ArchiveState::FAS_LOADING)
		throw Error::FailedAction("Other archive is being loaded!", CTL_CPP_PRETTY_SOURCE);
	try {
		state() = ArchiveState::FAS_LOADING;
		archive().close();
		archive().open(buffer.transfer(), password);
		state() = ArchiveState::FAS_OPEN;
		DEBUGLN("Archive Attached!");
	} catch (Error::Generic const& e) {
		DEBUGLN("Archive attachment failed!");
		DEBUGLN("Reason: ", e.report());
	} catch (std::exception const& e) {
		DEBUGLN("Archive attachment failed!");
		DEBUGLN("Reason: ", e.what());
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
static void assertArchive(String const& path) {
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

String Makai::File::loadText(String const& path) {
	assertPathIsValid(path);
	// Ensure directory exists
	assertFileExists(path);
	// Try and read file
	String content;
	InputFileStream<String> file{path};
	if (!file.isOpen())
		fileLoadError(path, "Failed to open file!");
	if (auto const v = file.tryReadAll())
		content = v.value();
	else fileLoadError(path, "Failed to read contents!");
	// Return contents
	return content;
}

BinaryData<> Makai::File::loadBinary(String const& path) {
	assertPathIsValid(path);
	// Ensure directory exists
	assertFileExists(path);
	// Try and read file
	Bytes<> content;
	InputFileStream<Bytes<>> file{path};
	if (!file.isOpen())
		fileLoadError(path, "Failed to open file!");
	if (auto const v = file.tryReadAll())
		content = v.value();
	else fileLoadError(path, "Failed to read contents!");
	// Return contents
	return content;
}

Makai::File::CSVData Makai::File::loadCSV(String const& path, char const delimiter) {
	// Try and read file
	StringList content;
	InputFileStream<String> file{path};
	if (!file.isOpen())
		fileLoadError(path, "Failed to open file!");
	if (auto const v = file.tryReadUntil(delimiter))
		content.pushBack(v.value());
	else fileLoadError(path, "Failed to read contents!");
	// Return contents
	return content;
}

void Makai::File::saveBinary(String const& path, CTL::ByteSpan<> const& data) {
	assertPathIsValid(path);
	try {OS::FS::makeDirectory(OS::FS::directoryFromPath(path));} catch (...) {}
	// Try and save data
	OutputFileStream<Bytes<>> file{path};
	if (!file.isOpen())
		fileSaveError(path, "Failed to open file!");
	file.write(Bytes<>(data));
}

void Makai::File::saveBinary(String const& path, BinaryData<> const& data) {
	assertPathIsValid(path);
	try {OS::FS::makeDirectory(OS::FS::directoryFromPath(path));} catch (...) {}
	// Try and save data
	OutputFileStream<Bytes<>> file{path};
	if (!file.isOpen())
		fileSaveError(path, "Failed to open file!");
	file.write(data);
}

void Makai::File::saveText(String const& path, String const& text) {
	assertPathIsValid(path);
	try {OS::FS::makeDirectory(OS::FS::directoryFromPath(path));} catch (...) {}
	// Try and save data
	OutputFileStream<String> file{path};
	if (!file.isOpen())
		fileSaveError(path, "Failed to open file!");
	file.write(text);
}

String Makai::File::loadTextFromArchive(String const& path) {
	assertPathIsValid(path);
	#ifdef IMPL_ARCHIVE_
	assertArchive(path);
	return archive().getTextFile(Regex::replace(path, "^(.*?)[\\\\\\/]", ""));
	#else
	fileLoadError(path, "Archive functionality disabled!");
	#endif
}

BinaryData<> Makai::File::loadBinaryFromArchive(String const& path) {
	assertPathIsValid(path);
	#ifdef IMPL_ARCHIVE_
	assertArchive(path);
	return archive().getBinaryFile(Regex::replace(path, "^(.*?)[\\\\\\/]", ""));
	#else
	fileLoadError(path, "Archive functionality disabled!");
	#endif
}

Makai::File::CSVData Makai::File::loadCSVFromArchive(String const& path, char const delimiter) {
	assertPathIsValid(path);
	#ifdef IMPL_ARCHIVE_
	assertArchive(path);
	return loadTextFromArchive(path).split(delimiter);
	#else
	fileLoadError(path, "Archive functionality disabled!");
	#endif
}

String Makai::File::getText(String const& path) {
	assertPathIsValid(path);
	#ifdef IMPL_ARCHIVE_
	String res;
	DEBUGLN("Getting text file '" + path + "'...");
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
	assertPathIsValid(path);
	#ifdef IMPL_ARCHIVE_
	BinaryData<> res;
	DEBUGLN("Getting binary file '" + path + "'...");
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
	assertPathIsValid(path);
	#ifdef IMPL_ARCHIVE_
	CSVData res;
	DEBUGLN("Getting CSV file '" + path + "'...");
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
