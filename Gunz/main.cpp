#include "stdafx.h"
//#include "../MatchServer/vld/vld.h"

#ifdef _HSHIELD
#include "HShield/HShield.h"
#endif

#ifdef _XTRAP
#include "./XTrap/Xtrap_C_Interface.h"						// update sgk 0702 start
#include "./XTrap/XTrap4Launcher.h"
#pragma comment (lib, "./XTrap/XTrap4Launcher_mt.lib")
#pragma comment (lib, "./XTrap/XTrap4Client_mt.lib")	// update sgk 0702 end
#endif

#include "ZPrerequisites.h"
#include "ZConfiguration.h"
#include "ZGameClient.h"
#include <windows.h>
#include <wingdi.h>
#include <mmsystem.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <Psapi.h>
#pragma comment (lib, "Psapi.lib")

#include "DxErr.h"


#include "main.h"
#include "resource.h"
#include "VersionNo.h"

#include "Mint4R2.h"
#include "ZApplication.h"
#include "MDebug.h"
#include "ZMessages.h"
#include "MMatchNotify.h"
#include "RealSpace2.h"
#include "Mint.h"
#include "ZGameInterface.h"
#include "RFrameWork.h"
#include "ZButton.h"
#include "ZDirectInput.h"
#include "ZActionDef.h"
#include "MRegistry.h"
#include "ZInitialLoading.h"
#include "MDebug.h"
#include "MCrashDump.h"
#include "ZEffectFlashBang.h"
#include "ZMsgBox.h"
#include "ZSecurity.h"
#include "ZStencilLight.h"
#include "ZReplay.h"
#include "ZUtil.h"
#include "ZOptionInterface.h"
#include "HMAC_SHA1.h"

#ifdef USING_VERTEX_SHADER
#include "RShaderMgr.h"
#endif

//#include "mempool.h"
#include "RLenzFlare.h"
#include "ZLocale.h"
#include "MSysInfo.h"

#include "MTraceMemory.h"
#include "ZInput.h"
#include "Mint4Gunz.h"
#include "SecurityTest.h"
#include "CheckReturnCallStack.h"
#include "MMD5.h"
#include "RGGlobal.h"
#ifdef _EAC
#include <thread>
#include "eos_platform_prereqs.h"
#include "eos_sdk.h"
#include "eos_common.h"
#include "eos_anticheatclient.h"
#include "eos_anticheatclient_types.h"
#include "eos_anticheatcommon_types.h"
#include "eos_auth.h"
#include "eos_auth_types.h"
#endif
#ifdef _EAC
EOS_HPlatform g_hPlatform = NULL;
EOS_HMetrics  g_hMetrics = NULL;
#endif

#include <stdlib.h>  
#include <crtdbg.h> 
//#include <vld.h>

HMODULE	g_hUser32;
DWORD g_ThreadLastUpdateTime = 0;
DWORD g_BanLastUpdateTime = 0;
#ifdef _DEBUG
//jintriple3 ¸Þ¸ð¸® ¸¯ vld
//#include "vld.h"
#endif

#include "RGMain.h"

#ifdef _DEBUG
RMODEPARAMS	g_ModeParams={640,480,false,D3DFMT_R5G6B5};
//RMODEPARAMS	g_ModeParams={1024,768,false,RPIXELFORMAT_565};
#else
RMODEPARAMS	g_ModeParams={800,600,true,D3DFMT_R5G6B5};
#endif

#ifndef _DEBUG
#define SUPPORT_EXCEPTIONHANDLING
#endif

// Custom: Disable NHN auth
//#ifdef LOCALE_NHNUSA
//#include "ZNHN_USA.h"
//#include "ZNHN_USA_Report.h"
//#include "ZNHN_USA_Poll.h"
//#endif

#ifdef _GAMEGUARD
#include "ZGameguard.h"
#endif

RRESULT RenderScene(void *pParam);

#define RD_STRING_LENGTH 512
char cstrReleaseDate[512];// = "ReleaseDate : 12/22/2003";

ZApplication	g_App;
MDrawContextR2* g_pDC = NULL;
MFontR2*		g_pDefFont = NULL;
ZDirectInput	g_DInput;
ZInput*			g_pInput = NULL;
Mint4Gunz		g_Mint;

// Antihack stuff
bool			g_bThreadChecker = true;
DWORD			g_dwThreadId = 0;
HANDLE			g_hIntegrityThread = NULL;
DWORD g_ShouldBanPlayer = 0;
DWORD g_ShouldUpdateCrcTimer = 0;

#ifdef _EAC
bool IsEACServiceRunningA()
{
	SC_HANDLE scm = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
	if (!scm) return false;

	SC_HANDLE svc = OpenServiceA(scm, "EasyAntiCheat_EOS", SERVICE_QUERY_STATUS);
	if (!svc) {
		CloseServiceHandle(scm);
		return false;
	}

	SERVICE_STATUS_PROCESS ssp;
	DWORD bytes = 0;

	BOOL ok = QueryServiceStatusEx(
		svc,
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)&ssp,
		sizeof(ssp),
		&bytes);

	CloseServiceHandle(svc);
	CloseServiceHandle(scm);

	return ok && ssp.dwCurrentState == SERVICE_RUNNING;
}

void KillGameWithMessage(const char* reason)
{
	MessageBoxA(NULL, reason, "Easy Anti-Cheat", MB_OK | MB_ICONERROR | MB_TOPMOST);

	// Cho log có thời gian flush nếu bạn muốn
	Sleep(300);

	// Thoát game an toàn (không gắt như TerminateProcess)
	//ExitProcess(0);
	TerminateProcess(GetCurrentProcess(), 0);
}

void WatchdogThread()
{
	Sleep(2000); // đợi EAC init

	while (true)
	{
		if (!IsEACServiceRunningA())
		{
			TerminateProcess(GetCurrentProcess(), 0); //tắt nhanh ko thông báo
			/*KillGameWithMessage(
				"Easy Anti-Cheat has been disabled or encountered a problem.\n"
				"Please restart the game."
			);*/
			return;
		}
		Sleep(800);
	}
}

DWORD WINAPI WatchdogThreadProc(LPVOID lpParam)
{
	WatchdogThread();
	return 0;
}
#endif

void IntegrityThread();

HRESULT GetDirectXVersionViaDxDiag( DWORD* pdwDirectXVersionMajor, DWORD* pdwDirectXVersionMinor, TCHAR* pcDirectXVersionLetter );

void zexit(int returnCode)
{
	// °ÔÀÓ°¡µå´Â Á¦´EÎ deleteµÇ¾ûÚß ¿À·ù¹ß»ý½Ã ÀÚÃ¼ ·Î±×¸¦ ¿Ã¹Ù¸£°Ô ³²±E¼EÀÖ´Ù.
	// ±×³É exit()ÇØµµ ZGameGuard¸¦ ½Ì±ÛÅÏÀ¸·Î ¸¸µé¾ú±E¶§¹®¿¡ ¼Ò¸EÚ¿¡¼­ °ÔÀÓ°¡µå°¡ deleteµÇÁö¸¸ ¾ûÞ°¼­ÀÎÁE±×¶§ Å©·¡½Ã°¡ ÀÏ¾ûÏ­´Ù.
	// exit()ÇÏ±EÀE¡ °ÔÀÓ°¡µå¸¦ ¼öµ¿À¸·Î ÇØÁ¦ÇÏ¸E±×·± ¹®Á¦°¡ ÀÏ¾ûÏªÁE¾Ê´Â´Ù.
	// ÇØÅ· °ËÃEµûÜÇ ÀÌÀ¯·Î Å¬¶óÀÌ¾ðÆ® Á¾·á½Ã exitÇÏÁö¸»°Ezexit¸¦ ¾²ÀÚ.
#ifdef _GAMEGUARD
	GetZGameguard().Release();
#endif
	exit(returnCode);
}

void CrcFailExitApp() { 
#ifdef _PUBLISH
#else
	int* crash = NULL;
	*crash = 0;
#endif
}

void _ZChangeGameState(int nIndex)
{
	GunzState state = GunzState(nIndex);

	if (ZApplication::GetGameInterface())
	{
		ZApplication::GetGameInterface()->SetState(state);
	}
}

