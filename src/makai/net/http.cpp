#include "http.hpp"
#include <curl/curl.h>

using namespace Makai::Net::HTTP;

namespace Wrap {
	static usize httpWrite(cstring const src, usize const size, usize const count, ref<Makai::String> const dst) {
		dst->appendBack(src, src + count * size);
		return count * size;
	}

	static usize httpRead(ref<char> const dst, usize const size, usize const count, ref<Makai::String> const src) {
		if (src->empty()) return 0;
		auto const sz = count * size;
		auto const trueSize = sz < src->size() ? sz : src->size();
		if (!trueSize) return 0;
		Makai::MX::memcpy(dst, src->data(), trueSize);
		*src = src->substring(trueSize);
		return trueSize;
	}
}

// Based off of https://gist.github.com/whoshuu/2dc858b8730079602044
Response Makai::Net::HTTP::fetch(Makai::String const& url, Request const& request) {
	auto const curl = curl_easy_init();
	if (!curl) throw Makai::Error::FailedAction("Failed to initialize cURL!", CTL_CPP_PRETTY_SOURCE);
	String err = String(CURL_ERROR_SIZE, '\0');
	String reqdat;
	if (request.data.size()) reqdat = request.data;
	Response resp;
	curl_easy_setopt(curl, CURLOPT_URL, url.cstr());
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, err.cstr());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Wrap::httpWrite);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, Wrap::httpRead);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp.content);
	curl_easy_setopt(curl, CURLOPT_READDATA, &reqdat);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &resp.header);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.42.0");
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
	curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
	switch (request.type) {
		case Request::Type::MN_HRT_GET: {
			curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
		} break;
		case Request::Type::MN_HRT_HEAD: {
			curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
		} break;
		case Request::Type::MN_HRT_POST: {
			curl_easy_setopt(curl, CURLOPT_MIMEPOST, 1);
		} break;
		case Request::Type::MN_HRT_PUT: {
			curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
		} break;
		case Request::Type::MN_HRT_PATCH: {
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
		} break;
		case Request::Type::MN_HRT_UPDATE: {
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "UPDATE");
		} break;
		case Request::Type::MN_HRT_DELETE: {
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
		} break;
	}
	if (request.ssl.size())
		curl_easy_setopt(curl, CURLOPT_CAINFO, request.ssl.cstr());
	else curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	auto const result = curl_easy_perform(curl);
	if (result != CURLE_OK) {
		resp.status = Response::Status::MN_HRS_CURL_ERROR;
		err = err.sliced(0, err.find('\0'));
		resp.content	= err;
		resp.header		= curl_easy_strerror(result);
	}
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp.status);
	curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &resp.time);
	cstring effectiveURL = NULL;
	curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &effectiveURL);
	resp.source = effectiveURL;
	curl_easy_cleanup(curl);
	return resp;
}
