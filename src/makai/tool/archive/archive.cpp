#include <cryptopp/aes.h>
#include <cryptopp/zlib.h>
#include <cryptopp/modes.h>
#include <cryptopp/sha3.h>
#include <cryptopp/randpool.h>
#include <cppcodec/base64_rfc4648.hpp>
#include <cppcodec/base32_rfc4648.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "archive.hpp"

#include "../../data/encdec.hpp"

using namespace CTL::Literals::Text;

// Legacy stuff, TODO: Remove this later
#ifdef ARCSYS_APPLICATION_
#define _ARCDEBUG(...)		DEBUG(__VA_ARGS__)
#define _ARCDEBUGLN(...)	DEBUGLN(__VA_ARGS__)
#define _ARCEXIT exit(-1)
#else
// This is now the default
#define _ARCDEBUG(...)
#define _ARCDEBUGLN(...)
#define _ARCEXIT
#endif // ARCSYS_ARCHIVE_APLLICATION_

namespace fs = std::filesystem;
using namespace CryptoPP;
namespace File = Makai::File;

using namespace Makai;

namespace Arch = Makai::Tool::Arch;
using namespace Arch;

using Nlohmann = nlohmann::json;
using Makai::JSON::Extern::JSONData;

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

String Arch::hashPassword(String const& str) {
	return hash<SHA3_256>(str);
}

template<class T>
static BinaryData<> cbcTransform(
	BinaryData<> const&		data,
	String					password	= "",
	uint8* const			block		= nullptr
) try {
	std::string result;
	T tf;
	uint8* iv = new uint8[16];
	if (block != nullptr)	MX::memcpy(iv, block, 16);
	else					MX::memset(iv, 0, 16);
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
	uint8* const			block		= nullptr
) {
	return cbcTransform<typename CBC_Mode<T>::Encryption>(data, password, block);
}

template<typename T>
static BinaryData<> cbcDecrypt(
	BinaryData<> const&		data,
	String const&			password	= "",
	uint8* const			block		= nullptr
) {
	return cbcTransform<typename CBC_Mode<T>::Decryption>(data, password, block);
}