RRESULT OnCreate(void *pParam)
{

#ifdef _DEBUG
	g_App.PreCheckArguments();
#endif

	//Custom: Voice Chat Draw
	GetRGMain().OnCreateDevice();

	//Custom: Load dll Vulkan+d3d9
	//GetRGMain().OnVulkanD3D9Create();
	

	RAdvancedGraphics::bDepthBuffering = Z_VIDEO_STENCILBUFFER;
	RAdvancedGraphics::nMultiSampling = Z_VIDEO_MULTISAMPLING;
	if (RAdvancedGraphics::bDepthBuffering || RAdvancedGraphics::nMultiSampling > 0)
	{
		RMODEPARAMS ModeParams = 
		{ 
			RGetScreenWidth(), RGetScreenHeight(), RGetScreenType(), RGetPixelFormat() 
		};
		RResetDevice(&ModeParams);
	}

#ifdef _DYNAMIC
	if (Z_ETC_DYNAMIC == true)
	{
		RMesh::SetPartsMeshLoadingSkip(1);
	}
#endif

	string strFileLenzFlare("System/LenzFlare.xml");
#ifndef _DEBUG
	strFileLenzFlare += ""; // MEF NULL
#endif
	RCreateLenzFlare(strFileLenzFlare.c_str());
	RGetLenzFlare()->Initialize();

	mlog("main : RGetLenzFlare()->Initialize() \n");

	RBspObject::CreateShadeMap("sfx/water_splash.bmp");
	//D3DCAPS9 caps;
	//RGetDevice()->GetDeviceCaps( &caps );
	//if( caps.VertexShaderVersion < D3DVS_VERSION(1, 1) )
	//{
	//	RGetShaderMgr()->mbUsingShader				= false;
	//	RGetShaderMgr()->shader_enabled				= false;
	//	mlog("main : VideoCard Dosen't support Vertex Shader...\n");
	//}
	//else
	//{
	//	mlog("main : VideoCard support Vertex Shader...\n");
	//}

#ifdef _SPRINTSCREEN
	sprintf_safe(cstrReleaseDate, "Last Patch: " __DATE__ "");
#endif
	g_DInput.Create(g_hWnd, FALSE, FALSE);
	g_pInput = new ZInput(&g_DInput);
	
	RSetGammaRamp(Z_VIDEO_GAMMA_VALUE);
	RSetRenderFlags(RRENDER_CLEAR_BACKBUFFER);

	ZGetInitialLoading()->Initialize(  1, 0, 0, RGetScreenWidth(), RGetScreenHeight(), 0, 0, 1024, 768 );
	mlog("InitialLoading success.\n");

	struct _finddata_t c_file;
	intptr_t hFile;
	char szFileName[256];
#define FONT_DIR	"Font/"
#define FONT_EXT	"ttf"
	if( (hFile = _findfirst(FONT_DIR "*." FONT_EXT, &c_file )) != -1L ){
		do{
			strcpy(szFileName, FONT_DIR);
			strcat(szFileName, c_file.name);
			AddFontResource(szFileName);
		}while( _findnext( hFile, &c_file ) == 0 );
		_findclose(hFile);
	}

	g_pDefFont = new MFontR2;

	if( !g_pDefFont->Create("Default", Z_LOCALE_DEFAULT_FONT, DEFAULT_FONT_HEIGHT, 1.0f) )
//	if( !g_pDefFont->Create("Default", RGetDevice(), "FONTb11b", 9, 1.0f, true, false) )
//	if( !g_pDefFont->Create("Default", RGetDevice(), "FONTb11b", 14, 1.0f, true, false) )
	{
		mlog("Fail to Create default font : MFontR2 / main.cpp.. onCreate\n" );
		g_pDefFont->Destroy();
		SAFE_DELETE( g_pDefFont );
		g_pDefFont	= NULL;
	}

	g_pDC = new MDrawContextR2(RGetDevice());

	if( ZGetInitialLoading()->IsUseEnable() )
	{
		if( ZGetLocale()->IsTeenMode() )
		{
			ZGetInitialLoading()->AddBitmap( 0, "Interface/Default/LOADING/loading_adult.jpg" );
		}
		else
		{
			ZGetInitialLoading()->AddBitmap( 0, "Interface/Default/LOADING/loading_adult.jpg" );
		}
		ZGetInitialLoading()->AddBitmapBar( "Interface/Default/LOADING/loading.bmp" );
		ZGetInitialLoading()->SetText( g_pDefFont, 45, 28, cstrReleaseDate );

		ZGetInitialLoading()->AddBitmapGrade( "Interface/Default/LOADING/loading_adult.jpg" );

		ZGetInitialLoading()->SetPercentage( 0.0f );
		ZGetInitialLoading()->Draw( MODE_FADEIN, 0 , true );
	}

//	ZGetInitialLoading()->SetPercentage( 10.0f );
//	ZGetInitialLoading()->Draw( MODE_DEFAULT, 0 , true );

	g_Mint.Initialize(800, 600, g_pDC, g_pDefFont);
	Mint::GetInstance()->SetHWND(RealSpace2::g_hWnd);

	mlog("interface Initialize success\n");

	ZLoadingProgress appLoading("application");
	if(!g_App.OnCreate(&appLoading))
	{
		ZGetInitialLoading()->Release();
		return R_ERROR_LOADING;
	}

	ZGetSoundEngine()->SetEffectVolume(Z_AUDIO_EFFECT_VOLUME);
	ZGetSoundEngine()->SetMusicVolume(Z_AUDIO_BGM_VOLUME);
	ZGetSoundEngine()->SetEffectMute(Z_AUDIO_EFFECT_MUTE);
	ZGetSoundEngine()->SetMusicMute(Z_AUDIO_BGM_MUTE);

	g_Mint.SetWorkspaceSize(g_ModeParams.nWidth, g_ModeParams.nHeight);
	g_Mint.GetMainFrame()->SetSize(g_ModeParams.nWidth, g_ModeParams.nHeight);
	ZGetOptionInterface()->Resize(g_ModeParams.nWidth, g_ModeParams.nHeight);

	for(int i=0; i<ZACTION_COUNT; i++){
//		g_Mint.RegisterActionKey(i, ZGetConfiguration()->GetKeyboard()->ActionKeys[i].nScanCode);
		ZACTIONKEYDESCRIPTION& keyDesc = ZGetConfiguration()->GetKeyboard()->ActionKeys[i];
		g_pInput->RegisterActionKey(i, keyDesc.nVirtualKey);
		if(keyDesc.nVirtualKeyAlt!=-1)
			g_pInput->RegisterActionKey(i, keyDesc.nVirtualKeyAlt);
	}

	g_App.SetInitialState();

//	ParseParameter(g_szCmdLine);

	ZGetFlashBangEffect()->SetDrawCopyScreen(true);

	static const char *szDone = "Complete";
	ZGetInitialLoading()->SetLoadingStr(szDone);
	if( ZGetInitialLoading()->IsUseEnable() )
	{
#ifndef _FASTDEBUG
		ZGetInitialLoading()->SetPercentage( 100.f );
		ZGetInitialLoading()->Draw( MODE_FADEOUT, 0 ,true  );
#endif
		ZGetInitialLoading()->Release();
	}

	mlog("main : OnCreate() done\n");

	SetFocus(g_hWnd);

	return R_OK;
}


bool CheckDll(char* fileName, BYTE* SHA1_Value)
{
	BYTE digest[20];
	BYTE Key[GUNZ_HMAC_KEY_LENGTH];

	memset(Key, 0, 20);
	memcpy(Key, GUNZ_HMAC_KEY, strlen(GUNZ_HMAC_KEY));

	CHMAC_SHA1 HMAC_SHA1 ;
	HMAC_SHA1.HMAC_SHA1_file(fileName, Key, GUNZ_HMAC_KEY_LENGTH, digest) ;

	if(memcmp(digest, SHA1_Value, 20) ==0)
	{
		return true;
	}

	return false;
}



RRESULT OnDestroy(void *pParam)
{
	mlog("Destroy gunz\n");

	g_App.OnDestroy();

	SAFE_DELETE(g_pDefFont);

	g_Mint.Finalize();

	mlog("interface finalize.\n");

	SAFE_DELETE(g_pInput);
	g_DInput.Destroy();

	mlog("game input destroy.\n");

	RGetShaderMgr()->Release();

//	g_App.OnDestroy();

	// mlog("main : g_App.OnDestroy()\n");

	ZGetConfiguration()->Destroy();

	mlog("game configuration destroy.\n");

	if (g_pDC != NULL)
	{
		delete g_pDC;
		g_pDC = NULL;
	}

	struct _finddata_t c_file;
	intptr_t hFile;
	char szFileName[256];
#define FONT_DIR	"Font/"
#define FONT_EXT	"ttf"
	if( (hFile = _findfirst(FONT_DIR "*." FONT_EXT, &c_file )) != -1L ){
		do{
			strcpy(szFileName, FONT_DIR);
			strcat(szFileName, c_file.name);
			RemoveFontResource(szFileName);
		}while( _findnext( hFile, &c_file ) == 0 );
		_findclose(hFile);
	}

	MFontManager::Destroy();
	MBitmapManager::Destroy();
	MBitmapManager::DestroyAniBitmap();

	mlog("Bitmap manager destroy Animation bitmap.\n");

	/*
	for(list<HANDLE>::iterator i=g_FontMemHandles.begin(); i!=g_FontMemHandles.end(); i++){
		RemoveFontMemResourceEx(*i);
	}
	*/

	//ReleaseMemPool(RealSoundEffectPlay);
	//UninitMemPool(RealSoundEffectPlay);

	//ReleaseMemPool(RealSoundEffect);
	//UninitMemPool(RealSoundEffect);

	//ReleaseMemPool(RealSoundEffectFx);
	//UninitMemPool(RealSoundEffectFx);

	//mlog("main : UninitMemPool(RealSoundEffectFx)\n");

	// ¸Þ¸ð¸®Ç® ÇE¦
	ZBasicInfoItem::Release(); // ÇÒ´çµÇ¾EÀÖ´Â ¸Þ¸ð¸® ÇØÁ¦
//	ZHPInfoItem::Release();

	ZGetStencilLight()->Destroy();
	LightSource::Release();

	RBspObject::DestroyShadeMap();
	RDestroyLenzFlare();
	RAnimationFileMgr::GetInstance()->Destroy();
	
	ZStringResManager::ResetInstance();

	DestroyRGMain();

	mlog("destroy gunz finish.\n");

	return R_OK;
}

RRESULT OnUpdate(void* pParam)
{
	__BP(100, "main::OnUpdate");

	g_pInput->Update();

	g_App.OnUpdate();

#ifdef _EAC
	if (g_hPlatform) {
		EOS_Platform_Tick(g_hPlatform);
	}
#endif

	const DWORD dwCurrUpdateTime = timeGetTime();

	__EP(100);

	return R_OK;
}

