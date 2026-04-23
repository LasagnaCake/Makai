#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_INTERMEDIATE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_INTERMEDIATE_H

#include "../../assembler/assembler.hpp"
#include "../../../core/core.hpp"
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
			(..., writePost(toString(values)));
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
			else return Makai::toString(value);
		}
	};

	struct Scoped {
		Handle<Namespace> scope;
	};

	struct ISerializable {
		virtual ~ISerializable() {}
		virtual Makai::Data::Value serialize() const = 0;
	};

	struct Implementation: IWritable, IComposable, ISerializable {
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

		Makai::Data::Value serialize() const override;

		Implementation() {}
		virtual ~Implementation() {}
	};

	struct Metadata {
		using Instance = Instance<Metadata>;

		Makai::Instance<Attribute>	attribute;
		Makai::Data::Value			value;
	};

	struct Namespace: Labeled, Positioned, IComposable, ISerializable {
		using TypeRef		= Instance<TypeDecl>;
		using FunctionRef	= Instance<Function>;
		using VariableRef	= Instance<Variable>;
		using AttributeRef	= Instance<Attribute>;
		using TraitRef		= Instance<Trait>;
		using PropertyRef	= Instance<Property>;

		using Instance		= Instance<Namespace>;

		usize varc = 0;

		UTF8Dictionary<Instance> subspaces;

		UTF8Dictionary<Metadata::Instance> meta;

		bool declaredAsNamespace = false;

		TypeRef			type;
		FunctionRef		function;
		VariableRef		variable;
		AttributeRef	attribute;
		TraitRef		trait;
		PropertyRef		property;

		Implementation::Instance	impl = impl.create();

		Instance resolve(UTF8StringList const& path) const;

		Namespace(UTF8String const& name = "");
		virtual ~Namespace();

		bool isPureNamespace() const;

		Implementation::Instance compose() const override;

		Makai::Data::Value serialize() const override;
	};

	struct TypeDecl: Labeled, Positioned, Scoped, ISerializable {
		enum class Definition {
			AV2_TCTD_BASIC,
			AV2_TCTD_ARRAY,
			AV2_TCTD_STRUCT,
			AV2_TCTD_TEMPLATE,
		};

		uint64									flags = 0;
		Definition								def;
		Nullable<Core::BasicType>				basic;
		Namespace::TypeRef						base;
		Nullable<UTF8String>					artEquivalent;
		UTF8Dictionary<Namespace::VariableRef>	fields;

		static Namespace::TypeRef stronger(Namespace::TypeRef const& a, Namespace::TypeRef const& b);

		bool derivedFrom(Namespace::TypeRef const& otherType) const;

		Makai::Data::Value serialize() const override;

		TypeDecl(UTF8String const& name = "");
		virtual ~TypeDecl();
	};

	struct Function: Labeled, Positioned, ISerializable {
		struct Overload: Scoped, ISerializable {
			enum class Variant {
				AV2_TCB_FOV_NONE,
				AV2_TCB_FOV_GLOBAL,
				AV2_TCB_FOV_CLASS,
				AV2_TCB_FOV_INSTANCE,
				AV2_TCB_FOV_ART_CALL,
			};
			Namespace::TypeRef				result;
			List<Namespace::VariableRef>	arguments;
			UTF8String						entry;
			Handle<TypeDecl>				methodOf;
			Variant							variant = Variant::AV2_TCB_FOV_NONE;

			UTF8String prototype() const;

			Makai::Data::Value serialize() const override;

			Overload();
			virtual ~Overload();
		};
		using OverloadRef = Instance<Overload>;

		List<OverloadRef> overloads;

		using ArgTypes = List<Namespace::TypeRef>;

		OverloadRef overloadFromVariables(List<Namespace::VariableRef> const& args) const;
		OverloadRef overloadFromTypes(List<Namespace::TypeRef> const& args) const;

		Makai::Data::Value serialize() const override;

		Function(UTF8String const& name = "");
		virtual ~Function();
	};

	struct Variable: Labeled, Positioned, Scoped, ISerializable {
		Namespace::TypeRef	type;
		Namespace::Instance	initializer;
		UTF8String			source;
		Data::Value			value;
		bool				defaulted = false;
		bool				global = false;
		bool				staticEntity = false;
		Handle<TypeDecl>	fieldOf;
		uint64				id = 0;
		Handle<Namespace>	parentScope;

		UTF8String getSource() {
			if (global) return source;
			else return "move local[" + Makai::toString(id) + "]";
		}

		Makai::Data::Value serialize() const override;

		Variable(UTF8String const& name = "");
		virtual ~Variable();
	};

	struct Attribute: Labeled, Positioned, ISerializable {
		enum class Target: uint64 {
			AV2_TAAT_EMPTY		= 0,
			AV2_TAAT_TYPE		= 1 << 0,
			AV2_TAAT_ATTRIBUTE	= 1 << 1,
			AV2_TAAT_VARIABLE	= 1 << 2,
			AV2_TAAT_FUNCTION	= 1 << 3,
			AV2_TAAT_PROPERTY	= 1 << 4,
			AV2_TAAT_NAMESPACE	= 1 << 5,
			AV2_TAAT_TRAIT		= 1 << 6,
			AV2_TAAT_EVERYTHING	= Makai::Limit::MAX<uint64>
		};

		Target	target		= Target::AV2_TAAT_EVERYTHING;
		usize	useCount	= 0;
		usize	globalMin	= 0;
		usize	globalMax	= -1;

		struct Field {
			Data::Value::Kind	type;
			Data::Value			defaultValue;
			bool				path = false;
		};

		UTF8Dictionary<Field> fields;

		Functor<void(Intermediate&, Namespace::Instance const&, Data::Value const&, Attribute&)> transform;

		static bool matchesTarget(Namespace const& ns, Target const target);

		Makai::Data::Value serialize() const override;

		Attribute(UTF8String const& name = "");
		virtual ~Attribute();
	};

	struct Property:  Labeled, Positioned, Scoped, ISerializable {
		Namespace::TypeRef		type;
		Namespace::FunctionRef	getter;
		Namespace::FunctionRef	setter;
		Handle<TypeDecl>		fieldOf;

		Makai::Data::Value serialize() const override;

		Property(UTF8String const& name = "");
		virtual ~Property();
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

	struct Trait: Labeled, Positioned, Scoped, ISerializable {
		Makai::Data::Value serialize() const override;

		Trait(UTF8String const& name = "");
		virtual ~Trait();
	};

	struct File {
		Namespace::Instance		content;
	 	Function::OverloadRef	entry;
	 	Function::OverloadRef	exit;
	};

	struct Intermediate: IWritable, ISerializable {
		using Instance = Instance<Intermediate>;

		Namespace::Instance root = root.create();

		Handle<Function::Overload> entry;
		Handle<Function::Overload> exit;

		void writePre(UTF8String const& what) override;
		void writeMain(UTF8String const& what) override;
		void writePost(UTF8String const& what) override;

		List<Namespace::Instance> scopeStack;

		Namespace::Instance resolve(UTF8StringList const& path) const;
		Namespace::Instance push(UTF8StringList const& path);
		void pop(usize const count);
		Namespace::Instance top() const;
		Namespace::Instance parent() const;

		void addGlobalAttribute(Namespace::AttributeRef const& attrib);

		Intermediate();
		virtual ~Intermediate();

		Makai::Data::Value serialize() const override;
	};
}

#endif
