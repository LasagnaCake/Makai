#ifndef MAKAILIB_ANIMA_V2_CORE_BINARY_H
#define MAKAILIB_ANIMA_V2_CORE_BINARY_H

#include "module.hpp"

namespace Makai::Anima::V2::Core::BinaryFormat {
	struct IConsumable {
		virtual ~IConsumable() {}

		virtual Bytes<>	consume(usize const count)	= 0;
		virtual void	go(usize const pos = 0)		= 0;
	};

	struct ByteReader: IConsumable {
		Bytes<>	source;
		usize	pointer = 0;

		Bytes<> consume(usize const count) override {
			if (pointer > source.size())
				return {};
			pointer += count;
			if (pointer > source.size())
				return source.sliced(pointer - count, -1);
			return source.sliced(pointer - count, pointer);
		}

		void go(usize const pos) override {
			pointer = pos < (source.size() - 1) ? pos : source.size() - 1;
		}

		ByteReader(Bytes<> const& source = {}): source(source) {}
	};

	template <class T>
	struct [[gnu::packed, gnu::aligned(1)]] Header {
		uint64 const size = sizeof (T);
		T data;

		static Nullable<Header> build(IConsumable& source) {
			auto block = source.consume(sizeof(uint64));
			if (block.size() < sizeof(uint64)) return null;
			auto const s = *(uint64*)block.data();
			auto const sz = s < sizeof(T) ? s : sizeof(T);
			Header out{.size = sz};
			auto block = source.consume(s);
			if (block.size() < sz) return null;
			MX::memmove(&out.data, block.data(), sz);
		}
	};

	struct [[gnu::packed, gnu::aligned(1)]] Entry {
		uint64 start;
		uint64 size;
	};

	template <class T, auto CONVERT = [] (Bytes<> const&) -> Nullable<T> {return null;}>
	struct [[gnu::packed, gnu::aligned(1)]] Table: Entry {
		template <Type::Functional<Nullable<T>(Bytes<> const&)> TFunc>
		Nullable<T> readFromSource(IConsumable& source, usize const index, TFunc const convert = CONVERT) const {
			if (index >= size) return null;
			source.go(start + index);
			auto entryBlock = source.consume(sizeof(Entry));
			if (entryBlock.size() < sizeof(Entry)) return null;
			auto const entry = *(Entry*)entryBlock.data();
			source.go(entry.start);
			auto block = source.consume(entry.size);
			if (block.size() != entry.size) return null;
			return convert(block);
		}
	};

	template <class T>
	constexpr Nullable<T> stringFromBytes(Bytes<> const& block) {
		return T(
			String(
				(ref<typename T::DataType>)block.data(),
				block.size() / sizeof(typename T::DataType)
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
	constexpr Nullable<Header<T>> headerFromBytes(Bytes<> const& block) {
		ByteReader reader{block};
		return T::build(reader);
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
		Nullable<T> fromBytes(IConsumable& source) {
			source.go(start);
			auto block = source.consume(size);
			if (block.size() != size) return "";
			return stringFromBytes<T>(block).value();
		}
	};

	template <class T>
	struct [[gnu::packed, gnu::aligned(1)]] Data: Entry {
		Nullable<List<T>> fromBytes(IConsumable& source) {
			source.go(start);
			auto block = source.consume(size);
			if (block.size() != size) return {};
			return listFromBytes<T>(block).value();
		}
	};

	template <Type::OneOf<String, UTF8String, UTF32String> T = String>
	using StringTable = Table<T, stringFromBytes<T>>;

	template <class T>
	using HeaderTable = Table<Header<T>, headerFromBytes<T>>;

	template <class T>
	using ValueTable = Table<T, valueFromBytes<T>>;

	struct [[gnu::packed, gnu::aligned(1)]] Label: Text {
		uint64 id;
	};

	struct [[gnu::packed, gnu::aligned(1)]] Base {
		uint64	id;
		Text	name;
		uint64	hash;
		uint64	flags;
		Text	meta;
	};

	struct [[gnu::packed, gnu::aligned(1)]] Method: Base {
		uint64				returnType;
		ValueTable<uint64>	argTypes;
		uint64				entry;
		uint64				size;
	};

	struct [[gnu::packed, gnu::aligned(1)]] Decl: Base {
		BasicType			basic;
		uint64				base;
		uint64				byteSize;
		uint64				alignment;
		ValueTable<uint64>	fields;
	};

	struct [[gnu::packed, gnu::aligned(1)]] Module {
		Entry	types;
		Entry	methods;
	};

	struct [[gnu::packed, gnu::aligned(1)]] External {
		StringTable<> modules;
	};

	struct [[gnu::packed, gnu::aligned(1)]] Shared {
		StringTable<> libraries;
		StringTable<> modules;
		StringTable<> interops;
	};

	struct [[gnu::packed, gnu::aligned(1)]] ANI {
		ValueTable<Label>	in;
		StringTable<Text>	out;
		Header<Shared>		shared;
	};

	struct [[gnu::packed, gnu::aligned(1)]] FileStructure {
		Type					type;
		As<unt64[4]>			version;
		StringTable<UTF8String>	strings;
		ValueTable<uint64>		jumps;
		Data<Instruction>		code;
		Data<uint64>			relocations;
		Header<ANI>				ani;
		Header<Module>			module;
		Header<External>		external;
	};
}

#endif