BinaryData<> Arch::encrypt(
	BinaryData<> const&		data,
	String const&			password,
	EncryptionMethod const&	method,
	uint8* const			block
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
	uint8* const			block
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

static JSONData getStructure(fs::path const& path, StringList& files, String const& root) {
	JSONData dir = Nlohmann::object();
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

static StringList getFileInfo(JSONData const& filestruct) {
	StringList res;
	for (auto& [name, data]: filestruct.items()) {
		if (data.is_string())
			res.pushBack(data.get<std::string>());
		else if (data.is_object() && !data.empty())
			for (String& s: getFileInfo(data))
				res.pushBack(s);
	}
	return res;
}

static void populateTree(JSONData& tree, String const& root = "") {
	if (!tree.is_object())
		throw Error::FailedAction("file tree is not a JSON object!", CTL_CPP_PRETTY_SOURCE);
	for (auto& [name, data]: tree.items()) {
		String path = OS::FS::concatenate(root, String(name));
		if (data.is_string()) data = path;
		else if (data.is_object()) populateTree(data, path);
		else throw Error::FailedAction("Invalid data type in file tree!", CTL_CPP_PRETTY_SOURCE);
	}
}

static usize populateTree(JSONData& tree, List<uint64> const& values, usize const start = 0) {
	if (!tree.is_object())
		throw Error::FailedAction("file tree is not a JSON object!", CTL_CPP_PRETTY_SOURCE);
	usize idx = start;
	for (auto& [name, data]: tree.items()) {
		if (data.is_string()) data = encoded(values[idx++]);
		else if (data.is_object()) idx = populateTree(data, values, idx);
		else throw Error::FailedAction("Invalid data type in file tree!", CTL_CPP_PRETTY_SOURCE);
	}
	return idx;
}

static void generateBlock(As<uint8[16]>& block) {
	uint64* b = (uint64*)block;
	b[0] = rng().next();
	b[1] = rng().next();
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
		DEBUGLN("FOLDER: ", folderPath, "\nARCHIVE: ", archivePath);
		// Get file structure
		DEBUGLN("Getting file structure...");
		JSONData dir;
		StringList files;
		JSONData tree = dir["tree"];
		tree = getStructure(fs::path(folderPath.std()), files, String(fs::path(folderPath.std()).stem().string()));
		DEBUGLN("\n", dir.dump(2, ' ', false, Nlohmann::error_handler_t::replace), "\n");
		// Populate with temporary values
		List<uint64> locations;
		locations.resize(files.size(), 0);
		// Open file
		std::ofstream file;
		file.exceptions(std::ofstream::badbit | std::ofstream::failbit);
		file.open(archivePath.cstr(), std::ios::binary | std::ios::trunc);
		// Populate header
		DEBUGLN("Creating header...\n");
		// Headers
		ArchiveHeader	header;
		// Set main header params
		header.version		= ARCHIVE_VERSION;		// file format version
		header.minVersion	= ARCHIVE_MIN_VERSION;	// file format minimum version
		header.encryption	= (uint16)enc;			// encryption mode
		header.compression	= (uint16)comp;			// compression mode
		header.level		= complvl;				// compression level
		/*header.flags =
			Flags::SHOULD_CHECK_CRC_BIT				// Do CRC step
		;*/
		DEBUGLN("             HEADER SIZE: ", (uint64)header.headerSize,		"B"	);
		DEBUGLN("        FILE HEADER SIZE: ", (uint64)header.fileHeaderSize,	"B"	);
		DEBUGLN("   DIRECTORY HEADER SIZE: ", (uint64)header.dirHeaderSize,		"B"	);
		DEBUGLN("     FILE FORMAT VERSION: ", (uint64)header.version				);
		DEBUGLN(" FILE FORMAT MIN VERSION: ", (uint64)header.minVersion				);
		DEBUGLN("         ENCRYPTION MODE: ", (uint64)header.encryption				);
		DEBUGLN("        COMPRESSION MODE: ", (uint64)header.compression			);
		DEBUGLN("       COMPRESSION LEVEL: ", (uint64)header.level					);
		DEBUGLN("\nDirectory structure layout:");
		DEBUGLN("       FILE COUNT: ", files.size()			);
		// Write main header first pass
		file.write((char*)&header, header.headerSize);
		// Write file info
		DEBUGLN("\nWriting files...\n");
		usize i = 0;
		for (auto const& f: files) {
			// Get current stream position as file location
			locations[i] = file.tellp();
			// Read file
			String const loc = Regex::replace(f, "^(.*?)[\\\\\\/]", "");
			DEBUGLN("Clean path: '", loc, "'");
			String const fpath = Makai::OS::FS::concatenate(folderPath, loc);
			DEBUGLN("Full path: '", fpath, "'");
			BinaryData<> contents = File::loadBinary(fpath);
			// Prepare header
			FileHeader fheader;
			fheader.uncSize = contents.size();				// Uncompressed file size
			// Generate block
			generateBlock(fheader.block);					// Encryption block
			// Process file
			if (!contents.empty()) {
				contents = compress(
					contents,
					comp,
					complvl
				);
				DEBUGLN("Before encryption: ", contents.size());
				contents = encrypt(
					contents,
					passhash,
					enc,
					fheader.block
				);
				DEBUGLN("After encryption: ", contents.size());
			}
			fheader.compSize	= contents.size();	// Compressed file size
			fheader.crc			= 0;				// CRC (currently not working)
			// Debug info
			DEBUGLN("'", files[i], "':");
			DEBUGLN("          FILE INDEX: ", i							);
			DEBUGLN("       FILE LOCATION: ", locations[i]				);
			DEBUGLN("                 ENCODED: ", encoded(locations[i])	);
			DEBUGLN("   UNCOMPRESSED SIZE: ", fheader.uncSize,	"B"		);
			DEBUGLN("     COMPRESSED SIZE: ", fheader.compSize,	"B"		);
			DEBUGLN("               CRC32: ", fheader.crc,		"\n"	);
			// Copy header & file data
			file.write((char*)&fheader, header.fileHeaderSize);
			file.write((char*)contents.data(), contents.size());
			++i;
		}
		// Populate file tree
		populateTree(tree, locations);
		dir["tree"] = tree;
		// Process directory structure
		DEBUGLN("\nWriting directory structure...\n");
		DEBUGLN("\n", dir.dump(2, ' ', false, Nlohmann::error_handler_t::replace), "\n");
		{
			// Directory header
			DirectoryHeader	dheader;
			// Generate header block
			generateBlock(dheader.block);
			// Get directory info
			String dirInfo = dir.dump(-1, ' ', false, Nlohmann::error_handler_t::replace);
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
			DEBUGLN("  DIRECTORY INFO LOCATION: ", header.dirHeaderLoc		);
			DEBUGLN("        UNCOMPRESSED SIZE: ", dheader.uncSize,		"B"	);
			DEBUGLN("          COMPRESSED SIZE: ", dheader.compSize,	"B"	);
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
		DEBUGLN("\nDone!");
		_ARCDEBUGLN("Please run [arcgen \"YOUR_PASSWORD_HERE\"] to generate the hash to use in your game.");
	#ifdef ARCSYS_APPLICATION_
	} catch (Error::Generic const& e) {
		_ARCDEBUGLN(e.report());
		_ARCEXIT;
	} catch (std::exception const& e) {
		_ARCDEBUGLN("ERROR: ", e.what());
		_ARCEXIT;
	}
	#else
	} catch (std::exception const& e) {
		throw File::FileLoadError(e.what(), CTL_CPP_PRETTY_SOURCE);
	}
	#endif // ARCSYS_APPLICATION_
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

Arch::FileArchive::FileArchive(DataBuffer& buffer, String const& password) {open(buffer, password);}

Arch::FileArchive::~FileArchive() {close();}

FileArchive& Arch::FileArchive::open(DataBuffer& buffer, String const& password) try {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	if (streamOpen) return *this;
	// Set archive
	archive.rdbuf(&buffer);
	// Set password
	pass = password;
	// Set exceptions
	archive.exceptions(std::ifstream::badbit | std::ifstream::failbit);
	// Read header
	usize hs = 0;
	archive.read((char*)&hs, sizeof(uint64));
	archive.seekg(0);
	hs = (hs < sizeof(ArchiveHeader)) ? hs : sizeof(ArchiveHeader);
	archive.read((char*)&header, hs);
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
	if (header.flags & Flags::SINGLE_FILE_ARCHIVE_BIT)
		singleFileArchiveError();
	if (!header.dirHeaderLoc)
		directoryTreeError();
	// Read directory tree info
	parseFileTree();
	// Set open flag
	streamOpen = true;
	return *this;
} catch (std::exception const& e) {
	throw File::FileLoadError(e.what(), CTL_CPP_PRETTY_SOURCE);
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

Makai::JSON::JSONData Arch::FileArchive::getFileTree(String const& root) const {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	assertOpen();
	JSONData dir = fstruct["tree"];
	populateTree((!root.empty()) ? dir[root] : dir, root);
	return dir;
}

ArchiveHeader Arch::FileArchive::getHeader(String const& path) {
	std::ifstream af;
	ArchiveHeader ah;
	// Set exceptions
	af.exceptions(std::ifstream::badbit | std::ifstream::failbit);
	// Open file
	af.open(path.cstr(), std::ios::binary | std::ios::in);
	// Read header
	usize hs = 0;
	af.read((char*)&hs, sizeof(uint64));
	af.seekg(0);
	if (hs > sizeof(ArchiveHeader)) hs = sizeof(ArchiveHeader);
	af.read((char*)&ah, hs);
	return ah;
}

FileArchive& Arch::FileArchive::unpackTo(String const& path) {
	if (!streamOpen) return *this;
	JSONData ftree = getFileTree().json();
	_ARCDEBUGLN(ftree.dump(2, ' ', false, Nlohmann::error_handler_t::replace), "\n");
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
		archive.read(fs.data(), fs.size());
		archive.seekg(0);
		break;
	case 1:
		DirectoryHeader dh;
		archive.seekg(header.dirHeaderLoc);
		archive.read((char*)&dh, header.dirHeaderSize);
		if (!dh.compSize || !dh.uncSize) directoryTreeError();
		DEBUGLN("  DIRECTORY INFO LOCATION: ", header.dirHeaderLoc		);
		DEBUGLN("        UNCOMPRESSED SIZE: ", dh.uncSize,			"B"	);
		DEBUGLN("          COMPRESSED SIZE: ", dh.compSize,			"B"	);
		BinaryData<> pfs;
		pfs.resize(dh.compSize, 0);
		archive.read((char*)pfs.data(), pfs.size());
		archive.seekg(0);
		DEBUGLN("Demangling tree data...");
		demangleData(pfs, dh.block);
		fs.resize(pfs.size(), 0);
		MX::memcpy(fs.data(), pfs.data(), fs.size());
		if (fs.size() != dh.uncSize) directoryTreeError();
		break;
	}
	try {
		DEBUGLN("Parsing tree...");
		fstruct = Nlohmann::parse(fs.std());
	} catch (Nlohmann::exception const& e) {
		throw File::FileLoadError(
			"Invalid or corrupted file structure!",
			e.what()
		);
	}
	_ARCDEBUGLN("File Structure:\n", fstruct.dump(2, ' ', false, Nlohmann::error_handler_t::replace), "\n");
}

void Arch::FileArchive::demangleData(BinaryData<>& data, uint8* const block) const {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	_ARCDEBUGLN("Before decryption: ", data.size());
	data = decrypt(
		data,
		pass,
		(EncryptionMethod)header.encryption,
		block
	);
	_ARCDEBUGLN("After decryption: ", data.size());
	_ARCDEBUGLN("After decompression: ", data.size());
	data = decompress(
		data,
		(CompressionMethod)header.compression,
		header.level
	);
	_ARCDEBUGLN("After decompression: ", data.size());
}

void Arch::FileArchive::unpackLayer(JSONData const& layer, String const& path) {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	assertOpen();
	List<KeyValuePair<String, String>> files;
	for (auto& [name, data]: layer.items()) {
		if (data.is_string()) files.pushBack(KeyValuePair<String, String>(name, data.get<std::string>()));
		else if (data.is_object()) unpackLayer(data, path);
		else directoryTreeError();
	}
	for (auto& [name, data]: files) {
		String filepath = OS::FS::concatenate(path, data);
		_ARCDEBUGLN(path, " + ", data, " = ", filepath);
		_ARCDEBUGLN(
			"'", name, "': ",
			filepath,
			" (dir: ", OS::FS::directoryFromPath(filepath), ")"
		);
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
	if (header.flags & Flags::SHOULD_CHECK_CRC_BIT && !true) // CRC currently not working
		crcFailError(entry.path);
	entry.data = data;
}

Arch::FileArchive::FileEntry Arch::FileArchive::getFileEntry(String const& path) try {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	if (!fstruct["tree"].is_object())
		directoryTreeError();
	DEBUGLN("Getting file entry location...");
	uint64		idx	= getFileEntryLocation(path.lower(), path);
	_ARCDEBUGLN("ENTRY LOCATION: ", idx);
	DEBUGLN("Getting file entry header...");
	FileHeader	fh	= getFileEntryHeader(idx);
	_ARCDEBUGLN("   UNCOMPRESSED SIZE: ", fh.uncSize,	"B"	);
	_ARCDEBUGLN("     COMPRESSED SIZE: ", fh.compSize,	"B"	);
	_ARCDEBUGLN("               CRC32: ", fh.crc			);
	DEBUGLN("Getting file entry data...");
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
	auto lp = archive.tellg();
	archive.seekg(index + header.fileHeaderSize);
	archive.read((char*)fd.data(), fh.compSize);
	archive.seekg(lp);
	return fd;
} catch (std::ios_base::failure const& e) {
	throw Error::FailedAction(
		"Failed at getting file entry data: "s + String(e.what()),
		CTL_CPP_PRETTY_SOURCE
	);
}

FileHeader Arch::FileArchive::getFileEntryHeader(uint64 const index) try {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	FileHeader fh;
	auto lp = archive.tellg();
	archive.seekg(index);
	archive.read((char*)&fh, header.fileHeaderSize);
	archive.seekg(lp);
	return fh;
} catch (std::ios_base::failure const& e) {
	throw Error::FailedAction(
		"Failed at getting file entry header: "s + String(e.what()),
		CTL_CPP_PRETTY_SOURCE
	);
}

uint64 Arch::FileArchive::getFileEntryLocation(String const& path, String const& origpath) try {
	CTL::ScopeLock<CTL::Mutex> lock(sync);
	List<JSONData> stack;
	JSONData entry = fstruct["tree"];
	DEBUGLN("Path: ", origpath);
	DEBUGLN("Cleaned: ", Regex::replace(path, "[\\\\\\/]+", "/"));
	// Loop through path and get entry location
	for (String fld: Regex::replace(path, "[\\\\\\/]+", "/").split('/')) {
		if (fld == "..") {
			if (stack.empty())
				outOfArchiveBoundsError(origpath);
			entry = stack.popBack();
			continue;
		} else if (entry.is_object()) {
			for (auto [k, v]: entry.items())
				if (String(k).lower() == fld) {
					stack.pushBack(entry);
					entry = v;
					break;
				}
		} else if (entry.is_string() && String(entry.get<std::string>()).lower() == fld)
			return decoded(entry.get<std::string>());
		else doesNotExistError(fld);
	}
	// Try and get entry location
	if (entry.is_string())
		return decoded(entry.get<std::string>());
	else notAFileError(origpath);
} catch (Nlohmann::exception const& e) {
	doesNotExistError(origpath);
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
	_ARCDEBUGLN("\nOpening archive...\n");
	FileBuffer buffer;
	buffer.open(archivePath.cstr(), std::ios::in | std::ios::binary);
	FileArchive arc(buffer, hashPassword(password));
	buffer.close();
	_ARCDEBUGLN("\nExtracting data...\n");
	arc.unpackTo(folderPath);
#ifdef ARCSYS_APPLICATION_
} catch (Error::Generic const& e) {
	_ARCDEBUGLN(e.report());
	_ARCEXIT;
} catch (std::exception const& e) {
	_ARCDEBUGLN("ERROR: ", e.what());
	_ARCEXIT;
}
#else
} catch (std::exception const& e) {
	throw File::FileLoadError(e.what(), CTL_CPP_PRETTY_SOURCE);
}
#endif // ARCSYS_APPLICATION_

static void unpackV0(
	String const& archivePath,
	String const folderPath,
	String const& password = ""
) try {
	_ARCDEBUGLN("\nOpening archive...\n");
	FileBuffer buffer;
	buffer.open(archivePath.cstr(), std::ios::in | std::ios::binary);
	FileArchive arc(buffer, password);
	buffer.close();
	_ARCDEBUGLN("\nExtracting data...\n");
	arc.unpackTo(folderPath);
#ifdef ARCSYS_APPLICATION_
} catch (Error::Generic const& e) {
	_ARCDEBUGLN(e.report());
	_ARCEXIT;
} catch (std::exception const& e) {
	_ARCDEBUGLN("ERROR: ", e.what());
	_ARCEXIT;
}
#else
} catch (std::exception const& e) {
	throw File::FileLoadError(e.what(), CTL_CPP_PRETTY_SOURCE);
}
#endif // ARCSYS_APPLICATION_

void Arch::unpack(
	String const& archivePath,
	String const folderPath,
	String const& password
) try {
	uint64 mv;
	{
		mv = FileArchive::getHeader(archivePath).minVersion;
		_ARCDEBUGLN("Minimum Version: ", toString(mv));
	}
	switch(mv) {
		case 1: unpackV1(archivePath, folderPath, password);	break;
		case 0: unpackV0(archivePath, folderPath, password);	break;
		default: throw Error::InvalidValue(
			"Unsupported or invalid minimum version!",
			CTL_CPP_PRETTY_SOURCE
		);
	}
#ifdef ARCSYS_APPLICATION_
} catch (Error::Generic const& e) {
	_ARCDEBUGLN(e.report());
	_ARCEXIT;
} catch (std::exception const& e) {
	_ARCDEBUGLN("ERROR: ", e.what());
	_ARCEXIT;
}
#else
} catch (std::exception const& e) {
	throw File::FileLoadError(e.what(), CTL_CPP_PRETTY_SOURCE);
}
#endif // ARCSYS_APPLICATION_

