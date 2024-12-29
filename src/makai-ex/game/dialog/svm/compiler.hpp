#ifndef MAKAILIB_EX_GAME_DIALOG_SVM_COMPILER_H
#define MAKAILIB_EX_GAME_DIALOG_SVM_COMPILER_H

#include <makai/makai.hpp>

#include "bytecode.hpp"

namespace Makai::Ex::Game::Dialog::SVM {
	struct Compiler {
		Compiler()										{					}
		Compiler(String const& script): script(script)	{compileScript();	}

		ByteCode result() {
			return out;
		}

		void compile(String const& script) {
			this->script = script;
			compiled = false;
			compileScript();
		}

		constexpr static bool isNameChar(char const& c) {
			return (
				isNumberChar(c)
			||	isUppercaseChar(c)
			||	isLowercaseChar(c)
			||	(c == '_')
			||	(c == '-')
			);
		}

		constexpr static bool isSPOperationChar(char const& c) {
			return (
				(c == '"')
			||	(c == '[')
			);
		}
	
	private:
		bool		compiled = false;
		String		script;
		ByteCode	out;
		usize		dataIndex = 0, lineIndex = 1, columnIndex = 1;

		struct ScopeDelimiter {
			char begin, end;
			constexpr ScopeDelimiter(char const ch) {
				switch(ch) {
					case '[':
					case ']': begin = '['; end = ']'; break;
					case '(':
					case ')': begin = '('; end = ')'; break;
					case '{':
					case '}': begin = '{'; end = '}'; break;
					case '<':
					case '>': begin = '<'; end = '>'; break;
					case '"': begin = end = '"'; break;
					case '\'': begin = end = '\''; break;
				}
			}
		};

		Operation curOp = Operation::DSO_NO_OP;

		static void opcopy(Decay::AsType<uint16[4]>& buf, uint64 const val) {
			MX::memcpy((void*)buf, (void*)&val, sizeof(uint64));
		}

		void initialize() {
			out = {};
			dataIndex	= 0;
			lineIndex	= 1;
			columnIndex	= 1;
		}

		void compileScript() {
			if (compiled) return;
			compiled = true;
			initialize();
			removeComments();
			processScript();
		}

		static String format(String str) {
			str = Regex::replace(str, "\\\\", "\\");
			str = Regex::replace(str, "\\n", "\n");
			str = Regex::replace(str, "\\t", "\t");
			str = Regex::replace(str, "\\v", "\v");
			str = Regex::replace(str, "\\r", "\r");
			str = Regex::replace(str, "\\f", "\f");
			str = Regex::replace(str, "\\b", "\b");
			str = Regex::replace(str, "\\0", "");
			str = Regex::replace(str, "\\\"", "\"");
			if (!str.nullTerminated())
				str.pushBack('\0');
			return str;
		}

		void addOperation(Operation const op, uint16 sp = 0) {
			out.code.pushBack(enumcast(op) | spFlag(sp));
		}

		void addOperand(uint64 const op) {
			uint16 opbuf[4];
			opcopy(opbuf, op);
			out.code.appendBack(opbuf);
		}

		void addStringOperand(String const& str) {
			addOperand(dataIndex++);
			out.data.pushBack(format(str));
		}

		void addLine(String const& str) {
			addOperation(Operation::DSO_LINE);
			if (str.empty())
				return addOperand(0);
			addStringOperand(str);
		}

		void addActors(StringList const& strs) {
			if (strs.empty()) {
				addOperation(Operation::DSO_ACTOR);
				addOperand(0);
				return;
			}
			for (usize i = 0; i < strs.size(); ++i) {
				addOperation(Operation::DSO_ACTOR,(i) ? 1 : 0);
				addOperand(Hasher::hash(strs[i]));
			}
		}

		void addEmotion(String const& str) {
			addOperation(Operation::DSO_EMOTION);
			addOperand(Hasher::hash(str));
		}

		void addAction(String const& str, bool const sp = false) {
			addOperation(Operation::DSO_ACTION, sp ? 1 : 0);
			addOperand(Hasher::hash(str));
		}

		void addFlag(String const& str, bool const state = false) {
			addOperation(Operation::DSO_SET_GLOBAL);
			addOperand(Hasher::hash(str));
			addStringOperand(state ? "true" : "false");
		}

		void addGlobal(String const& str, bool const sp = false) {
			addOperation(Operation::DSO_SET_GLOBAL, sp ? 1 : 0);
		}

		void addColor(String const& str) {
			addOperation(Operation::DSO_SET_GLOBAL);
			addOperand(Graph::Color::toHexCodeRGBA(Graph::Color::fromHexCodeString(str)));
		}

		void addWait(String const& str) {
			addOperation(Operation::DSO_SET_GLOBAL);
			addOperand(toUInt64(str));
		}

		void addParamPack(StringList const& strs) {
			for (String const& str: strs)
				out.data.pushBack(format(str));
			addOperand(dataIndex);
			addOperand(strs.size());
			dataIndex += strs.size();
		}

