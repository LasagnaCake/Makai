#ifndef MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_COMPOSER_H
#define MAKAILIB_ANIMA_V2_TOOLCHAIN_COMPILER_BREVE_COMPOSER_H

#include "intermediate.hpp"

namespace Makai::Anima::V2::Toolchain::Compiler::Breve {
	struct Composer;

	struct Composer {
		Intermediate& inter;

		Map<Namespace::Instance, bool> visited;

		UTF8StringList types;
		UTF8StringList functions;
		List<Implementation::Instance> funcDefs;

		usize staticVarCount = 0;

		Implementation::Instance impl = new Implementation();

		UTF8String toMinima();

		bool mustHaveMain = true;

		List<Namespace::Instance> preMain;

		List<Implementation::Instance> implStack;

		void push() {
			implStack.pushBack(new Implementation());
		}

		void pop() {
			if (implStack.empty()) return;
			auto const prevImpl = implStack.popBack();
			if (prevImpl->main.empty()) return;
			if (implStack.empty()) impl->writeMain(prevImpl->toString());
			else implStack.back()->writeMain(prevImpl->toString());
		}

		Implementation::Instance top() const {
			if (implStack.empty()) return impl;
			return implStack.back();
		}

		Composer(Intermediate& inter): inter(inter) {}

	private:
		UTF8String cache;
	};
}

#endif
