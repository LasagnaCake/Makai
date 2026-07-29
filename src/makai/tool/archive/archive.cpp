#include <cryptopp/aes.h>
#include <cryptopp/zlib.h>
#include <cryptopp/modes.h>
#include <cryptopp/sha3.h>
#include <cryptopp/randpool.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "archive.hpp"

#include "../../data/encdec.hpp"
#include "../../data/hash.hpp"
#include "../../file/get.hpp"

using namespace CTL::Literals::Text;

namespace fs = std::filesystem;
using namespace CryptoPP;
namespace File = Makai::File;

using namespace Makai;

namespace Arch = Makai::Tool::Arch;
using namespace Arch;

using Makai::JSON::Value;

static CTL::Random::Engine::Secure& rng() {
	static CTL::Random::Engine::Secure r;
	return r;
}

static String encoded(uint64 const v) {
	BinaryData<> data;
	data.resize(8, 0);
	for (usize i = 0; i < data.size(); ++i) data[i] = uint8((v >> (8 * i)) & 0xFF);
	return Makai::Data::encode(data, Makai::Data::EncodingType::ET_BASE64);
}

static uint64 decoded(String const& v) {
	BinaryData<> data = Makai::Data::decode(v, Makai::Data::EncodingType::ET_BASE64);
	usize result = 0;
	for (usize i = 0; i < data.size(); ++i)
		result |= (uint64(data[i]) << (8 * i));
	return result;
}

template<class T>
static String hash(String const& str) {
	String result;
	T hasher;
	hasher.Update((const byte*)str.data(), str.size());
	result.resize(hasher.DigestSize(), '\0');
	hasher.Final((byte*)result.data());
	return result;
}

constexpr String Arch::truncate(String const& str) {
	String result(str.size()/2, ' ');
	for (usize i = 0; i < str.size()/2; ++i)
		result[i] = (str[i*2] ^ str[i*2+1]);
	return result;
}

uint32 Arch::crcOf(BinaryData<> const& data) {
	auto const crc = Makai::hash(Data::hashed(data, Makai::Data::HashMode::HM_SHA3_512).toList<char>());
	return crc ^ (crc >> sizeof(uint32));
}

String Arch::hashPassword(String const& str) {
	return hash<SHA3_256>(str);
}

Arch::Block Arch::Block::create() {
	Arch::Block block;
	if (rng().next() % 2) {
		block.high	= rng().next();
		block.low	= rng().next();
	} else {
		block.low	= rng().next();
		block.high	= rng().next();
	}
	return block;
}

template<class T>
static BinaryData<> cbcTransform(
	BinaryData<> const&		data,
	String					password	= "",
	Arch::Block const&		block		= {}
) try {
	std::string result;
	T tf;
	uint8* iv = new uint8[16];
	MX::memcpy(iv, &block, 16);
	while (password.size() < tf.MaxKeyLength())
		password += " ";
	if (password.size() > 32)
		password = password.substring(0, 32);
	tf.SetKeyWithIV((uint8*)password.data(), password.size(), iv, 16);
	StringSource ss(
		data.data(),
		data.size(),
		true,
		new StreamTransformationFilter(
			tf,
			new StringSink(result)
		)
	);
	delete[] iv;
	return BinaryData<>((uint8*)result.data(), (uint8*)result.data() + result.size());
} catch (std::exception const& e) {
	throw Error::FailedAction(
		e.what(), CTL_CPP_PRETTY_SOURCE
	);
} catch (CTL::Exception const& e) {
	throw Error::FailedAction(
		e.what(), CTL_CPP_PRETTY_SOURCE
	);
}

template<Makai::Type::Equal<Deflator> T>
static T* getFlator(std::string& result, uint8 const level) {
	return new T(new StringSink(result), level);
}

template<Makai::Type::Equal<Inflator> T>
static T* getFlator(std::string& result, uint8 const level) {
	return new T(new StringSink(result));
}

