#ifndef __INCLUDED_SRC_HCI_GROUPS_H__
#define __INCLUDED_SRC_HCI_GROUPS_H__

#include "../intdisplay.h"
#include "objects_stats.h"

#include "../objmem.h"
#include "../input/keyconfig.h"
#include "../keybind.h"

class GroupButton : public DynamicIntFancyButton
{
private:
	typedef DynamicIntFancyButton BaseWidget;
public:
	size_t groupNumber;
	GroupButton(size_t groupNumber)
		: groupNumber(groupNumber) { }

	void clickPrimary() override
	{
		// select the group
		kf_SelectGrouping(groupNumber);

	}
protected:
	void display(int xOffset, int yOffset) override
	{

		// get droid that is in the group
		DROID	*psDroid;
		bool foundGroup = false;
		for (psDroid = apsDroidLists[selectedPlayer]; psDroid != nullptr; psDroid = psDroid->psNext) {
			// display whatever group has the most
			if (psDroid->group == groupNumber) {
				foundGroup = true;
				displayIMD(AtlasImage(), ImdObject::Droid(psDroid), xOffset, yOffset);
				break;
			}
			// TODO display the number of units in the group
		}
		if (!foundGroup) {
			displayBlank(xOffset, yOffset);
		}
		// find how the control group gets the droids
		// select a droid here so it's displayed
		// DROID* droidtest = 
		// displayIfHighlight(xOffset, yOffset);
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
