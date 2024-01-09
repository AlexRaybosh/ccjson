#include <jsonutils.h>
#include <iostream>
#include <string>
#include <exception>
#include <libgen.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>

namespace json {
	bool testConsumeOpenedString(std::string& str, const char* s);
}

int main() {
	std::string out;
	auto ret=json::testConsumeOpenedString(out, "hello\\t\\u0429\\\"\\uD834\\uDD1E\\\"\"");
	if (out!="hello\tЩ\"𝄞\"") throw std::runtime_error("Expected hello\tЩ\"𝄞\"");
	std::cout<<ret<<" "<<out<<std::endl;

	std::string str="{\"z\":123,\"b\":{"
			"\"v\":{\"empt1\":\"yes\",\"nn\":-1E-5,\"bt\":true,\"aa\\naa\":[null,true,false,{\"\":\"\\n\",\"a\":[[[true],false]]}]},\"hi there\":{\"true\":true},\"42\":42,"
			"\"arrays\":{\"hi array\":[]},\"na\":[null],\"emp\":{}"
			"},"
			"\"c\":456,\"ee🍋ee\\\"\\/\\/\\/😁\":123,\"e🇦🇨🇷🇺🇺🇸eee\":\"🇦🇨🇷🇺🇺🇸\",\"zzzzz\":\"😂😂😂😂😂😂😂😂😂\"}";
	try {
		auto obj=json::parseArrayOrObject(str);
		std::cout<<"Parsed: "<<json::Pretty(obj)<<std::endl;
		auto str1=obj->toString();
		std::cout<<"Parsed toString: "<<obj->toString()<<std::endl;
		if (str!=str1) throw std::runtime_error(std::string("not the same"));

		std::cout<<"Extracted: "<<*json::getStringPtr(obj, "b", "v", "empt1")<<std::endl;
		std::cout<<"Extracted: "<<json::getLong(obj, "b", "v", "nn").value()<<std::endl;
		std::cout<<"Extracted: "<<json::getDouble(obj, "b", "v", "nn").value()<<std::endl;
		std::cout<<"Extracted: "<<json::getBoolean(obj, "b", "v", "bt").value()<<std::endl;
		std::cout<<"Extracted: "<<json::Pretty(json::getJsonElement(obj, "b", "v", "aa\naa"))<<std::endl;
	} catch (const std::exception& e) {
		std::cerr<<e.what()<<std::endl;
		return 1;
	}

	return 0;
}