template<class T>
static BinaryData<> flate(
	BinaryData<>	const&		data,
	CompressionMethod const&	method	= CompressionMethod::ACM_ZIP,
	uint8 const					level	= 9
) try {
	std::string result;
	switch (method) {
	case CompressionMethod::ACM_NONE: return data;
	case CompressionMethod::ACM_ZIP: {
			StringSource ss(
				data.data(),
				data.size(),
				true,
				getFlator<T>(result, Makai::Math::clamp<uint8>(level, 0, 9))
			);
		}
	}
	return BinaryData<>((uint8*)result.data(), (uint8*)result.data() + result.size());
} catch (std::exception const& e) {
	throw Error::FailedAction(
		e.what(), CTL_CPP_PRETTY_SOURCE
	);
} catch (CTL::Exception const& e) {
	throw Error::FailedAction(
		e.what(), CTL_CPP_PRETTY_SOURCE
	);
}

template<typename T>
static BinaryData<> cbcEncrypt(
	BinaryData<> const&		data,
	String const&			password	= "",
	Arch::Block const&		block		= {}
) {
	return cbcTransform<typename CBC_Mode<T>::Encryption>(data, password, block);
}

template<typename T>
static BinaryData<> cbcDecrypt(
	BinaryData<> const&		data,
	String const&			password	= "",
	Arch::Block const&		block		= {}
) {
	return cbcTransform<typename CBC_Mode<T>::Decryption>(data, password, block);
}

BinaryData<> Arch::encrypt(
	BinaryData<> const&		data,
	String const&			password,
	EncryptionMethod const&	method,
	Arch::Block const&		block
) {
	switch (method) {
		default: throw Error::InvalidValue("Invalid encryption method!", CTL_CPP_PRETTY_SOURCE);
		case EncryptionMethod::AEM_NONE:	return data;
		case EncryptionMethod::AEM_AES256:	return cbcEncrypt<AES>(data, password, block);
	}
	return data;
}

BinaryData<> Arch::decrypt(
	BinaryData<> const&		data,
	String const&			password,
	EncryptionMethod const&	method,
	Arch::Block const&		block
) {
	switch (method) {
		default: throw Error::InvalidValue("Invalid decryption method!", CTL_CPP_PRETTY_SOURCE);
		case EncryptionMethod::AEM_NONE:	return data;
		case EncryptionMethod::AEM_AES256:	return cbcDecrypt<AES>(data, password, block);
	}
	return data;
}

BinaryData<> Arch::compress(
	BinaryData<>	const&		data,
	CompressionMethod const&	method,
	uint8 const				level
) {
	return flate<Deflator>(data, method, level);
}

BinaryData<> Arch::decompress(
	BinaryData<>	const&		data,
	CompressionMethod const&	method,
	uint8 const				level
) {
	return flate<Inflator>(data, method, level);
}

static Value getStructure(fs::path const& path, StringList& files, String const& root) {
	Value dir = Value::object();
	for (auto const& e : fs::directory_iterator(path)) {
		if (e.is_directory()) {
			String dirname = String(e.path().stem().string());
			dir[dirname] = getStructure(e, files, root + "/" + dirname);
		}
		else {
			String filename = String(e.path().filename().string());
			String filepath = root + "/" + filename;
			dir[filename] = filepath;
			files.pushBack(filepath);
		}
	}
	return dir;
}

static StringList getFileInfo(Value const& filestruct) {
	StringList res;
	for (auto& [name, data]: filestruct.items()) {
		if (data.isString())
			res.pushBack(data.get<String>());
		else if (data.isObject() && !data.empty())
			for (String& s: getFileInfo(data))
				res.pushBack(s);
	}
	return res;
}

static void populateTree(Value& tree, String const& root = "") {
	if (!tree.isObject())
		throw Error::FailedAction("file tree is not a JSON object!", CTL_CPP_PRETTY_SOURCE);
	for (auto& [name, data]: tree.items()) {
		String path = OS::FS::concatenate(root, String(name));
		if (data.isString()) data = path;
		else if (data.isObject()) populateTree(data, path);
		else throw Error::FailedAction("Invalid data type in file tree!", CTL_CPP_PRETTY_SOURCE);
	}
}

