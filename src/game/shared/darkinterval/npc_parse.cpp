//========================================================================//
//
// Purpose: Weapon data file parsing, shared by game & client dlls.
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include <keyvalues.h>
#include <tier0/mem.h>
#include "filesystem.h"
#include "utldict.h"
#include "npc_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CUtlDict< FileNPCInfo_t*, unsigned short > m_NPCInfoDatabase;

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *name - 
// Output : FileWeaponInfo_t
//-----------------------------------------------------------------------------
static NPC_FILE_INFO_HANDLE FindNPCInfoSlot(const char *name)
{
	// Complain about duplicately defined metaclass names...
	unsigned short lookup = m_NPCInfoDatabase.Find(name);
	if (lookup != m_NPCInfoDatabase.InvalidIndex())
	{
		return lookup;
	}

	FileNPCInfo_t *insert = CreateNPCInfo();

	lookup = m_NPCInfoDatabase.Insert(name, insert);
	Assert(lookup != m_NPCInfoDatabase.InvalidIndex());
	return lookup;
}

NPC_FILE_INFO_HANDLE LookupNPCInfoScript(const char *name)
{
	return m_NPCInfoDatabase.Find(name);
}

static FileNPCInfo_t gNullNPCInfo;

FileNPCInfo_t *GetFileNPCInfoFromHandle(NPC_FILE_INFO_HANDLE handle)
{
	if (handle < 0 || handle >= m_NPCInfoDatabase.Count())
	{
		return &gNullNPCInfo;
	}

	if (handle == m_NPCInfoDatabase.InvalidIndex())
	{
		return &gNullNPCInfo;
	}

	return m_NPCInfoDatabase[handle];
}

NPC_FILE_INFO_HANDLE GetInvalidNPCInfoHandle(void)
{
	return (NPC_FILE_INFO_HANDLE)m_NPCInfoDatabase.InvalidIndex();
}

#if 0
void ResetFileNPCInfoDatabase(void)
{
	int c = m_NPCInfoDatabase.Count();
	for (int i = 0; i < c; ++i)
	{
		delete m_NPCInfoDatabase[i];
	}
	m_NPCInfoDatabase.RemoveAll();
}
#endif
void PrecacheFileNPCInfoDatabase(IFileSystem *filesystem, const unsigned char *pICEKey)
{
	if (m_NPCInfoDatabase.Count())
		return;

	KeyValues *manifest = new KeyValues("NpcScripts");
	if (manifest->LoadFromFile(filesystem, "scripts/npc_manifest.txt", "GAME"))
	{
		for (KeyValues *sub = manifest->GetFirstSubKey(); sub != NULL; sub = sub->GetNextKey())
		{
			if (!Q_stricmp(sub->GetName(), "file"))
			{
				ConColorMsg(Color(255, 225, 25, 200), "Extracting NPC manifest entry\n");
				char fileBase[512];
				Q_FileBase(sub->GetString(), fileBase, sizeof(fileBase));
				NPC_FILE_INFO_HANDLE tmp;
				ReadNPCDataFromFileForSlot(filesystem, fileBase, &tmp, pICEKey);
			}
			else
			{
				Error("Expecting 'file', got %s\n", sub->GetName());
			}
		}
	}
	manifest->deleteThis();
}

KeyValues* ReadEncryptedNPCFile(IFileSystem *filesystem, const char *szFilenameWithoutExtension,
	const unsigned char *pICEKey, bool bForceReadEncryptedFile /*= false*/)
{
	Assert(strchr(szFilenameWithoutExtension, '.') == NULL);
	char szFullName[512];

	const char *pSearchPath = "MOD";

	if (pICEKey == NULL)
	{
		pSearchPath = "GAME";
	}

	// Open the weapon data file, and abort if we can't
	KeyValues *pKV = new KeyValues("NPCDatafile");

	Q_snprintf(szFullName, sizeof(szFullName), "%s.txt", szFilenameWithoutExtension);

	if (bForceReadEncryptedFile || !pKV->LoadFromFile(filesystem, szFullName, pSearchPath)) // try to load the normal .txt file first
	{
		pKV->deleteThis();
		return NULL;
	}

	return pKV;
}

bool ReadNPCDataFromFileForSlot(IFileSystem* filesystem, const char *szNPCName, NPC_FILE_INFO_HANDLE *phandle, const unsigned char *pICEKey)
{
	ConColorMsg(Color(255, 225, 25, 200), "Extracting NPC data from file for handle %s\n", MAKE_STRING(szNPCName));
	if (!phandle)
	{
	//	Msg("No handle!\n");
		Assert(0);
		return false;
	}

	*phandle = FindNPCInfoSlot(szNPCName);
	FileNPCInfo_t *pFileInfo = GetFileNPCInfoFromHandle(*phandle);
	Assert(pFileInfo);

	if (pFileInfo->NPC_Script_Parsed_boolean)
		return true;

	char sz[256];
	Q_snprintf(sz, sizeof(sz), "scripts/npc_gamedata/%s", szNPCName);

	KeyValues *pKV = ReadEncryptedNPCFile(filesystem, sz, pICEKey, false);

	if (!pKV)
		return false;

	pFileInfo->Parse(pKV, szNPCName);

	pKV->deleteThis();

	return true;
}

