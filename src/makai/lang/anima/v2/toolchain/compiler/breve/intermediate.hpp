#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_INTERMEDIATE_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_INTERMEDIATE_H

#include "../../assembler/assembler.hpp"
#include "../../../core/core.hpp"
#include "node.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler::Breve {
	struct Intermediate;

	struct Labeled {
		UTF8String name;
		UTF8String pureName;
	};

	struct Positioned {
		Instance<Node> node;
	};
	enum class ExecutionContext {
		AV2_TCB_EC_NONE,
		AV2_TCB_EC_RUNTIME,
		AV2_TCB_EC_MIXED,
		AV2_TCB_EC_COMPILE,
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

		virtual void addPreLine(UTF8String const& what) = 0;
		virtual void addMainLine(UTF8String const& what) = 0;
		virtual void addPostLine(UTF8String const& what) = 0;

		template <class... Types>
		void writePreLine(Types const&... values) {
			addPreLine((... + (" " + toString(values))));
		}

		template <class... Types>
		void writeMainLine(Types const&... values) {
			addMainLine((... + (" " + toString(values))));
		}

		template <class... Types>
		void writePostLine(Types const&... values) {
			addPostLine((... + (" " + toString(values))));
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
		UTF8StringList pre, main, post;

		void addPreLine(UTF8String const& what) override;
		void addMainLine(UTF8String const& what) override;
		void addPostLine(UTF8String const& what) override;

		Instance compose() const override {
			auto const impl = new Implementation;
			impl->pre = pre;
			impl->main = main;
			impl->post = post;
			return impl;
		}

		UTF8String toString() const {return pre.join("\n") + "\n" + main.join("\n") + "\n" + post.join("\n");}

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

		bool implementContents = false;

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
			AV2_TCTD_ENUM,
			AV2_TCTD_TEMPLATE,
		};

		Core::TypeFlags							flags = {};
		Definition								def;
		Nullable<Core::BasicType>				basic;
		Namespace::TypeRef						base;
		Nullable<UTF8String>					artEquivalent;
		UTF8Dictionary<Namespace::VariableRef>	fields;
		UTF8Dictionary<Namespace::FunctionRef>	methods;
		List<Namespace::TypeRef>				args;

		usize uses = 0;

		static Namespace::TypeRef stronger(Namespace::TypeRef const& a, Namespace::TypeRef const& b);

		bool derivedFrom(Namespace::TypeRef const& otherType) const;

		Makai::Data::Value serialize() const override;

		TypeDecl(UTF8String const& name = "");
		virtual ~TypeDecl();
	};

	struct Function: Labeled, Positioned, ISerializable {
		struct Overload: Scoped, ISerializable {
			struct Variant {
				enum class External {
					AV2_TCB_FO_VE_NONE,
					AV2_TCB_FO_VE_ART_CALL,
					AV2_TCB_FO_VE_DYNLIB,
				};
				enum class Object {
					AV2_TCB_FO_VO_NONE,
					AV2_TCB_FO_VO_GLOBAL,
					AV2_TCB_FO_VO_CLASS,
					AV2_TCB_FO_VO_INSTANCE,
				};

				External			external	= External::AV2_TCB_FO_VE_NONE;
				Object				object		= Object::AV2_TCB_FO_VO_NONE;
				ExecutionContext	context		= ExecutionContext::AV2_TCB_EC_NONE;

				constexpr bool operator==(External const variant) const			{return variant == external;	}
				constexpr bool operator==(Object const variant) const			{return variant == object;		}
				constexpr bool operator==(ExecutionContext const variant) const	{return variant == context;		}

				constexpr Variant& operator=(External const variant)		{return (external = variant, *this);	}
				constexpr Variant operator=(Object const variant)			{return (object = variant, *this);		}
				constexpr Variant operator=(ExecutionContext const variant)	{return (context = variant, *this);		}
			};
			Namespace::TypeRef				result;
			List<Namespace::VariableRef>	arguments;
			UTF8String						entry;
			UTF8String						outEntry;
			UTF8String						sigEntry;
			UTF8String						dynlib;
			Handle<TypeDecl>				methodOf;
			Variant							variant;
			bool							optional = false;
			bool							hasImplementation = false;
			bool							staticEntity = false;
			Handle<Overload>				fullImpl;
			Node::Instance					decl = nullptr;

			usize uses = 0;

			UTF8Dictionary<Metadata::Instance> meta;

			UTF8String prototype() const;

			Makai::Data::Value serialize() const override;

			Overload();
			virtual ~Overload();
		};

		using OverloadRef = Instance<Overload>;

		List<OverloadRef> overloads;
		List<OverloadRef> current;

		OverloadRef sigCall;

		using ArgTypes = List<Namespace::TypeRef>;

		OverloadRef overloadFromVariables(List<Namespace::VariableRef> const& args) const;
		OverloadRef overloadFromTypes(List<Namespace::TypeRef> const& args) const;

		Makai::Data::Value serialize() const override;

		Function(UTF8String const& name = "");
		virtual ~Function();
	};

	struct Variable: Labeled, Positioned, Scoped, ISerializable {
		Handle<TypeDecl>	type;
		Namespace::Instance	initializer;
		UTF8String			source;
		Data::Value			value;
		bool				defaulted = false;
		bool				global = false;
		bool				staticEntity = false;
		Handle<TypeDecl>	fieldOf;
		uint64				id = 0;
		Handle<Namespace>	parentScope;
		UTF8String			passBy = "move";

		ExecutionContext	context = ExecutionContext::AV2_TCB_EC_NONE;

		UTF8String getSource() {
			if (context > ExecutionContext::AV2_TCB_EC_RUNTIME)
				return value.toString();
			if (global) return passBy + " " + source;
			else return passBy + " local[" + Makai::toString(id) + "]";
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

		UTF8StringList fieldMap;

		uint64 baseTypeHash = 0;

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
	 	Function::OverloadRef	main;
	};

	struct Intermediate: IWritable, ISerializable {
		using Instance = Instance<Intermediate>;

		Namespace::Instance root = root.create();

		List<Handle<Function::Overload>>	before;
		Handle<Function::Overload>			main;
		List<Handle<Function::Overload>>	after;

		void addPreLine(UTF8String const& what) override;
		void addMainLine(UTF8String const& what) override;
		void addPostLine(UTF8String const& what) override;

		List<Namespace::Instance> scopeStack;
		List<Function::OverloadRef> functionStack;

		Namespace::Instance resolve(UTF8StringList const& path) const;
		Namespace::Instance push(UTF8StringList const& path);
		void pop(usize const count);
		Namespace::Instance top() const;
		Namespace::Instance parent() const;

		Implementation::Instance impl() const;

		void addGlobalAttribute(Namespace::AttributeRef const& attrib);

		Intermediate();
		virtual ~Intermediate();

		Makai::Data::Value serialize() const override;
	};
}

#endif
