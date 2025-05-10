//========================================================================//
//
// Purpose: The various ammo types for HL2	
//
//=============================================================================//

#include "cbase.h"
#include "items.h"
#include "item_dynamic_resupply.h"
#ifdef DARKINTERVAL
#include "point_template.h" // to allow crates to spawn specific template items, without needing env entity maker
#endif
#include "props.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const char *pszItemCrateModelName[] =
{
	"models/items/item_item_crate.mdl",
	"models/items/item_beacon_crate.mdl",
};

//-----------------------------------------------------------------------------
// A breakable crate that drops items
//-----------------------------------------------------------------------------
class CItem_ItemCrate : public CPhysicsProp
{
public:
	DECLARE_CLASS( CItem_ItemCrate, CPhysicsProp );
	DECLARE_DATADESC();

	void Precache( void );
	void Spawn( void );

	virtual int	ObjectCaps() { return BaseClass::ObjectCaps() | FCAP_WCEDIT_POSITION; };

	virtual int		OnTakeDamage( const CTakeDamageInfo &info );

	void InputKill( inputdata_t &data );
#ifdef DARKINTERVAL
	void InputSetItemCount(inputdata_t &inputdata);
	void InputSetItem2Count(inputdata_t &inputdata);
	void InputSetItem3Count(inputdata_t &inputdata);
	void InputSetItem4Count(inputdata_t &inputdata);
#endif
	virtual void VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );
	virtual void OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason );

protected:
	virtual void OnBreak( const Vector &vecVelocity, const AngularImpulse &angVel, CBaseEntity *pBreaker );

private:
	// Crate types. Add more!
	enum CrateType_t
	{
		CRATE_SPECIFIC_ITEM = 0,
#ifdef DARKINTERVAL
		CRATE_POINT_TEMPLATE,
#endif
		CRATE_TYPE_COUNT,
	};

	enum CrateAppearance_t
	{
		CRATE_APPEARANCE_DEFAULT = 0,
		CRATE_APPEARANCE_RADAR_BEACON,
	};

private:
	CrateType_t			m_CrateType;
	string_t			m_strItemClass;
	int					m_nItemCount;
#ifdef DARKINTERVAL // DI allows to declare up to 4 items to drop and counts for each
	string_t			m_strItem2Class;
	int					m_nItem2Count;
	string_t			m_strItem3Class;
	int					m_nItem3Count;
	string_t			m_strItem4Class;
	int					m_nItem4Count;
#endif
	string_t			m_strAlternateMaster;
	CrateAppearance_t	m_CrateAppearance;

	COutputEvent m_OnCacheInteraction;
};


LINK_ENTITY_TO_CLASS(item_item_crate, CItem_ItemCrate);


//-----------------------------------------------------------------------------
// Save/load: 
//-----------------------------------------------------------------------------
BEGIN_DATADESC( CItem_ItemCrate )

	DEFINE_KEYFIELD( m_CrateType, FIELD_INTEGER, "CrateType" ),	
	DEFINE_KEYFIELD( m_strItemClass, FIELD_STRING, "ItemClass" ),	
	DEFINE_KEYFIELD( m_nItemCount, FIELD_INTEGER, "ItemCount" ),
#ifdef DARKINTERVAL // DI allows to declare up to 4 items to drop and counts for each
	DEFINE_KEYFIELD(m_strItem2Class, FIELD_STRING, "Item2Class"),
	DEFINE_KEYFIELD(m_nItem2Count, FIELD_INTEGER, "Item2Count"),
	DEFINE_KEYFIELD(m_strItem3Class, FIELD_STRING, "Item3Class"),
	DEFINE_KEYFIELD(m_nItem3Count, FIELD_INTEGER, "Item3Count"),
	DEFINE_KEYFIELD(m_strItem4Class, FIELD_STRING, "Item4Class"),
	DEFINE_KEYFIELD(m_nItem4Count, FIELD_INTEGER, "Item4Count"),
