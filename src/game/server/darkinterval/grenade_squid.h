//========================================================================//
//
// Purpose:		Projectile shot by bullsquid 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	GRENADESQUID_H
#define	GRENADESQUID_H

#include "basegrenade_shared.h"
#include "grenade_toxiccloud.h"

class CParticleSystem;
class CToxicCloud;

enum SquidSpitSize_e
{
	SPIT_SMALL,
	SPIT_LARGE,
};

#define SPITBALLMODEL_LARGE "models/_Monsters/Xen/squid_spitball_large.mdl"
#define SPITBALLMODEL_SMALL "models/_Monsters/Xen/squid_spitball_small.mdl"

class CSquidSpit : public CBaseGrenade
{
	DECLARE_CLASS(CSquidSpit, CBaseGrenade);

public:
	CSquidSpit(void);

	virtual void		Spawn(void);
	virtual void		Precache(void);
	virtual void		Event_Killed(const CTakeDamageInfo &info);

	virtual	unsigned int	PhysicsSolidMaskForEntity(void) const { return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_WATER); }

	void 				SquidSpitTouch(CBaseEntity *pOther);
	void				SetSpitSize(int nSize);
	void				Detonate(void);
	void				Think(void);

private:
	DECLARE_DATADESC();

//	void	InitHissSound(void);

	CHandle< CParticleSystem >	m_hSpitEffect;
//	CSoundPatch					*m_pHissSound;

	CHandle<CToxicCloud> m_hGasCloud;
	bool						m_bSizeBig;
	float						m_flCreationTime;
};

#endif	//GRENADESPIT_H
