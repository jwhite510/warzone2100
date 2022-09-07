#include "../hci.h"
#include "groups.h"
void GroupsForum::display(int xOffset, int yOffset)
{
	// draw the background
	BaseWidget::display(xOffset, yOffset);
}

void GroupsForum::initialize()
{
	setCalcLayout(LAMBDA_CALCLAYOUT_SIMPLE({
		psWidget->setGeometry(OBJ_BACKX, OBJ_BACKY - 150, OBJ_BACKWIDTH, OBJ_BACKHEIGHT);
	}));
}