RRESULT OnRender(void *pParam)
{
	__BP(101, "main::OnRender");

	//Clip the cursor to the main screen
	if (GetFocus() == g_hWnd)
	{
		RECT rect;
		GetWindowRect(g_hWnd, &rect);
		ClipCursor(&rect);
	}
	//otherwise release the cursor
	else
	{
		ClipCursor(NULL);
	}
	//RUpdateMouseClip(); //điều khiển chuột mới bên Realspace2

	if( !RIsActive() && RGetScreenType() ==0 )
	{
		__EP(101);
		return R_NOTREADY;
	}


	g_App.OnDraw();

	// Custom: Fix bandicam support
	if (ZIsActionKeyPressed(ZACTION_SCREENSHOT)) 
	{
		if (g_App.GetGameInterface())
			g_App.GetGameInterface()->GetBandiCapturer()->CaptureImage();
	}
	if (ZIsActionKeyPressed(ZACTION_MOVING_PICTURE))
	{	
		if (g_App.GetGameInterface())
			g_App.GetGameInterface()->GetBandiCapturer()->ToggleStart();
    }

	if (ZGetGameInterface()->GetBandiCapturer() != NULL)
		ZGetGameInterface()->GetBandiCapturer()->DrawCapture(g_pDC);


#ifdef _SMOOTHLOOP
	Sleep(10);
#endif
	static char __buffer[256];
	float x = 10.f / 800.f;
	int screenx = x * MGetWorkspaceWidth();
	if (RGetIsWidthScreen() || RGetIsLongScreen())
	{
		x = (x * 800 + 80) / 590.f;
		screenx = x * MGetWorkspaceWidth();
	}
	if (g_pDefFont && ZGetConfiguration()->GetEtc()->bShowFPS)
	{
		//sprintf(__buffer, "FPS: %3.0f", g_fFPS); // Ẩn FPS ở góc trái

		float y = 0.0f / 600.f;

		if (RGetIsWidthScreen() || RGetIsLongScreen())
		{
			g_pDefFont->m_Font.DrawText(MGetWorkspaceWidth() - screenx, 0, __buffer);
		}
		else
			g_pDefFont->m_Font.DrawText(MGetWorkspaceWidth() - 75, 0, __buffer);
	}
	__EP(101);

	return R_OK;
}

RRESULT OnInvalidate(void *pParam)
{
	MBitmapR2::m_dwStateBlock=NULL;

	g_App.OnInvalidate();
	
	return R_OK;
}

RRESULT OnRestore(void *pParam)
{
	for(int i=0; i<MBitmapManager::GetCount(); i++){
		MBitmapR2* pBitmap = (MBitmapR2*)MBitmapManager::Get(i);
		pBitmap->OnLostDevice();
	}

	g_App.OnRestore();

	return R_OK;
}

RRESULT OnActivate(void *pParam)
{
	if (ZGetGameInterface() && ZGetGameClient() && Z_ETC_BOOST)
		ZGetGameClient()->PriorityBoost(true);
	return R_OK;
}

RRESULT OnDeActivate(void *pParam)
{

	if (ZGetGameInterface() && ZGetGameClient())
		ZGetGameClient()->PriorityBoost(false);
	return R_OK;
}

RRESULT OnError(void *pParam)
{
	mlog("RealSpace::OnError(%d) \n", RGetLastError());

	switch (RGetLastError())
	{
	case RERROR_INVALID_DEVICE:
		{
			D3DADAPTER_IDENTIFIER9 *ai=RGetAdapterID();
			char szLog[512];
			ZTransMsg( szLog, MSG_DONOTSUPPORT_GPCARD, 1, ai->Description);

			int ret=MessageBox(NULL, szLog, ZMsg( MSG_WARNING), MB_YESNO);
			if(ret!=IDYES)
				return R_UNKNOWN;
		}
		break;
	case RERROR_CANNOT_CREATE_D3D:
		{
			ShowCursor(TRUE);

			char szLog[512];
			sprintf(szLog, ZMsg( MSG_DIRECTX_NOT_INSTALL));

			int ret=MessageBox(NULL, szLog, ZMsg( MSG_WARNING), MB_YESNO);
			if(ret==IDYES)
			{
				ShellExecute(g_hWnd, "open", ZMsg( MSG_DIRECTX_DOWNLOAD_URL), NULL, NULL, SW_SHOWNORMAL); 
			}
		}
		break;

	};

	return R_OK;
}

void SetModeParams()
{

	g_ModeParams.nScreenType = ZGetConfiguration()->GetVideo()->nScreenType;
	if (g_ModeParams.nScreenType != 2)
	{
		g_ModeParams.nWidth = ZGetConfiguration()->GetVideo()->nWidth;
		g_ModeParams.nHeight = ZGetConfiguration()->GetVideo()->nHeight;
	}
	else
	{
		g_ModeParams.nWidth = GetSystemMetrics(SM_CXSCREEN);
		g_ModeParams.nHeight = GetSystemMetrics(SM_CYSCREEN) - getTaskBarHeight() - GetSystemMetrics(SM_CYBORDER);
	}
	ZGetConfiguration()->GetVideo()->nColorBits == 32 ? 
		g_ModeParams.PixelFormat = D3DFMT_X8R8G8B8 : g_ModeParams.PixelFormat = D3DFMT_R5G6B5 ;

}

void ResetAppResource()
{
	MLog("Language changed → restarting...\n");

	// Lưu config (đã chứa ngôn ngữ mới)
	ZGetConfiguration()->Destroy();
	ZGetConfiguration()->Load();
	ZGetConfiguration()->Load_StringResDependent();
	ZGetConfiguration()->SaveToFile(FILENAME_CONFIG, Z_LOCALE_XML_HEADER);

	// Restart game EXE, bỏ launcher
	ZApplication::GetInstance()->RestartGame();
}

int FindStringPos(char* str,char* word)
{
	if(!str || str[0]==0)	return -1;
	if(!word || word[0]==0)	return -1;

	int str_len = (int)strlen(str);
	int word_len = (int)strlen(word);

	char c;
	bool bCheck = false;

	for(int i=0;i<str_len;i++) {
		c = str[i];
		if(c == word[0]) {

			bCheck = true;

			for(int j=1;j<word_len;j++) {
				if(str[i+j]!=word[j]) {
					bCheck = false;
					break;
				}
			}

			if(bCheck) {
				return i;
			}
		}
	}
	return -1;
}

bool FindCrashFunc(char* pName)
{
//	Function Name
//	File Name 
	if(!pName) return false;

	FILE *fp;
	fp = fopen( "mlog.txt", "r" );
	if(fp==NULL)  return false;

	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	char* pBuffer = new char [size];

	fread(pBuffer,1,size,fp);

	fclose(fp);

	// ¿E® ½ûÙº¿¡¼­ Ã£´Â´Ù.
	int posSource = FindStringPos(pBuffer,"ublish");
	if(posSource==-1) return false;

	int posA = FindStringPos(pBuffer+posSource,"Function Name");
//	int posB = FindStringPos(pBuffer,"File Name");	
	// filename ÀÌ ¾ø´Â °æ¿Eµ ÀÖ¾ûØ­ ÀÌ·¸°Ô ¹Ù²å´Ù
	int posB = posA + FindStringPos(pBuffer+posSource+posA,"\n");

	if(posA==-1) return false;
	if(posB==-1) return false;

	posA += 16;

//	int memsize = posB-posA-6;
	int memsize = posB-posA;
	memcpy(pName,&pBuffer[posA+posSource],memsize);

	pName[memsize] = 0;

	delete [] pBuffer;

	for(int i=0;i<memsize;i++) {
		if(pName[i]==':') {
			pName[i] = '-';
		}
	}

	return true;
}

void HandleExceptionLog()
{
	/*#define ERROR_REPORT_FOLDER	"ReportError"

	extern char* logfilename;	// Instance on MDebug.cpp

	// ERROR_REPORT_FOLDER Á¸ÀçÇÏ´ÂÁE°Ë»çÇÏ°E ¾øÀ¸¸E»ý¼º
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFile(ERROR_REPORT_FOLDER, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		if (!CreateDirectory("ReportError", NULL)) {
			MessageBox(g_hWnd, "ReportError Æú´õ¸¦ »ý¼ºÇÒ ¼E¾ø½À´Ï´Ù.", APPLICATION_NAME , MB_ICONERROR|MB_OK);
			return;
		}
	} else 	{
		FindClose(hFind);
	}*/


/* 2007³E2¿E13ÀÏ BAReport ´õÀÌ»E»ç¿E¸øÇÏ°Ô ¸·À½


	// mlog.txt ¸¦ ERROR_REPORT_FOLDER ·Î º¹»E

	//acesaga_0928_911_moanus_rslog.txt
	//USAGE_EX) BAReport app=acesaga;addr=moon.maiet.net;port=21;id=ftp;passwd=ftp@;gid=10;user=moanus;localfile=rslog.txt;remotefile=remote_rslog.txt;

//	if(ZGetCharacterManager()) {
//		ZGetCharacterManager()->OutputDebugString_CharacterState();
//	}


	ZGameClient* pClient = (ZGameClient*)ZGameClient::GetInstance();

	char* pszCharName = NULL;
	MUID uidChar;
	MMatchObjCache* pObj;
	char szPlayer[128];

	if( pClient ) {

		uidChar = pClient->GetPlayerUID();
		pObj = pClient->FindObjCache(uidChar);
		if (pObj)
			pszCharName = pObj->GetName();

		wsprintf(szPlayer, "%s(%d%d)", pszCharName?pszCharName:"Unknown", uidChar.High, uidChar.Low);
	}
	else { 
		wsprintf(szPlayer, "Unknown(-1.-1)");
	}


//	if (pClient) {

		time_t currtime;
		time(&currtime);
		struct tm* pTM = localtime(&currtime);

		char cFuncName[1024];

		if(FindCrashFunc(cFuncName)==false) {
			strcpy(cFuncName,"Unknown Error");
		}

		char szFileName[_MAX_DIR], szDumpFileName[_MAX_DIR];
		wsprintf(szFileName, "%s_%s_%.2d%.2d_%.2d%.2d_%s_%s", cFuncName,
				APPLICATION_NAME, pTM->tm_mon+1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, szPlayer, "mlog.txt");
		wsprintf(szDumpFileName, "%s.dmp", szFileName);

		char szFullFileName[_MAX_DIR], szDumpFullFileName[_MAX_DIR];
		wsprintf(szFullFileName, "%s/%s", ERROR_REPORT_FOLDER, szFileName);
		wsprintf(szDumpFullFileName, "%s/%s", ERROR_REPORT_FOLDER, szDumpFileName);

		if (CopyFile("mlog.txt", szFullFileName, TRUE))
		{
			CopyFile("Gunz.dmp", szDumpFullFileName, TRUE);

			// BAReport ½ÇÇE
			char szCmd[4048];
			char szRemoteFileName[_MAX_DIR], szRemoteDumpFileName[_MAX_DIR];
			wsprintf(szRemoteFileName, "%s/%s/%s", ZGetConfiguration()->GetBAReportDir(), "gunzlog", szFileName);
			wsprintf(szRemoteDumpFileName, "%s/%s/%s", ZGetConfiguration()->GetBAReportDir(), "gunzlog", szDumpFileName);

			wsprintf(szCmd, "BAReport app=%s;addr=%s;port=21;id=ftp;passwd=ftp@;user=%s;localfile=%s,%s;remotefile=%s,%s", 
				APPLICATION_NAME, ZGetConfiguration()->GetBAReportAddr(), szPlayer, szFullFileName, szDumpFullFileName, szRemoteFileName, szRemoteDumpFileName);

			WinExec(szCmd, SW_SHOW);

			FILE *file = fopen("bareportpara.txt","w+");
			fprintf(file,szCmd);
			fclose(file);
		}
//	}
*/
}

