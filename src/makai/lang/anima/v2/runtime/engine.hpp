#ifndef MAKAILIB_ANIMA_V2_RUNTIME_ENGINE_H
#define MAKAILIB_ANIMA_V2_RUNTIME_ENGINE_H

#include "context.hpp"
#include "program.hpp"

namespace Makai::Anima::V2::Runtime {
	struct Engine {
		using Value = Data::Value;

		struct FunctionRegistry {
			using ExternalFunction = Context::Storage(List<Context::Storage> const&);

			using UUID = ID::VLUID;

			template <Type::Functional<ExternalFunction> TFunction>
			constexpr UUID add(String const& name, TFunction const& func) {
				auto const id = ++current;
				functions[id] = func;
				bind(name, id);
				return id;
			}

			constexpr void remove(UUID const& id) {
				nameMap.filter([&] (auto const& e) {return e.value == id;});
			}

			constexpr UUID bind(String const& name, UUID const& id) {
				auto const prev = nameMap[name];
				nameMap[name] = id;
				return prev;
			}

			constexpr void unbind(String const& name, UUID const& id) {
				nameMap.erase(name);
				functions.erase(id);
			}

			constexpr void clear(String const& name) {
				nameMap.erase(name);
			}

			constexpr Context::Storage invoke(String const& name, List<Context::Storage> const& args) {
				if (!has(name)) return new Value();
				return invoke(nameMap[name], args);
			}

			constexpr Context::Storage invoke(UUID const& id, List<Context::Storage> const& args) {
				if (!has(id)) return new Value();
				return functions[id](args);
			}

			constexpr bool has(String const& name) const {
				return nameMap.contains(name);
			}

			constexpr bool has(UUID const& id) const {
				return functions.contains(id);
			}

		private:
			Dictionary<UUID>						nameMap;
			Map<UUID, Function<ExternalFunction>>	functions;

			UUID current = UUID::create(0);
		};

		struct Error {
			String		message;
			usize		location;
			Instruction	instruction;
		};

		enum class BuiltInFunction: uint8 {
			AV2_EBIF_ADD			= '+',
			AV2_EBIF_SUB			= '-',
			AV2_EBIF_MUL			= '*',
			AV2_EBIF_DIV			= '/',
			AV2_EBIF_POW			= 'p',
			AV2_EBIF_REM			= '%',
			AV2_EBIF_COMP			= '=',
			AV2_EBIF_NEG			= 'n',
			AV2_EBIF_AND			= '&',
			AV2_EBIF_OR				= '|',
			AV2_EBIF_NOT			= '~',
			AV2_EBIF_XOR			= '^',
			AV2_EBIF_LAND			= 'a',
			AV2_EBIF_LOR			= 'o',
			AV2_EBIF_LNOT			= '!',
			AV2_EBIF_SIN			= 's',
			AV2_EBIF_COS			= 'c',
			AV2_EBIF_TAN			= 't',
			AV2_EBIF_ASIN			= 'S',
			AV2_EBIF_ACOS			= 'C',
			AV2_EBIF_ATAN			= 'T',
			AV2_EBIF_ATAN2			= '2',
			AV2_EBIF_INTERRUPT		= '.',
			AV2_EBIF_READ			= ':',
			AV2_EBIF_PRINT			= '@',
			AV2_EBIF_TOSTRING		= '_',
			AV2_EBIF_STR_MATCH		= 'm',
			AV2_EBIF_STR_FORMAT		= '$',
			AV2_EBIF_SIZEOF			= '#',
			AV2_EBIF_HTTP_REQUEST	= 'H',
			AV2_EBIF_STRING_OP		= '"',
			AV2_EBIF_ARRAY_OP		= '[',
			AV2_EBIF_OBJECT_OP		= '{',
		};

		enum class BuiltInStringOperation: uint8 {
			AV2_EBI_SO_SLICE	= '_',
			AV2_EBI_SO_REPLACE	= ':',
			AV2_EBI_SO_SPLIT	= '/',
			AV2_EBI_SO_JOIN		= '+',
			AV2_EBI_SO_MATCHES	= '=',
			AV2_EBI_SO_CONTAINS	= '@',
			AV2_EBI_SO_REMOVE	= '-',
			AV2_EBI_SO_FIND		= 'f',
		};

