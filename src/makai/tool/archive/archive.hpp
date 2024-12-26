#ifndef MAKAILIB_TOOL_ARCHIVE_H
#define MAKAILIB_TOOL_ARCHIVE_H

#include <fstream>
#include "../../compat/ctl.hpp"
#include "../../file/get.hpp"
#include "../../file/json.hpp"

/// @brief Custom file archive format facilites.
namespace Makai::Tool::Arch {
	using namespace CTL;
	/// @brief Encryption method.
	enum class EncryptionMethod: uint64 {
		AEM_NONE,
		AEM_AES256,
	};

	/// @brief Compression method.
	enum class CompressionMethod: uint64 {
		ACM_NONE,
		ACM_ZIP,
	};

	/// @brief Truncates a string.
	/// @param str String to truncate.
	/// @return Truncated string.
	/// @details XORs all odd-index characters with their even-index neighbours.
	constexpr String truncate(String const& str);

	/// @brief Hashes a string.
	/// @param str String to hash.
	/// @return Hashed string.
	String hashPassword(String const& str);

	/// @brief Encrypts data.
	/// @param data Data to encrypt.
	/// @param password Password to use. Can be a hashed string. By default, it is empty.
	/// @param method Encryption method. By default, it is `EncryptionMethod::AEM_AES256`.
	/// @param block IV block. By default, it is `nullptr`.
	/// @return Encrypted data.
	BinaryData<> encrypt(
		BinaryData<> const&		data,
		String const&			password	= "",
		EncryptionMethod const&	method		= EncryptionMethod::AEM_AES256,
		uint8* const			block		= nullptr
	);

	/// @brief Decrypts data.
	/// @param data Data to decrypt.
	/// @param password Password to use. Can be a hashed string. By default, it is empty.
	/// @param method Decryption method. By default, it is `EncryptionMethod::AEM_AES256`.
	/// @param block IV block. By default, it is `nullptr`.
	/// @return Decrypted data.
	BinaryData<> decrypt(
		BinaryData<> const&		data,
		String const&			password	= "",
		EncryptionMethod const&	method		= EncryptionMethod::AEM_AES256,
		uint8* const			block		= nullptr
	);

	/// @brief Compresses data.
	/// @param data Data to compress.
	/// @param method Compression method. By default, it is `CompressionMethod::ACM_ZIP`.
	/// @param level Compression level. By default, it is `9`.
	/// @return Compressed data.
	BinaryData<> compress(
		BinaryData<>	const&		data,
		CompressionMethod const&	method	= CompressionMethod::ACM_ZIP,
		uint8 const					level	= 9
	);

	/// @brief Decompresses data.
	/// @param data Data to Decompress.
	/// @param method Decompression method. By default, it is `CompressionMethod::ACM_ZIP`.
	/// @param level Decompression level. By default, it is `9`.
	/// @return Decompressed data.
	BinaryData<> decompress(
		BinaryData<>	const&		data,
		CompressionMethod const&	method	= CompressionMethod::ACM_ZIP,
		uint8 const					level	= 9
	);

	/// @brief File entry header.
	struct [[gnu::packed]] FileHeader {
		uint64	uncSize;
		uint64	compSize;
		uint32	crc			= 0;
		uint8	block[16]	= {0};
		// Put new things BELOW this line
	};

	/// @brief Directory structure header.
	struct [[gnu::packed]] DirectoryHeader {
		uint64	uncSize;
		uint64	compSize;
		uint32	crc			= 0;
		uint8	block[16]	= {0};
	};
	
	/// @brief Archive file format current version.
	constexpr uint64 ARCHIVE_VERSION		= 1;
	/// @brief Archive file format minimum version required.
	constexpr uint64 ARCHIVE_MIN_VERSION	= 1;

	/// @brief File archive header.
	struct [[gnu::packed]] ArchiveHeader {
		uint64	const headerSize		= sizeof(ArchiveHeader);
		uint64	const fileHeaderSize	= sizeof(FileHeader);
		uint64	const dirHeaderSize		= sizeof(DirectoryHeader);
		uint64	version			= ARCHIVE_VERSION;
		uint64	minVersion		= ARCHIVE_MIN_VERSION;
		uint16	encryption		= (uint16)EncryptionMethod::AEM_AES256;
		uint16	compression		= (uint16)CompressionMethod::ACM_ZIP;
		uint8	level			= 9;
		uint64	flags			= 0;
		uint64	dirHeaderLoc	= 0;
		// Put new things BELOW this line
	};

