#ifndef MAKAILIB_ANIMA_V2_RUNTIME_ENGINE_H
#define MAKAILIB_ANIMA_V2_RUNTIME_ENGINE_H

#include "context.hpp"
#include "program.hpp"

namespace Makai::Anima::V2::Runtime {
	struct Engine {
		using Value = Data::Value;

		struct FunctionRegistry {
			using ExternalFunction = Data::Value(Data::Value::ArrayType const&);

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

			constexpr Value invoke(String const& name, Value::ArrayType const& args) {
				if (!nameMap.contains(name)) return Value::undefined();
				Value::ArrayType result;
				return functions[nameMap[name]](args);
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

		enum class BuiltInFunction: char {
			AV2_EBIF_ADD	= '+',
			AV2_EBIF_SUB	= '-',
			AV2_EBIF_MUL	= '*',
			AV2_EBIF_DIV	= '/',
			AV2_EBIF_MOD	= '%',
			AV2_EBIF_COMP	= '=',
			AV2_EBIF_NEG	= 'n',
			AV2_EBIF_AND	= '&',
			AV2_EBIF_OR		= '|',
			AV2_EBIF_NOT	= '~',
			AV2_EBIF_XOR	= '^',
			AV2_EBIF_LAND	= 'a',
			AV2_EBIF_LOR	= 'o',
			AV2_EBIF_LNOT	= '!',
		};

		enum class Action {
			AV2_EA_NULL,
			AV2_EA_V1_COMMAND,
			AV2_EA_EXTERN_CALL
		};

		bool process();

		FunctionRegistry functions;

	protected:
		virtual Value fetchExternal(uint64 const valueID);

		static Value fetchInternal(uint64 const valueID);

	private:
		Engine::Error invalidInstructionEror();
		Engine::Error invalidSourceEror(String const& description);
		Engine::Error invalidFunctionEror(String const& description);

		Value consumeValue(DataLocation const from);
		Value getValueFromLocation(DataLocation const location, usize const id);
		
		Value& accessValue(DataLocation const from);
		Value& accessLocation(DataLocation const location, usize const id);

		void crash(Engine::Error const& error);

		void advance();
		void terminate();

		void v2Invoke();
		void v2Global();
		void v2Copy();
		void v2Call();
		void v2Negate();
		void v2StackPush();
		void v2StackPop();
		void v2StackSwap();
		void v2StackClear();
		void v2Return();
		void v2Halt();

		void callBuiltIn(BuiltInFunction const func);

		void jumpTo(usize const point, bool returnable);
		void returnBack();

		bool			isFinished = false;
		Program			program;
		Context			context;
		Instruction		current;
		Nullable<Error>	err;
	};
}

#endif
