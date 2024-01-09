#include <jsonutils_embeded_config.h>
#include <iostream>
#include <string>
#include <libgen.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>
#include <chrono>
#include <thread>


namespace json {
	bool testConsumeOpenedString(std::string& str, const char* s);
}

int main() {
	std::string out;
	auto ret=json::testConsumeOpenedString(out, "hello\\t\\u0429\\\"\\uD834\\uDD1E\\\"\"");
	if (out!="hello\tЩ\"𝄞\"") throw std::runtime_error("Expected hello\tЩ\"𝄞\"");
	std::cout<<ret<<" "<<out<<std::endl;

	std::string str="{\"z\":123,\"b\":[{\"aaaa\":[null,true,false,{\"\":\"\",\"a\":[[[true],false]]}]},\"hi there\",{\"true\":true},42,{\"hi array\":[]},[null],{}],\"c\":456,\"ee🍋ee\\\"\\/\\/\\/😁\":123,\"e🇦🇨🇷🇺🇺🇸eee\":\"🇦🇨🇷🇺🇺🇸\",\"zzzzz\":\"😂😂😂😂😂😂😂😂😂\"}";
	try {
	auto obj=json::parseArrayOrObject(str);
	} catch (const std::exception& e) {
		std::cerr<<e.what()<<std::endl;
	}


	std::cout<<"App embedded conf:  "<<json::Pretty(json::readEmbededConfig())<<std::endl;

	char* wd=::getcwd(NULL,0);
	std::cout<<"Assuming, im in: "<<wd<<out<<std::endl;
	::free(wd);


	/*
	const char *homedir;
	if ((homedir = getenv("HOME")) == NULL) {
	    homedir = getpwuid(getuid())->pw_dir;
	}
	std::cout<<"homedir ["<<homedir<<"]"<<out<<std::endl;
	 */

	auto start=std::chrono::system_clock::now();
	size_t consumed;
	auto largeObj=json::parseFromFile("./src/adhoc_tests/large-file.json", 0, &consumed);
	auto end=std::chrono::system_clock::now();
	std::cout<<"parsed "<<consumed<<" bytes of json in "<< (std::chrono::duration_cast<std::chrono::milliseconds>(end - start)).count() << " ms."<<std::endl;;
	//std::cout<<"large: "<<json::Pretty(largeObj)<<std::endl;


	return 0;
}