static usize populateTree(Value& tree, List<uint64> const& values, usize const start = 0) {
	if (!tree.isObject())
		throw Error::FailedAction("file tree is not a JSON object!", CTL_CPP_PRETTY_SOURCE);
	usize idx = start;
	for (auto& [name, data]: tree.items()) {
		if (data.isString()) data = encoded(values[idx++]);
		else if (data.isObject()) idx = populateTree(data, values, idx);
		else throw Error::FailedAction("Invalid data type in file tree!", CTL_CPP_PRETTY_SOURCE);
	}
	return idx;
}

void Arch::pack(
		String const& archivePath,
		String const& folderPath,
		String const& password,
		EncryptionMethod const& enc,
		CompressionMethod const& comp,
		uint8 const complvl
) {
	try {
		// Hash the password
		String passhash = hashPassword(password);
		MAKAILIB_DEBUGLN_FULL("FOLDER: ", folderPath, "\nARCHIVE: ", archivePath);
		// Get file structure
		MAKAILIB_DEBUGLN_FULL("Getting file structure...");
		Value dir;
		StringList files;
		Value tree = dir["tree"];
		tree = getStructure(fs::path(folderPath.std()), files, String(fs::path(folderPath.std()).stem().string()));
		MAKAILIB_DEBUGLN_FULL("\n", dir.toFLOWString(String{"  "}));
		// Populate with temporary values
		List<uint64> locations;
		locations.resize(files.size(), 0);
		// Open file
		std::ofstream file;
		file.exceptions(std::ofstream::badbit | std::ofstream::failbit);
		file.open(archivePath.cstr(), std::ios::binary | std::ios::trunc);
		// Populate header
		MAKAILIB_DEBUGLN_FULL("Creating header...\n");
		// Headers
		ArchiveHeader	header;
		// Set main header params
		header.version		= ARCHIVE_VERSION;				// file format version
		header.minVersion	= ARCHIVE_MIN_VERSION;			// file format minimum version
		header.encryption	= (uint16)enc;					// encryption mode
		header.compression	= (uint16)comp;					// compression mode
		header.level		= complvl;						// compression level
		header.flags = {
			.shouldCheckCRC = true
		};
		MAKAILIB_DEBUGLN_FULL("             HEADER SIZE: ", (uint64)header.headerSize,		"B"	);
		MAKAILIB_DEBUGLN_FULL("        FILE HEADER SIZE: ", (uint64)header.fileHeaderSize,	"B"	);
		MAKAILIB_DEBUGLN_FULL("   DIRECTORY HEADER SIZE: ", (uint64)header.dirHeaderSize,		"B"	);
		MAKAILIB_DEBUGLN_FULL("     FILE FORMAT VERSION: ", (uint64)header.version				);
		MAKAILIB_DEBUGLN_FULL(" FILE FORMAT MIN VERSION: ", (uint64)header.minVersion				);
		MAKAILIB_DEBUGLN_FULL("         ENCRYPTION MODE: ", (uint64)header.encryption				);
		MAKAILIB_DEBUGLN_FULL("        COMPRESSION MODE: ", (uint64)header.compression			);
		MAKAILIB_DEBUGLN_FULL("       COMPRESSION LEVEL: ", (uint64)header.level					);
		MAKAILIB_DEBUGLN_FULL("\nDirectory structure layout:");
		MAKAILIB_DEBUGLN_FULL("       FILE COUNT: ", files.size()			);
		// Write main header first pass
		file.write((char*)&header, header.headerSize);
		// Write file info
		MAKAILIB_DEBUGLN_FULL("\nWriting files...\n");
		usize i = 0;
		for (auto const& f: files) {
			// Get current stream position as file location
			locations[i] = file.tellp();
			// Read file
			String const loc = Regex::replace(f, "^(.*?)[\\\\\\/]", "");
			MAKAILIB_DEBUGLN_FULL("Clean path: '", loc, "'");
			String const fpath = Makai::OS::FS::concatenate(folderPath, loc);
			MAKAILIB_DEBUGLN_FULL("Full path: '", fpath, "'");
			BinaryData<> contents = File::loadBinary(fpath);
			// Prepare header
			FileHeader fheader;
			fheader.uncSize = contents.size();		// Uncompressed file size
			// Generate block
			fheader.block = Arch::Block::create();	// Encryption block
			// Process file
			if (!contents.empty()) {
				contents = compress(
					contents,
					comp,
					complvl
				);
				MAKAILIB_DEBUGLN_FULL("Before encryption: ", contents.size());
				contents = encrypt(
					contents,
					passhash,
					enc,
					fheader.block
				);
				MAKAILIB_DEBUGLN_FULL("After encryption: ", contents.size());
			}
			fheader.compSize	= contents.size();	// Compressed file size
			fheader.crc			= crcOf(contents);	// CRC
			// Debug info
			MAKAILIB_DEBUGLN_FULL("'", files[i], "':");
			MAKAILIB_DEBUGLN_FULL("          FILE INDEX: ", i							);
			MAKAILIB_DEBUGLN_FULL("       FILE LOCATION: ", locations[i]				);
			MAKAILIB_DEBUGLN_FULL("                 ENCODED: ", encoded(locations[i])	);
			MAKAILIB_DEBUGLN_FULL("   UNCOMPRESSED SIZE: ", fheader.uncSize,	"B"		);
			MAKAILIB_DEBUGLN_FULL("     COMPRESSED SIZE: ", fheader.compSize,	"B"		);
			MAKAILIB_DEBUGLN_FULL("               CRC32: ", fheader.crc,		"\n"	);
			// Copy header & file data
			file.write((char*)&fheader, header.fileHeaderSize);
			file.write((char*)contents.data(), contents.size());
			++i;
		}
		// Populate file tree
		populateTree(tree, locations);
		dir["tree"] = tree;
		// Process directory structure
		MAKAILIB_DEBUGLN_FULL("\nWriting directory structure...\n");
		MAKAILIB_DEBUGLN_FULL("\n", dir.toFLOWString(String{"  "}));
		{
			// Directory header
			DirectoryHeader	dheader;
			// Generate header block
			dheader.block = Arch::Block::create();
			// Get directory info
			String dirInfo = dir.toFLOWString();
			// Compress & encrypt directory info
			BinaryData<> pdi;
			pdi.resize(dirInfo.size(), 0);
			MX::memcpy(pdi.data(), dirInfo.data(), dirInfo.size());
			pdi = compress(pdi, comp, complvl);
			pdi = encrypt(pdi, passhash, enc, dheader.block);
			// Populate header
			dheader.compSize	= pdi.size();
			dheader.uncSize		= dirInfo.size();
			// Get directory header location
			header.dirHeaderLoc = file.tellp();
			// Debug info
			MAKAILIB_DEBUGLN_FULL("  DIRECTORY INFO LOCATION: ", header.dirHeaderLoc		);
			MAKAILIB_DEBUGLN_FULL("        UNCOMPRESSED SIZE: ", dheader.uncSize,		"B"	);
			MAKAILIB_DEBUGLN_FULL("          COMPRESSED SIZE: ", dheader.compSize,	"B"	);
			// Write header & directory info
			file.write((char*)&dheader, header.dirHeaderSize);
			file.write((char*)pdi.data(), pdi.size());
			// Write main header second pass
			file.seekp(0);
			file.write((char*)&header, header.headerSize);
		}
		// Close file
		file.flush();
		file.close();
		MAKAILIB_DEBUGLN_FULL("\nDone!");
	} catch (std::exception const& e) {
		throw File::FileLoadError(e.what(), CTL_CPP_PRETTY_SOURCE);
	}
}

