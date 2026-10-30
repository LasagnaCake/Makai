#ifndef MAKAILIB_ANIMA_V2_RUNTIME_MODEL_ENVIRONMENT_H
#define MAKAILIB_ANIMA_V2_RUNTIME_MODEL_ENVIRONMENT_H

#include "context.hpp"

namespace Makai::Anima::V2::Runtime {
	using Core::ContextMode;
	using Core::Instruction;

	struct Environment {
		using Storage = Context::Storage;

		struct Pointers {
			usize	offset		= 0;
			usize	function	= 0;
			usize	instruction	= -1;
		};

		struct Scope {
			List<Storage>		localStack;
			Core::ContextMode	mode			= Core::ContextMode::AV2_CM_STRICT;
			Core::ContextMode	prevMode		= Core::ContextMode::AV2_CM_STRICT;
			usize				pointerFrame	= -1;

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

		Environment& push(Storage const& value) {
			globalValueStack.pushBack(value);
			return *this;
		}

		template <Type::Different<Storage> T>
		Environment& push(T const& value) {
			globalValueStack.pushBack(newValue(value));
			return *this;
		}

		Environment& pushEmpty() {
			globalValueStack.pushBack(nullptr);
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
			return *scopeStack.back();
		}

		List<Storage>& locals() {
			return scope().localStack;
		}

		template <class T>
		Storage newValue(T const& value) {
			return art->newValue(value);
		}

		template <class T>
		Storage newEmpty() {
			return art->template newEmpty<T>();
		}

		template <class T>
		Promise<T> promise() {
			return art->template promise<T>();
		}

		Pointers				pointers;
		List<Storage>			globalValueStack;
		List<Pointers>			pointerStack;
		List<AtomicCell<Scope>>	scopeStack;
		Map<usize, Storage>		globals;
		AtomicCell<Context>		art;
	};
}

#endif