long FAR PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
#ifdef LOCALE_JAPAN
		case WM_COPYDATA:
			{
				ZBaseAuthInfo* pAuth = ZGetLocale()->GetAuthInfo();
				if( ((ZGameOnJPAuthInfo*)pAuth)->NewLogin(wParam, lParam) )
				{
					MessageBox(g_hWnd, "Same id accessing from a different PC", NULL, MB_OK);
					zexit(-1);
				}
			}
			break;
#endif

		case WM_SYSCHAR:
			if(ZIsLaunchDevelop() && wParam==VK_RETURN)
			{
#ifndef _PUBLISH
				RFrame_ToggleFullScreen();
#endif
				return 0;
			}
			break;

		case WM_CREATE:
			if (strlen(Z_LOCALE_HOMEPAGE_TITLE) > 0)
			{
				ShowIExplorer(false, Z_LOCALE_HOMEPAGE_TITLE);
			}
			break;
		case WM_DESTROY:
			if (strlen(Z_LOCALE_HOMEPAGE_TITLE) > 0)
			{
				ShowIExplorer(true, Z_LOCALE_HOMEPAGE_TITLE);
			}
			break;
		case WM_SETCURSOR:
			if(ZApplication::GetGameInterface())
				ZApplication::GetGameInterface()->OnResetCursor();
			return TRUE; // prevent Windows from setting cursor to window class cursor

		case WM_ENTERIDLE:
			// ¸ð´Þ ´E­»óÀÚ°¡ ÄÚµå¸¦ ºúÓ°ÇÏ°EÀÖÀ» ¶§ ºÎ¸ð¿¡°Ô º¸³»´Â idle ÅEö¸Þ½ÃÁE
			// (ÀÏº» IME¿¡ ¸ð´Þ ´E­»óÀÚ°¡ ÀÖ¾ûØ­ ³Ö¾úÀ½)
			// ¸ð´Þ ´E­»óÀÚ·Î ¾÷µ¥ÀÌÆ® ·çÇÁ¸¦ ºúÓ°ÇØ¼­ ¹«ÀE¾ûÖäÁûÓÎ ¾Ç¿EÇ±E¶§¹®¿¡ ¿©±â¼­ ¾÷µ¥ÀÌÆ®¸¦ ½ÇÇàÇÑ´Ù
			RFrame_UpdateRender();
			break;

			/*
		case  WM_LBUTTONDOWN:
			SetCapture(hWnd);
			return TRUE;
		case WM_LBUTTONUP:
			ReleaseCapture();
			return TRUE;
			*/
		case WM_KEYDOWN:
			{
				bool b = false;
			}
	}

	if(Mint::GetInstance()->ProcessEvent(hWnd, message, wParam, lParam)==true)
	{
		if (ZGetGameInterface() && ZGetGameInterface()->IsReservedResetApp())	// for language changing
		{
			ZGetGameInterface()->ReserveResetApp(false);
			ResetAppResource();
		}

		return 0;
	}

	// thread safeÇÏ±âÀ§ÇØ ³ÖÀ½
	if (message == WM_CHANGE_GAMESTATE)
	{
		_ZChangeGameState(wParam);
	}


	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
class mtrl {
public:

};

class node {
public:
	int		m_nIndex[5];
};


class _map{
public:
	mtrl* GetMtrl(node* node,int index) { return GetMtrl(node->m_nIndex[index]); }
	mtrl* GetMtrl(int id) { return m_pIndex[id]; }

	mtrl*	m_pIndex[5];
};

class game {
public:
	_map m_map;	
};

game _game;
game* g_game;
*/

bool CheckFileList()
{
	MZFileSystem *pfs=ZApplication::GetFileSystem();
	MZFile mzf;

	string strFileNameFillist(FILENAME_FILELIST);
#ifndef _DEBUG
	strFileNameFillist += ""; // MEF NULL
#endif

	if(!mzf.Open(strFileNameFillist.c_str() ,pfs))
		return false;

	char *buffer;
	buffer=new char[mzf.GetLength()+1];
	mzf.Read(buffer,mzf.GetLength());
	buffer[mzf.GetLength()]=0;

	MXmlDocument aXml;
	aXml.Create();
	if(!aXml.LoadFromMemory(buffer))
	{
		delete[] buffer;
		return false;
	}

	delete[] buffer;

	int iCount, i;
	MXmlElement		aParent, aChild;
	aParent = aXml.GetDocumentElement();
	iCount = aParent.GetChildNodeCount();

	char szTagName[256];
	for (i = 0; i < iCount; i++)
	{
		aChild = aParent.GetChildNode(i);
		aChild.GetTagName(szTagName);
		if(stricmp(szTagName,"FILE")==0)
		{
			char szContents[256],szCrc32[256];
			aChild.GetAttribute(szContents,"NAME");
			aChild.GetAttribute(szCrc32,"CRC32");

			if(stricmp(szContents,"config.xml")!=0)
			{
				unsigned int crc32_list = pfs->GetCRC32(szContents);
				unsigned int crc32_current;
				sscanf(szCrc32,"%x",&crc32_current);

#ifndef _DEBUG
				if(crc32_current!=crc32_list)
				{
					// mlog("crc error , file %s ( current = %x, original = %x ).\n",szContents,crc32_current,crc32_list);

					// ¸ðµEÆÄÀÏÀ» °Ë»ç´Â ÇÑ´Ù
					return false; 
				}
#endif
			}
		}
	}

	return true;
}


enum RBASE_FONT{
	RBASE_FONT_GULIM = 0,
	RBASE_FONT_BATANG = 1,

	RBASE_FONT_END
};

static int g_base_font[RBASE_FONT_END];
static char g_UserDefineFont[256];

bool _GetFileFontName(char* pUserDefineFont)
{
	if(pUserDefineFont==NULL) return false;

	FILE* fp = fopen("_Font", "rt");
	if (fp) {
		fgets(pUserDefineFont,256, fp);
		fclose(fp);
		return true;
	}
	return false;
}

bool CheckFont()
{
	char FontPath[MAX_PATH];
	char FontNames[MAX_PATH+100];

	::GetWindowsDirectory(FontPath, MAX_PATH);

	strcpy(FontNames,FontPath);
	strcat(FontNames, "\\Fonts\\gulim.ttc");

	if (_access(FontNames,0) != -1)	{ g_base_font[RBASE_FONT_GULIM] = 1; }
	else							{ g_base_font[RBASE_FONT_GULIM] = 0; }

	strcpy(FontNames,FontPath);
	strcat(FontNames, "\\Fonts\\batang.ttc");

	if (_access(FontNames,0) != -1)	{ g_base_font[RBASE_FONT_BATANG] = 1; }
	else							{ g_base_font[RBASE_FONT_BATANG] = 0; }

	//	strcpy(FontNames,FontPath);
	//	strcat(FontNames, "\\Fonts\\System.ttc");
	//	if (_access(FontNames,0) != -1)	{ g_font[RBASE_FONT_BATANG] = 1; }
	//	else							{ g_font[RBASE_FONT_BATANG] = 0; }

	if(g_base_font[RBASE_FONT_GULIM]==0 && g_base_font[RBASE_FONT_BATANG]==0) {//µÑ´Ù¾øÀ¸¸E.

		if( _access("_Font",0) != -1) { // ÀÌ¹Ì ±â·ÏµÇ¾EÀÖ´Ù¸E.
			_GetFileFontName( g_UserDefineFont );
		}
		else {

			int hr = IDOK;

			//hr = ::MessageBox(NULL,"±ÍÇÏÀÇ ÄÄÇ»ÅÍ¿¡´Â °ÇÁûÌ¡ »ç¿EÏ´Â (±¼¸²,µ¸¿E ÆùÆ®°¡ ¾ø´Â °Í °°½À´Ï´Ù.\n ´Ù¸¥ ÆùÆ®¸¦ ¼±ÅÃ ÇÏ½Ã°Ú½À´Ï±E","¾Ë¸²",MB_OKCANCEL);
			//hr = ::MessageBox(NULL,"±ÍÇÏÀÇ ÄÄÇ»ÅÍ¿¡´Â °ÇÁûÌ¡ »ç¿EÏ´Â (±¼¸²,µ¸¿E ÆùÆ®°¡ ¾ø´Â °Í °°½À´Ï´Ù.\n °è¼Ó ÁøÇEÇÏ½Ã°Ú½À´Ï±E","¾Ë¸²",MB_OKCANCEL);

			if(hr==IDOK) {
				/*			
				CFontDialog dlg;
				if(dlg.DoModal()==IDOK) {
				CString facename = dlg.GetFaceName();
				lstrcpy((LPSTR)g_UserDefineFont,(LPSTR)facename.operator const char*());

				hr = ::MessageBox(NULL,"¼±ÅÃÇÏ½Å ÆùÆ®¸¦ ÀúÀEÇÏ½Ã°Ú½À´Ï±E","¾Ë¸²",MB_OKCANCEL);

				if(hr==IDOK)
				_SetFileFontName(g_UserDefineFont);
				}
				*/
				return true;
			}
			else {
				return false;
			}
		}
	}
	return true;
}

