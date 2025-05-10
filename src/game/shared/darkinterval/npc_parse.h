#ifndef NPC_PARSE_H
#define NPC_PARSE_H
#ifdef _WIN32
#pragma once
#endif

#include "shareddefs.h"

class IFileSystem;
typedef unsigned short NPC_FILE_INFO_HANDLE;

class KeyValues;

class FileNPCInfo_t
{
public:

	FileNPCInfo_t();
	// Each game can override this to get whatever values it wants from the script.
	virtual void Parse(KeyValues *pKeyValuesData, const char *szWeaponName);

public:
	bool					NPC_Script_Parsed_boolean;
	char					NPC_ClassName_char[255];
	// SHARED
	char					NPC_Model_Path_char[255]; // the path to the model, relative to root, with extension
	char					NPC_AltModel_Path_char[255]; // the path to the alternative model, if desired
	int						NPC_Model_SkinMin_int; // min value of skin assigned on spawn 
	int						NPC_Model_SkinMax_int; // max value of skin assigned on spawn (make same as min to make it not randomise)
	float					NPC_Model_ScaleMin_float; // min value of modelscale on spawn (can cause problems)
	float					NPC_Model_ScaleMax_float; // max value of modelscale (can cause problems)
	int						NPC_AltModel_SkinMin_int; // min value of skin for when using alt model
	int						NPC_AltModel_SkinMax_int; // max value of skin for when using alt model
	float					NPC_AltModel_ScaleMin_float; // min value of modelscale for alt model 
	float					NPC_AltModel_ScaleMax_float; // max value of modelscale for alt model
	float					NPC_AltModel_Chance_float; // the chance the alt model will be used (0-1; default 0)
	int						NPC_Stats_Health_int; // initial health assigned on spawn
	int						NPC_Stats_MaxHealth_int; // total max health
	int						NPC_Stats_HullType_int; // hull type; currently has to be numerical, doesn't parse HULL_*
	int						NPC_Stats_BloodColor_int; // blood type; currently has to be numerical, doesn't parse BLOOD_COLOR_*
	float					NPC_Stats_FOV_float; // fov, 0-1
	int						NPC_Movement_MoveType_int; // movement type; currently has to be numerical, doesn't parse MOVE_*
	float					NPC_Movement_StepSize_float; // length of step
	bool					NPC_Capable_Jump_boolean; // if 1, will be given bits_CAP_MOVE_JUMP
	bool					NPC_Capable_Climb_boolean; // if 1, will be given bits_CAP_MOVE_CLIMB
	bool					NPC_Capable_Doors_boolean; // if 1, will be given bits_CAP_OPEN_DOORS
	bool					NPC_Capable_WeaponRangeAttack1_boolean; // if 1, will be given bits_CAP_WEAPON_RANGE_ATTACK1
	bool					NPC_Capable_WeaponRangeAttack2_boolean; // if 1, will be given bits_CAP_WEAPON_RANGE_ATTACK2
	bool					NPC_Capable_InnateRangeAttack1_boolean; // if 1, will be given bits_CAP_RANGE_ATTACK1
	bool					NPC_Capable_InnateRangeAttack2_boolean; // if 1, will be given bits_CAP_RANGE_ATTACK2
	bool					NPC_Capable_MeleeAttack1_boolean; // if 1, will be given bits_CAP_MELEE_ATTACK1
	bool					NPC_Capable_MeleeAttack2_boolean; // if 1, will be given bits_CAP_MELEE_ATTACK2
	bool					NPC_Capable_Squadslots_boolean; // if 1, will be given bits_CAP_SQUAD
	bool					NPC_Behavior_PatrolAfterSpawn_boolean; // if 1, *some* NPCs will be using idle patrol behavior
	bool					NPC_DisableShadows_boolean; // if 1, will be given EF_NOSHADOW on spawn
};

// The weapon parse function
bool ReadNPCDataFromFileForSlot(IFileSystem* filesystem, const char *szNPCName,
	NPC_FILE_INFO_HANDLE *phandle, const unsigned char *pICEKey);

// If weapon info has been loaded for the specified class name, this returns it.
NPC_FILE_INFO_HANDLE LookupNPCInfoScript(const char *name);

FileNPCInfo_t *GetFileNPCInfoFromHandle(NPC_FILE_INFO_HANDLE handle);
NPC_FILE_INFO_HANDLE GetInvalidNPCInfoHandle(void);

void PrecacheFileNPCInfoDatabase(IFileSystem *filesystem, const unsigned char *pICEKey);

KeyValues* ReadEncryptedNPCFile(IFileSystem *filesystem, const char *szFilenameWithoutExtension, 
	const unsigned char *pICEKey, bool bForceReadEncryptedFile = false);

// Each game implements this. It can return a derived class and override Parse() if it wants.
extern FileNPCInfo_t* CreateNPCInfo();
#endif
