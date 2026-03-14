#ifndef MAKAILIB_ANIMA_V2_RUNTIME_CONTEXT_H
#define MAKAILIB_ANIMA_V2_RUNTIME_CONTEXT_H

#include "../../../../compat/ctl.hpp"
#include "../core/core.hpp"

namespace Makai::Anima::V2::Runtime {
	struct Context {
		using Storage = Core::Object::Storage;

		struct Pointers {
			usize	offset		= 0;
			usize	function	= 0;
			usize	instruction	= -1;
		};

		struct Scope {
			List<Storage>		localStack;
			Core::ContextMode	mode		= Core::ContextMode::AV2_CM_STRICT;
			Core::ContextMode	prevMode	= Core::ContextMode::AV2_CM_STRICT;

			Scope& push(Storage const& value) {
				localStack.pushBack(value);
				return *this;
			}

			Storage pop() {
				return localStack.popBack();
			}

			Storage& top() {
				return localStack.back();
			}
		};

		using VariableBank = Map<uint64, Data::Value>;

		Context& push(Storage const& value) {
			globalValueStack.pushBack(value);
			return *this;
		}

		template <Type::Different<Storage> T>
		Context& push(T const& value) {
			globalValueStack.pushBack(newValue(value));
			return *this;
		}

		Storage pop() {
			return globalValueStack.popBack();
		}

		Storage& top() {
			return globalValueStack.back();
		}

		Storage& localTop() {
			return scope().top();
		}

		Scope& scope() {
			return scopeStack.back();
		}

		List<Storage>& locals() {
			return scope().localStack;
		}

		template <class T>
		Storage newValue(T const& value) {
			return art.newValue(value);
		}

		Pointers			pointers;
		List<Storage>		globalValueStack;
		List<Pointers>		pointerStack;
		List<Scope>			scopeStack;
		Map<usize, Storage>	globals;
		Core::Context		art;
	};
}

#endif
