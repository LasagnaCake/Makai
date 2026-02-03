#ifndef MAKAILIB_NET_HTTP_H
#define MAKAILIB_NET_HTTP_H

#include "../compat/ctl.hpp"

namespace Makai::Net::HTTP {
	struct Response {
		enum class Status: uint {
			MN_HRS_CURL_ERROR	= 1,
			MN_HRS_HTTP_START	= 100,
			MN_HRS_OK			= 200,
		};
		Status	status	= Status::MN_HRS_OK;
		double	time;
		String	source;
		String	header;
		String	content;
	};

	struct Request {
		enum class Type {
			MN_HRT_GET,
			MN_HRT_HEAD,
			MN_HRT_POST,
			MN_HRT_PUT,
			MN_HRT_PATCH,
			MN_HRT_UPDATE,
			MN_HRT_DELETE,
		};
		Type	type;
		String	data;
		String	ssl;
	};

	Response fetch(String const& url, Request const& request);
}

#endif