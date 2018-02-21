#pragma once
#include <string>

class ComponentMetadata {
public:
	ComponentMetadata(std::string name) : name(name) {}

	std::string get_name() { return name; }
	
private:
	std::string name;
};