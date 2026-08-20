#include "engine.hpp"

#define PCRE2_STATIC
#define PCRE2_CODE_UNIT_WIDTH 0

#include <pcre2.h>

using namespace Makai;
using namespace Makai::Regex;

struct Engine::Impl {
	~Impl() {
		pcre2_code_free_32(code);
	}

	pcre2_code_32*	code;
};

template <usize F>
static uint32 pcreFlag(bool const isSet) {
	return isSet ? F : 0;
}

Engine::Engine(String const& str, Flags const& flags): Engine(UTF32String(str), flags) {}

Engine::Engine(UTF8String const& str, Flags const& flags): Engine(UTF32String(str), flags) {}

Engine::Engine(UTF32String const& str, Flags const& flags) {
	impl = impl.create();
	int errNum;
	PCRE2_SIZE errOff;
	impl->code = pcre2_compile_32(
		(PCRE2_SPTR32)str.data(),
		str.size(),
		(
			pcreFlag<PCRE2_CASELESS>(flags.ignoreCase)
		|	PCRE2_ALLOW_EMPTY_CLASS
		|	PCRE2_ALT_BSUX
		|	pcreFlag<PCRE2_DOTALL>(flags.lineByLine)
		|	pcreFlag<PCRE2_MULTILINE>(!flags.lineByLine)
		|	PCRE2_EXTENDED
		|	PCRE2_EXTENDED_MORE
		|	PCRE2_UTF
		),
		&errNum,
		&errOff,
		NULL
	);
	if (!impl->code) {
		char ebuf[1024];
		pcre2_get_error_message_8(errNum, (ref<PCRE2_UCHAR8>)ebuf, 1024);
		throw Error::FailedAction(
			"Failed to compile regex [\"" + str + "\"]!",
			String(ebuf),
			CTL_CPP_PRETTY_SOURCE
		);
	}
}

template <class T>
static Engine::Match<T> unwrap(Engine::Match<UTF32String> const& match) {
	return {match.position, match.match};
}

Engine::Matches<UTF8String> Engine::matchIn(UTF8String const& str, bool const fullMatch) const {
	return matchIn(UTF32String(str), fullMatch).toList<Match<UTF8String>>(unwrap<UTF8String>);
}

Engine::Matches<String> Engine::matchIn(String const& str, bool const fullMatch) const {
	return matchIn(UTF32String(str), fullMatch).toList<Match<String>>(unwrap<String>);
}

Engine::Matches<UTF32String> Engine::matchIn(UTF32String const& str, bool const fullMatch) const {
	int vectors[64];
  	auto const match = pcre2_match_data_create_32(2, NULL);
	auto status = pcre2_dfa_match_32(
		impl->code,
		(PCRE2_SPTR32)str.data(),
		str.size(),
		0,
		(
			pcreFlag<PCRE_ANCHORED>(fullMatch)
		|	pcreFlag<PCRE_ENDANCHORED>(fullMatch)
		),
		match,
		NULL,
		vectors,
		64
	);
	if (status <= 0) {
		pcre2_match_data_free_32(match);
		return {};
	}
	auto const offset = pcre2_get_ovector_pointer_32(match);
	Engine::Matches<UTF32String> result;
	for (usize i = 0; i < status; i += 2) {
		usize const start = offset[i];
		usize const end = offset[i+1] - offset[i];
		result.pushBack({
			start,
			str.sliced(start, end)
		});
	}
	pcre2_match_data_free_32(match);
	return result;
}

String Engine::replaceIn(String const& str, String const& rep) const {
	return replace(UTF32String(str), UTF32String(rep));
}

UTF8String Engine::replaceIn(UTF8String const& str, UTF8String const& rep) const {
	return replace(UTF32String(str), UTF32String(rep));
}

UTF32String Engine::replaceIn(UTF32String const& str, UTF32String const& rep) const {
	auto const match = pcre2_match_data_create_from_pattern_32(impl->code, NULL);
	UTF32String out;
	usize sz = 0;
	auto total = pcre2_substitute_32(
		impl->code,
		(PCRE2_SPTR32)str.data(),
		str.size(),
		0,
		PCRE2_SUBSTITUTE_GLOBAL,
		NULL,
		NULL,
		(PCRE2_SPTR32)rep.data(),
		rep.size(),
		(ref<PCRE2_UCHAR32>)out.data(),
		&sz
	);
	out.resize(sz);
	total = pcre2_substitute_32(
		impl->code,
		(PCRE2_SPTR32)str.data(),
		str.size(),
		0,
		PCRE2_SUBSTITUTE_GLOBAL,
		NULL,
		NULL,
		(PCRE2_SPTR32)rep.data(),
		rep.size(),
		(ref<PCRE2_UCHAR32>)out.data(),
		&sz
	);
	return out;
}
