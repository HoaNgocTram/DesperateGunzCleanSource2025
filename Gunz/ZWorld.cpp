#include "stdafx.h"
#include "ZWorld.h"
#include "ZMap.h"
#include "ZMapDesc.h"
#include "ZSkyBox.h"
#include "ZInitialLoading.h"
#include "ZConfiguration.h"
#include "ZMapCache.h"
#include "ZWorldObject.h"
#include "ZWorldObject_Movable.h"
#include "ZWorldObject_Navigation.h"
#include <memory>

ZWorld::ZWorld() : m_pBsp(NULL), m_pMapDesc(NULL), m_nRefCount(1), m_bCreated(false)
{
	m_szName[0] = 0;

	m_bFog = false;
	m_dwFogColor = 0xFFFFFFFF;
	m_fFogNear = 0;
	m_fFogFar = 0;

	m_bWaterMap = false;
	m_fWaterHeight = 0.f;
	m_szBspName[0] = 0;

}

ZWorld::~ZWorld()
{
	Destroy();
}

void ZWorld::Update(float fDelta)
{
	if (m_bWaterMap)
		m_waters.Update();

	m_flags.Update();

	((RBspObject*)m_pBsp)->OnUpdate(fDelta);
}

void ZWorld::Draw()
{
	if (m_bFog) 
	{
		ComputeZPlane(RGetViewFrustum() + 5, m_fFogFar, -1);
	}
	m_pBsp->Draw();
	RealSpace2::g_poly_render_cnt = 0;

	__BP(16, "ZGame::Draw::flags");
	m_flags.Draw();
	__EP(16);
}

void ZWorldProgressCallBack(void *pUserParam,float fProgress)
{
	ZLoadingProgress *pLoadingProgress = (ZLoadingProgress*)pUserParam;
	pLoadingProgress->UpdateAndDraw(fProgress);
}

