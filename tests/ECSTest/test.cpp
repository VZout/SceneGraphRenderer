#include <assert.h>

#include <engine/EntityComponents/GameObject.h>

class TestComponent
	: public rle::Component
{
public:
	TestComponent() : Component() {}
};

class ParamTestComponent
	: public rle::Component
{
public:
	ParamTestComponent(int x, std::string y) : Component() {}
};

int main()
{
	rle::GameObject *go = new rle::GameObject();

	assert(go != nullptr);

	auto component_one = go->AddComponent<TestComponent>();

	assert(component_one != nullptr);
	assert(go->GetComponents().size() == 1);
	assert(go->get_name() == "");

	component_one->set_name("Test");

	assert(component_one->get_name() == "Test");

	auto component_two = go->AddComponent<ParamTestComponent>(12, "hello world");

	assert(component_two != nullptr);
	assert(go->GetComponents().size() == 2);

	auto required_component = go->RequireComponent<ParamTestComponent>(12, "hello world");

	assert(component_two == required_component);
	assert(go->GetComponents().size() == 2);

	auto get_component_one = go->GetComponent<TestComponent>();
	
	assert(get_component_one != nullptr);

	get_component_one->set_name("New Name");

	assert(get_component_one->get_name() == "New Name");
	assert(component_one->get_name() == "New Name");

	return 0;
}