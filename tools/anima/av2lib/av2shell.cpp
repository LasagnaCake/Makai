#include <makai/makai.hpp>
#include <iostream>

using namespace Makai;
using namespace Anima::V2::Core;

#define doWrite(WHAT) std::cout << WHAT
#define doWriteLine(WHAT) std::cout << WHAT << "\n"

struct ShellLib: ALibrary {
	static Mutex threadEditLock;
	static List<AtomicCell<Thread>> processes;

	static bool AV2Call cd(String const& str) {
		return chdir(str.cstr()) != -1;
	}

	static void handleExec(AtomicCell<Thread> self, Object::Storage out, String command, StringList args) {
		out->set(OS::launch(command, "", args));
		threadEditLock.lock();
		processes.eraseLike(self);
		threadEditLock.unlock();

	}

	static Object::Storage AV2Call exec(Context& context, String const& command, StringList const& args) {
		Object::Storage output = output.newEmpty<int64>();
		auto const thread = AtomicCell<Thread>::create();
		threadEditLock.lock();
		processes.pushBack(thread);
		threadEditLock.unlock();
		thread->invoke(handleExec, thread, output, command, args);
		return output;
	}

	void load(Context::Adder const& context) override {
		context.methods.add("av2/shell/cd", 	cd		);
		context.methods.add("av2/shell/exec",	exec	);
	}

	String name() const override {return "av2/shell";}
};

AV2_Library(ShellLib);