[[noreturn]] static void notOpenError() {
	throw File::FileLoadError(
		"Archive is not open!",
		CTL_CPP_UNKNOWN_SOURCE
	);
}

[[noreturn]] static void singleFileArchiveError() {
	throw File::FileLoadError(
		"Archive is not a multi-file archive!",
		CTL_CPP_UNKNOWN_SOURCE
	);
}

[[noreturn]] static void notAFileArchiveError() {
	throw File::FileLoadError(
		"File is not a file archive!",
		CTL_CPP_UNKNOWN_SOURCE
	);
}

[[noreturn]] static void doesNotExistError(String const& file) {
	throw File::FileLoadError(
		"Directory or file '" + file + "' does not exist!",
		CTL_CPP_UNKNOWN_SOURCE
	);
}

[[noreturn]] static void outOfArchiveBoundsError(String const& file) {
	throw File::FileLoadError(
		"Directory or file '" + file + "' lives outside the archive!",
		CTL_CPP_UNKNOWN_SOURCE
	);
}

[[noreturn]] static void notAFileError(String const& file) {
	throw File::FileLoadError(
		"Entry '" + file + "' is not a file!",
		CTL_CPP_UNKNOWN_SOURCE
	);
}

[[noreturn]] static void directoryTreeError() {
	throw File::FileLoadError(
		"Missing or corrupted directory tree info!",
		CTL_CPP_UNKNOWN_SOURCE
	);
}

