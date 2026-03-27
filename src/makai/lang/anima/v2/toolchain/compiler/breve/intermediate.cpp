#include "intermediate.hpp"

namespace Core = Makai::Anima::V2::Core;

using namespace Makai::Anima::V2::Toolchain::Compiler::Breve;

~IWritable::IWritable() {}

Namespace::Namespace(UTF8String const& name): Labeled(name) {}

Namespace::Instance Namespace::resolve(UTF8StringList const& path) const {
	if (path.empty()) return nullptr;
	if (!subspaces.contains(path.back())) return nullptr;
	if (path.size() == 1)
		return subspaces[path.back()];
	else return subspaces[path.back()]->resolve(path.sliced(1));
	return nullptr;
}

Namespace::Instance Intermediate::resolve(UTF8StringList const& path) const {
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

usize Intermediate::push(UTF8StringList const& path) {
	if (path.empty()) return 0;
	if (scopeStack.empty() && root->subspaces.contains(path.back()))
		return root->subspaces[path.back()];
	else if (scopeStack.back()->subspaces.contains(path.back()))
		return scopeStack.back()->subspaces[path.back()];
	Namespace::Instance ns = ns.create(path.back());
	if (scopeStack.empty())
		root->subspaces[ns->name] = ns;
	else scopeStack.back()->subspaces[ns->name] = ns;
	scopeStack.pushBack(ns);
	return push(path.sliced(1)) + 1;
}

void Intermediate::pop(usize count) {
	while (scopeStack.size() && count--) {
		auto const scope = scopeStack.popBack();
		if (scopeStack.empty())
			root->pre += scope->compose();
		else scopeStack.back()->main += scope->compose();
	}
}

void Implementable::writePre(UTF8String const& what) {
	pre += " " + what;
}

void Implementable::writeMain(UTF8String const& what) {
	main += " " + what;
}

void Implementable::writePost(UTF8String const& what) {
	post += " " + what;
}

void Intermediate::writePre(UTF8String const& what) {
	root->pre += " " + what;
}

void Intermediate::writeMain(UTF8String const& what) {
	root->main += " " + what;
}

void Intermediate::writePost(UTF8String const& what) {
	root->post += " " + what;
}

Function::OverloadRef Function::overload(List<Namespace::VariableRef> const& args) const {
	for (auto& ov: overloads) {
		if (!ov) continue;
		if (ov->arguments.size() != args.size()) continue;
		bool miss = false;
		for (auto const arg: Range::expand(ov->arguments))
			if (!(args[arg.index].exists() || arg.value.exists()))
				continue;
			else if (args[arg.index].exists() != arg.value.exists()) {
				miss = true;
				break;
			} else if (args[arg.index]->type != arg.value->type) {
				miss = true;
				break;
			}
		if (miss) continue;
		return ov;
	}
	return nullptr;
}
