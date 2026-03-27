#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_INTERMEDIATE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_INTERMEDIATE_H

#include "../../assembler/assembler.hpp"
#include "../../../core/core.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler::Breve {
	struct Intermediate;

	struct Labeled {
		String name;
	};

	struct Namespace;
	struct Type;
	struct Function;
	struct Variable;
	struct Attribute;
	struct Trait;

	struct Implementable {
		String pre, main, post;
	};

	struct Namespace: Labeled {
		using TypeRef		= Instance<Type>;
		using FunctionRef	= Instance<Function>;
		using VariableRef	= Instance<Variable>;
		using AttributeRef	= Instance<Attribute>;
		using TraitRef		= Instance<Trait>;

		using Instance		= Instance<Namespace>;

		Dictionary<Instance> subspaces;

		TypeRef			type;
		FunctionRef		function;
		VariableRef		variable;
		AttributeRef	attribute;
		TraitRef		trait;

		Instance resolve(StringList const& path) const;
	};

	struct Type: Labeled {
		enum class Definition {
			AV2_TCTD_BASIC,
			AV2_TCTD_ARRAY,
			AV2_TCTD_STRUCT,
			AV2_TCTD_TEMPLATE,
		};

		Definition def;
		Namespace::TypeRef base;
	};

	struct Function: Labeled, Implementable {
		struct Overload {
			Namespace::TypeRef				result;
			List<Namespace::VariableRef>	arguments;
		};
		List<Instance<Overload>> overloads;
	};

	struct Variable: Labeled, Implementable {
		Namespace::TypeRef	type;
		String				initializer;
	};

	struct Attribute: Labeled {
		enum class Target {
			AV2_TAAT_EMPTY		= 0,
			AV2_TAAT_STRUCT		= 1 << 0,
			AV2_TAAT_ATTRIBUTE	= 1 << 1,
			AV2_TAAT_VARIABLE	= 1 << 2,
			AV2_TAAT_FUNCTION	= 1 << 3,
			AV2_TAAT_PROPERTY	= 1 << 4,
			AV2_TAAT_VALUE		= 1 << 5,
		};
		Target target;
	};

	struct Trait: Labeled {
	};

	struct Intermediate {
		Namespace::Instance root = root.create();

		List<Namespace::Instance> scopeStack;

		Namespace::Instance resolve(StringList const& path) const;
		usize push(StringList const& path);
		void pop(usize const count);
	};
}

#endif
