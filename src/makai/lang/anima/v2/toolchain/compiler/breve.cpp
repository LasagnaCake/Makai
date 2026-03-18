#include "breve.hpp"

using namespace Makai;

namespace Core = Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain::Compiler;

using Context = Breve::Context;

using Type = Context::Tokenizer::Token::Type;
using enum Type;

CTL_DIAGBLOCK_BEGIN
CTL_DIAGBLOCK_IGNORE_SWITCH

namespace {
	struct BreveTreeDecomposer {

		struct Namespace {
			enum class Type {
				BTD_NT_NAMESPACE,
				BTD_NT_METHOD,
				BTD_NT_TYPE,
				BTD_NT_TRAIT,
			};

			String								name;
			Type								type;
			List<Makai::Instance<Namespace>>	children;
		};

		void decompose() {

		}
	};
}

void Breve::invoke() {
	Parser parser(context);
	auto const tree = parser.parse();
}

CTL_DIAGBLOCK_END
