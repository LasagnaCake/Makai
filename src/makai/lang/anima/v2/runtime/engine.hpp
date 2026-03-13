#ifndef MAKAILIB_ANIMA_V2_RUNTIME_ENGINE_H
#define MAKAILIB_ANIMA_V2_RUNTIME_ENGINE_H

#include "context.hpp"

namespace Makai::Anima::V2::Runtime {
	struct Engine {
		using Value = Data::Value;

		struct FunctionRegistry {
			template <class T> struct Function;

			template <class TReturn, class... TArgs>
			struct Function<TReturn(TArgs...)> {

			};
		};

		struct Error {
			String				message;
			usize				location;
			Core::Instruction	instruction;
		};

		bool process();

		FunctionRegistry functions;

		bool hasSignal(String const& name);
		void fire(String const& signal);

		void terminate();
		void reset();
		void load(Core::Module const& program);
		void execute();

		Nullable<Error>	error() const	{return err;			}

	protected:
		virtual Context::Storage	external	(String const& name, bool const byRef	);
		Context::Storage&			global		(uint64 const globalID					);
		Context::Storage&			iregister	(uint64 const registerID				);

		constexpr bool inStrictMode() const {return context.scopeStack.back().mode == Core::ContextMode::AV2_CM_STRICT;}

		void crash(Engine::Error const& error);

		bool hasFunction(String const& name);

	private:
		bool yieldCycle();

		Engine::Error invalidInstructionError();
		Engine::Error endOfProgramError();
		Engine::Error invalidOperationError(String const& description);
		Engine::Error invalidInternalValueError(uint64 const id);
		Engine::Error invalidLocationError(Core::DataLocation const& loc);
		Engine::Error invalidSourceError(String const& description);
		Engine::Error invalidDestinationError(String const& description);
		Engine::Error invalidFunctionError(String const& description);
		Engine::Error invalidComparisonError(String const& description);
		Engine::Error invalidFieldError(String const& description);
		Engine::Error invalidFetchRequest(String const& description);
		Engine::Error invalidCast(String const& description);
		Engine::Error invalidJump();
		Engine::Error missingArgumentsError();

		Engine::Error makeErrorHere(String const& message);

		Context::Storage consumeValue(Core::DataLocation const from);
		Context::Storage getValueFromLocation(Core::DataLocation const location, uint64 const id);

		Context::Storage& accessValue(Core::DataLocation const from);
		Context::Storage& accessLocation(Core::DataLocation const location, uint64 const id);

		void advance(bool isRequired = false);

		void v2Copy();
		void v2Call();
		void v2Negate();
		void v2StackBlit();
		void v2StackPush();
		void v2StackPop();
		void v2StackSwap();
		void v2StackClear();
		void v2StackFlush();
		void v2Return();
		void v2Halt();
		void v2Op();
		void v2SetContext();
		void v2Compare();
		void v2Cast();
		void v2Jump();
		void v2Yield();
		void v2ScopeBring();
		void v2ScopeBind();
		void v2ScopeEnter();
		void v2ScopeExit();
		void v2FieldGet();
		void v2Sizeof();
		void v2Typeof();

		void doBinaryOperation(Core::Operator const op);
		void doUnaryOperation(Core::Operator const op);

		void jumpBy(usize const tableID, bool returnable);
		void jumpTo(usize const point, bool returnable);
		void returnBack();

		bool				isFinished	= false;
		bool				paused		= false;
		Core::Module		program;
		Context				context;
		Core::Instruction	current;
		Nullable<Error>		err;
	};
}

#endif
