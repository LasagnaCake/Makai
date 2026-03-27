#include "intermediate.hpp"

namespace Core = Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;

Namespace::Instance Namespace::resolve(StringList const& path) const {
	if (path.empty()) return nullptr;
	if (!subspaces.contains(path.back())) return nullptr;
	if (path.size() == 1)
		return subspaces[path.back()];
	else return subspaces[path.back()]->resolve(path.sliced(1));
	return nullptr;
}

Namespace::Instance Intermediate::resolve(StringList const& path) const {
	if (path.empty()) return nullptr;
	for (auto& scope: Makai::Range::reverse(scopeStack)) {
		if (path.size() == 1 && scope->name == path.back())
			return scope;
		else if (auto ns = scope->resolve(path))
			return ns;
	}
	if (auto ns = root->resolve(path))
		return ns;
	return nullptr;
}

Namespace::Instance Intermediate::invokeScope(StringList const& path) {
	if (path.empty()) return nullptr;
	if (scopeStack.empty() && root->subspaces.contains(path.back()))
		return root->subspaces[path.back()];
	else if (scopeStack.back()->subspaces.contains(path.back()))
		return scopeStack.back()->subspaces[path.back()];
	Namespace::Instance ns = ns.create(Namespace{{.name = path.back()}});
	scopeStack.pushBack(ns);
	if (auto const sc = invokeScope(path.sliced(1))) return sc;
	else return ns;
}
