#include <makai/makai.hpp>

using namespace Makai;
using namespace Anima::V2::Core;

struct ShellLib: ALibrary {
	using ProcessThread = Makai::AtomicCell<Makai::Thread>;

	inline static Makai::Mutex threadEditLock;
	inline static List<ProcessThread> processes;

	static bool AV2Call cd(String const& str) {
		#ifdef CTL_ON_WINDOWS
		//return _chdir(str.cstr()) != -1;
		return false;
		#else
		return chdir(str.cstr()) != -1;
		#endif
	}

	static nulltype handleExec(AtomicCell<Thread> self, Anima::V2::Core::Promise<int64> out, String command, StringList args) {
		out.set(OS::launch(command, "", args));
		threadEditLock.lock();
		processes.eraseLike(self);
		threadEditLock.unlock();
		return null;
	}

	static Anima::V2::Core::Promise<int64> AV2Call exec(Context& context, String const& command, StringList const& args) {
		auto output = context.promise<int64>();
		auto const thread = ProcessThread::create();
		threadEditLock.lock();
		processes.pushBack(thread);
		threadEditLock.unlock();
		thread->invoke<nulltype>(handleExec, thread, output, command, args);
		return output;
	}

	void load(Context::Adder const& context) override {
		context.methods.add("av2/shell/cd",		cd		);
		context.methods.add("av2/shell/exec",	exec	);
	}

	String name() const override {return "av2/shell";}
};

AV2_Library(ShellLib);
