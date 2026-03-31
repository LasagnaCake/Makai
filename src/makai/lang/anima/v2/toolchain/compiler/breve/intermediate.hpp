#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_INTERMEDIATE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_INTERMEDIATE_H

#include "../../assembler/assembler.hpp"
#include "../../../core/core.hpp"
#include "makai/ctl/ctl/container/dictionary.hpp"
#include "node.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler::Breve {
	struct Intermediate;

	struct Labeled {
		UTF8String name;
	};

	struct Positioned {
		Instance<Node> node;
	};

	struct Namespace;
	struct TypeDecl;
	struct Function;
	struct Variable;
	struct Attribute;
	struct Trait;
	struct Property;

	struct Implementation;

	struct IComposable {
		virtual ~IComposable() {}
		virtual Instance<Implementation> compose() const = 0;
	};

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

		template <class... Types>
		void writePreLine(Types const&... values) {
			writePre(values..., "\n");
		}

		template <class... Types>
		void writeMainLine(Types const&... values) {
			writeMain(values..., "\n");
		}

		template <class... Types>
		void writePostLine(Types const&... values) {
			writePost(values..., "\n");
		}

	private:
		template <class T>
		constexpr static UTF8String toString(T const& value) {
			if constexpr (Makai::Type::OneOf<T, UTF8String, UTF32String, String>)
				return value;
			else return toString(value);
		}
	};

	struct Implementation: IWritable, IComposable {
		using Instance		= Instance<Implementation>;
		UTF8String pre, main, post;

		void writePre(UTF8String const& what) override;
		void writeMain(UTF8String const& what) override;
		void writePost(UTF8String const& what) override;

		Instance compose() const override {
			auto const impl = new Implementation;
			impl->pre = pre;
			impl->main = main;
			impl->post = post;
			return impl;
		}

		UTF8String toString() const {return pre + main + post;}
	};

	struct Metadata {
		using Instance = Instance<Metadata>;

		Makai::Instance<Attribute>	attribute;
		Makai::Data::Value			value;
	};

	struct Namespace: Labeled, Positioned, IComposable {
		using TypeRef		= Instance<TypeDecl>;
		using FunctionRef	= Instance<Function>;
		using VariableRef	= Instance<Variable>;
		using AttributeRef	= Instance<Attribute>;
		using TraitRef		= Instance<Trait>;
		using PropertyRef	= Instance<Property>;

		using Instance		= Instance<Namespace>;

		usize varc;

		UTF8Dictionary<Instance> subspaces;

		UTF8Dictionary<Metadata::Instance> meta;

		TypeRef			type;
		FunctionRef		function;
		VariableRef		variable;
		AttributeRef	attribute;
		TraitRef		trait;
		PropertyRef		property;

		Implementation::Instance	impl = impl.create();

		Instance resolve(UTF8StringList const& path) const;

		Namespace(UTF8String const& name = "");

		bool isPureNamespace() const;

		Implementation::Instance compose() const override;
	};

	struct TypeDecl: Labeled {
		enum class Definition {
			AV2_TCTD_BASIC,
			AV2_TCTD_ARRAY,
			AV2_TCTD_STRUCT,
			AV2_TCTD_TEMPLATE,
		};

		uint64									flags;
		Definition								def;
		Nullable<Core::BasicType>				basic;
		Namespace::TypeRef						base;
		Namespace::Instance						scope;
		Nullable<UTF8String>					artEquivalent;
		UTF8Dictionary<Namespace::VariableRef>	fields;

		static Namespace::TypeRef stronger(Namespace::TypeRef const& a, Namespace::TypeRef const& b);

		bool derivedFrom(Namespace::TypeRef const& otherType) const;
	};

	struct Function: Labeled, Positioned {
		struct Overload {
			Namespace::TypeRef				result;
			List<Namespace::VariableRef>	arguments;
			Namespace::Instance				scope;
			UTF8String						entry;
			UTF8String prototype() const;
		};
		using OverloadRef = Instance<Overload>;

		List<OverloadRef> overloads;

		using ArgTypes = List<Namespace::TypeRef>;

		OverloadRef overload(List<Namespace::VariableRef> const& args) const;
		OverloadRef overload(List<Namespace::TypeRef> const& args) const;
	};

	struct Variable: Labeled, Positioned {
		Namespace::TypeRef	type;
		Namespace::Instance	scope;
		Namespace::Instance	initializer;
		UTF8String			source;
		Data::Value			value;
		bool				defaulted;
		bool				global;
		bool				staticEntity;
		Handle<TypeDecl>	fieldOf;
	};

	struct Attribute: Labeled, Positioned {
		enum class Target: uint64 {
			AV2_TAAT_EMPTY		= 0,
			AV2_TAAT_STRUCT		= 1 << 0,
			AV2_TAAT_ATTRIBUTE	= 1 << 1,
			AV2_TAAT_VARIABLE	= 1 << 2,
			AV2_TAAT_FUNCTION	= 1 << 3,
			AV2_TAAT_PROPERTY	= 1 << 4,
			AV2_TAAT_NAMESPACE	= 1 << 5,
			AV2_TAAT_EVERYTHING	= Makai::Limit::MAX<uint64>
		};

		Target target	= Target::AV2_TAAT_EVERYTHING;
		usize	useCount	= 0;
		usize	globalMin	= 0;
		usize	globalMax	= -1;
		bool	saveUses	= true;

		List<Namespace::Instance> uses;

		struct Field {
			Data::Value::Kind	type;
			Data::Value			defaultValue;
		};

		UTF8Dictionary<Field> fields;

		Functor<void(Namespace::Instance const&, Data::Value const&, Attribute&)> transform;

		static bool matchesTarget(Namespace const& ns, Target const target);
	};

	struct Property:  Labeled, Positioned {
		Namespace::TypeRef		type;
		Namespace::Instance		scope;
		Namespace::FunctionRef	getter;
		Namespace::FunctionRef	setter;
	};

	constexpr Attribute::Target operator&(Attribute::Target const& a, Attribute::Target const& b) {
		return Makai::Cast::as<Attribute::Target>(enumcast(a) & enumcast(b));
	}

	constexpr Attribute::Target operator|(Attribute::Target const& a, Attribute::Target const& b) {
		return Makai::Cast::as<Attribute::Target>(enumcast(a) | enumcast(b));
	}

	constexpr Attribute::Target operator~(Attribute::Target const& a) {
		return Makai::Cast::as<Attribute::Target>(~enumcast(a));
	}

	struct Trait: Labeled, Positioned {
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

		void addGlobalAttribute(Namespace::AttributeRef const& attrib);

		Intermediate();
	};
}

#endif
