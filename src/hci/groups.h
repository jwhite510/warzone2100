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
	// BuildObjectButton(const std::shared_ptr<GroupController> &controller, size_t newObjectIndex)
	// 	: controller(controller)
	// {
	// 	objectIndex = newObjectIndex;
	// }
	void clickPrimary() override
	{
		// select the group
		kf_SelectGrouping(1);

	}
protected:
	void display(int xOffset, int yOffset) override
	{

		// get droid that is in the group
		DROID	*psDroid;
		for (psDroid = apsDroidLists[selectedPlayer]; psDroid != nullptr; psDroid = psDroid->psNext) {
			displayIMD(AtlasImage(), ImdObject::Droid(psDroid), xOffset, yOffset);
			break;
		}

		// find how the control group gets the droids
		// select a droid here so it's displayed
		// DROID* droidtest = 
		// displayIfHighlight(xOffset, yOffset);
		// displayBlank(xOffset, yOffset);
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
	std::shared_ptr<GroupButton> makeGroupButton();
	std::shared_ptr<IntListTabWidget> objectsList;
	void addTabList();
};

class GroupController
{
	// this is where the list of groups will be accessed
};

#endif // __INCLUDED_SRC_HCI_GROUPS_H__
