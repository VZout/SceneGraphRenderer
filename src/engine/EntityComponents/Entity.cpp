#include "Entity.h"

using namespace rle;

Entity::Entity()
	: enabled(true), name("")
{ }

Entity::~Entity()
{ }

bool Entity::get_enabled()
{
	return enabled;
}

std::string Entity::get_name()
{
	return name;
}

void Entity::set_enabled(bool enabled)
{
	this->enabled = enabled;
}

void Entity::set_name(std::string name)
{
	this->name = name;
}