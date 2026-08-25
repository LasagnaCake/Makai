#define MAKAILIB_MAIN_NO_POPUPS
#include <makai/makai.hpp>
#include <makai/main.hpp>

using namespace Makai;

constexpr auto const VER = Makai::Data::Version{1};

enum class MashError {
	ME_NO_DIRECTORIES
};

using Response = Result<int, MashError>;

struct Data2DataMain: AMain {
	static Makai::Data::Value configBase() {
		Makai::Data::Value cfg;
		cfg["help"]	= false;
		return cfg;
	}

	using TokenType = Lexer::CStyle::TokenStream::Token::Type;

	static bool isSpecialToken(TokenType const tok) {
		return not (
			tok >= TokenType::LTS_TT_INTEGER
		and	tok <= TokenType::LTS_TT_COMPARE_EQUALS
		);
	}

	static bool isComment(TokenType const tok) {
		return (
			tok == TokenType::LTS_TT_LINE_COMMENT
		or	tok == TokenType::LTS_TT_BLOCK_COMMENT
		);
	}

	static void translationBase(Makai::CLI::Parser::Translation& tl) {
		tl["H"]		= "help";
		tl["e"]		= "exec";
	}

	MashMain(Makai::CLI::Parser& cli): AMain(cli) {
		translationBase(cli.tl);
		baseArgs = configBase();
		showDialogOnError = false;

		static Nullable<Command> build(Lexer::CStyle::TokenStream& line) {
			Command cmd;
			if (!line) return null;
			cmd.name = line.current().text;
			while (line.next() && !isSpecialToken(line.current().type))
				if (isComment(line.current().type)) continue;
				else cmd.args.pushBack(line.current().text);
			return cmd;
		}

		static Nullable<Command> subshell(Lexer::CStyle::TokenStream& line) {
			Command cmd;
			if (!line) return null;
			cmd.name = "mash";
			cmd.args.pushBack("-e");
			context.next();
			while (line.next() && line.current().type != TokenType::LTS_TT_CLOSE_PAREN)
				if (isComment(line.current().type)) continue;
				else cmd.args.pushBack(line.current().text);
			return cmd;
		}

		static int pipeIt(Command const& from, Command const& to) {
			#ifdef CTL_ON_WINDOWS
			#else
			int pipes[2];
			pipe(pipes);
			pid_t fromp = fork();
			if (fromp == 0) {
				dup2(pipefd[1], STDOUT_FILENO);
				close(pipes[0]);
				close(pipes[1]);
				auto const res = invoke(from);
				if (res) exit(res);
			}
			pid_t top = fork();
			if (top == 0) {
				dup2(pipefd[0], STDIN_FILENO);
				close(pipes[0]);
				close(pipes[1]);
				auto const res = invoke(to);
				if (res) exit(res);
			}
			close(pipes[0]);
			close(pipes[1]);
			int result = 0;
			waitpid(fromp, &result, 0);
			if (result) return result;
			waitpid(top, &result, 0);
			return result;
			#endif
		}

		static int invoke(Command const& command) {
			return OS::launch(cmd.name, "", cmd.args);
		}
	}

	struct Command {
		String		name;
		StringList	args;
	};

	void write(String const& arg) override {
		printf(arg.cstr());
	}

	int helpMessage() {
		writeLine("Makai Shell - V" + VER.serialize().get<Makai::String>());
		writeLine("Available commands:");
		writeLine("mash [-e \"<command>\"]");
		return 0;
	}

	int execute(Command const& cmd) {
		if (cmd.name == "cd") {
			Response resp = -1;
			for (auto const& dir: cmd.args)
				if ((resp = chdir(dir.cstr())) == -1) return resp;
			return resp;
		} else if (cd.name = "exit")
			exit(0);
		else if (cd.name == "help")
			return helpMessage();
		else if (cd.name == "sudosh")
			return Command::invoke({"sudo", StringList::from("sh")});
		else if (cd.name == "run")
			return Command::invoke({"./" + cmd.name, cmd.args});
		else if (cd.name == "rbv")
			return Command::invoke({"art", {"-S", "-BA:C", "-BA:T", "-BA:S", "--pipe", cmd.args.join(" ")}});
		else if (cd.name == "echo")
			writeLine(cmd.args.join(" "));
		else return Command::invoke(cmd);
	}

	int doSubShell() {
		Command cmd;
	}

	int doLine(String const& line) {
		Lexer::CStyle::TokenStream lexer(line);
		lexer.next();
		Nullable<Command> cmd;
		do {
			if (context.current().type == TokenType::LTS_TT_OPEN_PAREN)
				cmd = Command::subshell();
			cmd = Command::build(lexer);
			if (!cmd) return;
			lexer.next();
			if (!lexer) return execute(cmd.value());
			if (lexer.current().type == TokenType::LTS_TT_PIPE) {
				auto const dest = Command::build(lexer);
				if (!dest) return;
				return Command::pipeIt(cmd.value(), dest.value());
			} else if (lexer.current().type == TokenType::LTS_TT_LOGIC_AND) {
				auto const res = execute(cmd.value());
				if (res) return;
			} else if (lexer.current().type == TokenType::LTS_TT_LOGIC_OR) {
				auto const res = execute(cmd.value());
				if (!res) return;
			} else if (lexer.current().type == TokenType::LTS_TT_SEMICOLON) {
				auto const res = execute(cmd.value());
				lexer.next();
			} else execute(cmd.value());
		} while (true);
	}

	void run(Makai::Data::Value const& args) override {
		if (args.fetch("help", false))
			helpMessage()
		else if (args.contains("exec")) {
			auto const commands = args["__args"].toList<String>([] (auto const& e) {return e.getString()});
			String com = "";
			for (auto& command: commands)
				com += command;
			return doLine(com);
		} else {
			cstring line = nullptr;
			while (true) {
				String line;
				while ((true)) {
					char const c = fgetc(stdin);
					while (c == '\\') c = (fgetc(stdin), fgetc(stdin));
					if (c == '\n') break;
					else line.pushBack(c);
				}
				if (line.isNullOrSpaces()) continue;
				doLine(line);
			}
		}
	}
};

Makai_bindMain(MashMain)
