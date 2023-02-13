#ifndef _APP_RESOURCE_CODES
#define _APP_RESOURCE_CODES


	//	gui
	#define GUI_STATUSBAR		(10)
	#define GUI_LOGWIN			(11)
	#define GUI_COMPROM			(12)
	#define GUI_COMBO_PORT		(13)
	#define GUI_COMBO_SPEED		(14)
	#define GUI_COMBO_LINE		(15)
	#define GUI_BTN_SEND		(16)
	#define GUI_BTN_UPD			(18)
	#define GUI_CHK_NLN			(19)
	#define GUI_CHK_QKAT		(20)
	#define CHECKBOX_TIMESTAMP	(100)
	#define CHECKBOX_ECHOCMD	(101)
	#define CHECKBOX_TEXTMODE	(102)

	//	context menus
	#define CONTEXT_FILE_SVLOG	(2001)
	#define CONTEXT_FILE_EXIT	(2002)
	#define CONTEXT_ABOUT		(2003)
	#define CONTEXT_CLEAR		(2004)

	//	images
	#define ICON_BUTTON_SEND	(3001)

	//	custom events
	#define KBEV_HISTORY		(5001)

	//	timers
	#define TIMER_PORTSLIST		(10001)
	#define TIMER_DATAREAD		(10002)

	//	timeouts (in ms, these are not IDs!)
	#define TIMEOUT_PORTSLIST	(500)
	#define TIMEOUT_DATAREAD	(250)

#endif