#include "shlobj.h"

void CheckFileAssociation()
{
#define GUNZ_REPLAY_CLASS_NAME	"GunzReplay"

	// Ã¼Å©ÇØºÁ¼­ µûÓÏÀÌ ¾ÈµÇ¾ûÜÖÀ¸¸EµûÓÏÇÑ´Ù. »ç¿EÚ¿¡°Ô ¹°¾ûÖ¼¼öµµ ÀÖ°Ú´Ù.
	char szValue[256];
	if(!MRegistry::Read(HKEY_CLASSES_ROOT,"." GUNZ_REC_FILE_EXT,NULL,szValue))
	{
		MRegistry::Write(HKEY_CLASSES_ROOT,"." GUNZ_REC_FILE_EXT,NULL,GUNZ_REPLAY_CLASS_NAME);

		char szModuleFileName[_MAX_PATH] = {0,};
		GetModuleFileName(NULL, szModuleFileName, _MAX_DIR);

		char szCommand[_MAX_PATH];
		sprintf(szCommand,"\"%s\" \"%%1\"",szModuleFileName);

		MRegistry::Write(HKEY_CLASSES_ROOT,GUNZ_REPLAY_CLASS_NAME"\\shell\\open\\command",NULL,szCommand);

		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH, NULL, NULL);
	}
}

// ÇØ´EÅØ½ºÆ® ÆÄÀÏ¿¡ ÇØ´E±Û±Í°¡ ÀÖÀ¸¸EXTRAP Å×½ºÆ® ÄÚµå°¡ ¼öÇàµÈ´Ù. (µÞ±¸¸Û) //
bool CheckXTrapIsGetTestCode()														// add sgk 0621
{
	char szBuf[256] = "";
	FILE* fp = fopen("XTrapTest.txt", "rt");
	if (fp)
	{
		fgets(szBuf, 256, fp);
		mlog("XTrapTest.txt : \n");
		mlog(szBuf);
		mlog("\n");
		fclose(fp);

		if (stricmp(szBuf, "RUN_XTRAP_TEST_CODE") == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		mlog("fail to open XTrapTest.txt\n");
		return false;
	}
}

void OnGetXTrapRealCodeArgv(char* szTemp, bool* bPatchSkip)							// add sgk 0621
{
	/* LOCALE_KOREA */
	wsprintf(szTemp, "660970B478F9CB6395356D98440FE8629F86DDCD4DBB54B513680296C770DFCDB63DCCFE8DFE34B3A6B2BAA8CFB5C8C9761A2D3F1E491F9EFF6757489912B1AB0F7D04245246E409A061005FC9527B0823FF73E0B9F4C91EB67C40AC1A16EABB8FE52BDE25C76F6955E9E52A0812CE");

#ifdef LOCALE_JAPAN
	memset(szTemp, 0, 256);
	wsprintf(szTemp, "660970B45869CA6395356D98440FE8624C8FEA6EF181FD7D095D6CBA9911AFB0B5661B972C3C82BB0FF2D47A32DFB56D407CB146190E29B1EA46F49C1E86160F0F7D04245246E409A061005FC9527B086EF578A8BCFCC91FB67C51F65E05AAB85F7E306086BDFF03DF1BA46A66C605FFBC6263206088B68D6930514A");
#endif

#ifdef LOCALE_US
	memset(szTemp, 0, 256);
	wsprintf(szTemp, "660970B497F9CB6395356D98440FE8629AE854BDDBD13EDCE69AC1898F7A23CEF138AD2BF2758B368950133F1B021D0D218BFB058146B32450591F8B22CBE6A2");
	*bPatchSkip = true;
#endif

#ifdef LOCALE_INDIA
	memset(szTemp, 0, 256);
//	wsprintf(szTemp, "660970B47C69CB6795356D98440FE8625582AC40166A109C00E4D6A6056D18A02BBAC0A19DA6BEE6B4D43AD07CFB61697FD7FF586D98ECFF1DA11222DD82028D0F7D04245246E417A4610E569557620395165EECCBF7CD9008C4C0120CA7A0AD9D568C0DC8C7BD38C629B7EAAE5435B46105721F036F7C5BF0");
	wsprintf(szTemp, "660970B47C69CB6795356D98490FE862FEBC286C65D77538F80891D97D18B65B43E538B6EADB14290A04CF119B162DE7AA91984B54023E368FB4C25D4A91F68A0F7D04245246E417A4610E569557620395165EECCBF7CD9008C4C0120CA7A0AD9D568C0DC8C7BD38C629B7EAAE5435B46105721F036F7C5BF0");
#endif

#ifdef LOCALE_BRAZIL
	memset(szTemp, 0, 256);
	wsprintf(szTemp, "660970B448FBCB6395356D98440FE8621A6EADB8532B3C5F1949386F921C6C0970FEF0A168B5352668BE414ADF1375136173F493A8A2C075AC0F919AC7241A650F7D04245246E401B574195DD31E6305975703051B9F4F5CA2A8046A5FF3331AB0C8F040AFA98BB5CE3134520AC79D1328E836DF645FC479");
#endif
}

void OnGetXTrapTestCodeArgv(char* szTemp, bool* bPatchSkip)							// add sgk 0621
{
	/* LOCALE_KOREA */
	wsprintf(szTemp, "660970B478F9CB6395356D98440FE8629F86DDCD4DBB54B513680296C770DFCDB63DCCFE8DFE34B3A6B2BAA8CFB5C8C9761A2D3F1E491F9EFF6757489912B1AB0F7D04245246E409A061005FC9527B0823FF73E0B9F4C91EB67C40AC1A16EABB8FE52BDE25C76F6955E9E52A0812A88323D4");

#ifdef LOCALE_JAPAN
	memset(szTemp, 0, 256);
	wsprintf(szTemp, "660970B45869CA6395356D98440FE8624C8FEA6EF181FD7D095D6CBA9911AFB0B5661B972C3C82BB0FF2D47A32DFB56D407CB146190E29B1EA46F49C1E86160F0F7D04245246E409A061005FC9527B086EF578A8BCFCC91FB67C51F65E05AAB85F7E306086BDFF03DF1BA46A66C605FFBC6263206088B68D6930512C295649");
#endif

#ifdef LOCALE_US
	memset(szTemp, 0, 256);
	wsprintf(szTemp, "660970B497F9CB6395356D98440FE8629AE854BDDBD13EDCE69AC1898F7A23CEF138AD2BF2758B368950133F1B021D0D218BFB058146B32450591F8B22CBE6A2");
	*bPatchSkip = true;
#endif

#ifdef LOCALE_INDIA
	memset(szTemp, 0, 256);
//	wsprintf(szTemp, "660970B47C69CB6795356D98440FE8625582AC40166A109C00E4D6A6056D18A02BBAC0A19DA6BEE6B4D43AD07CFB61697FD7FF586D98ECFF1DA11222DD82028D0F7D04245246E417A4610E569557620395165EECCBF7CD9008C4C0120CA7A0AD9D568C0DC8C7BD38C629B7EAAE5435B46105721F036F7C5B962980B7");
	wsprintf(szTemp, "660970B47C69CB6795356D98490FE862FEBC286C65D77538F80891D97D18B65B43E538B6EADB14290A04CF119B162DE7AA91984B54023E368FB4C25D4A91F68A0F7D04245246E417A4610E569557620395165EECCBF7CD9008C4C0120CA7A0AD9D568C0DC8C7BD38C629B7EAAE5435B46105721F036F7C5B962980B7");
#endif

#ifdef LOCALE_BRAZIL
	memset(szTemp, 0, 256);
	wsprintf(szTemp, "660970B448FBCB6395356D98440FE8621A6EADB8532B3C5F1949386F921C6C0970FEF0A168B5352668BE414ADF1375136173F493A8A2C075AC0F919AC7241A650F7D04245246E401B574195DD31E6305975703051B9F4F5CA2A8046A5FF3331AB0C8F040AFA98BB5CE3134520AC79D1328E836DF645FC41F2B9A7E");
#endif
}
#ifdef _LAUNCHER
void UpgradeMrsFile()
{
	char temp_path[1024];
	sprintf(temp_path, "*");

	FFileList file_list;
	GetFindFileListWin(temp_path, _EXTFILEMRS_E, file_list);
	file_list.UpgradeMrs();
}
#endif
HANDLE Mutex = 0;

#ifdef _HSHIELD
int __stdcall AhnHS_Callback(long lCode, long lParamSize, void* pParam);
#endif

DWORD g_dwMainThreadID;


//------------------------------------------- nhn usa -------------------------------------------------------------
bool InitReport()
{
	return true;

	// Custom: Disable NHN Auth
	/*
#ifdef LOCALE_NHNUSA
	mlog( "Init report start\n" );
	if( !GetNHNUSAReport().InitReport(((ZNHN_USAAuthInfo*)(ZGetLocale()->GetAuthInfo()))->GetUserID().c_str(),
		((ZNHN_USAAuthInfo*)(ZGetLocale()->GetAuthInfo()))->GetGameStr()) )
	{
		mlog( "Init nhn report fail.\n" );
		return false;
	}
	GetNHNUSAReport().ReportStartGame();
	mlog( "Init report success.\n" );
#endif

	return true;
	*/
}

bool InitPoll()
{
	return true;

	// Custom: Disable NHN Auth
	/*
#ifdef LOCALE_NHNUSA
	mlog( "Init poll start\n" );

	((ZNHN_USAAuthInfo*)(ZGetLocale()->GetAuthInfo()))->ZUpdateGameString();

	if( !GetNHNUSAPoll().ZHanPollInitGameString( ((ZNHN_USAAuthInfo*)(ZGetLocale()->GetAuthInfo()))->GetGameStr()) )
		return false;
#endif

	return true;
	*/
}


bool CheckGameGuardHackToolUser()
{
#ifdef _GAMEGUARD

	string strUserID;
	ZBaseAuthInfo* pAuth = ZGetLocale()->GetAuthInfo();

	if ( pAuth == NULL)
		return true;


#ifdef LOCALE_NHNUSA

#ifdef _DEBUG_NHN_USA_AUTH
	return true;
#endif

	strUserID = ((ZNHN_USAAuthInfo*)pAuth)->GetUserID();

#elif LOCALE_JAPAN

//	strUserID = ((ZGameOnJPAuthInfo*)pAuth)->GetUserID();
	return true;

#endif


	if( !GetZGameguard().CheckHackToolUser( strUserID.c_str()) )
		return false;


#endif	// _GAMEGUARD

	return true;
}


//------------------------------------------- nhn usa end----------------------------------------------------------

BOOL SetDumpPrivileges()
{
	BOOL       fSuccess  = FALSE;
	HANDLE      TokenHandle = NULL;
	TOKEN_PRIVILEGES TokenPrivileges;

	if (!OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
		&TokenHandle))
	{
		mlog("Dmp token fail\n");
		//printf("Could not get the process token");
		goto Cleanup;
	}

	TokenPrivileges.PrivilegeCount = 1;

	if (!LookupPrivilegeValue(NULL,
		SE_DEBUG_NAME,
		&TokenPrivileges.Privileges[0].Luid))
	{
		mlog("Dmp lookup fail\n");
		//printf("Couldn't lookup SeDebugPrivilege name");
		goto Cleanup;
	}

	TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	//Add privileges here.
	if (!AdjustTokenPrivileges(TokenHandle,
		FALSE,
		&TokenPrivileges,
		sizeof(TokenPrivileges),
		NULL,
		NULL))
	{
		mlog("Dmp adjust fail\n");
		//printf("Could not revoke the debug privilege");
		goto Cleanup;
	}

	fSuccess = TRUE;

Cleanup:

	if (TokenHandle)
	{
		CloseHandle(TokenHandle);
	}

	return fSuccess;
}
int PASCAL WinMain(HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow)
{

	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	InitLog(MLOGSTYLE_DEBUGSTRING|MLOGSTYLE_FILE);
#ifdef _EAC
	// Khởi tạo SDK
	EOS_InitializeOptions sdkOptions = { 0 };
	sdkOptions.ApiVersion = EOS_INITIALIZE_API_LATEST;
	sdkOptions.AllocateMemoryFunction = nullptr;
	sdkOptions.ReallocateMemoryFunction = nullptr;
	sdkOptions.ReleaseMemoryFunction = nullptr;
	sdkOptions.ProductName = "GunzVN";            // Phải có, nếu không biết đang làm gì thì đừng thay đổi
	sdkOptions.ProductVersion = "1.0";          // Phải có
	EOS_EResult initRes = EOS_Initialize(&sdkOptions);

	if (initRes != EOS_EResult::EOS_Success) {
		char msg[256];
		sprintf(msg, "Epic Online Services (EOS) Initialize failed: %s", EOS_EResult_ToString(initRes));
		MessageBoxA(NULL, msg, "EAC", MB_OK | MB_ICONERROR);
	}
	mlog("Epic Online Services (EOS) Initialize success.\n");

	// Khởi tạo Platform
	EOS_Platform_Options platformOptions = { 0 };
	platformOptions.ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST;
	platformOptions.ProductId = "a91729130eb949eaa22ef2e3bd7ea5cb";
	platformOptions.SandboxId = "61afb5ef566148b0bfdda3bba8b80407";
	platformOptions.DeploymentId = "3e39e527bef64ad095d5ae52781d3c95";
	platformOptions.ClientCredentials.ClientId = "";
	platformOptions.ClientCredentials.ClientSecret = "";
	platformOptions.bIsServer = EOS_FALSE;
	platformOptions.EncryptionKey = NULL;
	platformOptions.OverrideCountryCode = NULL;
	platformOptions.OverrideLocaleCode = NULL;
	platformOptions.CacheDirectory = NULL;
	platformOptions.TickBudgetInMilliseconds = 0;
	platformOptions.RTCOptions = NULL;
	platformOptions.IntegratedPlatformOptionsContainerHandle = NULL;
	platformOptions.SystemSpecificOptions = NULL;
	platformOptions.TaskNetworkTimeoutSeconds = NULL;

	g_hPlatform = EOS_Platform_Create(&platformOptions);
	if (!g_hPlatform) {
		MessageBoxA(NULL, "Epic Online Services (EOS) Platform Create failed - Check Dev Portal IDs", "Easy Anti-Cheat", MB_OK | MB_ICONERROR);
		EOS_Shutdown();
	}
	mlog("Epic Online Services (EOS) Platform Create success.\n");

	g_hMetrics = EOS_Platform_GetMetricsInterface(g_hPlatform);
	if (g_hMetrics == NULL) {
		mlog("EOS Metrics interface not available (g_hMetrics == NULL)\n");
	}
	else {
		mlog("Epic Online Services (EOS) Metrics interface acquired.\n");
	}

	// Lấy interface AntiCheat Client
	EOS_HAntiCheatClient hAntiCheat = EOS_Platform_GetAntiCheatClientInterface(g_hPlatform);
	if (!hAntiCheat) {
		mlog("Failed to get AntiCheat Client interface. Platform handle: %p\n", (void*)g_hPlatform);
		MessageBoxA(NULL, "Failed to get AntiCheat Client interface - Check EAC setup or Dev Portal", "Easy Anti-Cheat", MB_OK | MB_ICONERROR);
		EOS_Platform_Release(g_hPlatform);
	}
	else {
		mlog("Epic Online Services (EOS) AntiCheat Client interface success. Handle: %p\n =============Easy Anti-Cheat (EAC)==============\n\n", (void*)g_hPlatform);
	}

	CreateThread(NULL, 0, WatchdogThreadProc, NULL, 0, NULL);
#endif

	g_fpOnCrcFail = CrcFailExitApp;

#ifdef LOCALE_JAPAN
	ZGameOnJPAuthInfo::m_hLauncher = ::FindWindow( NULL, TITLE_PUBLAGENT );
#endif

	g_dwMainThreadID = GetCurrentThreadId();
	
#ifdef _MTRACEMEMORY
	MInitTraceMemory();
#endif

	char szModuleFileName[_MAX_DIR] = {0,};
	GetModuleFileName(NULL, szModuleFileName, _MAX_DIR);
	PathRemoveFileSpec(szModuleFileName);
	SetCurrentDirectory(szModuleFileName);

//#define _MULTICLIENT 1

#if defined(_MULTICLIENT)
	if (!fopen("mutex", "r"))
		Mutex = CreateMutex(NULL, TRUE, "Gunz");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBox(g_hWnd, "Cannot Open Gunz.exe", NULL, MB_OK);
		mlog("Cannot Open Gunz.exe\n");
		//zexit(-1);
		ExitProcess(NULL);
		return 0;
	}
