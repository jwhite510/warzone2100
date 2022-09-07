#include "../intdisplay.h"
class GroupsForum: public IntFormAnimated
{
private:
	typedef IntFormAnimated BaseWidget;
public:
	void display(int xOffset, int yOffset);
	void initialize();
	static std::shared_ptr<GroupsForum> make()
	{
		class make_shared_enabler: public GroupsForum {};
		auto widget = std::make_shared<make_shared_enabler>();
		// widget->controller = controller;
		widget->initialize();
		return widget;
	}
};

class GroupController
{
	// this is where the list of groups will be accessed
};
