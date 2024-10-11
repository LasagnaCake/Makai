#include <ctl/ctl.hpp>
#include <ctlex/ctlex.hpp>

using
	CTL::String,
	CTL::BaseSimpleMap,
	CTL::OrderedMap,
	CTL::SimpleMap,
	CTL::List,
	CTL::KeyValuePair,
	CTL::TypeInfo
;

void print(String const& str) {
	DEBUGLN(
		"S: ",
		str.size(),
		", C: ",
		str.capacity(),
		", S: [ \"",
		str,
		"\" ];"
	);
}

template<typename K, typename V>
void print(List<KeyValuePair<K, V>> const& m) {
	DEBUG(
		"S: ",
		m.size(),
		", C: ",
		m.capacity(),
		", P: [ "
	);
	for (auto& i: m) {
		DEBUG("[", i.key, " ", i.value, "] ");
	}
	DEBUGLN("];");
}

template<typename T>
void print(List<T> const& lst) {
	DEBUG(
		"S: ",
		lst.size(),
		", C: ",
		lst.capacity(),
		", L: [ "
	);
	for (auto& i : lst)
		DEBUG("\"", i ,"\" ");
	DEBUGLN(
		"];"
	);
}

template<typename I, bool S>
void printMap(BaseSimpleMap<String, String, I, S> const& m) {
	DEBUG(
		"S: ",
		m.size(),
		", C: ",
		m.capacity(),
		", I: [ "
	);
	for (auto& i: m) {
		DEBUG("[", i.key, " ", i.value, "] ");
	}
	DEBUGLN("];");
}

template<typename I, bool S>
void printMap(BaseSimpleMap<String, String, I, S> const& m, String const& k) {
	auto r = m.search(k);
	if (r != -1)
		CTL::Console::println("K: ", k, ", L: ", r, ", V: ", m[k]);
	else
		CTL::Console::println("K: ", k, ", L: ", r);
}

void testString() {
	String str;							print(str);
	str = "Henlo.";						print(str);
	str += " You?";						print(str);
	str = "O! " + str;					print(str);
	print(str.sliced(2, -3));
	str.sort();							print(str);
	str = String("Impedance. Voltage. Current.");
	print(str);
	auto sp = str.splitAtFirst(' ');	print(sp);
	sp = str.splitAtLast(' ');			print(sp);
	sp = str.split(' ');				print(sp);
	sp.sort();							print(sp);
}

template<template<typename K, typename V> class TMap = SimpleMap>
void testStringMap() {
	using MapType = TMap<String, String>;
	DEBUGLN("<", TypeInfo<MapType>::name(), ">");
	MapType mp = MapType({
		{"Avocado", "Abacate"},
//		{"Apple", "Maca"},
		{"Orange", "Laranja"},
		{"Mango", "Manga"},
		{"Tangerine", "Tangerina"},
//		{"Grape", "Uva"},
		{"Papaya", "Mamao"},
	}); printMap(mp);
	mp["Kiwi"] = "Kiwi";	printMap(mp);
	printMap(mp, "Orange");
	printMap(mp, "Papaya");
	printMap(mp, "Grape");
	printMap(mp, "Pineapple");
	printMap(mp, "Avocado");
	mp.append({
		{"Kiwi", "Kiwi"},
		{"Banana", "Alguma-fruta"},
		{"Banana", "Sei-la"},
		{"Banana", "Banana"},
		{"Tangerine", "Mexerica"},
		{"Pineapple", "Abacaxi"},
	});
	printMap(mp);
	print(mp.keys());
	print(mp.values());
	print(mp.items());
	mp.clear();		printMap(mp);
	mp.dispose();	printMap(mp);
	DEBUGLN("</", TypeInfo<MapType>::name(), ">");
}

int main() {
	testString();
	testStringMap<SimpleMap>();
	testStringMap<OrderedMap>();
	DEBUGLN("String test passed!");
	return 0;
}
