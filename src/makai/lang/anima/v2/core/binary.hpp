#ifndef MAKAILIB_ANIMA_V2_CORE_BINARY_H
#define MAKAILIB_ANIMA_V2_CORE_BINARY_H

#include "module.hpp"

namespace Makai::Anima::V2::Core::BinaryFormat {
	using IReadable = IInput<Bytes<>>;

	struct IWritable: IOutput<ConstByteSpan<>> {
		virtual ~IWritable() {}

		template <class T>
		void put(T const& data) {
			write({
				(ref<byte>)&data,
				sizeof(T)
			});
		}

		template <class T>
		void append(List<T> const& data) {
			write({
				(ref<byte>)data.data(),
				data.size() * sizeof(T)
			});
		}
	};

	struct ByteReader: IReadable {
		Bytes<>	input;
		usize	pointer = 0;

		Bytes<> read(usize const count) override {
			DEBUGLN("Size: ", input.size());
			DEBUGLN("Reading ", count, " bytes at index ", pointer, "...");
			if (pointer > input.size())
				return {};
			pointer += count;
			if (pointer > input.size())
				return input.sliced(pointer - count, -1);
			return input.sliced(pointer - count, pointer - 1);
		}

		void go(usize const pos) override {
			pointer = pos < (input.size() - 1) ? pos : input.size() - 1;
		}

		ByteReader(Bytes<> const& input): input(input) {}
	};

	struct ByteWriter: IWritable {
		Bytes<>&	output;
		usize		pointer = 0;

		void write(ConstByteSpan<> const& bytes) override {
			if (pointer < output.size()) {
				if (output.size() < pointer + bytes.size())
					output.reserve(pointer + bytes.size(), 0);
				MX::memmove(output.data() + pointer, bytes.data(), bytes.size());
			}
			else output.appendBack(bytes.begin(), bytes.end());
			pointer += bytes.size();
		}

		void go(usize const pos) override {
			pointer = pos < (output.size() - 1) ? pos : output.size() - 1;
		}

		ByteWriter(Bytes<>& output): output(output) {}
	};

	struct [[gnu::packed, gnu::aligned(1)]] Entry {
		uint64 start;
		uint64 size;

		static Nullable<Entry> build(IReadable& source) {
			auto block = source.read(sizeof(Entry));
			if (block.size() < sizeof(Entry)) return null;
			return {*(Entry*)block.data()};
		}
	};

	template <class T>
	struct [[gnu::packed, gnu::aligned(1)]] Header: Entry {
		Nullable<T> fromBytes(IReadable& source) const {
			source.go(start);
			auto const sz = size < sizeof(T) ? size : sizeof(T);
			T out;
			auto const block = source.read(size);
			DEBUGLN("Size [", size, " : ", block.size(), "]");
			if (block.size() < sz) return null;
			MX::memmove(&out, block.data(), sz);
			return out;
		}
	};

	template <class T, auto CONVERT = [] (Bytes<> const&) -> Nullable<T> {return null;}>
	struct [[gnu::packed, gnu::aligned(1)]] Table: Entry {
		using EntryType = T;
		constexpr static auto const convert = CONVERT;

		template <Type::Functional<Nullable<T>(Bytes<> const&)> TFunc = decltype(convert)>
		Nullable<T> readFromSource(IReadable& source, usize const index, TFunc const convert = CONVERT) const {
			DEBUGLN("[", String(nameof<T>()), " @ ", index, "]");
			if (index >= size) return null;
			source.go(start + index * sizeof(Entry));
			DEBUGLN("Reading entry...");
			auto entryBlock = source.read(sizeof(Entry));
			if (entryBlock.size() < sizeof(Entry)) return null;
			auto const entry = *(Entry*)entryBlock.data();
			source.go(entry.start);
			DEBUGLN("Reading data...");
			auto block = source.read(entry.size);
			DEBUGLN("Size [", size, " : ", block.size(), "]");
			if (block.size() != entry.size) return null;
			return convert(block);
		}

