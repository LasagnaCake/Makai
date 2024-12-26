#ifndef MAKAILIB_FILE_GET_H
#define MAKAILIB_FILE_GET_H

#include "../compat/ctl.hpp"

/// @brief File reading/writing facilities.
namespace Makai::File {
	// Until this puzzle is figured, this shall do
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wreturn-type"

	DEFINE_ERROR_TYPE(FileLoadError);
	DEFINE_ERROR_TYPE(FileSaveError);

	/// @brief CSV file contents.
	using CSVData		= StringList;

	/// @brief Attaches a file archive for the application to read.
	/// @param buffer Stream buffer to read from.
	/// @param password Archive password.
	/// @throw Error::FailedAction if another archive is being loaded.
	void attachArchive(DataBuffer& buffer, String const& password = "");
	/// @brief Attaches a file archive for the application to read.
	/// @param path Path to file archive.
	/// @param password Archive password.
	void attachArchive(String const& path, String const& password = "");

	/// @brief Returns whether there is a file archive present.
	/// @return Whether there is an attached file archive.
	bool isArchiveAttached();

	/// @brief Detaches the currently attached archive, if one is attached.
	[[gnu::destructor]] void detachArchive();

	/// @brief Reads a text file to disk.
	/// @param path Path to file.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	String loadText(String const& path);
	/// @brief Writes a text file to disk.
	/// @param path Path to file.
	/// @param text File contents.
	/// @throw Makai::File::FileSaveError on file save errors.
	void saveText(String const& path, String const& text);

	/// @brief Reads a binary file to disk.
	/// @param path Path to file.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	BinaryData<> loadBinary(String const& path);
	/// @brief Writes a binary file to disk.
	/// @param path Path to file.
	/// @param data File contents.
	/// @throw Makai::File::FileSaveError on file save errors.
	void saveBinary(String const& path, ByteSpan<> const& data);
	/// @brief Writes a binary file to disk.
	/// @param path Path to file.
	/// @param data File contents.
	/// @throw Makai::File::FileSaveError on file save errors.
	void saveBinary(String const& path, BinaryData<> const& data);

	/// @brief Writes a binary file to disk.
	/// @tparam T Value type.
	/// @param path Path to file.
	/// @param data Data to write.
	/// @param count Size of data to write.
	/// @throw Makai::File::FileSaveError on file save errors.
	template<Makai::Type::NonVoid T>
	inline void saveBinary(String const& path, T* const data, usize const count) {
		saveBinary(path, ByteSpan((ubyte*)data, count * sizeof(T)));
	}

	/// @brief Writes a binary file to disk.
	/// @tparam T Value type.
	/// @param data Data to write.
	/// @throw Makai::File::FileSaveError on file save errors.
	template<Makai::Type::NonVoid T>
	inline void saveBinary(String const& path, Span<T> const& data) {
		saveBinary(path, data.data(), data.size());
	}

	/// @brief Writes a binary file to disk.
	/// @tparam T Value type.
	/// @param data Data to write.
	/// @throw Makai::File::FileSaveError on file save errors.
	template<Makai::Type::NonVoid T>
	inline void saveBinary(String const& path, List<T> const& data) {
		saveBinary(path, data.data(), data.size());
	}

	/// @brief Writes a single object to a binary file to disk.
	/// @tparam T Value type.
	/// @param data Data to write.
	/// @throw Makai::File::FileSaveError on file save errors.
	template<Makai::Type::NonVoid T>
	inline void saveBinary(String const& path, T const& data) {
		saveBinary(path, ByteSpan(&data, sizeof(T)));
	}

	/// @brief Loads a CSV file from disk.
	/// @param path Path to file.
	/// @param delimiter Delimiter used in the file.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	CSVData loadCSV(String const& path, char const delimiter = ',');
	//CSVData saveCSV(String const& path, CSVData const& data, char const delimiter = ',');

	/// @brief Loads a text file from the attached archive.
	/// @param path Path to file.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	String loadTextFromArchive(String const& path);

	/// @brief Loads a binary file from the attached archive.
	/// @param path Path to file.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	BinaryData<> loadBinaryFromArchive(String const& path);

	/// @brief Loads a CSV file from the attached archive.
	/// @param path Path to file.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	CSVData loadCSVFromArchive(String const& path, char const delimiter = ',');

	/// @brief Loads a text file. Will try to load from attached archive. If that fails, tries to load from disk.
	/// @param path Path to file.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	String getText(String const& path);

	/// @brief Loads a binary file. Will try to load from attached archive. If that fails, tries to load from disk.
	/// @param path Path to file.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	BinaryData<> getBinary(String const& path);

	/// @brief Loads a CSV file. Will try to load from attached archive. If that fails, tries to load from disk.
	/// @param path Path to file.
	/// @return File contents.
	/// @throw Makai::File::FileLoadError on file load errors.
	CSVData getCSV(String const& path, char const delimiter = ',');

	#pragma GCC diagnostic pop
}

#endif // MAKAILIB_FILE_GET_H