[[noreturn]] static void corruptedFileError(String const& path) {
	throw File::FileLoadError(
		"Corrupted file '" + path + "'!",
		CTL_CPP_UNKNOWN_SOURCE
	);
}

[[noreturn]] static void crcFailError(String const& path) {
	throw File::FileLoadError(
		"CRC check failed for file '" + path + "'!",
		CTL_CPP_UNKNOWN_SOURCE
	);
}

Arch::FileArchive::FileArchive(Unique<IInputStream<Bytes<>>>&& buffer, String const& password) {open(buffer.transfer(), password);}

Arch::FileArchive::~FileArchive() {close();}

FileArchive& Arch::FileArchive::open(Unique<IInputStream<Bytes<>>>&& buffer, String const& password) {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	if (streamOpen) return *this;
	// Set archive
	archive = buffer.transfer();
	// Set password
	pass = password;
	// Read header
	usize hs = 0;
	archive->readInto((ref<byte>)&hs, sizeof(uint64));
	archive->go(0);
	hs = (hs < sizeof(ArchiveHeader)) ? hs : sizeof(ArchiveHeader);
	archive->readInto((ref<byte>)&header, hs);
	// Make sure header sizes are OK
	if (header.headerSize > sizeof(ArchiveHeader))
		header.headerSize = sizeof(ArchiveHeader);
	if (header.dirHeaderSize > sizeof(DirectoryHeader))
		header.dirHeaderSize = sizeof(DirectoryHeader);
	if (header.fileHeaderSize > sizeof(FileHeader))
		header.fileHeaderSize = sizeof(FileHeader);
	// check if file is archive
	if (header.minVersion > 1 && String(header.token) != "Makai::FileArchive")
		notAFileArchiveError();
	if (header.flags.isSingleFileArchive)
		singleFileArchiveError();
	if (!header.dirHeaderLoc)
		directoryTreeError();
	// Read directory tree info
	parseFileTree();
	// Set open flag
	streamOpen = true;
	return *this;
}

FileArchive& Arch::FileArchive::close() try {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	if (!streamOpen) return *this;
	streamOpen = false;
	return *this;
} catch (std::exception const& e) {
	throw File::FileLoadError(e.what(), CTL_CPP_PRETTY_SOURCE);
}

String Arch::FileArchive::getTextFile(String const& path) try {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	assertOpen();
	FileEntry fe = getFileEntry(path);
	processFileEntry(fe);
	String content;
	content.resize(fe.data.size(), 0);
	MX::memcpy(content.data(), fe.data.data(), content.size());
	return content;
} catch (Error::FailedAction const& e) {
	throw File::FileLoadError(
		"could not load file '" + path + "'!",
		e.message
	);
}

BinaryData<> Arch::FileArchive::getBinaryFile(String const& path) try {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	assertOpen();
	FileEntry fe = getFileEntry(path);
	processFileEntry(fe);
	return fe.data;
} catch (Error::FailedAction const& e) {
	throw File::FileLoadError(
		"could not load file '" + path + "'!",
		e.message
	);
}

Makai::JSON::Value Arch::FileArchive::getFileTree(String const& root) const {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	assertOpen();
	Value dir = fstruct["tree"];
	populateTree((!root.empty()) ? dir[root] : dir, root);
	return dir;
}

