#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_TRANSFORMER_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_TRANSFORMER_H

#include "../../assembler/assembler.hpp"
#include "../../../core/core.hpp"
#include "node.hpp"
#include "intermediate.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler::Breve::Transformer {
	struct ATransformer {
		struct Result {
			UTF8String			source;
			Namespace::Instance	scope;
			Namespace::TypeRef	type;
		};

		using Instance = Instance<ATransformer>;

		struct Context: Intermediate {
			template <Makai::Type::Derived<Error::Generic> E = Error::InvalidValue>
			[[noreturn]]
			static void error(String const& what, Node::Instance const& where = nullptr) {
				if (!where)
					throw E(
						"At: EOF",
						what,
						Makai::CPP::SourceFile{"n/a", -1, "???"}
					);
				else {
					auto const pos = where->base.at;
					throw E(
						Makai::toString(
							"At:\nLINE: ", pos.line,
							"\nCOLUMN: ", pos.column,
							"\n--> [", where->base.text, "]"
						),
						what,
						Makai::CPP::SourceFile{"n/a", Cast::as<int>(pos.line), where->base.sourceFile}
					);
				}
			}

			Namespace::Instance declare(UTF8StringList const& path);
			Namespace::Instance resolve(UTF8StringList const& path);
			Namespace::Instance get(UTF8StringList const& path);
			Namespace::Instance fetch(Node::Instance const& nodePath);
			Namespace::Instance fetch(UTF8StringList const& path, Node::Instance const& base);

			static UTF8StringList pathOf(Node::Instance const& node);
		};

		virtual ~ATransformer();

		bool allowPaths = true;

		UTF8StringList resolve(Context& context, Node::Instance const& node) const;

		virtual Result transform(Context& context, Node::Instance const& node) = 0;
	};

	struct StructureDecl: ATransformer {
		bool pathed = true;
		Result transform(Context& context, Node::Instance const& node) override;
	};

	struct FunctionDecl: ATransformer {
		bool pathed = true;
		Result transform(Context& context, Node::Instance const& node) override;
	};

	struct VariableDecl: ATransformer {
		bool pathed = true;
		Result transform(Context& context, Node::Instance const& node) override;
	};

	struct Expression: ATransformer {
		Result transform(Context& context, Node::Instance const& node) override;
	};

	struct BinaryExpression: ATransformer {
		Result transform(Context& context, Node::Instance const& node) override;
	};

	struct PrefixExpression: ATransformer {
		Result transform(Context& context, Node::Instance const& node) override;
	};

	struct PostfixExpression: ATransformer {
		Result transform(Context& context, Node::Instance const& node) override;
	};
}

#endif
