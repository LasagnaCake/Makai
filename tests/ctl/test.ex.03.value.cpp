#include <ctl/ctl.hpp>
#include <ctlex/ctlex.hpp>

using namespace CTL::Ex::Data;


int main() {
	DEBUGLN("Empty value...");
	Value val;
	DEBUGLN("Null...");
	val["null"] = nullptr;
	DEBUGLN("Boolean...");
	val["bool"] = true;
	DEBUGLN("Unsigned integer...");
	val["uint"] = 1u;
	DEBUGLN("Signed integer...");
	val["int"] = 1;
	DEBUGLN("String...");
	val["str"] = "This is a string literal";
	DEBUGLN("Array...");
	val["array"] = Value::ArrayType{1, 2, 4, 8, 16, 32};
	DEBUGLN("Object...");
	val["object"] = Value::ObjectType{
		Value::ObjectType::PairType{"key1", "val1"},
		Value::ObjectType::PairType{"key2", Value::ObjectType{Value::ObjectType::PairType{"subkey2", "val2"}}},
		Value::ObjectType::PairType{"key3", "val3"}
	};
	DEBUGLN("Is object: ", val.isObject());
	DEBUGLN("Result:\n");
	DEBUGLN("<json>", val.toJSONString(Value::StringType{"  "}), "</json>\n");
	DEBUGLN("<flow>", val.toFLOWString(Value::StringType{"  "}), "</flow>\n");
	DEBUGLN("Clearing...\n");
	val.clear();
	DEBUGLN("Test passed!");
	return 0;
}