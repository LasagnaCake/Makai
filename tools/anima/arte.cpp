#include <makai/makai.hpp>
#include <makai/main.hpp>

struct ARTE: Makai::Anima::V2::Runtime::Engine {

	void onPrint(Makai::Data::Value const& value) override {
		#ifdef ARTE_CLI
		if (value.isString())
			std::cout << value.getString();
		else std::cout << value.toFLOWString();
		#endif
	}
};

main(args) {
	ARTE engine;
}
