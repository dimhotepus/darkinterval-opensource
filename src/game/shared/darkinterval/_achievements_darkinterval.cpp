#include "cbase.h"

#if GAME_DLL

#include "achievementmgr.h"
#include "baseachievement.h"

CAchievementMgr g_AchievementMgrDI;	// global achievement mgr for DI


class CAchievementTest : public CBaseAchievement
{
	void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_WITH_GAME);
		SetVictimFilter("npc_crabsynth");
		SetGoal(3);	
	}
};
DECLARE_ACHIEVEMENT(CAchievementTest, ACHIEVEMENT_DI_ACHIEVEMENT_TEST, "DI_ACHIEVEMENT_TEST", 3); // crabsynths

class CAchievementTest2 : public CBaseAchievement
{
	void Init()
	{
		SetFlags(ACH_LISTEN_KILL_EVENTS | ACH_FILTER_ATTACKER_IS_PLAYER | ACH_SAVE_WITH_GAME);
		SetInflictorFilter("prop_physics");
		SetVictimFilter("npc_tree");
		SetGoal(3);
	}
};
DECLARE_ACHIEVEMENT(CAchievementTest2, ACHIEVEMENT_DI_ACHIEVEMENT_TEST2, "DI_ACHIEVEMENT_TEST2", 3); // trees w/ physics

class CAchievementTest3 : public CBaseAchievement
{
	void Init()
	{
		SetFlags(ACH_LISTEN_KILL_EVENTS | ACH_FILTER_ATTACKER_IS_PLAYER | ACH_SAVE_WITH_GAME);
		SetVictimFilter("npc_breen");
		SetGoal(3);
	}
};
DECLARE_ACHIEVEMENT(CAchievementTest3, ACHIEVEMENT_DI_ACHIEVEMENT_TEST3, "DI_ACHIEVEMENT_TEST3", 3); // breens

DECLARE_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_EVENT_DI_ACHIEVEMENT_TEST4, "DI_ACHIEVEMENT_TEST4", 1);
#endif