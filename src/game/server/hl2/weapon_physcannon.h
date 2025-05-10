//========================================================================//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_PHYSCANNON_H
#define WEAPON_PHYSCANNON_H
#ifdef _WIN32
#pragma once
#endif
//-----------------------------------------------------------------------------
// Do we have the super-phys gun?
//-----------------------------------------------------------------------------
bool PlayerHasMegaPhysCannon();

#ifdef PORTAL_COMPILE
class CGrabController;
// force the physcannon to drop an object (if carried)
void PhysCannonForceDrop(CBaseCombatWeapon *pActiveWeapon, CBaseEntity *pOnlyIfHoldingThis);
void PhysCannonBeginUpgrade(CBaseAnimating *pAnim);

bool PlayerPickupControllerIsHoldingEntity(CBaseEntity *pPickupController, CBaseEntity *pHeldEntity);
void ShutdownPickupController(CBaseEntity *pPickupControllerEntity);
float PlayerPickupGetHeldObjectMass(CBaseEntity *pPickupControllerEntity, IPhysicsObject *pHeldObject);
float PhysCannonGetHeldObjectMass(CBaseCombatWeapon *pActiveWeapon, IPhysicsObject *pHeldObject);

CBaseEntity *PhysCannonGetHeldEntity(CBaseCombatWeapon *pActiveWeapon);
CBaseEntity *GetPlayerHeldEntity(CBasePlayer *pPlayer);
CBasePlayer *GetPlayerHoldingEntity(CBaseEntity *pEntity); //

CGrabController *GetGrabControllerForPlayer(CBasePlayer *pPlayer);
CGrabController *GetGrabControllerForPhysCannon(CBaseCombatWeapon *pActiveWeapon);
void GetSavedParamsForCarriedPhysObject(CGrabController *pGrabController, IPhysicsObject *pObject, float *pSavedMassOut, float *pSavedRotationalDampingOut);
void UpdateGrabControllerTargetPosition(CBasePlayer *pPlayer, Vector *vPosition, QAngle *qAngles);
void GrabController_SetPortalPenetratingEntity(CGrabController *pController, CBaseEntity *pPenetrated);

bool PhysCannonAccountableForObject(CBaseCombatWeapon *pPhysCannon, CBaseEntity *pObject);

#else
// force the physcannon to drop an object (if carried)
void PhysCannonForceDrop( CBaseCombatWeapon *pActiveWeapon, CBaseEntity *pOnlyIfHoldingThis );
void PhysCannonBeginUpgrade( CBaseAnimating *pAnim );

bool PlayerPickupControllerIsHoldingEntity( CBaseEntity *pPickupController, CBaseEntity *pHeldEntity );
float PlayerPickupGetHeldObjectMass( CBaseEntity *pPickupControllerEntity, IPhysicsObject *pHeldObject );
float PhysCannonGetHeldObjectMass( CBaseCombatWeapon *pActiveWeapon, IPhysicsObject *pHeldObject );

CBaseEntity *PhysCannonGetHeldEntity( CBaseCombatWeapon *pActiveWeapon );
CBaseEntity *GetPlayerHeldEntity( CBasePlayer *pPlayer );

bool PhysCannonAccountableForObject( CBaseCombatWeapon *pPhysCannon, CBaseEntity *pObject );
#endif
#endif // WEAPON_PHYSCANNON_H