		enum class BuiltInArrayOperation: uint8 {
			AV2_EBI_AO_SLICE			= '_',
			AV2_EBI_SO_REMOVE			= '-',
			AV2_EBI_AO_JOIN				= '+',
			AV2_EBI_AO_FILTER_LIKE		= '=',
			AV2_EBI_AO_FILTER_UNLIKE	= '!',
			AV2_EBI_AO_FIND				= 'f',
			AV2_EBI_AO_FUZZY_FIND		= 'F',
			AV2_EBI_AO_PUSH				= '<',
			AV2_EBI_AO_POP				= '>',
		};

		enum class BuiltInObjectOperation: uint8 {
			AV2_EBI_OO_REMOVE_BY_KEY		= 'd',
			AV2_EBI_OO_REMOVE_BY_VALUE		= 'D',
			AV2_EBI_OO_JOIN					= '+',
			AV2_EBI_OO_FIND_BY_KEY			= 'f',
			AV2_EBI_OO_FUZZY_FIND_BY_KEY	= 'F',
			AV2_EBI_OO_FIND_BY_VALUE		= 'x',
			AV2_EBI_OO_FUZZY_FIND_BY_VALUE	= 'X',
			AV2_EBI_OO_KEYS					= 'k',
			AV2_EBI_OO_VALUES				= 'v',
			AV2_EBI_OO_ITEMS				= 'i',
		};

		bool process();

		FunctionRegistry functions;

		virtual void		onPrint(Data::Value const& value);
		virtual Data::Value	onHTTPRequest(String const& url, String const& action, Data::Value const& value);

		bool hasSignal(String const& name);
		void fire(String const& signal);

		void terminate();
		void reset();
		void load(Program const& program);
		void execute();

	protected:
		virtual Context::Storage	external	(String const& name, bool const byRef	);
		Context::Storage			internal	(uint64 const valueID					);
		Context::Storage&			temporary	(										);
		Context::Storage&			global		(uint64 const globalID					);

		constexpr bool inStrictMode() const {return context.mode == ContextMode::AV2_CM_STRICT;}

		void crash(Engine::Error const& error);

		bool hasFunction(String const& name);

	private:
		bool yieldCycle();

		Engine::Error invalidInstructionError();
		Engine::Error endOfProgramError();
		Engine::Error invalidBinaryMathError(String const& description);
		Engine::Error invalidUnaryMathError(String const& description);
		Engine::Error invalidInternalValueError(uint64 const id);
		Engine::Error invalidLocationError(DataLocation const& loc);
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

		void pushUndefinedIfInLooseMode(String const& fname);

		Context::Storage consumeValue(DataLocation const from);
		Context::Storage getValueFromLocation(DataLocation const location, usize const id);

		Context::Storage& accessValue(DataLocation const from);
		Context::Storage& accessLocation(DataLocation const location, usize const id);

		void advance(bool isRequired = false);

		void v2Invoke();
		void v2Copy();
		void v2Call();
		void v2Negate();
		void v2StackPush();
		void v2StackPop();
		void v2StackSwap();
		void v2StackClear();
		void v2StackFlush();
		void v2Return();
		void v2Halt();
		void v2BinaryMath();
		void v2UnaryMath();
		void v2SetContext();
		void v2Compare();
		void v2Get();
		void v2Set();
		void v2Cast();
		void v2Jump();
		void v2Await();
		void v2Yield();

		void callBuiltIn(BuiltInFunction const func);

		void callBuiltInStringOp(BuiltInFunction const func);
		void callBuiltInArrayOp(BuiltInFunction const func);
		void callBuiltInObjectOp(BuiltInFunction const func);

		void jumpBy(usize const tableID, bool returnable);
		void jumpTo(usize const point, bool returnable);
		void returnBack();

		struct WaitState {
			Context::Storage				condition;
			Instruction::WaitRequest::Wait	type;

			constexpr void clear() {if (condition) condition = nullptr;}

			constexpr bool waiting() const {
				if (!condition) return false;
				switch (type) {
					case Instruction::WaitRequest::Wait::AV2_IWRW_TRUTHY:	return !*condition;
					case Instruction::WaitRequest::Wait::AV2_IWRW_FALSY:	return *condition;
				}
				return false;
			}

			constexpr operator bool() const {return waiting();}
		} wait;

		bool				isFinished	= false;
		bool				paused		= false;
		Program				program;
		Context				context;
		Instruction			current;
		Nullable<Error>		err;
	};
}

#endif