	/// @brief Archive flags.
	namespace Flags {
		/// @brief Flags the archive as a special single-file archive.
		constexpr uint64 SINGLE_FILE_ARCHIVE_BIT	= (1 << 0);
		/// @brief Flags the archive as containing a checksum to check.
		constexpr uint64 SHOULD_CHECK_CRC_BIT		= (1 << 1);
	}

	/// @brief Packs a folder into a file archive.
	/// @param archivePath Path to resulting file archive.
	/// @param folderPath Path to folder to pack.
	/// @param password Archive password. It is hashed. By default, it is empty.
	/// @param enc Encryption method. By default, it is `EncryptionMethod::AEM_AES256`.
	/// @param comp Compression method. By default, it is `CompressionMethod::ACM_ZIP`.
	/// @param complvl Compression level. By default, it is `9`.
	void pack(
			String const& archivePath,
			String const& folderPath,
			String const& password = "",
			EncryptionMethod const& enc = EncryptionMethod::AEM_AES256,
			CompressionMethod const& comp = CompressionMethod::ACM_ZIP,
			uint8 const complvl = 9
	);

	/// @brief Unpacks a file archive into a given folder.
	/// @param archivePath Path to archive to unpack.
	/// @param folderPath Path to folder to unpack file archive into.
	/// @param password Archive password. If version is 1 or higher, it is hashed.
	void unpack(
		String const& archivePath,
		String const folderPath,
		String const& password = ""
	);

	/// @brief File archive.
	struct FileArchive {
		/// @brief File entry information.
		struct FileEntry {
			uint64 const	index;
			String const	path;
			FileHeader		header;
			BinaryData<>	data;
		};

		/// @brief Archive version.
		struct ArchiveVersion {
			/// @brief File version.
			uint64 const version;
			/// @brief Minimum version required.
			uint64 const minimum;
		};

		/// @brief Empty constructor.
		FileArchive() {}

		/// @brief Opens a file archive for reading.
		/// @param buffer Buffer to stream data from.
		/// @param password Archive password. Can be a password hash. By default, it is empty.
		/// @details If archive version is 1 or greater, then `password` is the password hash.
		FileArchive(DataBuffer& buffer, String const& password = "");

		/// @brief Destructor.
		~FileArchive();

		/// @brief Opens a file archive for reading.
		/// @param buffer Buffer to stream data from.
		/// @param password Archive password. Can be a password hash. By default, it is empty.
		/// @details If archive version is 1 or greater, then `password` is the password hash.
		/// @return Reference to self.
		/// @throw File::FileLoadError for errors that happen.
		FileArchive& open(DataBuffer& buffer, String const& password);

		/// @brief Closes the file archive.
		/// @return Reference to self.
		FileArchive& close();

		/// @brief Reads a text file from the archive.
		/// @param path Path to file in the archive.
		/// @return File contents.
		/// @throw File::FileLoadError on file read errors.
		String getTextFile(String const& path);

		/// @brief Reads a binary file from the archive.
		/// @param path Path to file in the archive.
		/// @return File contents.
		/// @throw File::FileLoadError on file read errors.
		BinaryData<> getBinaryFile(String const& path);

		/// @brief Gets the archive's file tree.
		/// @param root Root name. By default, it is empty.
		/// @return Archive file tree.
		/// @throw Error::FailedAction on file tree acquisition failures.
		JSON::JSONData getFileTree(String const& root = "") const;

		/// @brief Gets the archive header for a given file archive.
		/// @param path Path to archive to get header for.
		/// @return Archive header.
		static ArchiveHeader getHeader(String const& path);

		/// @brief Unpacks the archive to a given folder.
		/// @param path Path to folder to unpack to.
		/// @return Reference to self.
		FileArchive& unpackTo(String const& path);

		/// @brief Returns whether there is an open file archive.
		/// @return Whether file archive is open.
		bool isOpen() const;

	private:

		void parseFileTree();

		void demangleData(BinaryData<>& data, uint8* const block) const;

		void unpackLayer(JSON::Extern::JSONData const& layer, String const& path);

		void processFileEntry(FileEntry& entry) const;

		FileEntry getFileEntry(String const& path);

		BinaryData<> getFileEntryData(uint64 const index, FileHeader const& fh);

		FileHeader getFileEntryHeader(uint64 const index);

		uint64 getFileEntryLocation(String const& path, String const& origpath);

		void assertOpen() const;