BinaryData<> Arch::loadEncryptedBinaryFile(String const& path, String const& password) try {
	std::ifstream archive;
	// Set exceptions
	archive.exceptions(std::ifstream::badbit | std::ifstream::failbit);
	// Open file
	archive.open(path.cstr(), std::ios::binary | std::ios::in);
	// Get archive header
	ArchiveHeader header;
	{
		uint64 hs = 0;
		archive.read((char*)&hs, sizeof(uint64));
		archive.seekg(0);
		archive.read((char*)&header, hs);
	}
	// Check if single-file archive
	if (!(header.flags & Flags::SINGLE_FILE_ARCHIVE_BIT))
		File::FileLoadError(
			"Failed to load '" + path + "'!",
			"File is not a single-file archive!"
		);
	// Get file header
	FileHeader fh;
	archive.read((char*)&fh, header.fileHeaderSize);
	// Get file data
	BinaryData<> fd(fh.compSize, 0);
	archive.read((char*)fd.data(), fh.compSize);
	// Extract file contents
	{
		if (fh.uncSize == 0) return BinaryData<>();
		fd = decrypt(
			fd,
			password,
			(EncryptionMethod)header.encryption,
			fh.block
		);
		fd = decompress(
			fd,
			(CompressionMethod)header.compression,
			header.level
		);
		if (fd.size() != fh.uncSize)
			File::FileLoadError(
				"Failed to load '" + path + "'!",
				"Uncompressed size doesn't match!"
			);
		if ((header.flags & Flags::SHOULD_CHECK_CRC_BIT) && !true) // CRC currently not working
			File::FileLoadError(
				"Failed to load '" + path + "'!",
				"CRC check failed!"
			);
	}
	// Return file
	return fd;
} catch (std::exception const& e) {
	throw File::FileLoadError(
		"Failed to load '" + path + "'!",
		e.what(),
		CTL_CPP_PRETTY_SOURCE
	);
}

