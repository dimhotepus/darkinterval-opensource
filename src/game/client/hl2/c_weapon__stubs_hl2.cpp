//========================================================================//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "c_weapon__stubs.h"
#include "basehlcombatweapon_shared.h"
#include "c_basehlcombatweapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

STUB_WEAPON_CLASS( cycler_weapon, WeaponCycler, C_BaseCombatWeapon );
#ifndef DARKINTERVAL
STUB_WEAPON_CLASS( weapon_binoculars, WeaponBinoculars, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_gauss, WeaponGaussGun, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_flaregun, Flaregun, C_BaseHLCombatWeapon);
#endif
STUB_WEAPON_CLASS( weapon_hopwire, WeaponHopwire, C_BaseHLCombatWeapon);
//STUB_WEAPON_CLASS( weapon_proto1, WeaponProto1, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_357, Weapon357, C_BaseHLCombatWeapon);
#ifdef DARKINTERVAL
STUB_WEAPON_CLASS( weapon_alyxgun, WeaponAlyxGun, C_HLMachineGun);
#else
STUB_WEAPON_CLASS(weapon_alyxgun, WeaponAlyxGun, C_HLSelectFireMachineGun);
#endif
STUB_WEAPON_CLASS( weapon_annabelle, WeaponAnnabelle, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_ar2, WeaponAR2, C_HLMachineGun);
STUB_WEAPON_CLASS( weapon_bugbait, WeaponBugBait, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_citizenpackage, WeaponCitizenPackage, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_citizensuitcase, WeaponCitizenSuitcase, C_WeaponCitizenPackage );
#ifdef DARKINTERVAL
STUB_WEAPON_CLASS( weapon_crowbar, PlayerCrowbar, C_BaseHLCombatWeapon);
#else
STUB_WEAPON_CLASS(weapon_crowbar, WeaponCrowbar, C_BaseHLBludgeonWeapon);
#endif
STUB_WEAPON_CLASS( weapon_cubemap, WeaponCubemap, C_BaseCombatWeapon);
STUB_WEAPON_CLASS( weapon_frag, WeaponFrag, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_pistol, WeaponPistol, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_rpg, WeaponRPG, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_shotgun, WeaponShotgun, C_BaseHLCombatWeapon );
#ifdef DARKINTERVAL
STUB_WEAPON_CLASS( weapon_smg1, WeaponSMG1, C_HLMachineGun );
#else
STUB_WEAPON_CLASS(weapon_smg1, WeaponSMG1, C_HLSelectFireMachineGun);
#endif
// DI
// these are all commented because their stub implementations are in their clientside code.
#ifndef DARKINTERVAL
STUB_WEAPON_CLASS( weapon_crossbow, WeaponCrossbow, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_stunstick, WeaponStunStick, C_BaseHLBludgeonWeapon);
#else
//STUB_WEAPON_CLASS( weapon_crowbar_worker, WeaponCrowbarWorker, C_BaseHLBludgeonWeapon); // Part 1, obsolete now
STUB_WEAPON_CLASS( weapon_flaregun, Flaregun, C_BaseHLCombatWeapon);
//STUB_WEAPON_CLASS( weapon_immolator, WeaponImmolator, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_manhack, WeaponManhack, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_oicw, WeaponOICW, C_HLMachineGun);
//STUB_WEAPON_CLASS( weapon_pistol_worker, WeaponPistolWorker, C_BaseHLCombatWeapon); // Part 1, obsolete now
STUB_WEAPON_CLASS( weapon_plasma, WeaponPlasma, C_HLMachineGun);
STUB_WEAPON_CLASS( weapon_shotgun_virt, WeaponShotgunVirt, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_slam, WeaponSlam, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_smg1virt, WeaponSMG1Virt, C_HLMachineGun);
STUB_WEAPON_CLASS( weapon_sniper, WeaponSniper, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_tau, WeaponTau, C_BaseHLCombatWeapon);
#endif // DARKINTERVAL
#ifdef PORTAL_COMPILE
//STUB_WEAPON_CLASS(weapon_portalgun, WeaponPortalgun, C_BaseHLCombatWeapon);
#endif
#ifdef HL2_LOSTCOAST
STUB_WEAPON_CLASS(weapon_oldmanharpoon, WeaponOldManHarpoon, C_WeaponCitizenPackage);
#endif

