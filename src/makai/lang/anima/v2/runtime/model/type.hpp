#ifndef MAKAILIB_ANIMA_V2_RUNTIME_MODEL_TYPE_H
#define MAKAILIB_ANIMA_V2_RUNTIME_MODEL_TYPE_H

#include "allocator.hpp"
#include "forward.hpp"
#include "../../core/core.hpp"

namespace Makai::Anima::V2::Runtime {
	using Core::Operator;
	using Core::BasicType;
	using Core::TypeFlags;
	using Core::Flagged;
	using Core::Entry;

	struct TypeModel: Entry, Flagged<TypeFlags> {
		using Source = MemorySlice<byte, Allocator>;

		struct Proxy {
			using Constructor	= Functor<void(Source&)>;
			using Destructor	= Functor<void(Source&)>;
			using Cloner		= Functor<void(Source&, ConstByteSpan<> const&)>;
			using Comparator	= Functor<int64(ConstByteSpan<> const&, ConstByteSpan<> const&)>;

			Constructor	construct;
			Destructor	destruct;
			Cloner		copy;
			Comparator	compare;

			static AtomicCell<Proxy> implementFor(BasicType const type);
		};

		bool canBecome(AtomicCell<TypeModel> const& type) const {
			if (type == base) return true;
			auto current = base;
			if (!base) return false;
			do if (current == type) return true;
			while ((current = current->base));
			return false;
		}

		static void makeBasic(TypeModel& type);

		String cleanName() const;

		Nullable<BasicType>			basic;
		AtomicCell<TypeModel>		base;
		uint64						byteSize	= 0;
		uint64						alignment	= 1;
		List<AtomicCell<TypeModel>>	fields;

		Data::Value::ObjectType		meta;

		AtomicCell<Proxy>			proxy;
	};
}

#endif
