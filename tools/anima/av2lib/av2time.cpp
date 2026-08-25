#include <makai/makai.hpp>
#include <iostream>

using namespace Makai;
using namespace Anima::V2::Core;

#define doWrite(WHAT) std::cout << WHAT
#define doWriteLine(WHAT) std::cout << WHAT << "\n"

struct TimeLib: ALibrary {
	AV2Call
	static int64 procNow(int64 const precision) {
		namespace Time = Makai::OS::Time;
		switch (precision) {
			case -2:	return Time::Clock::sinceStart<Time::Hours>();
			case -1:	return Time::Clock::sinceStart<Time::Minutes>();
			case 0:		return Time::Clock::sinceStart<Time::Seconds>();
			case +1:	return Time::Clock::sinceStart<Time::Millis>();
			case +2:	return Time::Clock::sinceStart<Time::Micros>();
			case +3:	return Time::Clock::sinceStart<Time::Nanos>();
			default:	return Makai::Limit::MAX<int64>;
		}
	}

	AV2Call
	static int64 localNow(int64 const precision) {
		namespace Time = Makai::OS::Time;
		switch (precision) {
			case -2:	return Time::Clock::sinceEpoch<Time::Hours>();
			case -1:	return Time::Clock::sinceEpoch<Time::Minutes>();
			case 0:		return Time::Clock::sinceEpoch<Time::Seconds>();
			case +1:	return Time::Clock::sinceEpoch<Time::Millis>();
			case +2:	return Time::Clock::sinceEpoch<Time::Micros>();
			case +3:	return Time::Clock::sinceEpoch<Time::Nanos>();
			default:	return Makai::Limit::MAX<int64>;
		}
	}

	AV2Call
	static int64 utcNow(int64 const precision) {
		using Zone = Makai::Zone;
		auto const local = localNow(precision);
		if (local == Makai::Limit::MAX<int64>) return local;
		auto secs = int64(local * Makai::Math::pow<double>(10, -precision));
		secs -= Zone::convert(secs, Zone::current(), Zone::utc());
		return local + int64(secs * Makai::Math::pow<double>(10, precision));
	}


	void load(Context::Adder const& context) override {
		context.methods.add("av2/time/localNow",	localNow	);
		context.methods.add("av2/time/utcNow",		utcNow		);
		context.methods.add("av2/time/procNow",		procNow		);
	}

	String name() const override {return "av2/time";}
};

AV2_Library(TimeLib);
