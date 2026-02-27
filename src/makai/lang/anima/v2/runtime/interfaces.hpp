#ifndef MAKAILIB_ANIMA_V2_RUNTIME_INTERFACES_H
#define MAKAILIB_ANIMA_V2_RUNTIME_INTERFACES_H

#include "../instruction.hpp"

namespace Makai::Anima::V2::Runtime {
	struct Object;

	struct IOperator: ID::Identifiable<IOperator const> {
		enum class Operand {
			AV2_ROO_VALUE	= 0,
			AV2_ROO_LHS		= 0,
			AV2_ROO_RHS		= 1
		};

		IOperator() {}

		virtual ~IOperator() {}

		virtual UTF8String name()										= 0;
		virtual void set(Operand const oper, Instance<Object> const& v)	= 0;
		virtual Instance<Object> execute()								= 0;
	};

	struct AType: ID::Identifiable<AType const> {
		AType(uint64 const flags = 0): flags(flags) {}

		virtual ~AType() {}

		uint64 const flags;

		constexpr static uint64 FLAG_BASIC		= 1 << 0;
		constexpr static uint64 FLAG_NUMBER		= 1 << 1;
		constexpr static uint64 FLAG_INTEGER	= 1 << 2;
		constexpr static uint64 FLAG_UNSIGNED	= 1 << 3;
		constexpr static uint64 FLAG_STRING		= 1 << 4;
		constexpr static uint64 FLAG_BYTES		= 1 << 5;
		constexpr static uint64 FLAG_ARRAY		= 1 << 6;

		virtual UTF8String			name() const		= 0;
		virtual Instance<Object>	instantiate() const	= 0;

		Map<IOperator::IdentifierType, Instance<IOperator>> operators;

		Instance<AType> base;

		ID::VLUID const ID = gen.id();
	private:
		inline static ID::SequenceGenerator<ID::VLUID> gen;
	};

	struct Object {
		virtual ~Object() {}

		Object() {}

		Instance<AType> type;

		template <Type::Derived<Object> TNew> constexpr TNew& as()				{return *Cast::morph<TNew*>(this);			}
		template <Type::Derived<Object> TNew> constexpr TNew const& as() const	{return *Cast::morph<TNew const*>(this);	}
	};
}

#endif