#endif															// update sgk 0702 end

	srand( (unsigned)time( NULL ));

	// Custom: Added defined(LOCALE_NHNUSA)
#if defined(LOCALE_BRAZIL) || defined(LOCALE_INDIA) || defined(LOCALE_US) || defined(LOCALE_KOREA) || defined(LOCALE_NHNUSA)
	//#ifndef _DEBUG
	#ifdef _PUBLISH
		
	#endif
#endif	// LOCALE_NHNUSA

	// ·Î±× ½ÃÀÛ
	//mlog("GUNZ " STRFILEVER " launched. build (" __DATE__ " " __TIME__ ") \n");
	//char szDateRun[128]="";
	//char szTimeRun[128]="";
	//_strdate( szDateRun );
	//_strtime( szTimeRun );
	//mlog("Log time (%s %s)\n", szDateRun, szTimeRun);
	//mlog("Coded By Desperate\n"); // Custom: My runnable string

#ifndef _PUBLISH
	mlog("cmdline = %s\n",cmdline);

#endif

#ifdef _LAUNCHER
	UpgradeMrsFile();
#endif

	MSysInfoLog();

	// Custom: Force unsupported OS
	OSVERSIONINFOEX os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx( (OSVERSIONINFO*)&os );

	if (os.dwMajorVersion <= 4)
	{
		mlog("Unsupported OS... exiting.\n");
		return false;
	}

	bool bCompatibilityMode = false;
	char szModule[MAX_PATH] = { 0 };
	GetModuleFileName(GetModuleHandle(NULL), szModule, sizeof(szModule));
	
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers", NULL, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		char szValue[MAX_PATH] = { 0 };
		
		DWORD nLen = _MAX_PATH;
		DWORD dwType = REG_SZ;

		if (RegQueryValueEx(hKey, szModule, NULL, &dwType, (unsigned char *)szValue, &nLen) == ERROR_SUCCESS)
		{
			if (strstr(szValue, "WINXPSP2") || strstr(szValue, "WINXPSP3") || strstr(szValue, "VISTARTM") || strstr(szValue, "VISTASP1") || strstr(szValue, "VISTASP2") || strstr(szValue, "WIN7RTM") || strstr(szValue, "WIN8RTM"))
			{
				bCompatibilityMode = true;
			}
		}

		RegCloseKey(hKey);
	}

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers", NULL, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		char szValue[MAX_PATH] = { 0 };
		
		DWORD nLen = _MAX_PATH;
		DWORD dwType = REG_SZ;

		if (RegQueryValueEx(hKey, szModule, NULL, &dwType, (unsigned char *)szValue, &nLen) == ERROR_SUCCESS)
		{
			if (strstr(szValue, "WINXPSP2") || strstr(szValue, "WINXPSP3") || strstr(szValue, "VISTARTM") || strstr(szValue, "VISTASP1") || strstr(szValue, "VISTASP2") || strstr(szValue, "WIN7RTM") || strstr(szValue, "WIN8RTM"))
			{
				bCompatibilityMode = true;
			}
		}

		RegCloseKey(hKey);
	}

	if (bCompatibilityMode)
	{
		mlog("ERROR: Running under compatibility mode.\n");
		MessageBox(g_hWnd, "Failed to run GunZ in compatibility mode.\nPlease remove compatibility mode settings and launch the game again!", "GunZ", MB_OK | MB_ICONERROR);
		return false;
	}

