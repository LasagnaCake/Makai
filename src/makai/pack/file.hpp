#ifndef MAKAILIB_PACK_FILE_H
#define MAKAILIB_PACK_FILE_H

#include "core.hpp"

namespace Makai::Pack {
	namespace BF = BinaryFormat;
	using namespace BF;

	/// @brief Hash IV block.
	struct Block: Text {
		/// @brief Creates an IV block with a cryptographically-secure random number generator operation.
		/// @return IV block.
		static String createFor(Encryption const method);
	};

	struct [[CTL_PACKED_STRUCT]] PackedEntry {
		struct [[CTL_PACKED_STRUCT]] Mode {
			Compression		compression	= Compression::PC_ZIP;
			Encryption		encryption	= Encryption::PE_AES256;
			u8				level		= 9;
		};

		u64		unpacked	= 0;
		u64		packed		= 0;
		Mode	mode;
		u64		hash		= 0;
		Text	block;
	};

	struct [[CTL_PACKED_STRUCT]] File: PackedEntry {
		BF::RawBytes contents;

		Nullable<Bytes<>> unpackFrom(BF::IReadable& reader, String const& key) const;
	};

	struct [[CTL_PACKED_STRUCT]] Directory: PackedEntry {
		StringTable<String>	names;
		HeaderTable<File>	files;
	};

	constexpr BF::Version const FILE_FORMAT_VERSION = {1};

	struct [[CTL_PACKED_STRUCT]] PackInfo {
		/// @brief Size of file token identifier.
		constexpr static usize const FILE_TOKEN_SIZE = 32;
		/// @brief File token type.
		using FileToken = As<char[FILE_TOKEN_SIZE]>;


		/// @brief File archive header flags.
		struct [[CTL_FLAG_STRUCT(uint64)]] Flags {
			uint64: 0;
		};

		FileToken			token		= "Makai::NewFileArchive::Pack";
		BF::Version			version		= FILE_FORMAT_VERSION;
		Flags				flags;
		Header<Directory>	directory;
	};

	struct Archive {
		struct FileEntry {
			Bytes<>				data;
			PackedEntry::Mode	mode;
		};

		enum class LoadError: u16 {
			PLE_ALREADY_OPEN,
			PLE_FILE_IS_TOO_NEW,
			PLE_BROKEN_HEADER,
			PLE_BROKEN_DIR_HEADER,
			PLE_BROKEN_DIR_STRUCTURE,
			PLE_NOT_A_PACK,
			PLE_NO_HEADER
		};

		enum class GetError: u16 {
			PGE_NOT_OPEN,
			PGE_WRITE_ONLY,
			PGE_NO_FILE,
			PGE_BROKEN_FILE_HEADER,
			PGE_BROKEN_FILE
		};

		enum class AddError: u16 {
			PAE_NOT_OPEN,
			PAE_READ_ONLY
		};

		using Source = Unique<BF::IReadable>;

		using Destination = Unique<BF::IWritable>;

		Archive();

		Archive(String const& passkey, Source&& in, Destination&& out, Nullable<PackInfo> const& info = null);
		Archive(String const& passkey, Source&& in);
		Archive(String const& passkey, Destination&& out, PackInfo const& info = {});

		~Archive();

		Nullable<LoadError> open(String const& passkey, Source&& in, Destination&& out, Nullable<PackInfo> const& info = null);
		Nullable<LoadError> open(String const& passkey, Source&& in);
		Nullable<LoadError> open(String const& passkey, Destination&& out, PackInfo const& info = {});

		Result<FileEntry, GetError> tryGet(UTF8String const& path);
		Nullable<AddError> tryAdd(UTF8String const& path, FileEntry const& file);

		Archive&	remove(UTF8String const& path);
		bool		contains(UTF8String const& path) const;

		Nullable<UTF8StringList> files() const;

		Archive& close();

		bool cacheLoadedFiles = false;

		constexpr bool exists() const	{return isOpen;		}
		constexpr operator bool() const	{return exists();	}

	private:
		bool							isOpen = false;
		String							key;
		Nullable<PackInfo>				pack;
		Source							in;
		Destination						out;
		UTF8Dictionary<Nullable<File>>	inBuf;
		UTF8Dictionary<FileEntry>		outBuf;
	};
}

#endif
