#ifndef	GRENADETOXICCLOUD_H
#define	GRENADETOXICCLOUD_H
#include "ieffects.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "te_effect_dispatch.h"

enum ToxicCloudDamageType
{
	TOXICCLOUD_ACID,
	TOXICCLOUD_VENOM,
	TOXICCLOUD_SLOWDOWN,
};

class CToxicCloud : public CBaseEntity
{
	DECLARE_CLASS(CToxicCloud, CBaseEntity);

public:
	static CToxicCloud *CreateGasCloud(const Vector &vecOrigin, CBaseEntity *pOwner, 
		const char *szParticleName, 
		float flDie, int iDamageType, float flRadius);

	CToxicCloud(void);

	virtual void	Spawn(const char *szParticleName, float flDie);
	void			Think(void);
private:
	float			m_flDamageTime;
	float			m_flRadius;
	int				m_iDamageType;
	float			m_flDieTime;
	const char		*m_szParticleName;

private:
	DECLARE_DATADESC();
};
#endif // GRENADETOXICCLOUD_H