#endif
	DEFINE_KEYFIELD( m_strAlternateMaster, FIELD_STRING, "SpecificResupply" ),	
	DEFINE_KEYFIELD( m_CrateAppearance, FIELD_INTEGER, "CrateAppearance" ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Kill", InputKill ),
#ifdef DARKINTERVAL // for whenever we want to tweak items, maybe per difficulty
	DEFINE_INPUTFUNC(FIELD_INTEGER, "SetItemCount", InputSetItemCount),
	DEFINE_INPUTFUNC(FIELD_INTEGER, "SetItem2Count", InputSetItem2Count),
	DEFINE_INPUTFUNC(FIELD_INTEGER, "SetItem3Count", InputSetItem3Count),
	DEFINE_INPUTFUNC(FIELD_INTEGER, "SetItem4Count", InputSetItem4Count),
#endif
	DEFINE_OUTPUT( m_OnCacheInteraction, "OnCacheInteraction" ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_ItemCrate::Precache( void )
{
	// Set this here to quiet base prop warnings
	PrecacheModel( pszItemCrateModelName[m_CrateAppearance] );
	SetModel( pszItemCrateModelName[m_CrateAppearance] );

	BaseClass::Precache();
	if ( m_CrateType == CRATE_SPECIFIC_ITEM )
	{
		if ( NULL_STRING != m_strItemClass )
		{
			// Don't precache if this is a null string. 
			UTIL_PrecacheOther( STRING(m_strItemClass) );
		}
#ifdef DARKINTERVAL // DI allows to declare up to 4 items to drop and counts for each
		if (NULL_STRING != m_strItem2Class)
		{
			// Don't precache if this is a null string. 
			UTIL_PrecacheOther(STRING(m_strItem2Class));
		}
		if (NULL_STRING != m_strItem3Class)
		{
			// Don't precache if this is a null string. 
			UTIL_PrecacheOther(STRING(m_strItem3Class));
		}
		if (NULL_STRING != m_strItem4Class)
		{
			// Don't precache if this is a null string. 
			UTIL_PrecacheOther(STRING(m_strItem4Class));
		}
#endif
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_ItemCrate::Spawn( void )
{
	if ( g_pGameRules->IsAllowedToSpawn( this ) == false )
	{
		UTIL_Remove( this );
		return;
	}

	DisableAutoFade();
	SetModelName( AllocPooledString( pszItemCrateModelName[m_CrateAppearance] ) );

	if ( NULL_STRING == m_strItemClass 
#ifdef DARKINTERVAL // DI allows to declare up to 4 items to drop and counts for each
		&& NULL_STRING == m_strItem2Class && NULL_STRING == m_strItem3Class && NULL_STRING == m_strItem4Class
#endif
		)
	{
		Warning( "CItem_ItemCrate(%i):  CRATE_SPECIFIC_ITEM with NULL ItemClass strings (deleted)!!!\n", entindex() );
		UTIL_Remove( this );
		return;
	}

	Precache( );
	SetModel( pszItemCrateModelName[m_CrateAppearance] );
	AddEFlags( EFL_NO_ROTORWASH_PUSH );
	BaseClass::Spawn( );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CItem_ItemCrate::InputKill( inputdata_t &data )
{
	UTIL_Remove( this );
}
#ifdef DARKINTERVAL
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CItem_ItemCrate::InputSetItemCount(inputdata_t &inputdata)
{
	m_nItemCount = inputdata.value.Int();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CItem_ItemCrate::InputSetItem2Count(inputdata_t &inputdata)
{
	m_nItem2Count = inputdata.value.Int();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CItem_ItemCrate::InputSetItem3Count(inputdata_t &inputdata)
{
	m_nItem3Count = inputdata.value.Int();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CItem_ItemCrate::InputSetItem4Count(inputdata_t &inputdata)
{
	m_nItem4Count = inputdata.value.Int();
}
#endif
//-----------------------------------------------------------------------------
// Item crates blow up immediately
//-----------------------------------------------------------------------------
int CItem_ItemCrate::OnTakeDamage(const CTakeDamageInfo &info)
{
#ifdef DARKINTERVAL
	if (info.GetDamageType() & DMG_VEHICLE || info.GetDamageType() & DMG_AIRBOAT)
#else
	if ( info.GetDamageType() & DMG_AIRBOAT )
#endif
	{
		CTakeDamageInfo dmgInfo = info;
		dmgInfo.ScaleDamage( 10.0 );
		return BaseClass::OnTakeDamage( dmgInfo );
	}

	return BaseClass::OnTakeDamage( info );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_ItemCrate::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	float flDamageScale = 1.0f;
	if ( FClassnameIs( pEvent->pEntities[!index], "prop_vehicle_airboat" ) ||
		 FClassnameIs( pEvent->pEntities[!index], "prop_vehicle_jeep" ) 
#ifdef DARKINTERVAL
		|| FClassnameIs(pEvent->pEntities[!index], "prop_vehicle_apc") ||
		 FClassnameIs(pEvent->pEntities[!index], "prop_vehicle_digger")
#endif
		)
	{
		flDamageScale = 100.0f;
	}

	m_impactEnergyScale *= flDamageScale;
	BaseClass::VPhysicsCollision( index, pEvent );
	m_impactEnergyScale /= flDamageScale;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_ItemCrate::OnBreak( const Vector &vecVelocity, const AngularImpulse &angImpulse, CBaseEntity *pBreaker )
{
	m_OnCacheInteraction.FireOutput(pBreaker,this);

	for ( int i = 0; i < m_nItemCount; ++i )
	{
		CBaseEntity *pSpawn = NULL;
		switch( m_CrateType )
		{
		case CRATE_SPECIFIC_ITEM:
			pSpawn = CreateEntityByName( STRING(m_strItemClass) );
			break;
#ifdef DARKINTERVAL
		case CRATE_POINT_TEMPLATE:
		{
			CPointTemplate *pTemplate = dynamic_cast<CPointTemplate *>(gEntList.FindEntityByName(NULL, STRING(m_strItemClass)));
			if (!pTemplate)
			{
				Warning("item_item_crate %s failed to find template %s!\n", GetEntityName().ToCStr(), STRING(m_strItemClass));
				return;
			}

			// Give a little randomness...
		//	Vector vecTempOrigin;
		//	CollisionProp()->RandomPointInBounds(Vector(0.25, 0.25, 0.25), Vector(0.75, 0.75, 0.75), &vecTempOrigin);

			QAngle vecTempAngles;
			vecTempAngles.x = random->RandomFloat(-20.0f, 20.0f);
			vecTempAngles.y = random->RandomFloat(0.0f, 360.0f);
			vecTempAngles.z = random->RandomFloat(-20.0f, 20.0f);

			CUtlVector<CBaseEntity*> hNewEntities;
			if (!pTemplate->CreateInstance(WorldSpaceCenter(), vecTempAngles, &hNewEntities))
			{
				Warning("item_item_crate %s failed to create template %s!\n", GetEntityName().ToCStr(), STRING(m_strItemClass));
				return;
			}

			//We couldn't spawn the entity (or entities) for some reason!
			if (hNewEntities.Count() == 0)
			{
				Warning("item_item_crate %s failed to spawn template %s!\n", GetEntityName().ToCStr(), STRING(m_strItemClass));
				return;
			}

			for (int i = 0; i < hNewEntities.Count(); i++)
			{
				if (hNewEntities[i] != NULL && hNewEntities[i]->VPhysicsGetObject())
				{
					hNewEntities[i]->VPhysicsGetObject()->Wake();

					Vector vecActualVelocity;
					vecActualVelocity.Random(-10.0f, 10.0f);
					//		vecActualVelocity += vecVelocity;
					hNewEntities[i]->SetAbsVelocity(vecActualVelocity);

					QAngle angVel;
					AngularImpulseToQAngle(angImpulse, angVel);
					hNewEntities[i]->SetLocalAngularVelocity(angVel);
				}
			}
			return;
		} 
		break;
#endif // DARKINTERVAL
		default:
			break;
		}

		if ( !pSpawn )
			return;

		// Give a little randomness...
		Vector vecOrigin;
		CollisionProp()->RandomPointInBounds( Vector(0.25, 0.25, 0.25), Vector( 0.75, 0.75, 0.75 ), &vecOrigin );
		pSpawn->SetAbsOrigin( vecOrigin );

		QAngle vecAngles;
		vecAngles.x = random->RandomFloat( -20.0f, 20.0f );
		vecAngles.y = random->RandomFloat( 0.0f, 360.0f );
		vecAngles.z = random->RandomFloat( -20.0f, 20.0f );
		pSpawn->SetAbsAngles( vecAngles );

		Vector vecActualVelocity;
		vecActualVelocity.Random( -10.0f, 10.0f );
//		vecActualVelocity += vecVelocity;
		pSpawn->SetAbsVelocity( vecActualVelocity );

		QAngle angVel;
		AngularImpulseToQAngle( angImpulse, angVel );
		pSpawn->SetLocalAngularVelocity( angVel );

		// If we're creating an item, it can't be picked up until it comes to rest
		// But only if it wasn't broken by a vehicle
		CItem *pItem = dynamic_cast<CItem*>(pSpawn);
		if ( pItem && !pBreaker->GetServerVehicle())
		{
			pItem->ActivateWhenAtRest();
		}

		pSpawn->Spawn();

		// Avoid missing items drops by a dynamic resupply because they don't think immediately
		if ( FClassnameIs( pSpawn, "item_dynamic_resupply" ) )
		{
			if ( m_strAlternateMaster != NULL_STRING )
			{
				DynamicResupply_InitFromAlternateMaster( pSpawn, m_strAlternateMaster );
			}
			if ( i == 0 )
			{
				pSpawn->AddSpawnFlags( SF_DYNAMICRESUPPLY_ALWAYS_SPAWN );
			}
			pSpawn->SetNextThink( CURTIME );
		}
	}
#ifdef DARKINTERVAL // DI allows to declare up to 4 items to drop and counts for each
	if (NULL_STRING != m_strItem2Class)
	{
		for (int i = 0; i < m_nItem2Count; ++i)
		{
			CBaseEntity *pSpawn = NULL;
			switch (m_CrateType)
			{
			case CRATE_SPECIFIC_ITEM:
				pSpawn = CreateEntityByName(STRING(m_strItem2Class));
				break;
			case CRATE_POINT_TEMPLATE:
			{
				CPointTemplate *pTemplate = dynamic_cast<CPointTemplate *>(gEntList.FindEntityByName(NULL, STRING(m_strItem2Class)));
				if (!pTemplate)
				{
					Warning("item_item_crate %s failed to find template %s!\n", GetEntityName().ToCStr(), STRING(m_strItem2Class));
					return;
				}

				// Give a little randomness...
			//	Vector vecTempOrigin;
			//	CollisionProp()->RandomPointInBounds(Vector(0.25, 0.25, 0.25), Vector(0.75, 0.75, 0.75), &vecTempOrigin);

				QAngle vecTempAngles;
				vecTempAngles.x = random->RandomFloat(-20.0f, 20.0f);
				vecTempAngles.y = random->RandomFloat(0.0f, 360.0f);
				vecTempAngles.z = random->RandomFloat(-20.0f, 20.0f);

				CUtlVector<CBaseEntity*> hNewEntities;
				if (!pTemplate->CreateInstance(WorldSpaceCenter(), vecTempAngles, &hNewEntities))
				{
					Warning("item_item_crate %s failed to create template %s!\n", GetEntityName().ToCStr(), STRING(m_strItem2Class));
					return;
				}

				//We couldn't spawn the entity (or entities) for some reason!
				if (hNewEntities.Count() == 0)
				{
					Warning("item_item_crate %s failed to spawn template %s!\n", GetEntityName().ToCStr(), STRING(m_strItem2Class));
					return;
				}

				for (int i = 0; i < hNewEntities.Count(); i++)
				{
					if (hNewEntities[i] != NULL && hNewEntities[i]->VPhysicsGetObject())
					{
						hNewEntities[i]->VPhysicsGetObject()->Wake();
						Vector vecActualVelocity;
						vecActualVelocity.Random(-10.0f, 10.0f);
						//		vecActualVelocity += vecVelocity;
						hNewEntities[i]->SetAbsVelocity(vecActualVelocity);

						QAngle angVel;
						AngularImpulseToQAngle(angImpulse, angVel);
						hNewEntities[i]->SetLocalAngularVelocity(angVel);
					}
				}
				return;
			}
			break;
			default:
				break;
			}

			if (!pSpawn)
				return;

			// Give a little randomness...
			Vector vecOrigin;
			CollisionProp()->RandomPointInBounds(Vector(0.25, 0.25, 0.25), Vector(0.75, 0.75, 0.75), &vecOrigin);
			pSpawn->SetAbsOrigin(vecOrigin);

			QAngle vecAngles;
			vecAngles.x = random->RandomFloat(-20.0f, 20.0f);
			vecAngles.y = random->RandomFloat(0.0f, 360.0f);
			vecAngles.z = random->RandomFloat(-20.0f, 20.0f);
			pSpawn->SetAbsAngles(vecAngles);

			Vector vecActualVelocity;
			vecActualVelocity.Random(-10.0f, 10.0f);
			//		vecActualVelocity += vecVelocity;
			pSpawn->SetAbsVelocity(vecActualVelocity);

			QAngle angVel;
			AngularImpulseToQAngle(angImpulse, angVel);
			pSpawn->SetLocalAngularVelocity(angVel);

			// If we're creating an item, it can't be picked up until it comes to rest
			// But only if it wasn't broken by a vehicle
			CItem *pItem = dynamic_cast<CItem*>(pSpawn);
			if (pItem && !pBreaker->GetServerVehicle())
			{
				pItem->ActivateWhenAtRest();
			}

			pSpawn->Spawn();

			// Avoid missing items drops by a dynamic resupply because they don't think immediately
			if (FClassnameIs(pSpawn, "item_dynamic_resupply"))
			{
				if (m_strAlternateMaster != NULL_STRING)
				{
					DynamicResupply_InitFromAlternateMaster(pSpawn, m_strAlternateMaster);
				}
				if (i == 0)
				{
					pSpawn->AddSpawnFlags(SF_DYNAMICRESUPPLY_ALWAYS_SPAWN);
				}
				pSpawn->SetNextThink(CURTIME);
			}
		}
	}
	
	if (NULL_STRING != m_strItem3Class)
	{
		for (int i = 0; i < m_nItem3Count; ++i)
		{
			CBaseEntity *pSpawn = NULL;
			switch (m_CrateType)
			{
			case CRATE_SPECIFIC_ITEM:
				pSpawn = CreateEntityByName(STRING(m_strItem3Class));
				break;
			case CRATE_POINT_TEMPLATE:
			{
				CPointTemplate *pTemplate = dynamic_cast<CPointTemplate *>(gEntList.FindEntityByName(NULL, STRING(m_strItem3Class)));
				if (!pTemplate)
				{
					Warning("item_item_crate %s failed to find template %s!\n", GetEntityName().ToCStr(), STRING(m_strItem3Class));
					return;
				}

				// Give a little randomness...
			//	Vector vecTempOrigin;
			//	CollisionProp()->RandomPointInBounds(Vector(0.25, 0.25, 0.25), Vector(0.75, 0.75, 0.75), &vecTempOrigin);

				QAngle vecTempAngles;
				vecTempAngles.x = random->RandomFloat(-20.0f, 20.0f);
				vecTempAngles.y = random->RandomFloat(0.0f, 360.0f);
				vecTempAngles.z = random->RandomFloat(-20.0f, 20.0f);

				CUtlVector<CBaseEntity*> hNewEntities;
				if (!pTemplate->CreateInstance(WorldSpaceCenter(), vecTempAngles, &hNewEntities))
				{
					Warning("item_item_crate %s failed to create template %s!\n", GetEntityName().ToCStr(), STRING(m_strItem3Class));
					return;
				}

				//We couldn't spawn the entity (or entities) for some reason!
				if (hNewEntities.Count() == 0)
				{
					Warning("item_item_crate %s failed to spawn template %s!\n", GetEntityName().ToCStr(), STRING(m_strItem3Class));
					return;
				}

				for (int i = 0; i < hNewEntities.Count(); i++)
				{
					if (hNewEntities[i] != NULL && hNewEntities[i]->VPhysicsGetObject())
					{
						hNewEntities[i]->VPhysicsGetObject()->Wake();
						Vector vecActualVelocity;
						vecActualVelocity.Random(-10.0f, 10.0f);
						//		vecActualVelocity += vecVelocity;
						hNewEntities[i]->SetAbsVelocity(vecActualVelocity);

						QAngle angVel;
						AngularImpulseToQAngle(angImpulse, angVel);
						hNewEntities[i]->SetLocalAngularVelocity(angVel);
					}
				}
				return;
			}
			break;
			default:
				break;
			}

			if (!pSpawn)
				return;

			// Give a little randomness...
			Vector vecOrigin;
			CollisionProp()->RandomPointInBounds(Vector(0.25, 0.25, 0.25), Vector(0.75, 0.75, 0.75), &vecOrigin);
			pSpawn->SetAbsOrigin(vecOrigin);

			QAngle vecAngles;
			vecAngles.x = random->RandomFloat(-20.0f, 20.0f);
			vecAngles.y = random->RandomFloat(0.0f, 360.0f);
			vecAngles.z = random->RandomFloat(-20.0f, 20.0f);
			pSpawn->SetAbsAngles(vecAngles);

			Vector vecActualVelocity;
			vecActualVelocity.Random(-10.0f, 10.0f);
			//		vecActualVelocity += vecVelocity;
			pSpawn->SetAbsVelocity(vecActualVelocity);

			QAngle angVel;
			AngularImpulseToQAngle(angImpulse, angVel);
			pSpawn->SetLocalAngularVelocity(angVel);

			// If we're creating an item, it can't be picked up until it comes to rest
			// But only if it wasn't broken by a vehicle
			CItem *pItem = dynamic_cast<CItem*>(pSpawn);
			if (pItem && !pBreaker->GetServerVehicle())
			{
				pItem->ActivateWhenAtRest();
			}

			pSpawn->Spawn();

			// Avoid missing items drops by a dynamic resupply because they don't think immediately
			if (FClassnameIs(pSpawn, "item_dynamic_resupply"))
			{
				if (m_strAlternateMaster != NULL_STRING)
				{
					DynamicResupply_InitFromAlternateMaster(pSpawn, m_strAlternateMaster);
				}
				if (i == 0)
				{
					pSpawn->AddSpawnFlags(SF_DYNAMICRESUPPLY_ALWAYS_SPAWN);
				}
				pSpawn->SetNextThink(CURTIME);
			}
		}
	}
	
	if (NULL_STRING != m_strItem4Class)
	{
		for (int i = 0; i < m_nItem3Count; ++i)
		{
			CBaseEntity *pSpawn = NULL;
			switch (m_CrateType)
			{
			case CRATE_SPECIFIC_ITEM:
				pSpawn = CreateEntityByName(STRING(m_strItem4Class));
				break;
			case CRATE_POINT_TEMPLATE:
			{
				CPointTemplate *pTemplate = dynamic_cast<CPointTemplate *>(gEntList.FindEntityByName(NULL, STRING(m_strItem4Class)));
				if (!pTemplate)
				{
					Warning("item_item_crate %s failed to find template %s!\n", GetEntityName().ToCStr(), STRING(m_strItem4Class));
					return;
				}

				// Give a little randomness...
			//	Vector vecTempOrigin;
			//	CollisionProp()->RandomPointInBounds(Vector(0.25, 0.25, 0.25), Vector(0.75, 0.75, 0.75), &vecTempOrigin);

				QAngle vecTempAngles;
				vecTempAngles.x = random->RandomFloat(-20.0f, 20.0f);
				vecTempAngles.y = random->RandomFloat(0.0f, 360.0f);
				vecTempAngles.z = random->RandomFloat(-20.0f, 20.0f);

				CUtlVector<CBaseEntity*> hNewEntities;
				if (!pTemplate->CreateInstance(WorldSpaceCenter(), vecTempAngles, &hNewEntities))
				{
					Warning("item_item_crate %s failed to create template %s!\n", GetEntityName().ToCStr(), STRING(m_strItem4Class));
					return;
				}

				//We couldn't spawn the entity (or entities) for some reason!
				if (hNewEntities.Count() == 0)
				{
					Warning("item_item_crate %s failed to spawn template %s!\n", GetEntityName().ToCStr(), STRING(m_strItem4Class));
					return;
				}

				for (int i = 0; i < hNewEntities.Count(); i++)
				{
					if (hNewEntities[i] != NULL && hNewEntities[i]->VPhysicsGetObject())
					{
						hNewEntities[i]->VPhysicsGetObject()->Wake();
						Vector vecActualVelocity;
						vecActualVelocity.Random(-10.0f, 10.0f);
						//		vecActualVelocity += vecVelocity;
						hNewEntities[i]->SetAbsVelocity(vecActualVelocity);

						QAngle angVel;
						AngularImpulseToQAngle(angImpulse, angVel);
						hNewEntities[i]->SetLocalAngularVelocity(angVel);
					}
				}
				return;
			}
			break;
			default:
				break;
			}

			if (!pSpawn)
				return;

			// Give a little randomness...
			Vector vecOrigin;
			CollisionProp()->RandomPointInBounds(Vector(0.25, 0.25, 0.25), Vector(0.75, 0.75, 0.75), &vecOrigin);
			pSpawn->SetAbsOrigin(vecOrigin);

			QAngle vecAngles;
			vecAngles.x = random->RandomFloat(-20.0f, 20.0f);
			vecAngles.y = random->RandomFloat(0.0f, 360.0f);
			vecAngles.z = random->RandomFloat(-20.0f, 20.0f);
			pSpawn->SetAbsAngles(vecAngles);

			Vector vecActualVelocity;
			vecActualVelocity.Random(-10.0f, 10.0f);
			//		vecActualVelocity += vecVelocity;
			pSpawn->SetAbsVelocity(vecActualVelocity);

			QAngle angVel;
			AngularImpulseToQAngle(angImpulse, angVel);
			pSpawn->SetLocalAngularVelocity(angVel);

			// If we're creating an item, it can't be picked up until it comes to rest
			// But only if it wasn't broken by a vehicle
			CItem *pItem = dynamic_cast<CItem*>(pSpawn);
			if (pItem && !pBreaker->GetServerVehicle())
			{
				pItem->ActivateWhenAtRest();
			}

			pSpawn->Spawn();

			// Avoid missing items drops by a dynamic resupply because they don't think immediately
			if (FClassnameIs(pSpawn, "item_dynamic_resupply"))
			{
				if (m_strAlternateMaster != NULL_STRING)
				{
					DynamicResupply_InitFromAlternateMaster(pSpawn, m_strAlternateMaster);
				}
				if (i == 0)
				{
					pSpawn->AddSpawnFlags(SF_DYNAMICRESUPPLY_ALWAYS_SPAWN);
				}
				pSpawn->SetNextThink(CURTIME);
			}
		}
	}
#endif // DARKINTERVAL
}

void CItem_ItemCrate::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	BaseClass::OnPhysGunPickup( pPhysGunUser, reason );

	m_OnCacheInteraction.FireOutput( pPhysGunUser, this );
#ifndef DARKINTERVAL // DI change: we don't need this. Get it out
	if ( reason == PUNTED_BY_CANNON && m_CrateAppearance != CRATE_APPEARANCE_RADAR_BEACON )
	{
		Vector vForward;
		AngleVectors( pPhysGunUser->EyeAngles(), &vForward, NULL, NULL );
		Vector vForce = Pickup_PhysGunLaunchVelocity( this, vForward, PHYSGUN_FORCE_PUNTED );
		AngularImpulse angular = AngularImpulse( 0, 0, 0 );

		IPhysicsObject *pPhysics = VPhysicsGetObject();

		if ( pPhysics )
		{
			pPhysics->AddVelocity( &vForce, &angular );
		}

		TakeDamage( CTakeDamageInfo( pPhysGunUser, pPhysGunUser, GetHealth(), DMG_GENERIC ) );
	}
#endif
}
