#include "file.hpp"
#include "http.hpp"
#include <curl/curl.h>

Makai::String Makai::Net::File::fetchText(Makai::String const& url) {
	auto const result = HTTP::fetch(url, {.type = HTTP::Request::Type::MN_HRT_GET});
	if (
		result.status != HTTP::Response::Status::MN_HRS_OK
	&&	result.status != HTTP::Response::Status::MN_HRS_HTTP_OK
	) throw Makai::Error::FailedAction(
		"Failed to fetch file from '"+url+"'!",
		result.header + "\n" + result.content,
		CTL_CPP_PRETTY_SOURCE
	);
	return result.content;
}