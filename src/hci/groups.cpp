#include "../hci.h"
#include "groups.h"
#include "objects_stats.h"

static bool testv = true;


void GroupsForum::display(int xOffset, int yOffset)
{
	// add buttons here
	if (testv) {
		// attach a first button
		auto buttonHolder1 = std::make_shared<WIDGET>();
		objectsList->addWidgetToLayout(buttonHolder1);
		// // create a button and attach it
		auto groupButton = makeGroupButton((size_t)1);
		buttonHolder1->attach(groupButton);
		groupButton->setGeometry(0, 0, OBJ_BUTWIDTH, OBJ_BUTHEIGHT);


		auto buttonHolder2 = std::make_shared<WIDGET>();
		objectsList->addWidgetToLayout(buttonHolder2);
		// // attach a second button
		auto groupButton2 = makeGroupButton((size_t)2);
		buttonHolder2->attach(groupButton2);
		groupButton2->setGeometry(0, 0, OBJ_BUTWIDTH, OBJ_BUTHEIGHT);
	}
	testv = false;



	// draw the background
	BaseWidget::display(xOffset, yOffset);
}

void GroupsForum::initialize()
{
	setCalcLayout(LAMBDA_CALCLAYOUT_SIMPLE({
		psWidget->setGeometry(OBJ_BACKX, OBJ_BACKY - 250, OBJ_BACKWIDTH, OBJ_BACKHEIGHT);
	}));
	addTabList();
}

void GroupsForum::addTabList()
{
	// attach(objectsList = IntListTabWidget::make());
	// objectsList->id = IDOBJ_GROUP;
	// objectsList->setCalcLayout(LAMBDA_CALCLAYOUT_SIMPLE({
	// 	IntListTabWidget *pObjectsList = static_cast<IntListTabWidget *>(psWidget);
	// 	assert(pObjectsList != nullptr);
	// 	pObjectsList->setChildSize(OBJ_BUTWIDTH, OBJ_BUTHEIGHT * 2);
	// 	pObjectsList->setChildSpacing(OBJ_GAP, OBJ_GAP);
	// 	int objListWidth = OBJ_BUTWIDTH * 5 + STAT_GAP * 4;
	// 	pObjectsList->setGeometry((OBJ_BACKWIDTH - objListWidth) / 2, OBJ_TABY, objListWidth, OBJ_BACKHEIGHT - OBJ_TABY);
	// }));

	attach(objectsList = IntListTabWidget::make());
	objectsList->id = IDOBJ_GROUP;
	objectsList->setChildSize(STAT_BUTWIDTH, STAT_BUTHEIGHT);
	objectsList->setChildSpacing(STAT_GAP, STAT_GAP);
	int statListWidth = STAT_BUTWIDTH * 2 + STAT_GAP;
	objectsList->setGeometry((STAT_WIDTH - statListWidth) / 2, STAT_TABFORMY, statListWidth, STAT_HEIGHT - STAT_TABFORMY);
}

std::shared_ptr<GroupButton> GroupsForum::makeGroupButton(size_t groupNumber)
{
	return std::make_shared<GroupButton>(groupNumber);
}



