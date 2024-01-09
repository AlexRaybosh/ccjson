#include <jsonutils.h>
#include <iostream>
#include <string>
#include <libgen.h>
#include <math.h>
#include <string.h>
#include <iostream>


int main() {
	std::vector<std::string> errs;
	try {
		json::parseArrayOrObject("{\"a\":[]");
		errs.emplace_back("unterminated object passed");
	} catch (std::exception& e) {
		std::string w=e.what();
		std::cout<<"correct error: "<<w<<std::endl;
	}
	try {
		json::parseArrayOrObject("\"a\":[]}");
		errs.emplace_back("no object start");
	} catch (std::exception& e) {
		std::string w=e.what();
		std::cout<<"correct error: "<<w<<std::endl;
	}
	try {
		auto obj=json::parseArrayOrObject("{\"a\":[],}");
		errs.emplace_back("comma after last object element");
		std::cerr<<json::Pretty(obj)<<std::endl;
	} catch (std::exception& e) {
		std::string w=e.what();
		std::cout<<"correct error: "<<w<<std::endl;
	}

	try {
		auto obj=json::parseArrayOrObject("{\"a\":[],3}");
		errs.emplace_back("comma after last object element");
		std::cerr<<json::Pretty(obj)<<std::endl;
	} catch (std::exception& e) {
		std::string w=e.what();
		std::cout<<"correct error: "<<w<<std::endl;
	}
	try {
		auto obj=json::parseArrayOrObject("{\"a\":[],\"b\":}");
		errs.emplace_back("last object element value is missing");
		std::cerr<<json::Pretty(obj)<<std::endl;
	} catch (std::exception& e) {
		std::string w=e.what();
		std::cout<<"correct error: "<<w<<std::endl;
	}
	try {
		auto obj=json::parseArrayOrObject("[\"a\",]");
		errs.emplace_back("comma after last array element");
		std::cerr<<json::Pretty(obj)<<std::endl;
	} catch (std::exception& e) {
		std::string w=e.what();
		std::cout<<"correct error: "<<w<<std::endl;
	}
	try {
		auto obj=json::parseArrayOrObject("[\"a\",z]");
		errs.emplace_back("invalid last array element");
		std::cerr<<json::Pretty(obj)<<std::endl;
	} catch (std::exception& e) {
		std::string w=e.what();
		std::cout<<"correct error: "<<w<<std::endl;
	}
	std::string error;
	for (auto& e : errs) {
		error.append(e).append("\n");
	}
	if (!error.empty()) throw std::runtime_error(error);


	return 0;
}


