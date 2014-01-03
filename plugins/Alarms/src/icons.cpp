#include "common.h"
#include "icons.h"

HICON hIconMenuSet, hIconList1, hIconList2, hIconMenuShowHide, hIconSystray;

static IconItem iconList[] = 
{
	{ LPGEN("Menu: Set Alarm"),     "alarms_menu_set", IDI_MAINMENU },
	{ LPGEN("Reminder: Soon"),      "alarms_list1",    IDI_LIST1 },
	{ LPGEN("Reminder: Very Soon"), "alarms_list2",    IDI_LIST2 },
	{ LPGEN("Alarm: System Tray"),  "alarms_systray",  IDI_MAINMENU },
	{ LPGEN("Menu: Show/Hide Reminders"), "alarms_menu_showhide", IDI_MAINMENU }
};

int ReloadIcons(WPARAM wParam, LPARAM lParam)
{
	hIconMenuSet = Skin_GetIcon("alarms_menu_set");
	hIconList1 = Skin_GetIcon("alarms_list1");
	hIconList2 = Skin_GetIcon("alarms_list2");
	if ( !ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
		hIconMenuShowHide = Skin_GetIcon("alarms_menu_showhide");

	RefreshReminderFrame();
	return 0;
}

void InitIcons()
{
	Icon_Register(hInst, "Alarms", iconList, SIZEOF(iconList));

	if ( !ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
		hIconMenuShowHide = Skin_GetIcon("alarms_menu_showhide");

	ReloadIcons(0, 0);
	HookEvent(ME_SKIN2_ICONSCHANGED, ReloadIcons);
}