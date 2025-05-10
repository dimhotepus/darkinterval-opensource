#include "cbase.h"
#include "grenade_toxiccloud.h"

LINK_ENTITY_TO_CLASS(toxic_cloud, CToxicCloud);

BEGIN_DATADESC(CToxicCloud)
DEFINE_FIELD(m_flDieTime, FIELD_TIME),
DEFINE_FIELD(m_flDamageTime, FIELD_TIME),
DEFINE_FIELD(m_iDamageType, FIELD_INTEGER),
DEFINE_FIELD(m_szParticleName, FIELD_STRING),
DEFINE_THINKFUNC(Think),
END_DATADESC()

ConVar sk_toxic_cloud_dmg_plr("sk_toxic_cloud_dmg_plr", "1");
ConVar sk_toxic_cloud_dmg_npc("sk_toxic_cloud_dmg_npc", "10");

CToxicCloud::CToxicCloud(void)
{
	m_szParticleName = NULL;
}


CToxicCloud *CToxicCloud::CreateGasCloud(const Vector &vecOrigin, CBaseEntity *pOwner, const char *szParticleName, float flDie, int iDamageType, float flRadius)
{
	CToxicCloud *pToken = (CToxicCloud *)CreateEntityByName("toxic_cloud");
	if (pToken == NULL)
		return NULL;

	// Set up our internal data
	UTIL_SetOrigin(pToken, vecOrigin);
	pToken->SetOwnerEntity(pOwner);
	pToken->m_iDamageType = iDamageType;
	pToken->m_flRadius = flRadius;
	pToken->Spawn(szParticleName, flDie);
	return pToken;
}

void CToxicCloud::Spawn(const char *szParticleName, float flDie)
{
	Precache();

	UTIL_SetSize(this, Vector(-m_flRadius, -m_flRadius, -m_flRadius), Vector(m_flRadius, m_flRadius, m_flRadius)); // 128 by 128 by 128 cube of everyone choke and die

	m_szParticleName = szParticleName;
	m_flDieTime = flDie;
	DispatchParticleEffect(m_szParticleName, GetAbsOrigin(), vec3_angle);
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_TRIGGER);
	AddSolidFlags(FSOLID_NOT_SOLID);
	m_flDamageTime = CURTIME;

	SetThink(&CToxicCloud::Think);
	SetNextThink(CURTIME);
	
	BaseClass::Spawn();
}

void CToxicCloud::Think(void)
{
	if (CURTIME < m_flDieTime )
	{
		CBaseEntity *pOther = NULL;
		while ((pOther = gEntList.FindEntityInSphere(pOther, WorldSpaceCenter(), m_flRadius)) != NULL)
		{
			if (pOther && pOther->m_takedamage)
			{
				if (CURTIME >= m_flDamageTime)
				{
					switch (m_iDamageType)
					{
					case TOXICCLOUD_ACID:
					case TOXICCLOUD_VENOM:
					{
						if (pOther->IsPlayer())
						{
							CBasePlayer *player = ToBasePlayer(pOther);
							if (player && player->IsInAVehicle()) return;

							CTakeDamageInfo dmgInfo(this, this, sk_toxic_cloud_dmg_plr.GetFloat(), DMG_NERVEGAS);
							dmgInfo.SetDamagePosition(pOther->WorldSpaceCenter());
							pOther->TakeDamage(dmgInfo);

							color32 pain = { 128,0,0,50 };
							pain.a = (byte)((float)pain.a * 0.9f);
							float flFadeTime = 0.2f;
							UTIL_ScreenFade(pOther, pain, flFadeTime, 0.5, FFADE_IN);
						}
						else if (pOther->IsNPC() && !pOther->ClassMatches("npc_squid")) // squids cannot die to their own poison (overlaps with poisonous polyps, though)
						{
							CTakeDamageInfo dmgInfo(this, this, sk_toxic_cloud_dmg_npc.GetFloat(), DMG_CRUSH);
							dmgInfo.SetDamagePosition(pOther->WorldSpaceCenter());
							pOther->TakeDamage(dmgInfo);
						}
						else
						{
						}
					}
					break;
					case TOXICCLOUD_SLOWDOWN:
					{
						if (pOther->IsPlayer())
						{
							CBasePlayer *player = ToBasePlayer(pOther);
							if (player && player->IsInAVehicle()) return;

							CTakeDamageInfo dmgInfo(this, this, 20, DMG_PARALYZE);
							dmgInfo.SetDamagePosition(GetAbsOrigin());
							pOther->TakeDamage(dmgInfo);

							color32 pain = { 128,40,108,50 };
							pain.a = (byte)((float)pain.a * 0.4f);
							float flFadeTime = 10.0f;
							UTIL_ScreenFade(pOther, pain, flFadeTime, 0.5, FFADE_IN);
						}
					}
					break;
					}

					m_flDamageTime = CURTIME + ((m_flDieTime - CURTIME <= 5) ? 1 : 0.5f);
				}
			}
		}
		SetNextThink(CURTIME + 0.1f);
	}
	else
	{
		SetThink(&CBaseEntity::SUB_Remove);
		SetNextThink(CURTIME + 1.0f);
	}
}