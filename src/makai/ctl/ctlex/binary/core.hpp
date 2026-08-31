#ifndef CTL_EX_BINARY_CORE_H
#define CTL_EX_BINARY_CORE_H

#include "../../ctl/exnamespace.hpp"
#include "../../ctl/ctl.hpp"

CTL_EX_NAMESPACE_BEGIN

namespace BinaryFormat {
	using IReadable = IInputStream<Bytes<>>;

	struct IWritable: IOutputStream<ConstByteSpan<>> {
		virtual ~IWritable() {}

		template <class T>
		void put(T const& data) {
			write({
				(ref<byte const>)&data,
				sizeof(T)
			});
		}

		template <class T>
		void append(List<T> const& data) {
			write({
				(ref<byte const>)data.data(),
				data.size() * sizeof(T)
			});
		}
	};

	struct Reader: IReadable {
		Bytes<>	input;
		usize	pointer = 0;

		Bytes<> read(usize const count) override {
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

		usize position() const override {return pointer;}

		Reader(Bytes<> const& input): input(input) {}
	};

	struct Writer: IWritable {
		Bytes<>&	output;
		usize		pointer = 0;

		usize position() const override {return pointer;}

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

		Writer(Bytes<>& output): output(output) {}
	};

	struct [[CTL_PACKED_STRUCT]] Version {
		uint64 major			= 0;
		uint64 minor			= 0;
		uint64 patch			= 0;
		uint64 hotfix			= 0;
		uint64 lateNightPush	= 0;
	};

	struct [[CTL_PACKED_STRUCT]] Entry {
		uint64 start	= 0;
		uint64 size		= 0;

		static Nullable<Entry> build(IReadable& source) {
			auto block = source.read(sizeof(Entry));
			if (block.size() < sizeof(Entry)) return null;
			return {*(Entry*)block.data()};
		}
	};

	template <class T>
	struct [[CTL_PACKED_STRUCT]] Header: Entry {
		Nullable<T> fromBytes(IReadable& source) const {
			source.go(start);
			auto const sz = size < sizeof(T) ? size : sizeof(T);
			T out;
			auto const block = source.read(size);
			if (block.size() < sz) return null;
			MX::memmove(&out, block.data(), sz);
			return out;
		}
	};
	template <class T, auto CONVERT = [] (Bytes<> const&) -> Nullable<T> {return null;}>
	struct [[CTL_PACKED_STRUCT]] Table: Entry {
		using EntryType = T;
		constexpr static auto const convert = CONVERT;

		template <class TFunc = decltype(convert)>
		Nullable<T> readFromSource(IReadable& source, usize const index, TFunc const convert = CONVERT) const
		requires (
			Type::Functional<TFunc, Nullable<T>(Bytes<> const&)>
		or	Type::Functional<TFunc, T(Bytes<> const&)>
		) {
			if (index >= size) return null;
			source.go(start + index * sizeof(Entry));
			auto entryBlock = source.read(sizeof(Entry));
			if (entryBlock.size() < sizeof(Entry)) return null;
			auto const entry = *(Entry*)entryBlock.data();
			source.go(entry.start);
			auto block = source.read(entry.size);
			if (block.size() != entry.size) return null;
			return wrap<Nullable>(convert(block));
		}

		template <class TFunc = decltype(convert)>
		Nullable<List<T>> fromBytes(IReadable& source, TFunc const convert = CONVERT) const
	 	requires (
			Type::Functional<TFunc, Nullable<T>(Bytes<> const&)>
		or	Type::Functional<TFunc, T(Bytes<> const&)>
		) {
			List<T> out;
			auto const sz = (size / sizeof(Entry));
			for (usize i = 0; i < sz; ++i)
				if (auto const v = readFromSource(source, i))
					out.pushBack(v.value());
				else return null;
			return wrap<Nullable>(out);
		}
	};

	namespace Convert {
		template <class T>
		constexpr T toString(Bytes<> const& block) {
			return T(
				String(
					(cstring)block.cbegin(),
					(cstring)block.cend()
				)
			);
		}

		template <class T>
		constexpr List<T> toList(Bytes<> const& block) {
			List<T> out;
			out.reserve(block.size() / sizeof(T), T());
			MX::excopy<T>(out.data(), (ref<T const>)block.data(), out.size());
			return wrap<Nullable>(out);
		}

		template <class T>
		constexpr Nullable<T> toValue(Bytes<> const& block) {
			T out;
			if (block.size() < sizeof(T)) return null;
			MX::construct(&out, *(ref<T const>)block.data());
			return wrap<Nullable>(out);
		}

		template <class T>
		constexpr T toHeader(Bytes<> const& block) {
			T out;
			MX::memmove(&out, block.data(), block.size() < sizeof(T) ? block.size() : sizeof(T));
			return out;
		}
	}

	struct [[CTL_PACKED_STRUCT]] Text: Entry {
		template <Type::OneOf<String, UTF8String, UTF32String> T>
		Nullable<T> fromBytes(IReadable& source) const {
			if (!size) return {T()};
			source.go(start);
			auto block = source.read(size);
			if (block.size() != size) return null;
			return wrap<Nullable>(Convert::toString<T>(block));
		}
	};

	template <class T>
	struct [[CTL_PACKED_STRUCT]] Data: Entry {
		Nullable<List<T>> fromBytes(IReadable& source) const {
			if (!size) return {List<T>()};
			source.go(start);
			auto block = source.read(size);
			if (block.size() != size) return null;
			return wrap<Nullable>(listFromBytes<T>(block));
		}
	};

	template <Type::OneOf<String, UTF8String, UTF32String> T = String>
	using StringTable = Table<T, Convert::toString<T>>;

	template <Type::NoneOf<String, UTF8String, UTF32String> T>
	using ValueTable = Table<T, Convert::toValue<T>>;

	template <Type::NoneOf<String, UTF8String, UTF32String> T>
	using HeaderTable = Table<T, Convert::toHeader<T>>;

	template <class T, auto F = Table<T>::convert>
	constexpr Nullable<List<T>> unpack(Table<T, F> const& table, IReadable& source) {
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
}

CTL_EX_NAMESPACE_END

#endif
