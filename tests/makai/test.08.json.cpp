#include <makai/makai.hpp>

int main() {
	DEBUGLN("Running app ", __FILE__, "...");
	try {
		Makai::FLOW::Value val = Makai::JSON::parse(R"::(
			{
				"null_v": null,
				"bool_v": false,
				"int_v": -1,
				"uint_v": 1,
				"dq_str": "String with double quotes",
				"array_v": [1, 2, 4, 8, 16, 32],
				"object_v": {
					"key1": "value 1",
					"key2": {
						"subkey1": "value 1" ,
						"subkey2": "value 2"
					},
					"key3": "value 3"
				}
			}
		)::");
		DEBUGLN(val.toJSONString("  "));
	} catch (Makai::Error::Generic const& e) {
		Makai::Popup::showError(e.report());
	}
	return 0;
}