		/// @brief Whether there is an archive stream.
		bool					streamOpen	= false;
		/// @brief Archive password.
		String					pass;
		/// @brief Archive stream.
		DataStream				archive		= DataStream(nullptr);
		/// @brief Archive header.
		ArchiveHeader			header;
		/// @brief Archive file structure.
		JSON::Extern::JSONData	fstruct;
	};

	/// @brief Reads an encrypted (single-file archive) text file from disk.
	/// @param path Path to file.
	/// @param password File password. Can be a password hash. By default, it is empty.
	/// @return File contents.
	/// @details If archive version is 1 or greater, then `password` is the password hash.
	/// @throw File::FileLoadError on file read errors.
	String loadEncryptedTextFile(String const& path, String const& password = "");

	/// @brief Reads an encrypted (single-file archive) binary file from disk.
	/// @param path Path to file.
	/// @param password File password. Can be a password hash. By default, it is empty.
	/// @return File contents.
	/// @details If archive version is 1 or greater, then `password` is the password hash.
	/// @throw File::FileLoadError on file read errors.
	BinaryData<> loadEncryptedBinaryFile(String const& path, String const& password = "");

	/// @brief Writes an encrypted (single-file archive) binary file to disk.
	/// @tparam T Value type.
	/// @param path Path to file.
	/// @param data Data to write.
	/// @param size Size of data to write.
	/// @param password File password. It is hashed. By default, it is empty.
	/// @param enc Encryption method. By default, it is `EncryptionMethod::AEM_AES256`.
	/// @param comp Compression method. By default, it is `CompressionMethod::ACM_ZIP`.
	/// @param complvl Compression level. By default, it is `9`.
	template<typename T>
	void saveEncryptedBinaryFile(
		String const&				path,
		T* const					data,
		usize const					size,
		String const&				password	= "",
		EncryptionMethod const&		enc			= EncryptionMethod::AEM_AES256,
		CompressionMethod const&	comp		= CompressionMethod::ACM_ZIP,
		uint8 const					lvl			= 9
	);

	/// @brief Writes an encrypted (single-file archive) text file to disk.
	/// @param path Path to file.
	/// @param data Data to write.
	/// @param password File password. It is hashed. By default, it is empty.
	/// @param enc Encryption method. By default, it is `EncryptionMethod::AEM_AES256`.
	/// @param comp Compression method. By default, it is `CompressionMethod::ACM_ZIP`.
	/// @param complvl Compression level. By default, it is `9`.
	void saveEncryptedTextFile(
		String const&				path,
		BinaryData<> const&			data,
		String const&				password	= "",
		EncryptionMethod const&		enc			= EncryptionMethod::AEM_AES256,
		CompressionMethod const&	comp		= CompressionMethod::ACM_ZIP,
		uint8 const					lvl			= 9
	);

	/// @brief Writes an encrypted (single-file archive) binary file to disk.
	/// @tparam T Value type.
	/// @param path Path to file.
	/// @param data Data to write.
	/// @param password File password. It is hashed. By default, it is empty.
	/// @param enc Encryption method. By default, it is `EncryptionMethod::AEM_AES256`.
	/// @param comp Compression method. By default, it is `CompressionMethod::ACM_ZIP`.
	/// @param complvl Compression level. By default, it is `9`.
	template<typename T>
	void saveEncryptedBinaryFile(
		String const&				path,
		List<T> const&				data,
		String const&				password	= "",
		EncryptionMethod const&		enc			= EncryptionMethod::AEM_AES256,
		CompressionMethod const&	comp		= CompressionMethod::ACM_ZIP,
		uint8 const					lvl			= 9
	);

	/// @brief Writes an encrypted (single-file archive) text file to disk.
	/// @param path Path to file.
	/// @param data Data to write.
	/// @param password File password. It is hashed. By default, it is empty.
	/// @param enc Encryption method. By default, it is `EncryptionMethod::AEM_AES256`.
	/// @param comp Compression method. By default, it is `CompressionMethod::ACM_ZIP`.
	/// @param complvl Compression level. By default, it is `9`.
	void saveEncryptedTextFile(
		String const&				path,
		String const&				data,
		String const&				password	= "",
		EncryptionMethod const&		enc			= EncryptionMethod::AEM_AES256,
		CompressionMethod const&	comp		= CompressionMethod::ACM_ZIP,
		uint8 const					lvl			= 9
	);
}

#endif // MAKAILIB_TOOL_ARCHIVE_H
