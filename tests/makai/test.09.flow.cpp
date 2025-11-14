#include <makai/makai.hpp>

int main() {
	DEBUGLN("Running app ", __FILE__, "...");
	try {
		Makai::String const str = R"::(
			{
				null_v null
				bool_v false
				int_v -1
				uint_v 1
				unq_str string_with_no_quotes
				sq_str 'String with single quotes'
				dq_str "String with double quotes"
				ml_str "
					Both string types
					can also be
					multi-line
				"
				byte_str !64"F0415EDCC99923CDD33124EEBBBAA9A4410000A"
				array_v [1 2 4 8 16 32]
				obj_array_v [{} {} {} {} {} {}]
				object_v {
					key1 "value 1"
					key2 {
						subkey1 "value 1" 
						'subkey2' "value 2"
					}
					"key3" "value 3"
				}
			}
		)::";
		Makai::FLOW::Value val = Makai::FLOW::parse(str);
		DEBUGLN("Has signed integer: ", val.contains("int_v") ? "true" : "false");
		DEBUGLN(val.toJSONString("  "));
		DEBUGLN(val.toFLOWString("  "));
	} catch (Makai::Error::Generic const& e) {
		Makai::Popup::showError(e.report());
	}
	return 0;
}