ArchiveHeader Arch::FileArchive::getHeader(String const& path) {
	InputFileStream<Bytes<>> af{path};
	ArchiveHeader ah;
	// Read header
	usize hs = 0;
	af.readInto((ref<byte>)&hs, sizeof(uint64));
	af.go(0);
	if (hs > sizeof(ArchiveHeader)) hs = sizeof(ArchiveHeader);
	af.readInto((ref<byte>)&ah, hs);
	return ah;
}

FileArchive& Arch::FileArchive::unpackTo(String const& path) {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	if (!streamOpen) return *this;
	Value ftree = getFileTree();
	unpackLayer(ftree, path);
	return *this;
}

bool Arch::FileArchive::isOpen() const {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	return streamOpen;
}

void Arch::FileArchive::parseFileTree() {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	String fs;
	switch (header.minVersion) {
	default:
	case 0:
		// "dirHeaderSize" is located in the old "dirInfoSize" parameter
		fs.resize(header.dirHeaderSize, ' ');
		archive->readInto((ref<byte>)fs.data(), fs.size());
		archive->go(0);
		break;
	case 1:
		DirectoryHeader dh;
		archive->go(header.dirHeaderLoc);
		archive->readInto((ref<byte>)&dh, header.dirHeaderSize);
		if (!dh.compSize || !dh.uncSize) directoryTreeError();
		MAKAILIB_DEBUGLN_FULL("  DIRECTORY INFO LOCATION: ", header.dirHeaderLoc		);
		MAKAILIB_DEBUGLN_FULL("        UNCOMPRESSED SIZE: ", dh.uncSize,			"B"	);
		MAKAILIB_DEBUGLN_FULL("          COMPRESSED SIZE: ", dh.compSize,			"B"	);
		BinaryData<> pfs;
		pfs.resize(dh.compSize, 0);
		archive->readInto((ref<byte>)pfs.data(), pfs.size());
		archive->go(0);
		MAKAILIB_DEBUGLN_FULL("Demangling tree data...");
		demangleData(pfs, dh.block);
		fs.resize(pfs.size(), 0);
		MX::memcpy(fs.data(), pfs.data(), fs.size());
		if (fs.size() != dh.uncSize) directoryTreeError();
		break;
	}
	try {
		MAKAILIB_DEBUGLN_FULL("Parsing tree...");
		fstruct = Makai::JSON::parse(fs);
	} catch (Error::FailedAction const& e) {
		throw File::FileLoadError(
			"Invalid or corrupted file structure!",
			e.what()
		);
	}
}

void Arch::FileArchive::demangleData(BinaryData<>& data, Block const& block) const {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	data = decrypt(
		data,
		pass,
		(EncryptionMethod)header.encryption,
		block
	);
	data = decompress(
		data,
		(CompressionMethod)header.compression,
		header.level
	);
}

void Arch::FileArchive::unpackLayer(Value const& layer, String const& path) {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	assertOpen();
	List<KeyValuePair<String, String>> files;
	for (auto& [name, data]: layer.items()) {
		if (data.isString()) files.pushBack(KeyValuePair<String, String>(name, data.get<String>()));
		else if (data.isObject()) unpackLayer(data, path);
		else directoryTreeError();
	}
	for (auto& [name, data]: files) {
		String filepath = OS::FS::concatenate(path, data);
		BinaryData<> contents = getBinaryFile(data);
		OS::FS::makeDirectory(OS::FS::directoryFromPath(filepath));
		File::saveBinary(filepath, contents);
	}
}

void Arch::FileArchive::processFileEntry(FileEntry& entry) const {
	BinaryData<> data = entry.data;
	if (entry.header.uncSize == 0) return;
	demangleData(data, entry.header.block);
	if (data.size() != entry.header.uncSize)
		corruptedFileError(entry.path);
	if (header.flags.shouldCheckCRC && (entry.header.crc != crcOf(data)))
		crcFailError(entry.path);
	entry.data = data;
}