String Arch::loadEncryptedTextFile(String const& path, String const& password) {
	BinaryData<> fd = loadEncryptedBinaryFile(path, password);
	return String(List<char>(fd));
}

template<typename T>
void Arch::saveEncryptedBinaryFile(
	String const&				path,
	T* const					data,
	usize const					size,
	String const&				password,
	EncryptionMethod const&		enc,
	CompressionMethod const&	comp,
	uint8 const					lvl
) {
	if (enc != EncryptionMethod::AEM_NONE && password.empty())
		throw Error::InvalidValue(
			"Missing password for encrypted file!",
			CTL_CPP_PRETTY_SOURCE
		);
	// Open file
	std::ofstream file(path.cstr(), std::ios::binary | std::ios::trunc);
	file.exceptions(std::ofstream::badbit | std::ofstream::failbit);
	// Header
	ArchiveHeader header;
	// Set main header params
	header.dirHeaderLoc	= 0;			// directory info size
	header.encryption	= (uint16)enc;	// encryption mode
	header.compression	= (uint16)comp;	// compression mode
	header.level		= lvl;			// compression level
	header.flags =
		Flags::SINGLE_FILE_ARCHIVE_BIT	// Single-file archive
	//|	Flags::SHOULD_CHECK_CRC_BIT		// Do CRC step
	;
	// Write header
	file.write((char*)&header, header.headerSize);
	// Write file info
	{
		usize uncSize = (size*sizeof(T));
		BinaryData<> contents((uint8*)data, ((uint8*)data) + uncSize);
		// Prepare header
		FileHeader fheader;
		fheader.uncSize = uncSize;		// Uncompressed file size
		// Generate block
		generateBlock(fheader.block);	// Encryption block
		// Process file
		if (!contents.empty()) {
			contents = compress(
				contents,
				comp,
				lvl
			);
			contents = encrypt(
				contents,
				password,
				enc,
				fheader.block
			);
		}
		fheader.compSize	= contents.size();	// Compressed file size
		fheader.crc			= 0;				// CRC (currently not working)
		// Copy header & file data
		file.write((char*)&fheader, header.fileHeaderSize);
		file.write((char*)contents.data(), contents.size());
	}
	// Flush & close file
	file.flush();
	file.close();
}

void Arch::saveEncryptedTextFile(
	String const&				path,
	BinaryData<> const&			data,
	String const&				password,
	EncryptionMethod const&		enc,
	CompressionMethod const&	comp,
	uint8 const				lvl
) {
	saveEncryptedBinaryFile(path, data.data(), data.size(), password, enc, comp, lvl);
}

template<typename T>
void Arch::saveEncryptedBinaryFile(
	String const&				path,
	List<T> const&				data,
	String const&				password,
	EncryptionMethod const&		enc,
	CompressionMethod const&	comp,
	uint8 const				lvl
) {
	saveEncryptedBinaryFile<T>(path, data.data(), data.size(), password, enc, comp, lvl);
}

void Arch::saveEncryptedTextFile(
	String const&				path,
	String const&				data,
	String const&				password,
	EncryptionMethod const&		enc,
	CompressionMethod const&	comp,
	uint8 const				lvl
) {
	saveEncryptedBinaryFile(path, data.data(), data.size(), password, enc, comp, lvl);
}