//	if (CheckVideoAdapterSupported() == false)
//		return 0;

	CheckFileAssociation();

	// Initialize MZFileSystem - MUpdate 
	MRegistry::szApplicationName=APPLICATION_NAME;

	g_App.InitFileSystem();

//	mlog("CheckSum: %u \n", ZApplication::GetFileSystem()->GetTotalCRC());

	//if(!InitializeMessage(ZApplication::GetFileSystem())) {
	//	MLog("Check Messages.xml\n");
	//	return 0;
	//}

//	³Ý¸¶ºE¹öÀEº ±¸ºÐÇØ¾ßÇÔ... ³Ý¸¶ºE¹öÀEº MZIPREADFLAG_MRS1 µµ ÀÐ¾ûÚßÇÔ...

#ifdef _PUBLISH
//	#ifndef NETMARBLE_VERSION
		//MZFile::SetReadMode( MZIPREADFLAG_MRS2 );
//	#endif
#endif


#ifdef LOCALE_NHNUSA
	// NHNUSA´Â Ä¿¸Çµå¶óÀÎÀ¸·Î ¾ð¾ûØ±ÅÃÀ» ¾Ë·ÁÁØ´Ù, ´Ù¸¥ Áö¿ªºôµå¶óµµ µðÆÄÀÎ ÇÊ¿ä¾øÀ½
	ZGetLanguageSetting_forNHNUSA()->SetLanguageIndexFromCmdLineStr(cmdline);
#endif

	CreateRGMain();

	ZGetConfiguration()->Load();

	/*if (os.dwMajorVersion >= 6 && Z_ETC_D3D9E)
	{
		g_isDirect3D9ExEnabled = true;
	}*/

	ZStringResManager::MakeInstance();
	if( !ZApplication::GetInstance()->InitLocale() )
	{
		MLog("Locale Init error !!!\n");
		return false;
	}

	ZGetConfiguration()->Load_StringResDependent();

	// ¿©±â¼­ ¸ÞÅ©·Î ÄÁ¹öÆÃ... ¸Õ°¡ ±¸¸®±¸¸®~~ -by SungE.
	if( !ZGetConfiguration()->LateStringConvert() )
	{
		MLog( "main.cpp - Late string convert fale.\n" );
		return false;
	}

	DWORD ver_major = 0;
	DWORD ver_minor = 0;
	TCHAR ver_letter = ' ';

	// ÀÇ¹Ì¾øÀ½ ... ¿ÜºÎ¿¡¼­ dll ÀÌ ¾ø´Ù°E¸ÕÀú¶E..

/*_
	bool DXCheck = false;

	if( SUCCEEDED( GetDirectXVersionViaDxDiag( &ver_major, &ver_minor, &ver_letter ) ) ) {
		if(ver_major >= 8)
			DXCheck = true;
	} // ¼º°E¸øÇÑ °æ¿E¾Ë¼ö¾øÀ¸¹Ç·Î ½ÇÆÐ~

	if(DXCheck==false) {
		::MessageBox(NULL,"DirectX 8.0 ÀÌ»óÀ» ¼³Ä¡ÇÏ°E´Ù½Ã ½ÇÇàÇØ ÁÖ½Ã±E¹Ù¶ø´Ï´Ù.","¾Ë¸²",MB_OK);
	}
*/

#ifdef SUPPORT_EXCEPTIONHANDLING
	if (SetDumpPrivileges())
		mlog(" =================Load XML Files=================\n\n");
	else
		mlog("Dmp - FAILED\n");

	char szDumpFileName[256];
	sprintf(szDumpFileName, "Gunz.dmp");
	__try{
#endif
	if (ZApplication::GetInstance()->ParseArguments(cmdline) == false)
	{
		// Korean or Japan Version
		if ((ZGetLocale()->GetCountry() == MC_KOREA) || (ZGetLocale()->GetCountry() == MC_JAPAN))
		{
			mlog("Routed to Website \n");

			ShellExecute(NULL, "open", ZGetConfiguration()->GetLocale()->szHomepageUrl, NULL, NULL, SW_SHOWNORMAL);

			char szMsgWarning[128]="";
			char szMsgCertFail[128]="";
			ZTransMsg(szMsgWarning,MSG_WARNING);
			ZTransMsg(szMsgCertFail,MSG_REROUTE_TO_WEBSITE);
//			MessageBox(g_hWnd, szMsgCertFail, szMsgWarning, MB_OK);

			mlog(szMsgWarning);
			mlog(" : ");
			mlog(szMsgCertFail);

			return 0;
		}
		else
		{
			return 0;
		}
	}

//#ifdef _PUBLISH
	// if(!CheckFileList()) {
		// Á¾·áÇÏ´Â°ÍÀº ÀÏ´Ü º¸·E
		// int ret=MessageBox(NULL, "ÆÄÀÏÀÌ ¼Õ»óµÇ¾ú½À´Ï´Ù.", "Áß¿E", MB_OK);
		// return 0;
	//}
//#endif

	// Custom: Disable NHN Auth
	/*
#ifdef LOCALE_NHNUSA
	BYTE SHA_HanAuthForClient[20] = {0x29,0xc0,0x7e,0x6b,0x8d,0x1d,0x30,0xd2,0xed,0xac,0xaf,0xea,0x78,0x16,0x51,0xf0,0x50,0x52,0x26,0x91};
	BYTE SHA_hanpollforclient[20] = {0x09,0x04,0x51,0x9d,0x95,0xbb,0x66,0x2a,0xfb,0x93,0x87,0x2d,0x21,0xa2,0x93,0x1d,0x6a,0xcb,0xa5,0x4f};
	BYTE SHA_HanReportForClient[20] = {0x4c,0x62,0xaf,0x4d,0x5b,0x54,0xb8,0x96,0x46,0x66,0x8f,0x1e,0x12,0xe7,0xf2,0xd7,0xe4,0x58,0x65,0xc9}; 
	if(!CheckDll("hanauthforclient.dll", SHA_HanAuthForClient) || 
	   !CheckDll("hanpollforclient.dll", SHA_hanpollforclient) ||
	   !CheckDll("hanreportforclient.dll", SHA_HanReportForClient) )
	{
		MessageBox(g_hWnd,"Dll Hacking detected",  NULL, MB_OK);
		return false;
	}

	if( !InitReport() ) 
		return 0;


	if ( !InitPoll())
		return 0;
#endif
	*/



#ifdef _EAC_FILE
	BYTE SHA_basebin[20] = { 0x31,0x11,0x3f,0x41,0x4b,0xb9,0x2d,0xd7,0x89,0xbe,0xfc,0x98,0x37,0x84,0x6b,0x59,0x02,0x59,0x65,0x66 };
	BYTE SHA_basecer[20] = { 0x31,0x11,0x3f,0x41,0x4b,0xb9,0x2d,0xd7,0x89,0xbe,0xfc,0x98,0x37,0x84,0x6b,0x59,0x02,0x59,0x65,0x66 };
	BYTE SHA_runtime[20] = { 0x31,0x11,0x3f,0x41,0x4b,0xb9,0x2d,0xd7,0x89,0xbe,0xfc,0x98,0x37,0x84,0x6b,0x59,0x02,0x59,0x65,0x66 };
	if(!CheckDll("EasyAntiCheat\\Certificates\\base.bin", SHA_basebin) ||
		!CheckDll("EasyAntiCheat\\Certificates\\base.cer", SHA_basecer) ||
		!CheckDll("EasyAntiCheat\\Certificates\\runtime.conf", SHA_runtime))
	{
		MessageBox(g_hWnd,"Error validating EasyAntiCheat code signing certificate.", "Easy Anti-Cheat", MB_OK | MB_ICONERROR);
		return false;
	}
#endif

// Custom: Made hack check automatic (w/o debug) and enabled for LOCALE_NHNUSA
	if (ZCheckHackProcess() == true)
	{
		//		MessageBox(NULL,
		//			ZMsg(MSG_HACKING_DETECTED), ZMsg( MSG_WARNING), MB_OK);
		return 0;
	}

	if(!InitializeNotify(ZApplication::GetFileSystem())) 
	{
		MLog("Check notify.xml\n");
		return 0;
	}
	else 
	{
		mlog( "InitializeNotify ok.\n" );
	}



	if(CheckFont()==false) {
		MLog("ÆùÆ®°¡ ¾ø´Â À¯Àú°¡ ÆùÆ® ¼±ÅÃÀ» ÃEÒ\n");
		return 0;
	}

	RSetFunction(RF_CREATE	,	OnCreate);
	RSetFunction(RF_RENDER	,	OnRender);
	RSetFunction(RF_UPDATE	,	OnUpdate);
	RSetFunction(RF_DESTROY ,	OnDestroy);
	RSetFunction(RF_INVALIDATE,	OnInvalidate);
	RSetFunction(RF_RESTORE,	OnRestore);
	RSetFunction(RF_ACTIVATE,	OnActivate);
	RSetFunction(RF_DEACTIVATE,	OnDeActivate);
	RSetFunction(RF_ERROR,		OnError);

	SetModeParams();

//	while(ShowCursor(FALSE)>0);

	const int nRMainReturn = RMain(APPLICATION_NAME,this_inst,prev_inst,cmdline,cmdshow,&g_ModeParams,WndProc,IDI_ICON1);
	if( 0 != nRMainReturn )
		return nRMainReturn;


#ifdef _GAMEGUARD
	mlog("start gameguard\n");

	ZGMAEGUARD_MODE mode = ZGGM_END;
	char szArg[ 64] = "";
	ZBaseAuthInfo* pAuth = ZGetLocale()->GetAuthInfo();
	

#ifdef LOCALE_NHNUSA
	if ( ((ZNHN_USAAuthInfo*)pAuth)->IsReal())
	{
		mode = ZGGM_REAL;
		strcpy( szArg, "GunzUS");
	}
	else if ( ((ZNHN_USAAuthInfo*)pAuth)->IsAlpha())
	{
		mode = ZGGM_ALPHA;
		strcpy( szArg, "GunzUSTest");
	}
	else
	{
		mlog( "error in gameguard mode...\n" );
		zexit( -1);
		return 0;
	}

#elif LOCALE_JAPAN
	if ( ((ZGameOnJPAuthInfo*)pAuth)->IsReal())
	{
		mode = ZGGM_REAL;
		strcpy( szArg, "GUNZWEI");
	}
	else if ( ((ZGameOnJPAuthInfo*)pAuth)->IsAlpha())
	{
		mode = ZGGM_ALPHA;
		strcpy( szArg, "GUNZWEITest");
	}
	else
	{
		mlog( "error in gameguard mode...\n" );
		zexit( -1);
		return 0;
	}

	if( !((ZGameOnJPAuthInfo*)pAuth)->SendMsgToLauncher(GET_MSG_HWND_TERMINATE_PUBGAME) )
	{
		mlog("Can't find GameOn Agent");
	}
#endif	// LOCALE_JAPAN


	if( !GetZGameguard().Init( mode, szArg) )
	{
		mlog( "error init gameguard...\n" );
		zexit( -1 );
		return 0;
	}


	GetZGameguard().SetMainWnd( g_hWnd );

	if( !CheckGameGuardHackToolUser() )
		return 0;

#endif	// _GAMEGUARD


#ifdef _GAMEGUARD
	#ifdef _PUBLISH
		// Áßº¹ ½ÇÇE±ÝÁE
		Mutex = CreateMutex(NULL, TRUE, "Gunz");
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			zexit(-1);
			return 0;
		}
	#endif
