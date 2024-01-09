#include <jsonutils.h>
#include <iostream>
#include <exception>

int main() {


	json::jobjptr ptr=json::makeJsonObject();
	ptr->add("hello", "world");
	ptr->add("number", 5);
	ptr->add("bool", true);
	ptr->add("😁",json::makeJsonString("hello\nworld\\😁😂\n/a//b///c\\🍋Текущие события特色列表 سروص"));
	json::jarrptr arr=ptr->addJsonArray("my array");
	arr->add(json::makeJsonString("hi there"));
	auto x=arr->addJsonObject();
	arr->add(true);
	arr->add(55);
	auto y=arr->addJsonObject();
	y->add("true",true);
	y->add("false",false);
	y->add("explicit null",json::makeJsonNull());
	y->add("implicit null",json::jptr());


	std::cout<<json::Pretty(ptr)<<std::endl;
	std::string expected="{\"hello\":\"world\",\"number\":5,\"bool\":true,\"😁\":\"hello\\nworld\\\\😁😂\\n\\/a\\/\\/b\\/\\/\\/c\\\\🍋Текущие события特色列表 سروص\",\"my array\":[\"hi there\",{},true,55,{\"true\":true,\"false\":false,\"explicit null\":null,\"implicit null\":null}]}";
	if (expected!=ptr->toString()) throw std::runtime_error("not expected result");

	return 0;
}
