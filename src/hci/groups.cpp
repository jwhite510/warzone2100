#include "../hci.h"
#include "groups.h"
#include "objects_stats.h"

class GroupButton : public ObjectButton
{
private:
	typedef ObjectButton BaseWidget;

public:
	// BuildObjectButton(const std::shared_ptr<GroupController> &controller, size_t newObjectIndex)
	// 	: controller(controller)
	// {
	// 	objectIndex = newObjectIndex;
	// }
	void clickPrimary() override
	{

	}
protected:
	void display(int xOffset, int yOffset) override
	{

	}

	// GroupController &getController() const override
	// {
	// 	// TODO
	// 	// need to create a controller
	// 	// need to return a controller here
	// }

	std::string getTip() override
	{
		return "";
	}
};

void GroupsForum::display(int xOffset, int yOffset)
{
	// add buttons here
	auto buttonHolder = std::make_shared<WIDGET>();
	objectsList->addWidgetToLayout(buttonHolder);


	// create a button and attach it
	auto groupButton = makeGroupButton();
	// buttonHolder->attach(groupButton);
	// groupButton->setGeometry(0, 0, OBJ_BUTWIDTH, OBJ_BUTHEIGHT);

	// draw the background
	BaseWidget::display(xOffset, yOffset);
}

void GroupsForum::initialize()
{
	setCalcLayout(LAMBDA_CALCLAYOUT_SIMPLE({
		psWidget->setGeometry(OBJ_BACKX, OBJ_BACKY - 250, OBJ_BACKWIDTH, OBJ_BACKHEIGHT);
	}));
}

std::shared_ptr<ObjectButton> GroupsForum::makeGroupButton()
{
	// return std::make_shared<GroupButton>();
	return NULL;
}



