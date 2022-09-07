#include "../hci.h"
#include "groups.h"
#include "objects_stats.h"


void GroupsForum::display(int xOffset, int yOffset)
{
	// add buttons here
	auto buttonHolder = std::make_shared<WIDGET>();
	objectsList->addWidgetToLayout(buttonHolder);


	// create a button and attach it
	auto groupButton = makeGroupButton();
	buttonHolder->attach(groupButton);
	groupButton->setGeometry(0, 0, OBJ_BUTWIDTH, OBJ_BUTHEIGHT);

	// draw the background
	BaseWidget::display(xOffset, yOffset);
}

void GroupsForum::initialize()
{
	setCalcLayout(LAMBDA_CALCLAYOUT_SIMPLE({
		psWidget->setGeometry(OBJ_BACKX, OBJ_BACKY - 250, OBJ_BACKWIDTH, OBJ_BACKHEIGHT);
	}));
}

std::shared_ptr<GroupButton> GroupsForum::makeGroupButton()
{
	return std::make_shared<GroupButton>();
}



