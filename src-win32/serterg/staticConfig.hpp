//  2022 maddsua | https://gitlab.com/maddsua
//	No warranties are given, etc...
//	This file is a component of the Serial Terminal (graphical version)


#define VER_AUTHSTAMP	"2022 maddsua"

/*		internals		*/
#define windowSizeX				640
#define windowSizeY				480
#define userinputBuffer			128
#define ascinumbShift			48
#define TTOUT					10
#define NNLTTOUT				50
#define comlongmsg				128
#define commsgbuff				512
#define comlogbuff				640
#define atcomlen				32
#define CYCLE_PRINT				666
#define winapi_textedit_OVF		28000


/*		GUI		*/
#define GUI_LOGWIN				11
#define GUI_COMPROM				12
#define GUI_COMBO_PORT			13
#define GUI_COMBO_SPEED			14
#define GUI_BTN_SEND			15
#define GUI_BTN_CLR				16
#define GUI_BTN_UPD				17
#define GUI_CHK_NLN				18
#define GUI_CHK_QKAT			19

#define GUI_AT_PREF				20
#define GUI_AT_AT				21
#define GUI_AT_OK				22
#define GUI_AT_ID				23


#define CM_FILE_SVLOG			9070
#define CM_FILE_EXIT			9071
#define CM_ABOUT				9072

#define ICEV_CMDLIST			10000



/*		serial IO		*/
#define comSyncDelayMs			1
#define comMsgReadSize			32
#define comMsgSendSize			(userinputBuffer)
#define scanSerialPorts			64
#define comPortNamePrefix		"COM"
#define comPortPathPrefix		"\\\\.\\"
#define portNameLen				8
#define portPathLen				16
#define serialSpeeds			15
#define defSerialSpeed			6

