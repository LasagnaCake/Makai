#include "core.hpp"

using namespace Makai::Pack;

using namespace Makai;

Pack::Pack() {}

Pack::~Pack() {
	close();
}

Nullable<Pack::LoadError> Pack::open(String const& passHash, Source&& inx, Destination&& outx, PackInfo const& info) {
	if (isOpen) return LoadError::PLE_ALREADY_OPEN;
	inBuf.clear();
	outBuf.clear();
	pass = passHash;
	in = inx;
	out = outx;
	if (in) {
		auto const pp = Header<PackInfo>::build(*in);
		if (!pp) return LoadError::PLE_BROKEN_HEADER;
		if (pack = Header<PackInfo>(*pp).fromBytes(*in)) {
			if (auto const dir = unpack(pack->directory)) {
				auto const files = unpack(dir->files);
				auto const names = unpack<Bytes<>>(dir->names);
				if (files && names) {
					auto const fx = *files;
					auto const nx = *names;
					for (usize i = 0; i < Math::lowest(fx.size(), nx.size()); ++i) {
						decrypt(nx[i], dir.block, pass, dir.mode.encryption).then([&] (auto const e) {
							decompress(e, dir.mode.compression, dir.mode.level).then([&] (auto const ex) {
								inBuf[UTF8String(ex)] = files[i];
							});
						});
					}
				} else return LoadError::PLE_BROKEN_DIR_STRUCTURE;
			} else return LoadError::PLE_BROKEN_DIR_HEADER;
		} else return LoadError::PLE_BROKEN_HEADER;
	} else if (!info)
		return LoadError::PLE_NO_HEADER;
	else pack = info;
	isOpen = true;
	return null;
}

Result<Pack::FileEntry, Pack::GetError> Pack::tryGet(UTF8String const& path) {
	if (!isOpen) return GetError::PGE_NOT_OPEN;
	if (!in) return GetError::PGE_WRITE_ONLY;
	if (outBuf.contains(path))
		return inBuf[path].data;
	else if (inBuf.contains(path)) {
		if (auto const fhp = inBuf[path]) {
			auto const fheader = *fhp;
			auto const bbx = fheader.block.fromBytes(*in);
			if (!bbx) return GetError::PGE_BROKEN_FILE;
			auto const block + *bbx;
			if (auto const fdat = fheader.contents.fromBytes(*in)) {
				if (fdat->size() != fheader.compressed) return GetError::PGE_BROKEN_FILE;
				Nullable<Bytes<>> fout;
				fdat.then([&] (auto const& fe) {
					decrypt(fe, block, hash, fheader.mode.encryption).then([&] (auto const& ce) {
						decompress(ce, fheader.mode.compression, fheader.mode.level).then([&] (auto const& xe) {
							fout = xe;
						});
					});
				});
				if (!fout) return GetError::PGE_BROKEN_FILE;
				if (fout->size() != fheader.uncompressed)
					return GetError::PGE_BROKEN_FILE;
				return {*fout, fheader.mode};
			} else return GetError::PGE_BROKEN_FILE;
		} else return GetError::PGE_BROKEN_FILE_HEADER;
	} else return GetError::PGE_NO_FILE;
}

Nullable<Pack::AddError> Pack::tryAdd(UTF8String const& path, Pack::FileEntry const& file) {
	if (!isOpen) return GetError::PAE_NOT_OPEN;
	if (!out) return GetError::PAE_READ_ONLY;
	inBuf.erase(path);
	outBuf[path] = file;
	return null;
}

Pack& remove(UTF8String const& path) {
	if (!isOpen) return *this;
	inBuf.erase(path);
	outBuf.erase(path);
	return *this;
}

bool contains(UTF8String const& path) const {
	if (!isOpen) return false;
	return (inBuf.contains(path) or outBuf.contains(path));
}

Pack& close() {
	if (!isOpen) return *this;
	if (out) {
		BF::Encoder<PackInfo> builder(*out);
		builder
			.begin()
			.run([&] (BF::Encoder<PackInfo>& builder) {
				builder.file = pack;
				for (auto const file: files()) {
					tryGet(file).then([&] (auto const& fd) {
						File fzx {.uncompressed = fd.data.size(), .mode = fd.mode};
						compress(fd.data, fd.mode.compression, fd.mode.level).then([&] (auto const cd) {
							fzx.compressed = cd.size();
							auto const blk = Block::createFor(fd.mode.encryption);
							encrypt(cd, blk, pass, fd.mode.encryption).then([&] (auto const& ed) {
								fzx.block = builder.append(blk);
							});
						});
						builder.append();
					})
				}
			})
			.end();
	}
	return *this;
}