FileNPCInfo_t::FileNPCInfo_t()
{
	NPC_Script_Parsed_boolean = false;
	NPC_ClassName_char[0] = 0;
	NPC_Model_Path_char[0] = 0;
	NPC_Model_SkinMin_int = 0;
	NPC_Model_SkinMax_int = 0;
	NPC_Model_ScaleMin_float = 1.00f;
	NPC_Model_ScaleMax_float = 1.00f;
	NPC_AltModel_Path_char[0] = 0;
	NPC_AltModel_SkinMin_int = 0;
	NPC_AltModel_SkinMax_int = 0;
	NPC_AltModel_ScaleMin_float = 1.00f;
	NPC_AltModel_ScaleMax_float = 1.00f;
	NPC_AltModel_Chance_float = 0.00f;
	NPC_Stats_Health_int = 0;
	NPC_Stats_MaxHealth_int = 0;
	NPC_Stats_HullType_int = int(HULL_HUMAN);
	NPC_Stats_BloodColor_int = int(BLOOD_COLOR_RED);
	NPC_Stats_FOV_float = VIEW_FIELD_WIDE;
	NPC_Movement_MoveType_int = int(MOVETYPE_STEP);
	NPC_Movement_StepSize_float = 0.0f;
	NPC_Capable_Jump_boolean = false;
	NPC_Capable_Climb_boolean = false;
	NPC_Capable_Doors_boolean = true;
	NPC_Capable_WeaponRangeAttack1_boolean = false;
	NPC_Capable_WeaponRangeAttack2_boolean = false;
	NPC_Capable_InnateRangeAttack1_boolean = false;
	NPC_Capable_InnateRangeAttack2_boolean = false;
	NPC_Capable_MeleeAttack1_boolean = false;
	NPC_Capable_Climb_boolean = false;
	NPC_Capable_Squadslots_boolean = false;
	NPC_Behavior_PatrolAfterSpawn_boolean = false;
	NPC_DisableShadows_boolean = false;
}

void FileNPCInfo_t::Parse(KeyValues *pKeyValuesData, const char *szNPCName)
{
	ConColorMsg(Color(255, 225, 25, 200), "Performing parsing pass for NPC %s\n", MAKE_STRING(szNPCName));
	// Okay, we tried at least once to look this up...
	NPC_Script_Parsed_boolean = true;
	// Classname
	Q_strncpy(NPC_ClassName_char, szNPCName, 255);
	// World model
	Q_strncpy(NPC_Model_Path_char, pKeyValuesData->GetString("ModelName"), 255);
	NPC_Model_SkinMin_int = pKeyValuesData->GetInt("SkinMin", 0);
	NPC_Model_SkinMax_int = pKeyValuesData->GetInt("SkinMax", 0);
	NPC_Model_ScaleMin_float = pKeyValuesData->GetFloat("ScaleMin", 1.00f);
	NPC_Model_ScaleMax_float = pKeyValuesData->GetFloat("ScaleMax", 1.00f);
	Q_strncpy(NPC_AltModel_Path_char, pKeyValuesData->GetString("AltModelName"), 255);
	NPC_AltModel_SkinMin_int = pKeyValuesData->GetInt("AltSkinMin", 0);
	NPC_AltModel_SkinMax_int = pKeyValuesData->GetInt("AltSkinMax", 0);
	NPC_AltModel_ScaleMin_float = pKeyValuesData->GetFloat("AltScaleMin", 1.00f);
	NPC_AltModel_ScaleMax_float = pKeyValuesData->GetFloat("AltScaleMax", 1.00f);
	NPC_AltModel_Chance_float = pKeyValuesData->GetFloat("AltModelChance", 0.00f);
	NPC_Stats_Health_int = pKeyValuesData->GetInt("Health", 50);
	NPC_Stats_MaxHealth_int = pKeyValuesData->GetInt("MaxHealth", 50);
	NPC_Stats_HullType_int = pKeyValuesData->GetInt("HullType", 0);
	NPC_Stats_BloodColor_int = pKeyValuesData->GetInt("BloodColor", 0);
	NPC_Movement_MoveType_int = pKeyValuesData->GetInt("MoveType", 0);
	NPC_Movement_StepSize_float = pKeyValuesData->GetFloat("StepSize", 18.0f);
	NPC_Stats_FOV_float = pKeyValuesData->GetFloat("FieldOfView", VIEW_FIELD_WIDE);
	NPC_Capable_Jump_boolean = pKeyValuesData->GetBool("CanJump", 0);
	NPC_Capable_Climb_boolean = pKeyValuesData->GetBool("CanClimb", 0);
	NPC_Capable_Doors_boolean = pKeyValuesData->GetBool("CanOpenDoors", 0);
	NPC_Capable_WeaponRangeAttack1_boolean = pKeyValuesData->GetBool("CanWeaponRangeAttack1", 0);
	NPC_Capable_WeaponRangeAttack2_boolean = pKeyValuesData->GetBool("CanWeaponRangeAttack2", 0);
	NPC_Capable_InnateRangeAttack1_boolean = pKeyValuesData->GetBool("CanInnateRangeAttack1", 0);
	NPC_Capable_InnateRangeAttack2_boolean = pKeyValuesData->GetBool("CanInnateRangeAttack2", 0);
	NPC_Capable_MeleeAttack1_boolean = pKeyValuesData->GetBool("CanMeleeAttack1", 0);
	NPC_Capable_MeleeAttack2_boolean = pKeyValuesData->GetBool("CanMeleeAttack2", 0);
	NPC_Capable_Squadslots_boolean = pKeyValuesData->GetBool("CanUseSquads", 0);
	NPC_Behavior_PatrolAfterSpawn_boolean = pKeyValuesData->GetBool("ShouldPatrolAfterSpawn", 0);
	NPC_DisableShadows_boolean = pKeyValuesData->GetBool("DisableShadows", 0);
}

FileNPCInfo_t* CreateNPCInfo()
{
	return new FileNPCInfo_t;
}