Arch::FileArchive::FileEntry Arch::FileArchive::getFileEntry(String const& path) try {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	if (!fstruct["tree"].isObject())
		directoryTreeError();
	MAKAILIB_DEBUGLN_FULL("Getting file entry location...");
	uint64		idx	= getFileEntryLocation(path.lower(), path);
	MAKAILIB_DEBUGLN_FULL("Getting file entry header...");
	FileHeader	fh	= getFileEntryHeader(idx);
	MAKAILIB_DEBUGLN_FULL("Getting file entry data...");
	return Arch::FileArchive::FileEntry{idx, path, fh, getFileEntryData(idx, fh)};
} catch (File::FileLoadError const& e) {
	Error::rethrow(e);
} catch (std::exception const& e) {
	throw File::FileLoadError(
		"Failed at getting file entry '" + path + "'!",
		e.what()
	);
}

BinaryData<> Arch::FileArchive::getFileEntryData(uint64 const index, FileHeader const& fh) try {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	BinaryData<> fd;
	fd.resize(fh.compSize, 0);
	auto const lp = archive->position();
	archive->go(index + header.fileHeaderSize);
	archive->readInto(fd.data(), fh.compSize);
	archive->go(lp);
	return fd;
} catch (std::ios_base::failure const& e) {
	throw Error::FailedAction(
		"Failed at getting file entry data: "s + String(e.what()),
		CTL_CPP_PRETTY_SOURCE
	);
}

FileHeader Arch::FileArchive::getFileEntryHeader(uint64 const index) {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	FileHeader fh;
	auto const lp = archive->position();
	archive->go(index);
	archive->readInto((ref<byte>)&fh, header.fileHeaderSize);
	archive->go(lp);
	return fh;
}

uint64 Arch::FileArchive::getFileEntryLocation(String const& path, String const& origpath) {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	List<Value> stack;
	Value entry = fstruct["tree"];
	MAKAILIB_DEBUGLN_FULL("Path: ", origpath);
	MAKAILIB_DEBUGLN_FULL("Cleaned: ", Regex::replace(path, "[\\\\\\/]+", "/"));
	// Loop through path and get entry location
	for (String fld: Regex::replace(path, "[\\\\\\/]+", "/").split('/')) {
		if (fld == "..") {
			if (stack.empty())
				outOfArchiveBoundsError(origpath);
			entry = stack.popBack();
			continue;
		} else if (entry.isObject()) {
			for (auto [k, v]: entry.items())
				if (String(k).lower() == fld) {
					stack.pushBack(entry);
					entry = v;
					break;
				}
		} else if (entry.isString() && String(entry.get<String>()).lower() == fld)
			return decoded(entry.get<String>());
		else doesNotExistError(fld);
	}
	// Try and get entry location
	if (entry.isString())
		return decoded(entry.get<String>());
	else notAFileError(origpath);
}

void Arch::FileArchive::assertOpen() const {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	if (!streamOpen)
		notOpenError();
}

static void unpackV1(
	String const& archivePath,
	String const folderPath,
	String const& password = ""
) try {
	FileArchive arc(FileArchive::Source::create<InputByteFileStream>(archivePath), hashPassword(password));
	arc.unpackTo(folderPath);
} catch (std::exception const& e) {
	throw File::FileLoadError(e.what(), CTL_CPP_PRETTY_SOURCE);
}

static void unpackV0(
	String const& archivePath,
	String const folderPath,
	String const& password = ""
) try {
	FileArchive arc(FileArchive::Source::create<InputByteFileStream>(archivePath), password);
	arc.unpackTo(folderPath);
} catch (std::exception const& e) {
	throw File::FileLoadError(e.what(), CTL_CPP_PRETTY_SOURCE);
}

void Arch::unpack(
	String const& archivePath,
	String const folderPath,
	String const& password
) try {
	uint64 mv = FileArchive::getHeader(archivePath).minVersion;
	switch(mv) {
		case 1: unpackV1(archivePath, folderPath, password);	break;
		case 0: unpackV0(archivePath, folderPath, password);	break;
		default: throw Error::InvalidValue(
			"Unsupported or invalid minimum version!",
			CTL_CPP_PRETTY_SOURCE
		);
	}
} catch (std::exception const& e) {
	throw File::FileLoadError(e.what(), CTL_CPP_PRETTY_SOURCE);
}
