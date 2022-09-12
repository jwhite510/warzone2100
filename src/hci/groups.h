#ifndef __INCLUDED_SRC_HCI_GROUPS_H__
#define __INCLUDED_SRC_HCI_GROUPS_H__

#include "../intdisplay.h"
#include "objects_stats.h"

#include "../objmem.h"
#include "../input/keyconfig.h"
#include "../keybind.h"
#include "lib/widget/label.h"

class GroupButton : public DynamicIntFancyButton
{
private:
	typedef DynamicIntFancyButton BaseWidget;
	std::shared_ptr<W_LABEL> groupNumberLabel;
	std::shared_ptr<W_LABEL> groupCountLabel;
public:
	size_t groupNumber;
	static std::shared_ptr<GroupButton> make(size_t groupNumber)
	{
		class make_shared_enabler: public GroupButton {};
		auto widget = std::make_shared<make_shared_enabler>();
		widget->groupNumber = groupNumber;
		widget->initialize();
		return widget;
	}
	void initialize()
	{
		attach(groupNumberLabel = std::make_shared<W_LABEL>());
		groupNumberLabel->setGeometry(OBJ_TEXTX, OBJ_B1TEXTY - 5, 16, 16);
		groupNumberLabel->setString(WzString::fromUtf8(astringf("%u", groupNumber)));

		attach(groupCountLabel = std::make_shared<W_LABEL>());
		groupCountLabel->setGeometry(OBJ_TEXTX + 40, OBJ_B1TEXTY + 20, 16, 16);
		groupCountLabel->setString("");
	}
	GroupButton() { }

	void clickPrimary() override
	{
		// select the group
		kf_SelectGrouping(groupNumber);

	}
	void clickSecondary() override
	{
		assignDroidsToGroup(selectedPlayer, groupNumber, true);

	}
protected:
	void display(int xOffset, int yOffset) override
	{
		DROID	*psDroid, *displayDroid = NULL;
		int numberInGroup = 0;

		for (psDroid = apsDroidLists[selectedPlayer]; psDroid != nullptr; psDroid = psDroid->psNext) {
			// display whatever group has the most
			if (psDroid->group == groupNumber) {
				numberInGroup++;
				if (!displayDroid) {
					displayDroid = psDroid;
				}
			}
		}
		if (!numberInGroup) {
			groupCountLabel->setString("");
			displayBlank(xOffset, yOffset);
		} else {
			displayIMD(AtlasImage(), ImdObject::Droid(displayDroid), xOffset, yOffset);
			groupCountLabel->setString(WzString::fromUtf8(astringf("%u", numberInGroup)));
		}
	}
	std::string getTip() override
	{
		return "";
	}
	bool isHighlighted() const override
	{
		return false;
	}
};
// class GroupController: public BaseObjectsController
// {
// 
// }

class GroupsForum: public IntFormAnimated
{
private:
	typedef IntFormAnimated BaseWidget;
	bool createdGroupButtons = false;
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
	std::shared_ptr<GroupButton> makeGroupButton(size_t groupNumber);
	std::shared_ptr<IntListTabWidget> objectsList;
	void addTabList();
};

class GroupController
{
	// this is where the list of groups will be accessed
};

#endif // __INCLUDED_SRC_HCI_GROUPS_H__
