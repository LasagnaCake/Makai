#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_TRANSFORMER_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_TRANSFORMER_H

#include "../../assembler/assembler.hpp"
#include "../../../core/core.hpp"
#include "node.hpp"
#include "intermediate.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler::Breve::Transformer {
	struct ATransformer {
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

		virtual Namespace::Instance transform(Context& context, Node::Instance const& node) = 0;
	};

	struct StructureDecl: ATransformer {
		Namespace::Instance transform(Context& context, Node::Instance const& node) override;
	};

	struct FunctionDecl: ATransformer {
		Namespace::Instance transform(Context& context, Node::Instance const& node) override;
	};

	struct VariableDecl: ATransformer {
		Namespace::Instance transform(Context& context, Node::Instance const& node) override;
	};

	struct Expression: ATransformer {
		Namespace::Instance transform(Context& context, Node::Instance const& node) override;
	};

	struct BinaryExpression: ATransformer {
		Namespace::Instance transform(Context& context, Node::Instance const& node) override;
	};

	struct PrefixExpression: ATransformer {
		Namespace::Instance transform(Context& context, Node::Instance const& node) override;
	};

	struct PostfixExpression: ATransformer {
		Namespace::Instance transform(Context& context, Node::Instance const& node) override;
	};
}

#endif