		template <Type::Functional<Nullable<T>(Bytes<> const&)> TFunc = decltype(convert)>
		Nullable<List<T>> fromBytes(IReadable& source, TFunc const convert = CONVERT) const {
			List<T> out;
			auto const sz = (size / sizeof(Entry));
			DEBUGLN("[Table<", String(nameof<T>()), "> : ", sz, "]");
			for (usize i = 0; i < sz; ++i)
				if (auto const v = readFromSource(source, i))
					out.pushBack(v.value());
				else return null;
			return out;
		}
	};

	template <class T>
	constexpr Nullable<T> stringFromBytes(Bytes<> const& block) {
		return T(
			String(
				(cstring)block.cbegin(),
				(cstring)block.cend()
			)
		);
	}

	template <class T>
	constexpr Nullable<List<T>> listFromBytes(Bytes<> const& block) {
		return List<T>(
			(ref<T>)block.cbegin(),
			(ref<T>)block.cend()
		);
	}

	template <class T>
	constexpr Nullable<T> valueFromBytes(Bytes<> const& block) {
		T out;
		if (block.size() < sizeof(T)) return null;
		MX::memmove(&out, block.data(), sizeof(T));
		return out;
	}

	struct [[gnu::packed, gnu::aligned(1)]] Text: Entry {
		template <Type::OneOf<String, UTF8String, UTF32String> T>
		Nullable<T> fromBytes(IReadable& source) const {
			if (!size) return {T()};
			source.go(start);
			auto block = source.read(size);
			DEBUGLN("Size [", size, " : ", block.size(), "]");
			if (block.size() != size) return null;
			return stringFromBytes<T>(block).value();
		}
	};

	template <class T>
	struct [[gnu::packed, gnu::aligned(1)]] Data: Entry {
		Nullable<List<T>> fromBytes(IReadable& source) const {
			if (!size) return {List<T>()};
			source.go(start);
			auto block = source.read(size);
			DEBUGLN("Size [", size, " : ", block.size(), "]");
			if (block.size() != size) return null;
			return listFromBytes<T>(block).value();
		}
	};

	template <Type::OneOf<String, UTF8String, UTF32String> T = String>
	using StringTable = Table<T, stringFromBytes<T>>;

	template <Type::NoneOf<String, UTF8String, UTF32String> T>
	using ValueTable = Table<T, valueFromBytes<T>>;

	template <class T>
	constexpr Nullable<List<T>> unpack(StringTable<T> const& table, IReadable& source) {
		return table.fromBytes(source);
	}

	template <class T>
	constexpr Nullable<List<T>> unpack(ValueTable<T> const& table, IReadable& source) {
		return table.fromBytes(source);
	}

	template <class T>
	constexpr Nullable<List<T>> unpack(Data<T> const& data, IReadable& source) {
		return data.fromBytes(source);
	}

	template <class T>
	constexpr Nullable<T> unpack(Text const& data, IReadable& source) {
		return data.fromBytes<T>(source);
	}

	struct [[gnu::packed, gnu::aligned(1)]] Label: Text {
		uint64 id;
	};

	struct [[gnu::packed, gnu::aligned(1)]] Record {
		uint64	id;
		Text	name;
	};

	struct [[gnu::packed, gnu::aligned(1)]] Symbol: Record {
		uint64	hash;
		uint64	flags;
		Text	meta;
	};

	struct [[gnu::packed, gnu::aligned(1)]] Mapping {
		uint64 src;
		uint64 dst;
	};

	struct [[gnu::packed, gnu::aligned(1)]] Method: Symbol {
		uint64				returnType;
		ValueTable<uint64>	argTypes;
		uint64				entry;
		uint64				size;
	};

	struct [[gnu::packed, gnu::aligned(1)]] Decl: Symbol {
		BasicType			basic;
		uint64				base;
		uint64				byteSize;
		uint64				alignment;
		ValueTable<uint64>	fields;
	};

	struct [[gnu::packed, gnu::aligned(1)]] Module {
		ValueTable<Decl>	types;
		ValueTable<Method>	methods;
	};

	struct [[gnu::packed, gnu::aligned(1)]] Include {
		uint64				module;
		ValueTable<Record>	types;
		ValueTable<Record>	methods;
	};

