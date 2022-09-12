#include "../hci.h"
#include "groups.h"
#include "objects_stats.h"



void GroupsForum::display(int xOffset, int yOffset)
{
	// add buttons here
	if (!createdGroupButtons) {
		// create the 11 buttons for each group
		for (size_t i = 1; i <= 10; i++) {
			// check if the 10th group works
			auto buttonHolder = std::make_shared<WIDGET>();
			objectsList->addWidgetToLayout(buttonHolder);
			auto groupButton = makeGroupButton(i % 10);
			buttonHolder->attach(groupButton);
			groupButton->setGeometry(0, 0, OBJ_BUTWIDTH, OBJ_BUTHEIGHT);
		}
	}
	createdGroupButtons = true;
	// draw the background
	BaseWidget::display(xOffset, yOffset);
}

void GroupsForum::initialize()
{
	setCalcLayout(LAMBDA_CALCLAYOUT_SIMPLE({
		psWidget->setGeometry(OBJ_BACKX, OBJ_BACKY - 80, OBJ_BACKWIDTH, OBJ_BACKHEIGHT - 40);
	}));
	addTabList();
}

void GroupsForum::addTabList()
{
	attach(objectsList = IntListTabWidget::make());
	objectsList->id = IDOBJ_GROUP;
	objectsList->setCalcLayout(LAMBDA_CALCLAYOUT_SIMPLE({
		IntListTabWidget *pObjectsList = static_cast<IntListTabWidget *>(psWidget);
		assert(pObjectsList != nullptr);
		pObjectsList->setChildSize(OBJ_BUTWIDTH, OBJ_BUTHEIGHT * 2);
		pObjectsList->setChildSpacing(OBJ_GAP, OBJ_GAP);
		int objListWidth = OBJ_BUTWIDTH * 5 + STAT_GAP * 4;
		pObjectsList->setGeometry((OBJ_BACKWIDTH - objListWidth) / 2, OBJ_TABY, objListWidth, OBJ_BACKHEIGHT - OBJ_TABY);
	}));

	// attach(objectsList = IntListTabWidget::make());
	// objectsList->id = IDOBJ_GROUP;
	// objectsList->setChildSize(OBJ_BUTWIDTH, OBJ_BUTHEIGHT);
	// objectsList->setChildSpacing(OBJ_GAP, OBJ_GAP);
	// int statListWidth = OBJ_BUTWIDTH * 5 + STAT_GAP * 4;
	// objectsList->setGeometry((OBJ_BACKWIDTH - objListWidth) / 2, OBJ_TABY, objListWidth, OBJ_BACKHEIGHT - OBJ_TABY);
}

std::shared_ptr<GroupButton> GroupsForum::makeGroupButton(size_t groupNumber)
{
	return GroupButton::make(groupNumber);
	// return NULL;
	// return std::make_shared<GroupButton>(groupNumber);
}



