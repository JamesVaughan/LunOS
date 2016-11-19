#ifndef GUI_CONTAINER_H
#define GUI_CONTAINER_H
#include <GUI/GUI.hpp>
namespace LunOS
{
namespace GUI
{
	class Container : public Component
	{
		// Attempt to add a child component to this component
		virtual bool AddChild(Component* component) = 0;
		// Attempt to remove a child component from this component
		virtual bool RemoveChild(Component* component) = 0;

		~Container();
	};
}
}

#endif
