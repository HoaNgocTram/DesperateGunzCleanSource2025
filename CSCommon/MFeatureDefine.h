#pragma once
#include "../sdk/xor/include/xorstr.h"
/////////////////////
//  Toggle system  //
/////////////////////
// Enabled Settings
#define _SPYMODE 1
#define _SPYLADDER 1
#define _COMMANDLOGS 1
#define _2PCLAN	1
#define _SYSTEMUPDATETIME (10 * 60 * 1000)
#define _SRVRPNG 1
#define _MULTILANGUAGE 1
#define _COUNTRYFLAG 1
#define _KILLSTREAK 1
#define _STAFFCHAT 1
#define _AVATAR_ENABLE 1
#define _QUEST 1
#define _QUEST_ITEM	1
#define _MONSTER_BIBLE 1
#define _DUELTOURNAMENT	1
#define _CW_VOTE 1
#define _LADDERGAME 1
#define _MIPING	1
#define _LADDERGAMETEST 1	// ladder
#define _VIPGRADES 1
#define _EVENTGRD 1
#define _NEWGRADE 1
#define _SHOPFIXCANT 1
#define _REPORT 1
#define _AGENTPORT 1
#define _ANTISPAM 1
#define _GRADECHANGE 1
#define _EXPCHANNEL 1
#define _CHANNELNEW 1
#define _LIGHTURNOFF 1
#define _VOICE_CHAT 1
#define _KILLFEED 1
#define _ROTATION 1
#define _BOXLEAD 1
#define _SWORDCOLOR 1
#define _NOLEAD 1
#define _CALCDMG 1
#define _HWID 1
#define _CMD_ALL 1
#define _FORHPAP 1
#define _UPCHARCMD 1
#define _MAGICBOX 1	// thiếu database
#define _PAINTMODE 1
#define _DYNAMIC 1	// Dynamic không load elu quần áo
#define _ICONCHAT 1	// icon voice chat trên đầu nhân vật
#define _ITEMSTORAGE 1
#define _SYSTEMZITEM 1
#define _LADDERWARSPACKETS 1
#define _LOGIN_AUTH 1
#define _POSTABINFO 1
#define UPDATE_STAGE_CHARVIEWER 1
#define UPDATE_STAGE_EQUIP_LOOK 1
#define _KOR_THINGS 1
#define _ROCKETGUIDED 1
#define _PORTALGUN 1
#define _TYPENET 1
#define _SPEC 1
#define _ANTISQL 1

//#define _SERIALKEY 1
#define _FLOATDMG 1

//////////////////////////////////////////////////////////////////////////
//																		//
//#define _EAC_FILE 1		// check EAC license	SHA1 + HMAC			//			
//#define _EAC 1			// Easy Anti-Cheat							//
//																		//
//	if you really want to use this, don't change my code EAC			//
//  It will get the ProductID parameter from Epic Game to work			//
//																		//
//////////////////////////////////////////////////////////////////////////
#define _UTF8 1				// bên cml

//Gunz IP
#define _AGENT_IP       "127.0.0.1"


#if defined(_DEBUG) || defined(_RELEASE) || defined(LOCALE_KOREA) || defined(LOCALE_NHNUSA)
#endif

#if 0
#	define _SELL_CASHITEM
#endif