		template<class T>
		StringList processParamPack(T& c, T& end, ScopeDelimiter const& sd = ']') {
			String buf;
			StringList params;
			bool space = true;
			while (c != end && c != sd.end) {
				lineIterate(*c);
				if (!isNullOrSpaceChar(*c) && *c != ',' && space)
					invalidParameterError();
				else if (isNullOrSpaceChar(*c)) {
					if (!buf.empty())
						space = true;
				} else if (isScopeChar(*c) && !isQuoteChar(*c))
					params.appendBack(processParamPack(c, end, *c++))
				else if (*c == ',') {
					params.pushBack(buf);
					buf.clear();
				} else if (isQuoteChar(*c)) {
					params.pushBack(processString(c, end, *c++));
				}
				else buf.pushBack(*c);
				++c;
			}
			return params;
		}

		template<class T>
		String processString(T& c, T& end, char const scope = '"') {
			String buf;
			while (c != end && *c != scope) {
				lineIterate(*c);
				if (*c == '\\') {
					buf.pushBack(*c++);
					if (c >= end) malformedError();
				}
				buf.pushBack(*c++);
			}
			return format(buf);
		}

		template<class T>
		String processCommand(T& c, T& end) {
			String buf;
			if (!isNameChar(*c))
				malformedError();
			while (c != end && isNameChar(*c)) {
				lineIterate(*c);
				buf.pushBack(*c++);
			}
			return buf;
		}

		template<class T>
		String processHex(T& c, T& end) {
			String buf;
			auto start = c;
			if (!isHexChar(*c))
				malformedError();
			while (c != end && isHexChar(*c) && (c - start) < 8) {
				lineIterate(*c);
				buf.pushBack(*c++);
			}
			return buf;
		}

		template<class T>
		String processNumber(T& c, T& end) {
			String buf;
			auto start = c;
			if (!isNumberChar(*c))
				malformedError();
			while (c != end && isNumberChar(*c)) {
				lineIterate(*c);
				buf.pushBack(*c++);
			}
			return buf;
		}

		void processScript() {
			auto c		= script.begin();
			auto end	= script.end();
			while (c != end) {
				lineIterate(*c);
				switch (*c) {
					case '*':
						while (c != end && isNullOrSpaceChar(*c))
							lineIterate(*++c);
						if (!isSPOperationChar(*c))
							invalidExtendedOperationError();
						addOperation(Operation::DSO_NO_OP, 1);
						break;
					case '.':
						addOperation(Operation::DSO_SYNC);
						break;
					case ';':
						addOperation(Operation::DSO_USER_INPUT);
						break;
					case '"':
						addLine(processString(++c, end));
						break;
					case '[':
						addActors(processParamPack(++c, end, ']'));
						break;
					case '(':
						addParamPack(processParamPack(++c, end, ')'));
						break;
					case '@': {
						String cmd = processCommand(++c, end);
						while (c != end && isNullOrSpaceChar(*c)) lineIterate(*++c);
						addAction(cmd, *c == '(');
						if (*c == '(')
							addParamPack(processParamPack(++c, end, ')'));
						break;
					}
					case '!':
						addEmotion(processCommand(++c, end));
						break;
					case '+':
						addFlag(processCommand(++c, end), true);
						break;
					case '-':
						addFlag(processCommand(++c, end), false);
						break;
					case '$': {
						String cmd = processCommand(++c, end);
						while (c != end && isNullOrSpaceChar(*c)) lineIterate(*++c);
						addGlobal(processCommand(++c, end), *c == '(');
						if (*c == '(')
							addParamPack(processParamPack(++c, end, ')'));
						else addStringOperand(
							isQuoteChar(*c++)
							? processString(c, end)
							: processCommand(c, end)
						);
						break;
					}
					case '#':
						addColor(processHex(++c, end));
					case '\'':
						addWait(processNumber(++c, end));
						break;
					default: if (!isNullOrSpaceChar(*c)) invalidOperationError();
				}
				++c;
			}
		}

		void removeComments() {
			script = Regex::replace(script, "(\\/\\/.*$|\\/\\*(.|\\n)*?(\\*\\/))+", "");
			script = Regex::replace(script, "(\\/\\*(.*|\\n)*)", "");
		}

		void lineIterate(char const c) {
			if (c == '\n') {
				columnIndex = 1;
				++lineIndex;
			} else ++columnIndex;
		}
		
		[[noreturn]]
		void invalidOperationError() {
			throw Error::InvalidValue(
				"Invalid operation!",
				toString("Line: ", lineIndex, "\nColumn: ", columnIndex),
				CTL_CPP_PRETTY_SOURCE
			);
		}

		[[noreturn]]
		void invalidExtendedOperationError() {
			throw Error::InvalidValue(
				"This operation is not an extensible operation!",
				toString("Line: ", lineIndex, "\nColumn: ", columnIndex),
				CTL_CPP_PRETTY_SOURCE
			);
		}
		
		[[noreturn]]
		void malformedError() {
			throw Error::InvalidValue(
				"Malformed operation/parameter!",
				toString("Line: ", lineIndex, "\nColumn: ", columnIndex),
				CTL_CPP_PRETTY_SOURCE
			);
		}

		[[noreturn]]
		void invalidParameterError() {
			throw Error::InvalidValue(
				"Invalid parameter!",
				toString("Line: ", lineIndex, "\nColumn: ", columnIndex),
				CTL_CPP_PRETTY_SOURCE
			);
		}
	};
}

#endif