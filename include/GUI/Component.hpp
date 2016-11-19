#ifndef GUI_COMPONENT_H
#define GUI_COMPONENT_H
/*
 * This class defines the base class for all components in the GUI system for LunOS
 */
namespace LunOS
{
namespace GUI
{

class Component
{
public:
	// Cause the component to re-render itself
	void Invalidate();
	// Destroy this component and any child components and notifies its parent
	void Dispose();
protected:
	virtual void Render() = 0;
	int X;
	int Y;
	int Width;
	int Height;
	Component* Parent;

	~Component();
};

}
}

#endif
