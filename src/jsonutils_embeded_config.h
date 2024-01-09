#ifndef JSONUTILS_EC_H_
#define JSONUTILS_EC_H_

#include <jsonutils.h>

extern "C" {
	extern char _binary_conf_json_start;
	extern char _binary_conf_json_end;
}

namespace json {
	inline json::jptr readEmbededConfig() {
		return json::parseArrayOrObject(&_binary_conf_json_start, &_binary_conf_json_end);
	}
}
#endif

