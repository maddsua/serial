#ifndef _APP_RESOURCE_CODES
#define _APP_RESOURCE_CODES


	#define APP_MAIN_MENU_ID		"MAINMENU"

	//	gui
	#define GUI_STATUSBAR			(10)
	#define GUI_TERMINAL			(11)
	#define GUI_COMPROM				(12)
	#define GUI_DROP_PORT			(13)
	#define GUI_DROP_SPEED			(14)
	#define GUI_DROP_LINE			(15)
	#define GUI_BUTTON_SEND			(16)
	#define GUI_CHECK_HEXMODE		(102)

	//	context menus
	#define MENUITEM_FILE_SVLOG		(2001)
	#define MENUITEM_FILE_EXIT		(2002)

	#define MENUITEM_CLEAR			(2101)
	#define MENUITEM_SPECCHARS		(2102)
	#define MENUITEM_TIMESTAMP		(2103)
	#define MENUITEM_ECHOCMD		(2104)
	#define SUBMENU_HEXSTYLE_SHORT	(2111)
	#define SUBMENU_HEXSTYLE_FULL	(2112)

	#define MENUITEM_ABOUT			(2201)
	#define MENUITEM_HELP			(2202)

	//	images
	#define ICON_BUTTON_SEND		(3001)

	//	custom events
	#define KEYBOARD_ARROWS			(5001)
	#define KEYBOARD_ESCAPE			(5002)

	//	timers
	#define TIMER_PORTSLIST			(10001)
	#define TIMER_DATAREAD			(10002)

	//	timeouts (in ms, these are not IDs!)
	#define TIMEOUT_PORTSLIST		(500)
	#define TIMEOUT_DATAREAD		(250)

#endif
