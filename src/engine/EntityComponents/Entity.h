#pragma once
#include <string>

namespace rle
{
	class Entity
	{
	public:
		Entity();
		~Entity();

		bool get_enabled();
		std::string get_name();

		void set_enabled(bool enabled);
		void set_name(std::string name);

	protected:
		bool enabled;
		std::string name;
	};
}

