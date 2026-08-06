#ifndef MAKAILIB_ANIMA_V2_RUNTIME_ENGINE_H
#define MAKAILIB_ANIMA_V2_RUNTIME_ENGINE_H

#include "context.hpp"

namespace Makai::Anima::V2::Runtime {
	struct Engine {
		struct Config {
			bool allowDynamicLibraries = false;

			static Config createDefault() {
				return Config();
			}
		};

		enum class State {
			AV2_RES_READY,
			AV2_RES_INITIALIZING,
			AV2_RES_RUNNING,
			AV2_RES_EXITING,
			AV2_RES_FINISHED,
		};

		struct Error {
			String				message;
			usize				location;
			Core::Instruction	instruction;
		};

		struct ILibraryLoader {
			virtual ~ILibraryLoader() {}

			virtual bool loadLibrary(Context& context, String const& path) = 0;
		};

		struct DefaultLibraryLoader: ILibraryLoader {
			bool loadLibrary(Context& context, String const& path) override;
		};

		Engine(Config const& cfg = Config::createDefault()): config(cfg) {}

		virtual ~Engine() {}

		bool process();

		bool hasExposedCall(String const& name);

		template <class TFunc>
		struct ExposedCall;

		template <class TReturn, class... TArgs>
		struct ExposedCall<TReturn(TArgs...)> {
			using ReturnType = Meta::If<Type::Void<TReturn>, void, Nullable<TReturn>>;

			ReturnType call(TArgs... args) const		{return engine.template callExposed<TReturn, TArgs...>(name, args...);	}
			ReturnType operator()(TArgs... args) const	{return call(args...);													}

			ExposedCall(Engine& engine, String const& name): engine(engine), name(name) {}

		private:
			Engine&			engine;
			String const	name;
		};

		template <class TFunc>
		Nullable<ExposedCall<TFunc>> expose(String const& name) {
			if (!hasExposedCall(name)) return null;
			return ExposedCall<TFunc>{*this, name};
		}

		template <class... TArgs>
		void callExposed(String const& name, TArgs... args) {
			return invokeExposedCall(
				name,
				Core::Context::Arguments::from(
					context.newValue<TArgs>(args)...
				)
			);
		}

		template <class TReturn, class... TArgs>
		Meta::If<Type::Void<TReturn>, void, Nullable<TReturn>>
		callExposed(String const& name, TArgs... args) {
			invokeExposedCall(
				name,
				Core::Context::Arguments::from(
					context.newValue<TArgs>(args)...
				)
			);
			if constexpr (Type::NonVoid<TReturn>)
				return context->top() ? context->pop()->toValue<TReturn>() : null;
			else return;
		}

		void terminate();
		void reset();
		void load(Core::Module const& program);
		void execute();

		Nullable<Error>	error() const	{return err;}

		bool running() const;
		bool finished() const;

	protected:
		void invokeExposedCall(String const& name, Core::Context::Arguments const& args);

		virtual Context::Storage	external	(String const& name, bool const doNotCopy	);
		Context::Storage&			global		(uint64 const globalID						);

		constexpr bool inStrictMode() const {return context->scopeStack.back()->mode == Core::ContextMode::AV2_CM_STRICT;}

		void crash(Engine::Error const& error);

		bool hasFunction(String const& name);

		Random::SecureGenerator	srng;
		Random::SimpleGenerator	prng;

		AtomicCell<Context> context;

		Instance<ILibraryLoader> loader = new DefaultLibraryLoader();

		Config config;

		virtual void onLoad() {}

	private:
		void load();
		void unload();

		bool yieldCycle();

		Engine::Error invalidInstructionError();
		Engine::Error endOfProgramError();
		Engine::Error invalidOperationError(String const& description);
		Engine::Error invalidInternalValueError(uint64 const id);
		Engine::Error invalidLocationError(Core::ValueLocation const& loc);
		Engine::Error invalidSourceError(String const& description);
		Engine::Error invalidDestinationError(String const& description);
		Engine::Error invalidFunctionError(String const& description);
		Engine::Error invalidTypeError(String const& description);
		Engine::Error invalidComparisonError(String const& description);
		Engine::Error invalidFieldError(String const& description);
		Engine::Error invalidCast(String const& description);
		Engine::Error outOfRangeError(String const& description);
		Engine::Error invalidJump();
		Engine::Error missingArgumentsError();

		Engine::Error makeErrorHere(String const& message);

		Context::Storage consumeValue(Core::ValueLocation const from);
		Context::Storage getValueFromLocation(Core::ValueLocation const location, uint64 const id);

		Context::Storage& accessValue(Core::ValueLocation const from);
		Context::Storage& accessLocation(Core::ValueLocation const location, uint64 const id);

		Context::Storage validate(Context::Storage const& value, bool const passByCopy);

		void advance(bool isRequired = false);

		void v2Copy();
		void v2Call();
		void v2Negate();
		void v2StackBlit();
		void v2StackPush();
		void v2StackPop();
		void v2StackSwap();
		void v2StackGrow();
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
		void v2ScopeDeclare();
		void v2ScopeKeep();
		void v2FieldGet();
		void v2FieldSet();
		void v2Sizeof();
		void v2Typeof();
		void v2Random();
		void v2Select();
		void v2Clear();
		void v2Create();
		void v2Initialize();
		void v2Breakpoint();

		void doBinaryOperation(Core::Operator const op);
		void doUnaryOperation(Core::Operator const op);

		void fastBinaryOperation(Core::Operator const op, Core::BasicType const type);
		void fastUnaryOperation(Core::Operator const op, Core::BasicType const type);

		void immediateBinaryOperation(Core::Operator const op, Core::BasicType const type, pointer const data);

		void shortCircuitOperation(Core::Operator const op, usize const count);
		void fastShortCircuitOperation(Core::Operator const op, Core::BasicType const type, usize const count);

		struct Leap {
			using Mode = Core::Instruction::Leap::Mode;
			Mode	mode;
			usize	to;
			bool	returnable	= true;
			bool	async		= false;
		};

		void jumpByTableIndex(usize const tableID, bool returnable);
		void jumpTo(usize const point, bool returnable);
		void jumpByMode(Core::Instruction::Leap::Mode const mode, usize const location, bool returnable, bool async);
		void jump(Leap const& jump);
		void returnBack();

		usize spawn();
		void clone(AtomicCell<Context> const& other);
		void restore(usize const contextID);
		void kill(usize const contextID);

		void initializeObject(Core::Object::Storage const& object);

		State							engineState	= State::AV2_RES_READY;
		usize							delay		= 0;
		Core::Module					program;
		Core::Instruction				current;
		Nullable<Error>					err;
		Map<usize, AtomicCell<Context>>	spawned;
		usize							currentCID = 0;
	};
}

#endif
