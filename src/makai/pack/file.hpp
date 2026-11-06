#ifndef MAKAILIB_PACK_FILE_H
#define MAKAILIB_PACK_FILE_H

#include "core.hpp"

namespace Makai::Pack {
	namespace BF = BinaryFormat;
	using namespace BF;

	/// @brief Hash IV block.
	struct Block: Text<String> {
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

		u64				uncompressed;
		u64				compressed;
		Mode			mode;
		u64				hash;
		Text<String>	block;
	};

	struct [[CTL_PACKED_STRUCT]] File: PackedEntry {
		Text<String>	contents;

		Bytes<> unpackFrom(Reader& reader);
	};

	struct [[CTL_PACKED_STRUCT]] Directory: PackedEntry {
		StringTable<UTF8String>	names;
		HeaderTable<File>		files;
	};

	struct [[CTL_PACKED_STRUCT]] PackInfo {
		/// @brief Size of file token identifier.
		constexpr usize FILE_TOKEN_SIZE = 20;
		/// @brief File token type.
		using FileToken = As<char const[FILE_TOKEN_SIZE]>;


		/// @brief File archive header flags.
		struct [[CTL_FLAG_STRUCT(uint64)]] Flags {
			uint64: 0;
		};

		FileToken			token		= "Makai::FAV2::Pack";
		BF::Version			version;
		Flags				flags;
		Header<Directory>	directory;
	};

	struct Pack {
		struct FileEntry {
			Bytes<>				data;
			PackedEntry::Mode	mode;
		};

		enum class LoadError: u16 {
			PLE_ALREADY_OPEN,
			PLE_BROKEN_HEADER,
			PLE_BROKEN_DIR_HEADER,
			PLE_BROKEN_DIR_STRUCTURE,
			PLE_NOT_A_PACK,
			PLE_NO_HEADER,
		};

		enum class GetError: u16 {
			PGE_NOT_OPEN,
			PGE_WRITE_ONLY,
			PGE_NO_FILE,
			PGE_BROKEN_FILE_HEADER,
			PGE_BROKEN_FILE,
		};

		enum class AddError: u16 {
			PAE_NOT_OPEN,
			PAE_READ_ONLY,
		};

		using Source = Unique<BF::Reader>;

		using Destination = Unique<BF::Writer>;

		Pack();

		Pack(String const& passHash, Source&& in, Destination&& out, Nullable<PackInfo> const& info = null);
		Pack(String const& passHash, Source&& in);
		Pack(String const& passHash, Destination&& out, PackInfo const& info = {});

		~Pack();

		Nullable<LoadError> open(String const& passHash, Source&& in, Destination&& out, Nullable<PackInfo> const& info = null);
		Nullable<LoadError> open(String const& passHash, Source&& in);
		Nullable<LoadError> open(String const& passHash, Destination&& out, PackInfo const& info = {});

		Result<FileEntry, GetError> tryGet(UTF8String const& path) const;
		Nullable<AddError> tryAdd(UTF8String const& path, FileEntry const& file);

		Pack&	remove(UTF8String const& path);
		bool	contains(UTF8String const& path) const;

		StringList files() const;

		Pack& close();

	private:
		bool							isOpen = false;
		String							hash;
		Nullable<PackInfo>				pack;
		Source							in;
		Destination						out;
		UTF8Dictionary<Nullable<File>>	inBuf;
		UTF8Dictionary<CachedFile>		outBuf;
	};
}

#endif
