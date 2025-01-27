#include <makai/makai.hpp>
#include <makai-ex/anima/anima.hpp>

struct TestEngine: Makai::Ex::AVM::Engine {
	void opSay(ActiveCast const& actors, Makai::String const& line) override {
		DEBUGLN("-----");
		printActors(actors);
		DEBUGLN("Say: '", line, "'");
	}

	void opAdd(ActiveCast const& actors, Makai::String const& line) override {
		DEBUGLN("-----");
		printActors(actors);
		DEBUGLN("Add: '", line, "'");
	}

	void opEmote(ActiveCast const& actors, uint64 const emotion) override {
		DEBUGLN("-----");
		printActors(actors);
		DEBUGLN("Emotion: ", emotion);
	}

	void opPerform(ActiveCast const& actors, uint64 const action, Parameters const& params) override {
		DEBUGLN("-----");
		printActors(actors);
		DEBUGLN("Action: ", action);
		if (params.size()) DEBUGLN("Params: ['", params.join("', '"), "']");
	}

	void opColor(ActiveCast const& actors, uint64 const color) override {
		DEBUGLN("-----");
		printActors(actors);
		DEBUGLN("Text color HEX: ", CTL::Format::pad(CTL::toString<uint64>(color, 16), '0', 8));
	}

	void opColorRef(ActiveCast const& actors, uint64 const color) override {
		DEBUGLN("-----");
		printActors(actors);
		DEBUGLN("Text color name: ", color);
	}

	void opDelay(uint64 const time) override {
		DEBUGLN("-----");
		DEBUGLN("Delay: ", time);
	}

	void opWaitForActions(bool const async) override {
		DEBUGLN("-----");
		DEBUGLN("Sync");
	}

	void opWaitForUser() override {
		DEBUGLN("-----");
		DEBUGLN("User input");
	}

	void opNamedCallSingle(uint64 const param, Makai::String const& value) override {
		DEBUGLN("-----");
		DEBUGLN("Call: ", param);
		DEBUGLN("Value: ", value);
	}

	void opNamedCallMultiple(uint64 const param, Parameters const& values) override {
		DEBUGLN("-----");
		DEBUGLN("Call: ", param);
		DEBUGLN("Values: ['", values.join("', '"), "']");
	}

	void opGetInt(uint64 const name, ssize& out) {
		DEBUGLN("-----");
		DEBUGLN("Int: ", name);
		out = 0;
	}

	void opGetString(uint64 const name, Makai::String& out) override {
		DEBUGLN("-----");
		DEBUGLN("String: ", name);
		out = "";
	}

	static void printActors(ActiveCast const& actors) {
		DEBUG("Actors: [ ");
		if (actors.exclude) DEBUG("All except: ");
		for (auto& actor: actors.actors)
			DEBUG(actor, " ");
		DEBUGLN(" ]");
	}
};

int main(int argc, char** argv) {
	if (argc < 2) return 0;
	TestEngine engine;
	auto const anb = Makai::Ex::AVM::Anima::fromBytes(Makai::File::getBinary(argv[1]));
	DEBUGLN("Binding binary...");
	engine.setProgram(anb);
	DEBUGLN("Starting program...");
	engine.beginProgram();
	while (engine)
		engine.process();
	DEBUGLN("-----");
	DEBUGLN("Done!");
	if (engine.state() == TestEngine::State::AVM_ES_ERROR) {
		DEBUGLN("ERROR: ", CTL::enumcast(engine.error()));
		return -1;
	}
	DEBUGLN("No errors!");
	return 0;
}