#include <ctl/ctl.hpp>
#include <ctlex/ctlex.hpp>

using
	CTL::String,
	CTL::BaseListMap,
	CTL::OrderedMap,
	CTL::ListMap,
	CTL::List,
	CTL::KeyValuePair,
	CTL::TypeInfo
;

using namespace CTL::Literals::Text;

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
	for (auto const& i: m) {
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
	for (auto const& i : lst)
		DEBUG("\"", i ,"\" ");
	DEBUGLN(
		"];"
	);
}

template<typename I, bool S>
void printMap(BaseListMap<String, String, I, S> const& m) {
	DEBUG(
		"S: ",
		m.size(),
		", C: ",
		m.capacity(),
		", I: [ "
	);
	for (auto const& i: m) {
		DEBUG("[", i.key, " ", i.value, "] ");
	}
	DEBUGLN("];");
}

template<typename I, bool S>
void printMap(BaseListMap<String, String, I, S> const& m, String const& k) {
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
	str = "Impedance. Voltage. Current."s;
	print(str);
	auto sp = str.splitAtFirst(' ');	print(sp);
	sp = str.splitAtLast(' ');			print(sp);
	sp = str.split(' ');				print(sp);
	sp.sort();							print(sp);
}

template<template<typename K, typename V> class TMap = ListMap>
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
	// ERROR: `OrderedMap` causes `AllocationFailure` here
	printMap(mp, "Avocado");
	mp.insert({
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

void testStringConversion() {
	DEBUGLN("N2S:\t", String::fromNumber<int>(465));
	DEBUGLN("N2S:\t", String::fromNumber<int>(-465));
	DEBUGLN("N2S:\t", String::fromNumber<float>(46.15));
	DEBUGLN("N2S:\t", String::fromNumber<float>(-46.15));
	DEBUGLN("S2N:\t", String::toNumber<int>("465"));
	DEBUGLN("S2N:\t", String::toNumber<int>("-465"));
	DEBUGLN("S2N:\t", String::toNumber<float>("46.15"));
	DEBUGLN("S2N:\t", String::toNumber<float>("-46.15"));
	DEBUGLN("S2N2S:\t", String::fromNumber<int>(String::toNumber<int>("465")));
	DEBUGLN("S2N2S:\t", String::fromNumber<int>(String::toNumber<int>("-465")));
	DEBUGLN("S2N2S:\t", String::fromNumber<float>(String::toNumber<float>("46.15")));
	DEBUGLN("S2N2S:\t", String::fromNumber<float>(String::toNumber<float>("-46.15")));
}

int main() {
	testString();
	testStringMap<ListMap>();
	testStringMap<OrderedMap>();
	testStringConversion();
	DEBUGLN("String test passed!");
	return 0;
}
