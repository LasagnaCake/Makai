#ifndef MAKAILIB_EX_GAME_DIALOG_SVM_COMPILER_H
#define MAKAILIB_EX_GAME_DIALOG_SVM_COMPILER_H

#include <makai/makai.hpp>

#include "bytecode.hpp"

namespace Makai::Ex::Game::Dialog::SVM {
	struct Compiler {

		ByteCode compile() {
			ByteCode out;
			removeComments();
			processScript();
			return out;
		}
	
	private:
		String		script;
		ByteCode	out;
		usize		dataIndex, lineIndex, columnIndex;

		Operation curOp = Operation::DSO_NO_OP;

		static void opcopy(Decay::AsType<uint16[4]>& buf, uint64 const val) {
			MX::memcpy((void*)buf, (void*)&val, sizeof(uint64));
		}

		usize getStringLineCount(String const& str) {
			usize count = 0;
			for(char const& c: str)
				if (c == '\n')
					++count;
			return count;
		}

		String format(String str) {
			str = Regex::replace(str, "\\n", "\n");
			str = Regex::replace(str, "\\t", "\t");
			str = Regex::replace(str, "\\v", "\v");
			str = Regex::replace(str, "\\r", "\r");
			str = Regex::replace(str, "\\f", "\f");
			str = Regex::replace(str, "\\b", "\b");
			str = Regex::replace(str, "\\0", "");
			str = Regex::replace(str, "\\\"", "\"");
			str = Regex::replace(str, "\\\\", "\\");
			if (!str.nullTerminated())
				str.pushBack('\0');
			return str;
		}

		void addOperation(Operation const op) {
			out.code.pushBack(enumcast(op));
		}

		void addOperand(uint64 const op) {
			uint16 opbuf[4];
			opcopy(opbuf, dataIndex++);
			out.code.appendBack(opbuf);
		}

		void addStringOperand(String const& str) {
			addOperand(dataIndex++);
			out.data.pushBack(format(str));
		}

		void addLine(String const& str) {
			addOperation(Operation::DSO_LINE);
			addStringOperand(str);
		}

		void addActors(StringList const& strs) {
			for (usize i = 0; i < strs.size(); ++i) {
				if (i) addOperation(Operation::DSO_SP);
				addOperation(Operation::DSO_ACTOR);
				addOperand(Hasher::hash(strs[i]));
			}
		}

		void addEmotion(String const& str) {
			addOperation(Operation::DSO_EMOTION);
			addOperand(Hasher::hash(str));
		}

		void addAction(String const& str, bool const sp = false) {
			if (sp) addOperation(Operation::DSO_SP);
			addOperation(Operation::DSO_ACTION);
			addOperand(Hasher::hash(str));
			addOperand(Hasher::hash(str));
		}

		void addFlag(String const& str, bool const state = false) {
			addOperation(Operation::DSO_SET_GLOBAL);
			addOperand(Hasher::hash(str));
			addStringOperand(state ? "true" : "false");
		}

		void addGlobal(String const& str, bool const sp = false) {
			if (sp) addOperation(Operation::DSO_SP);
			addOperation(Operation::DSO_SET_GLOBAL);
		}

		void addColor(String const& str) {
			addOperation(Operation::DSO_SET_GLOBAL);
			addOperand(Graph::Color::toHexCodeRGBA(Graph::Color::fromHexCodeString(str)));
		}

		void addParamPack(StringList const& strs) {
			for (String const& str: strs)
				out.data.pushBack(format(str));
			addOperand(dataIndex);
			addOperand(strs.size());
			dataIndex += strs.size();
		}

		template<class T>
		StringList processParamPack(T& c, T& end, char const ch = ']') {
			String buf;
			StringList params;
			bool space = true;
			while (c != end && c != ch) {
				lineIterate(*c);
				if (!isNullOrSpaceChar(*c) && *c != ',' && space)
					invalidParameterError();
				else if (isNullOrSpaceChar(*c)) {
					if (!buf.empty())
						space = true;
				} else if (*c == ',') {
					params.pushBack(buf);
					buf.clear();
				} else if (*c == '"') {
					params.pushBack(processString(++c, end));
				}
				else buf.pushBack(*c);
				++c;
			}
			return buf;
		}

		template<class T>
		String processString(T& c, T& end) {
			String buf;
			while (c != end && *c != '"') {
				lineIterate(*c);
				if (*c == '\\') {
					buf.pushBack(*c++);
					if (c >= end) sizeError();
				}
				buf.pushBack(*c++);
			}
			return format(buf);
		}

		template<class T>
		String processCommand(T& c, T& end) {
			String buf;
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
			while (c != end && isHexChar(*c) && (c - start) < 8) {
				lineIterate(*c);
				buf.pushBack(*c++);
			}
			return buf;
		}

		static bool isNameChar(char const& c) {
			return (
				(c >= '0' && c <= '9')
			||	(c >= 'a' && c <= 'z')
			||	(c >= 'A' && c <= 'Z')
			||	(c == '_')
			||	(c == '-')
			);
		}

		void processScript() {
			auto c = script.begin();
			auto end = script.end();
			String buf;
			while (c != end) {
				lineIterate(*c);
				switch (*c) {
					case '*':
						addOperation(Operation::DSO_SP);
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
						while (c != end && isNullOrSpaceChar(*c)) ++c;
						addAction(cmd, *c == '(');
						break;
					}
					case '!': {
						addEmotion(processCommand(++c, end));
					}
					case '+':
						addFlag(processCommand(++c, end), true);
						break;
					case '-':
						addFlag(processCommand(++c, end), false);
						break;
					case '$': {
						String cmd = processCommand(++c, end);
						while (c != end && isNullOrSpaceChar(*c)) ++c;
						addGlobal(processCommand(++c, end), *c == '(');
						if (*c == '(')
							addParamPack(processParamPack(++c, end));
						else addStringOperand(
							*c++ == '"'
							? processString(c, end)
							: processCommand(c, end)
						);
						break;
					}
					case '#':
						addColor(processHex(++c, end));
					case '\'':
						break;
					default: invalidOperationError();
				}
				++c;
			}
		}

		void removeComments() {
			script = Regex::replace(script, "(\\/\\/.*$|\\/\\*(.|\\n)*?(\\*\\/))+", "");
			script = Regex::replace(script, "(\\/\\*(.*|\\n)*)", "");
		}

		static StringList consumeStrings(String& str) {
			auto matches = Regex::find(str, ("(.|\n)*?"));
			str = Regex::replace(str, "(\"(.|\\n)*?\")", "\"");
			StringList strings;
			strings.resize(matches.size());
			for (Regex::Match& m: matches)
				strings.pushBack(m.match);
			return strings;
		}

		void lineIterate(char const c) {
			if (c == '\n') {
				columnIndex = 0;
				++lineIndex;
			} else ++columnIndex;
		}
		
		[[noreturn]]
		void invalidOperationError();
		
		[[noreturn]]
		void sizeError();

		[[noreturn]]
		void invalidParameterError();
	};
}

#endif