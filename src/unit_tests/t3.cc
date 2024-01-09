#include <jsonutils.h>
#include <iostream>
#include <string>
#include <libgen.h>
#include <math.h>
#include <string.h>
#include <iostream>


int main() {

	json::jobjptr obj=json::makeJsonObject();
	obj->addJsonObject("z");
	obj->addJsonObject("b");
	obj->add(std::string("z"),123);
	obj->add("c",456);
	obj->addJsonArray("b")->addJsonObject()->add("aaaa",json::makeJsonArray());

	obj->add("ee🍋ee\"///😁", json::makeJsonNumber(123) );
	obj->add("uuuu", json::makeJsonNumber(123) );
	std::cout<<json::Pretty(obj)<<std::endl;
	obj->add(std::string("e🇷🇺🇺🇸eee"), json::makeJsonString("🇷🇺🇺🇸") );
	obj->add(std::string("zzzzz"), "😂😂😂😂😂😂😂😂😂");
	std::cout<<obj<<std::endl;

	json::jptr p=obj;
	json::JsonObjectIterator end;
	json::JsonObjectIterator zzz=end;
	end=zzz;

	json::JsonObjectIterator it(p);

	while (it) {
		auto pair=*it;
		std::cout<<"key: "<<pair.first<<" "<<json::Pretty(pair.second)<<std::endl;
		if (pair.second->getJsonType()==json::JsonType::JNULL)
			it.remove();
		else ++it;
	}
	for (auto pair : json::JsonObjectPairs(obj)) {
		std::cout<<"1. key: "<<pair.first<<" "<<json::jsonTypeString(pair.second->getJsonType())<<std::endl;
	}
	for (auto& key : json::JsonObjectKeys(obj)) {
			std::cout<<"2 key: "<<key<<std::endl;
	}

	obj->remove("uuuu");

	for (json::JsonObjectSortedIterator it(p); it; ++it) {
		auto pair=*it;
		std::cout<<"sorted key: "<<pair.first<<" "<<json::jsonTypeString(pair.second->getJsonType())<<std::endl;
	}

	for (auto pair : json::JsonObjectSortedPairs(obj)) {
		std::cout<<"again sorted key: "<<pair.first<<" "<<json::jsonTypeString(pair.second->getJsonType())<<std::endl;
	}
	for (auto& key : json::JsonObjectSortedKeys(obj)) {
		std::cout<<"yet again sorted key: "<<key<<std::endl;
	}

	auto r=obj->get("b")->getAsJsonArray();
	std::cout<<"arra "<<json::jsonTypeString(r->getJsonType())<<std::endl;
	r->add(true);
	r->add("hi there");
	r->addJsonObject()->add("true",true);
	r->add(42);
	r->addJsonObject()->addJsonArray("hi array");
	r->addJsonArray()->addNull();
	r->addJsonObject();
	std::cout<<json::Pretty(obj)<<std::endl;

	auto ii=json::JsonArrayIterator(r);

	while (ii) {
		const auto& e=*ii;
		std::cout<<"element "<<json::jsonTypeString(e->getJsonType())<<std::endl;
		if (e->getJsonType()==json::JsonType::JBOOLEAN)
			ii.remove();
		else ++ii;
	}
	for (auto e : json::JsonArrayElements(obj->get("b"))) {
		std::cout<<"last array "<<json::jsonTypeString(e->getJsonType())<<std::endl;
	}
	std::cout<<obj<<std::endl;
	std::cout<<json::Pretty(obj)<<std::endl;
	return 0;
}