#endif

#ifdef LOCALE_NHNUSA
	// Custom: Disable NHN related
	//GetNHNUSAReport().ReportInitGameGuard();
#endif

	if( 0 != RInitD3D(&g_ModeParams) )
	{
		MessageBox(g_hWnd, "fail to initialize DirectX", NULL, MB_OK);
		mlog( "error init RInitD3D\n" );
		return 0;
	}

	const int nRRunReturn = RRun();

	//Á¾·á½Ã °ÔÀÓ°¡µå°¡ XfireÀÇ ¸Þ¸ð¸®¾²±E¿¡·¯¸¦ À¯¹ßÇÏ´Âµ¥ ÀÌ¶§ ¿À·ùÃ¢ÀÌ Ç®½ºÅ©¸° µÚ¿¡ ¶ß´Â °Í ¹æÁöÇÏ±EÀ§ÇØ
	//Á¾·áÀE¡ °ÇÁûÔ¦ ÃÖ¼ÒÈ­/ºñÈ°¼ºÈ­ ½ÃÄÑ³õ´Â´Ù. xfireÀÇ ÁE¢ÀûÀÎ ¹®Á¦ ÇØ°áÀ» ±â´EÏ±E¾ûÓÁ¿EÇ·Î ÀÌ·¸°Ô Ã³¸®
	ShowWindow(g_hWnd, SW_MINIMIZE);

	g_bThreadChecker = false;
	//ahCleanup(43320);

#ifdef _GAMEGUARD
	GetZGameguard().Release();
#endif

#ifdef _MTRACEMEMORY
	MShutdownTraceMemory();
#endif

#ifdef _HSHIELD
	_AhnHS_StopService();
	_AhnHS_Uninitialize();		
#endif


#ifdef LOCALE_NHNUSA
	// Custom: Disable NHN poll
	//mlog( "Poll Process\n" );
	//int nRetPoll = GetNHNUSAPoll().ZHanPollProcess( NULL);
#endif

	ZStringResManager::FreeInstance();

	return nRRunReturn;

//	ShowCursor(TRUE);

#ifdef SUPPORT_EXCEPTIONHANDLING
	}

	//__except(MFilterException(GetExceptionInformation())){
	__except(CrashExceptionDump(GetExceptionInformation(), szDumpFileName, true))
	{
#ifdef LOCALE_NHNUSA
		// Custom: Disable NHN related
		//GetNHNUSAReport().ReportCrashedGame();
#endif

		HandleExceptionLog();
//		MessageBox(g_hWnd, "¿¹»óÄ¡ ¸øÇÑ ¿À·ù°¡ ¹ß»ýÇß½À´Ï´Ù.", APPLICATION_NAME , MB_ICONERROR|MB_OK);
	}
#endif

#ifdef _PUBLISH
	//if (Mutex != 0) CloseHandle(Mutex);
#endif

//	CoUninitialize();

	return 0;
}

#ifdef _HSHIELD
int __stdcall AhnHS_Callback(long lCode, long lParamSize, void* pParam)
{
//	TCHAR szTitle[256];

	switch(lCode)
	{
		//Engine Callback
	case AHNHS_ENGINE_DETECT_GAME_HACK:
		{
			TCHAR szMsg[255];
			wsprintf(szMsg, _T("´ÙÀ½ À§Ä¡¿¡¼­ ÇØÅ·ÅøÀÌ ¹ß°ßµÇ¾EÇÁ·Î±×·¥À» Á¾·á½ÃÄ×½À´Ï´Ù.\n%s"), (char*)pParam);
//			MessageBox(NULL, szMsg, szTitle, MB_OK);
			mlog(szMsg);
			PostThreadMessage(g_dwMainThreadID, WM_QUIT, 0, 0);
			break;
		}

		//ÀÏºÎ API°¡ ÀÌ¹Ì ÈÄÅ·µÇ¾EÀÖ´Â »óÅÂ
		//±×·¯³ª ½ÇÁ¦·Î´Â ÀÌ¸¦ Â÷´ÜÇÏ°EÀÖ±E¶§¹®¿¡ ´Ù¸¥ ÈÄÅ·½Ãµµ ÇÁ·Î±×·¥Àº µ¿ÀÛÇÏÁE¾Ê½À´Ï´Ù.
		//ÀÌ CallbackÀº ´ÜÁE°æ°E³»Áö´Â Á¤º¸Á¦°EÂ÷¿ø¿¡¼­ Á¦°øµÇ¹Ç·Î °ÔÀÓÀ» Á¾·áÇÒ ÇÊ¿ä°¡ ¾ø½À´Ï´Ù.
	case AHNHS_ACTAPC_DETECT_ALREADYHOOKED:
		{
			PACTAPCPARAM_DETECT_HOOKFUNCTION pHookFunction = (PACTAPCPARAM_DETECT_HOOKFUNCTION)pParam;
			TCHAR szMsg[255];
			wsprintf(szMsg, _T("[HACKSHIELD] Already Hooked\n- szFunctionName : %s\n- szModuleName : %s\n"), 
				pHookFunction->szFunctionName, pHookFunction->szModuleName);
			OutputDebugString(szMsg);
			break;
		}

		//Speed °EÃ
	case AHNHS_ACTAPC_DETECT_SPEEDHACK:
	case AHNHS_ACTAPC_DETECT_SPEEDHACK_APP:
		{
//			MessageBox(NULL, _T("ÇöÀEÀÌ PC¿¡¼­ SpeedHackÀ¸·Î ÀÇ½ÉµÇ´Â µ¿ÀÛÀÌ °¨ÁöµÇ¾ú½À´Ï´Ù."), szTitle, MB_OK);
			mlog("ÇöÀEÀÌ PC¿¡¼­ SpeedHackÀ¸·Î ÀÇ½ÉµÇ´Â µ¿ÀÛÀÌ °¨ÁöµÇ¾ú½À´Ï´Ù.");
			PostThreadMessage(g_dwMainThreadID, WM_QUIT, 0, 0);
			break;
		}

		//µð¹ö±E¹æÁE
	case AHNHS_ACTAPC_DETECT_KDTRACE:	
	case AHNHS_ACTAPC_DETECT_KDTRACE_CHANGED:
		{
			TCHAR szMsg[255];
			wsprintf(szMsg, _T("ÇÁ·Î±×·¥¿¡ ´EÏ¿© µð¹ö±E½Ãµµ°¡ ¹ß»ýÇÏ¿´½À´Ï´Ù. (Code = %x)\nÇÁ·Î±×·¥À» Á¾·áÇÕ´Ï´Ù."), lCode);
//			MessageBox(NULL, szMsg, szTitle, MB_OK);
			mlog(szMsg);
			PostThreadMessage(g_dwMainThreadID, WM_QUIT, 0, 0);
			break;
		}

		//±×¿Ü ÇØÅ· ¹æÁE±â´É ÀÌ»E
	case AHNHS_ACTAPC_DETECT_AUTOMOUSE:
	case AHNHS_ACTAPC_DETECT_DRIVERFAILED:
	case AHNHS_ACTAPC_DETECT_HOOKFUNCTION:
	case AHNHS_ACTAPC_DETECT_MESSAGEHOOK:
	case AHNHS_ACTAPC_DETECT_MODULE_CHANGE:
		{
			TCHAR szMsg[255];
			wsprintf(szMsg, _T("ÇØÅ· ¹æ¾E±â´É¿¡ ÀÌ»óÀÌ ¹ß»ýÇÏ¿´½À´Ï´Ù. (Code = %x)\nÇÁ·Î±×·¥À» Á¾·áÇÕ´Ï´Ù."), lCode);
//			MessageBox(NULL, szMsg, szTitle, MB_OK);
			mlog(szMsg);
			PostThreadMessage(g_dwMainThreadID, WM_QUIT, 0, 0);
			break;
		}
	}

	return 1;
}
#endif

void drawDebugText(int x, int y, const char * str)
{
    MFontR2* pFont = (MFontR2*)MFontManager::Get(NULL);
    pFont->m_Font.BeginFont();
    pFont->m_Font.DrawText(x, y, str);
    pFont->m_Font.EndFont();  
}
