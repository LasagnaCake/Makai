#include "file.hpp"

using namespace Makai::Pack;

using namespace Makai;

Archive::Archive() {}

Archive::~Archive() {
	close();
}

static CTL::Random::Engine::Secure& rng() {
	static CTL::Random::Engine::Secure r;
	return r;
}

String Block::createFor(Encryption const mode) {
	switch (mode) {
		case Encryption::PE_NO_ENCRYPTION: return "";
		case Encryption::PE_AES256: {
			String str;
			str.resize(16, 0);
			for (auto& c: str)
				c = rng().next() % 0xFF;
			return str;
		};
	}
	return "";
}

Nullable<Archive::LoadError> Archive::open(String const& passkey, Source&& inx, Destination&& outx, Nullable<PackInfo> const& info) {
	if (isOpen) return LoadError::PLE_ALREADY_OPEN;
	inBuf.clear();
	outBuf.clear();
	key = passkey;
	if (inx)	in = move(inx);
	if (outx)	out = move(outx);
	if (in) {
		auto const pp = Header<PackInfo>::build(*in);
		if (!pp) return LoadError::PLE_BROKEN_HEADER;
		if ((pack = Header<PackInfo>(*pp).fromBytes(*in))) {
			if (FILE_FORMAT_VERSION.major < pack.value().version.major)
				return LoadError::PLE_FILE_IS_TOO_NEW;
			if (auto const dir = pack.value().directory.fromBytes(*in)) {
				auto const ddx = dir.value();
				auto const files = unpack(ddx.files, *in);
				auto const names = unpack<String>(ddx.names, *in);
				if (files && names) {
					auto const fx = *files;
					auto const nx = *names;
					auto const bbx = ddx.block.fromBytes<String>(*in);
					for (usize i = 0; i < Math::lowest(fx.size(), nx.size()); ++i) {
						auto const q = nx[i].toBytes();
						unpack(q.span(), bbx, hash, ddx.mode.encryption, ddx.mode.compression, ddx.mode.level)
							.then([&] (auto const ex) {
								inBuf[UTF8String::fromBytes(ex)] = fx[i];
							})
						;
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

Nullable<Archive::LoadError> Archive::open(String const& passkey, Source&& inx) {
	return open(passkey, move(inx), {}, null);
}

Nullable<Archive::LoadError> Archive::open(String const& passkey, Destination&& outx, PackInfo const& info) {
	return open(passkey, {}, move(outx), info);
}

Nullable<Bytes<>> File::unpackFrom(BF::IReadable& reader, String const& key) const {
	auto const bbx = block.fromBytes<String>(reader);
	if (!bbx) return null;
	auto const block = bbx.value();
	if (auto const fdat = contents.fromBytes(reader)) {
		Nullable<Bytes<>> fout;
		fdat.then([&] (auto const& fe) {
			if (fe.size() != packed) return;
			unpack(fe.span(), block, key, mode.encryption, mode.compression, mode.level)
				.then([&] (auto const& xe) {
					fout = xe;
				})
			;
		});
		if (!fout) return null;
		if (fout.value().size() != unpacked)
			return null;
		return fout;
	} else return null;
}

Result<Archive::FileEntry, Archive::GetError> Archive::tryGet(UTF8String const& path) const {
	if (!isOpen) return GetError::PGE_NOT_OPEN;
	if (!in) return GetError::PGE_WRITE_ONLY;
	if (outBuf.contains(path))
		return outBuf[path];
	else if (inBuf.contains(path) && inBuf[path]) {
		auto const fheader = *inBuf[path];
		if (auto const fdat = fheader.unpackFrom(*in, key)) {
			FileEntry const fe = {*fdat, fheader.mode};
			outBuf[path] = fe;
			return fe;
		} else return GetError::PGE_BROKEN_FILE;
	} else return GetError::PGE_NO_FILE;
}

Nullable<Archive::AddError> Archive::tryAdd(UTF8String const& path, Archive::FileEntry const& file) {
	if (!isOpen) return AddError::PAE_NOT_OPEN;
	if (!out) return AddError::PAE_READ_ONLY;
	inBuf[path] = null;
	outBuf[path] = file;
	return null;
}

Archive& Archive::remove(UTF8String const& path) {
	if (!isOpen) return *this;
	inBuf.erase(path);
	outBuf.erase(path);
	return *this;
}

bool Archive::contains(UTF8String const& path) const {
	if (!isOpen) return false;
	return (inBuf.contains(path) or outBuf.contains(path));
}

Nullable<UTF8StringList> Archive::files() const {
	if (!isOpen) return null;
	UTF8Dictionary<bool> checked;
	for (auto const& [file, x]: inBuf)
		if (x) checked[file] = true;
	for (auto const& [file, _]: outBuf)
		checked[file] = true;
	return checked.keys();
}

Archive& Archive::close() {
	if (!isOpen) return *this;
	if (out) {
		BF::Encoder<PackInfo> builder(*out);
		builder
			.begin()
			.run([&] (BF::Encoder<PackInfo>& builder) {
				builder.file = pack;
				Directory dd;
				List<Entry> files;
				List<Bytes<>> names;
				auto const dblk = Block::createFor(fd.mode.encryption);
				dd.block = builder.append(dblk);
				for (auto const file: files().orElse({})) {
					tryGet(file).then([&] (auto const& fd) {
						File fzx {.unpacked = fd.data.size(), .mode = fd.mode};
						auto const blk = Block::createFor(fd.mode.encryption);
						pack(fd.data.span(), blk, key, fd.mode.encryption, fd.mode.compression, fd.mode.level)
							.then([&] (auto const cd) {
								fzx.packed = cd.size();
								fzx.data = builder.append(cd);
								fzx.block = builder.append(blk);
							})
						;
						files.pushBack(builder.put(fzx));
						auto const fnb = fd.name.toString().toBytes();
						pack(fnb.span(), blk, key, fd.mode.encryption, dd.mode.compression, dd.mode.level)
							.then([&] (auto const& ed) {
								names.pushBack(ed);
							})
						;
					})
				}
				dd.files = builder.embed(files);
				dd.names = builder.embed(names);
				builder.file.directory = builder.put(dd);
			})
			.end();
	}
	return *this;
}
