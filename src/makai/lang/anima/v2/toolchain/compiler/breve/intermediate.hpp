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

	struct Scoped {
		using NamespaceRef = Instance<Namespace>;
		NamespaceRef subspace;
	};

	struct Namespace: Labeled {
		using TypeRef		= Instance<Type>;
		using FunctionRef	= Instance<Function>;
		using VariableRef	= Instance<Variable>;
		using AttributeRef	= Instance<Attribute>;
		using TraitRef		= Instance<Trait>;

		using Instance		= Instance<Namespace>;

		Dictionary<Instance>		subspaces;
		Dictionary<TypeRef>			types;
		Dictionary<FunctionRef>		functions;
		Dictionary<VariableRef>		variables;
		Dictionary<AttributeRef>	attributes;
		Dictionary<TraitRef>		traits;
	};

	struct Type: Labeled, Scoped {
		enum class Definition {
			AV2_TCTD_BASIC,
			AV2_TCTD_ARRAY,
			AV2_TCTD_STRUCT,
			AV2_TCTD_TEMPLATE,
		};

		Definition def;
		Namespace::TypeRef base;
	};

	struct Function: Labeled {
		struct Overload: Scoped {
			Namespace::TypeRef				result;
			List<Namespace::VariableRef>	arguments;
			String							implementation;
		};
		List<Instance<Overload>> overloads;
	};

	struct Variable: Labeled {
		Namespace::TypeRef	type;
		String				initializer;
	};

	struct Attribute: Labeled {

	};

	struct Trait: Labeled, Scoped {

	};

	struct Intermediate {
		Instance<Namespace> root = root.create();
	};
}

#endif