	struct [[gnu::packed, gnu::aligned(1)]] External {
		ValueTable<Include> modules;
	};

	struct [[gnu::packed, gnu::aligned(1)]] Shared {
		StringTable<String> libraries;
		StringTable<String> modules;
		StringTable<String> interops;
	};

	struct [[gnu::packed, gnu::aligned(1)]] ANI {
		ValueTable<Label>	in;
		StringTable<String>	out;
		Header<Shared>		shared;
	};

	struct [[gnu::packed, gnu::aligned(1)]] FileStructure {
		struct [[gnu::packed, gnu::aligned(1)]] Version {
			uint64 major	= 0;
			uint64 minor	= 0;
			uint64 patch	= 0;
			uint64 hotfix	= 0;
		};
		As<char const[10]>		magic = "AV2::ANPB";
		Core::Module::Type		type;
		Version					artVersion		= {1};
		Version					binVersion		= {1};
		Version					moduleVersion	= {1};
		uint64					flags;
		uint64					moduleFlags;
		uint64					entry;
		uint64					totalTypes;
		uint64					totalMethods;
		StringTable<String>		strings;
		Data<uint64>			jumps;
		Data<Instruction>		code;
		Data<uint64>			relocations;
		Header<ANI>				ani;
		Header<Module>			module;
		Header<External>		external;

		bool has(uint64 const bit) const {
			return flags & bit;
		}
	};

	using FileHeader = Header<FileStructure>;

	struct Error {
		String message;
	};

	struct Builder {
		Bytes<> output;
		FileStructure file;
		ByteWriter writer = {output};

		template <class T>
		Entry put(T const& value) {
			Entry entry;
			entry.start = writer.pointer;
			writer.put(value);
			entry.size = sizeof(T);
			return entry;
		}

		template <class T>
		Entry append(List<T> const& values) {
			Entry entry;
			entry.start = writer.pointer;
			writer.append(values);
			entry.size = writer.pointer - entry.start;
			return entry;
		}


		template <Type::OneOf<String, UTF8String, UTF32String> T>
		Entry append(T const& value) {
			Entry entry;
			entry.start = writer.pointer;
			String s = value;
			writer.write({(ref<byte const>)s.data(), s.size()});
			entry.size = writer.pointer - entry.start;
			return entry;
		}

		template <class T>
		Entry add(T const& value) {
			return put(put(value));
		}

		template <class T>
		Entry insert(T const& value) {
			return put(append(value));
		}

		template <class T>
		Entry include(List<T> const& values) {
			List<Entry> headers;
			for (auto const& value: values)
				headers.pushBack(put(value));
			return append(headers);
		}

		template <class T>
		Entry embed(List<T> const& values) {
			List<Entry> headers;
			for (auto const& value: values)
				headers.pushBack(append(value));
			return append(headers);
		}

		template <class T>
		Entry pack(List<T> const& values) {
			List<Entry> headers;
			for (auto const& value: values)
				headers.pushBack(add(value));
			return append(headers);
		}

		template <class T>
		Entry pack(List<List<T>> const& values) {
			List<Entry> headers;
			for (auto const& value: values)
				headers.pushBack(insert(value));
			return append(headers);
		}

		Builder& begin() {
			put(Entry(0, sizeof(FileStructure)));
			return *this;
		}


		template <Type::Functional<void(Builder&)> TFunc>
		Builder& run(TFunc const& func) {
			func(*this);
			return *this;
		}

		template <Type::Functional<Entry(Builder&)> TFunc>
		Entry process(TFunc const& func) {
			return func(*this);
		}

		template <Type::Functional<Entry(Builder&)> TFunc>
		Entry processIf(bool const cond, TFunc const& func) {
			if (!cond) return {};
			return func(*this);
		}

		Builder& end() {
			Entry fin = put(file);
			writer.go(0);
			writer.put(fin);
			return *this;
		}
	};

	Result<Core::Module, Error>	fromBytes(Bytes<> const& source);
	Result<Bytes<>, Error>		toBytes(Core::Module const& source);
}

#endif
