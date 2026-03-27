#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_INTERMEDIATE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_INTERMEDIATE_H

#include "../../assembler/assembler.hpp"
#include "../../../core/core.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler::Breve {
	struct Intermediate;

	struct Labeled {
		UTF8String name;
	};

	struct Namespace;
	struct Type;
	struct Function;
	struct Variable;
	struct Attribute;
	struct Trait;

	struct IWritable {
		virtual ~IWritable();

		virtual void writePre(UTF8String const& what) = 0;
		virtual void writeMain(UTF8String const& what) = 0;
		virtual void writePost(UTF8String const& what) = 0;

		template <Makai::Type::NoneOf<UTF8String, UTF32String, String> T>
		void writePre(T const& what) {
			writePre(toString(what));
		}

		template <Makai::Type::NoneOf<UTF8String, UTF32String, String> T>
		void writeMain(T const& what) {
			writeMain(toString(what));
		}

		template <Makai::Type::NoneOf<UTF8String, UTF32String, String> T>
		void writePost(T const& what) {
			writePost(toString(what));
		}

		template <class... Types>
		void writePre(Types const&... values)
		requires (sizeof...(Types) > 1) {
			(..., writePre(toString(values)));
		}

		template <class... Types>
		void writeMain(Types const&... values)
		requires (sizeof...(Types) > 1) {
			(..., writeMain(toString(values)));
		}

		template <class... Types>
		void writePost(Types const&... values)
		requires (sizeof...(Types) > 1) {
			(..., writeMain(toString(values)));
		}

		void writePreLine(UTF8String const& what);
		void writeMainLine(UTF8String const& what);
		void writePostLine(UTF8String const& what);

	private:
		template <class T>
		constexpr static UTF8String toString(T const& value) {
			if constexpr (Makai::Type::OneOf<T, UTF8String, UTF32String, String>)
				return value;
			else return toString(value);
		}
	};

	struct Implementation: IWritable {
		using Instance		= Instance<Implementation>;
		UTF8String pre, main, post;

		void writePre(UTF8String const& what) override;
		void writeMain(UTF8String const& what) override;
		void writePost(UTF8String const& what) override;

		UTF8String compose() const {return pre + main + post;}
	};

	struct Namespace: Labeled {
		using TypeRef		= Instance<Type>;
		using FunctionRef	= Instance<Function>;
		using VariableRef	= Instance<Variable>;
		using AttributeRef	= Instance<Attribute>;
		using TraitRef		= Instance<Trait>;

		using Instance		= Instance<Namespace>;

		UTF8Dictionary<Instance> subspaces;

		TypeRef						type;
		FunctionRef					function;
		VariableRef					variable;
		AttributeRef				attribute;
		TraitRef					trait;

		Implementation::Instance	impl;

		Instance resolve(UTF8StringList const& path) const;

		Namespace(UTF8String const& name = "");
	};

	struct Type: Labeled {
		enum class Definition {
			AV2_TCTD_BASIC,
			AV2_TCTD_ARRAY,
			AV2_TCTD_STRUCT,
			AV2_TCTD_TEMPLATE,
		};

		Definition					def;
		Namespace::TypeRef			base;
		Implementation::Instance	impl = impl.create();
	};

	struct Function: Labeled {
		struct Overload {
			Namespace::TypeRef				result;
			List<Namespace::VariableRef>	arguments;
			Implementation::Instance		impl 		= impl.create();
		};
		using OverloadRef = Instance<Overload>;

		List<OverloadRef> overloads;

		OverloadRef overload(List<Namespace::VariableRef> const& args) const;
	};

	struct Variable: Labeled {
		Namespace::TypeRef			type;
		Implementation::Instance	impl = impl.create();
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
		usize useCount;
		usize globalMax;
		usize localMax;
	};

	struct Trait: Labeled {
	};

	struct Intermediate: IWritable {
		Namespace::Instance root = root.create();

		void writePre(UTF8String const& what) override;
		void writeMain(UTF8String const& what) override;
		void writePost(UTF8String const& what) override;

		List<Namespace::Instance> scopeStack;

		Namespace::Instance resolve(UTF8StringList const& path) const;
		usize push(UTF8StringList const& path);
		void pop(usize const count);
		Namespace::Instance top() const;
		Namespace::Instance parent() const;
	};
}

#endif