bool ZWorld::Create(ZLoadingProgress* pLoading)
{
	if (m_bCreated) return true;
	m_pBsp = new RBspObject;

	if (!m_pBsp->Open(m_szBspName, "xml", ROF_RUNTIME, ZWorldProgressCallBack, pLoading))
	{
		MLog("error while loading %s \n", m_szName);
		ZGetWorldManager()->Clear();
		return false;
	}

	m_pBsp->OptimizeBoundingBox();

	//Custom: Lightmaps Time Code
	if (ZGetGameClient()->GetMatchStageSetting()->IsQuestDrived() || ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_QUEST_CHALLENGE)
	{
		m_pBsp->SetLightMapIndex(0);
	}
	else
	{
		m_pBsp->SetLightMapIndex(ZGetGameClient()->GetMatchStageSetting()->GetLightMapIndex());
	}


	//if (0 == stricmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Labyrinth"))
	//	ZGetGame()->GetWorld()->GetBsp()->LightMapOnOff(0);
	//else
	//	RBspObject::SetDrawLightMap(true);
	//return 0;


	char szMapPath[64] = "";
	ZGetCurrMapPath(szMapPath);

	ZWater* water_instance;
	RMapObjectList* map_object_list = m_pBsp->GetMapObjectList();
	RMeshMgr* mesh_mgr = m_pBsp->GetMeshManager();

	for (auto iter = map_object_list->m_MapObjectList.begin(); iter != map_object_list->m_MapObjectList.end(); )
	{
		ROBJECTINFO* object_info = *iter;
		RMesh* pMesh = mesh_mgr->GetFast(object_info->nMeshID);
		if (pMesh->m_data_num <= 0)
		{
			++iter;
			continue;
		}
		RMeshNode* pMeshNode = pMesh->m_data[0];

		char* object_name = (char*)object_info->name.c_str();

		int len = int(strlen(m_szName) + 1);
		object_name += len;

		if (pMeshNode->m_point_color_num > 0) 
		{
			ZClothEmblem* new_instance = new ZClothEmblem;
			new_instance->CreateFromMeshNode(pMeshNode, this);
			m_flags.Add(new_instance, object_name);
			iter = map_object_list->Delete(iter);
			continue;
		}

		{	
			int nWater = 0;

			if (!strncmp(object_name, "obj_water", 9))	nWater = 1;
			if (!strncmp(object_name, "obj_water2", 10))	nWater = 3;
			if (!strncmp(object_name, "obj_sea", 7))		nWater = 2;

			if (nWater) {
				m_bWaterMap = true;
				m_fWaterHeight = pMeshNode->m_mat_base._42;
			}
			else {
				m_bWaterMap = false;
				m_fWaterHeight = 0.f;
			}

			if (nWater)
			{
				int id = object_info->nMeshID;

				RMesh* mesh = mesh_mgr->GetFast(id);
				RMeshNode* node = mesh->m_data[0];

				water_instance = new ZWater;

				water_instance->SetMesh(node);
				m_waters.push_back(water_instance);

				if (nWater == 1) water_instance->m_nWaterType = WaterType1;
				else if (nWater == 3) water_instance->m_nWaterType = WaterType2;


				if (nWater == 2)
				{
					water_instance->m_isRender = false; 
					pMesh->m_LitVertexModel = true;	
					++iter;
				}
				else
				{
					iter = map_object_list->Delete(iter);
				}
				continue;
			}
		}

		++iter;
	}

	char szBuf[128];

	if (m_flags.size() > 0)
	{
		sprintf(szBuf, "%s%s/flag.xml", szMapPath, ZGetGameClient()->GetMatchStageSetting()->GetMapName());
		m_flags.InitEnv(szBuf);
		mlog("create game enrvironment.\n");
	}

	m_pMapDesc = new ZMapDesc;
	m_pMapDesc->Open(m_pBsp);

	sprintf(szBuf, "%s%s/smoke.xml", szMapPath, ZGetGameClient()->GetMatchStageSetting()->GetMapName());
	m_pMapDesc->LoadSmokeDesc(szBuf);

	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_INFECTED)
	{
		m_bFog = true;
		m_fFogNear = 0;
		m_fFogFar = 5000;
		// Custom: red fog
		m_dwFogColor = 0xffff0000;
	}
	else if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_VAMPIRE)
	{
		m_bFog = true;
		m_fFogNear = 0.0f;
		m_fFogFar = 6000.0f;
		m_dwFogColor = 0xC30404;
	}
	else if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_SNIPERMODE)
	{
		m_bFog = true;
		m_fFogNear = 20.8f;
		m_fFogFar = 9000.0f;
		m_dwFogColor = 0xCCF1FF;
	}
	else if (ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->bXmas == true)
	{
		m_bFog = true;
		m_fFogNear = 0;
		m_fFogFar = 5000;
		m_dwFogColor = 0xFF00AEFF;
	}
	else if (ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->bFog == true)
	{
		m_bFog = true;
		m_fFogNear = 20.8f;
		m_fFogFar = 9000.0f;
		m_dwFogColor = 0xCCF1FF;
	}
	else if (ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->bHall == true)
	{
		m_bFog = true;
		m_fFogNear = 0.0f;
		m_fFogFar = 6000.0f;
		m_dwFogColor = 0xFFB00FF;
	}
	else
	{
		FogInfo finfo = GetBsp()->GetFogInfo();
		m_bFog = finfo.bFogEnable;
		m_fFogNear = finfo.fNear;
		m_fFogFar = finfo.fFar;
		m_dwFogColor = finfo.dwFogColor;
	}

	m_bCreated = true;

	mlog( "game world create success.\n" );

	return true;
}

void ZWorld::Destroy()
{
	SAFE_DELETE(m_pBsp);

	SAFE_DELETE(m_pMapDesc);

	m_flags.Clear();
	m_flags.OnInvalidate();
	m_waters.Clear();

}

void ZWorld::OnInvalidate()
{
	m_pBsp->OnInvalidate();
	m_flags.OnInvalidate();
}

void ZWorld::OnRestore()
{
	m_pBsp->OnRestore();
	m_flags.OnRestore();
}

void ZWorld::SetFog(bool bFog)
{
	if (bFog) {
		RSetFog(m_bFog, m_fFogNear, m_fFogFar, m_dwFogColor);
	}
	else {
		RSetFog(FALSE);
	}
	if (ZGetGame() && ZGetConfiguration()->GetEtc()->bNoFog)
	{
		RSetFog(FALSE);
	}
}