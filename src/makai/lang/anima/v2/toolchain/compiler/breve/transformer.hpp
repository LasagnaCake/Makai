#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_TRANSFORMER_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_TRANSFORMER_H

#include "../../assembler/assembler.hpp"
#include "../../../core/core.hpp"
#include "node.hpp"
#include "intermediate.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler::Breve::Transformer {
	struct ITransformer {
		using Instance = Instance<ITransformer>;

		struct Context: Intermediate {
			template <Makai::Type::Derived<Error::Generic> E = Error::InvalidValue>
			[[noreturn]]
			void error(String const& what, Node::Instance const& where = nullptr) const {
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
							"\n--> [", where-.text, "]"
						),
						what,
						Makai::CPP::SourceFile{"n/a", Cast::as<int>(pos.line), where-.sourceFile}
					);
				}
			}
		};

		virtual ~ITransformer();

		virtual Instance transform(Context& context, Node::Instance const& node) = 0;

		usize stack;

		Namespace::Instance declare(Context& context, UTF8StringList const& node);
		Namespace::Instance resolve(Context& context, UTF8StringList const& node);
		static UTF8StringList pathOf(Node::Instance const& node);
	};

	struct StructureDecl: ITransformer {
		Instance transform(Context& context, Node::Instance const& node) override;
	};

	struct FunctionDecl: ITransformer {
		Instance transform(Context& context, Node::Instance const& node) override;
	};
}

#endif
