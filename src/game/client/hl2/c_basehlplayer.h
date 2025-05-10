//========================================================================//
//
// Purpose: 
//
// $Workfile:     $
// $NoKeywords: $
//=============================================================================//

#if !defined( C_BASEHLPLAYER_H )
#define C_BASEHLPLAYER_H
#ifdef _WIN32
#pragma once
#endif


#include "c_baseplayer.h"
#include "c_hl2_playerlocaldata.h"
#ifdef DARKINTERVAL
#include "colorcorrectionmgr.h"
#include "debugoverlay_shared.h"
#endif
#ifdef PORTAL_COMPILE
#include "c_prop_portal.h"
#include "weapon_portalbase.h"
#include "c_func_liquidportal.h"
#endif

class C_BaseHLPlayer : public C_BasePlayer
{
public:
	DECLARE_CLASS( C_BaseHLPlayer, C_BasePlayer );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
#ifdef PORTAL_COMPILE
	DECLARE_INTERPOLATION();
#endif
	C_BaseHLPlayer(); 
#ifdef DARKINTERVAL
	~C_BaseHLPlayer();
	void ClientThink(void);
#endif
#ifdef PORTAL_COMPILE
public:
//	~C_BaseHLPlayer(void);

//	void ClientThink(void);
	void FixTeleportationRoll(void);

	static inline C_BaseHLPlayer* GetLocalBaseHLPlayer()
	{
		return (C_BaseHLPlayer*)C_BasePlayer::GetLocalPlayer();
	}

	static inline C_BaseHLPlayer* GetLocalPlayer()
	{
		return (C_BaseHLPlayer*)C_BasePlayer::GetLocalPlayer();
	}

//	virtual const QAngle& GetRenderAngles();

	// Used by prediction, sets the view angles for the player
	virtual void SetLocalViewAngles(const QAngle &viewAngles);
	virtual void SetViewAngles(const QAngle &ang);
	bool		 DetectAndHandlePortalTeleportation(void); //detects if the player has portalled and fixes views
	virtual void OnPreDataChanged(DataUpdateType_t type);

	virtual Vector			EyePosition();
	Vector					EyeFootPosition(const QAngle &qEyeAngles);//interpolates between eyes and feet based on view angle roll
	inline Vector			EyeFootPosition(void) { return EyeFootPosition(EyeAngles()); };
	void					PlayerPortalled(C_Prop_Portal *pEnteredPortal);

	virtual void	CalcView(Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov);
	void			CalcPortalView(Vector &eyeOrigin, QAngle &eyeAngles);
	//virtual void	CalcViewModelView(const Vector& eyeOrigin, const QAngle& eyeAngles);

	CBaseEntity*	FindUseEntity(void);
	CBaseEntity*	FindUseEntityThroughPortal(void);

	inline bool		IsCloseToPortal(void) //it's usually a good idea to turn on draw hacks when this is true
	{
		return ((PortalEyeInterpolation.m_bEyePositionIsInterpolating) || (m_hPortalEnvironment.Get() != NULL));
	}

	void ToggleHeldObjectOnOppositeSideOfPortal(void) { m_bHeldObjectOnOppositeSideOfPortal = !m_bHeldObjectOnOppositeSideOfPortal; }
	void SetHeldObjectOnOppositeSideOfPortal(bool p_bHeldObjectOnOppositeSideOfPortal) { m_bHeldObjectOnOppositeSideOfPortal = p_bHeldObjectOnOppositeSideOfPortal; }
	bool IsHeldObjectOnOppositeSideOfPortal(void) { return m_bHeldObjectOnOppositeSideOfPortal; }
	CProp_Portal *GetHeldObjectPortal(void) { return m_pHeldObjectPortal; }

	CWeaponPortalBase* GetActivePortalWeapon() const;
private:
	void UpdatePortalEyeInterpolation(void);
	QAngle	m_angEyeAngles;
	CInterpolatedVar< QAngle >	m_iv_angEyeAngles;

	int	  m_iSpawnInterpCounter;
	int	  m_iSpawnInterpCounterCache;
	int	  m_iPlayerSoundType;

	bool  m_bHeldObjectOnOppositeSideOfPortal;
	CProp_Portal *m_pHeldObjectPortal;

	int	m_iForceNoDrawInPortalSurface; //only valid for one frame, used to temp disable drawing of the player model in a surface because of freaky artifacts

	struct PortalEyeInterpolation_t
	{
		bool	m_bEyePositionIsInterpolating; //flagged when the eye position would have popped between two distinct positions and we're smoothing it over
		Vector	m_vEyePosition_Interpolated; //we'll be giving the interpolation a certain amount of instant movement per frame based on how much an uninterpolated eye would have moved
		Vector	m_vEyePosition_Uninterpolated; //can't have smooth movement without tracking where we just were
											   //bool	m_bNeedToUpdateEyePosition;
											   //int		m_iFrameLastUpdated;

		int		m_iTickLastUpdated;
		float	m_fTickInterpolationAmountLastUpdated;
		bool	m_bDisableFreeMovement; //used for one frame usually when error in free movement is likely to be high
		bool	m_bUpdatePosition_FreeMove;

		PortalEyeInterpolation_t(void) : m_iTickLastUpdated(0), m_fTickInterpolationAmountLastUpdated(0.0f), m_bDisableFreeMovement(false), m_bUpdatePosition_FreeMove(false) { };
	} 
	PortalEyeInterpolation;

	struct PreDataChanged_Backup_t
	{
		CHandle<C_Prop_Portal>	m_hPortalEnvironment;
		CHandle<C_Func_LiquidPortal>	m_hSurroundingLiquidPortal;
		//Vector					m_ptPlayerPosition;
		QAngle					m_qEyeAngles;
	} PreDataChanged_Backup;

	Vector	m_ptEyePosition_LastCalcView;
	QAngle	m_qEyeAngles_LastCalcView; //we've got some VERY persistent single frame errors while teleporting, this will be updated every frame in CalcView() and will serve as a central source for fixed angles
	C_Prop_Portal *m_pPortalEnvironment_LastCalcView;

	bool	m_bPortalledMessagePending; //Player portalled. It's easier to wait until we get a OnDataChanged() event or a CalcView() before we do anything about it. Otherwise bits and pieces can get undone
	VMatrix m_PendingPortalMatrix;

public:
	bool	m_bPitchReorientation;
	float	m_fReorientationRate;
	bool	m_bEyePositionIsTransformedByPortal; //when the eye and body positions are not on the same side of a portal

	CHandle<C_Prop_Portal>	m_hPortalEnvironment; //a portal whose environment the player is currently in, should be invalid most of the time
	CHandle<C_Func_LiquidPortal>	m_hSurroundingLiquidPortal; //a liquid portal whose volume the player is standing in
#endif // PORTAL_COMPILE
public:
	virtual void		OnDataChanged( DataUpdateType_t updateType );

	void				Weapon_DropPrimary( void );
		
	float				GetFOV();
	void				Zoom( float FOVOffset, float time );
	float				GetZoom( void );
	bool				IsZoomed( void )	{ return m_HL2Local.m_bZooming; }
#ifdef DARKINTERVAL
	int					GetZoomType(void) {	return m_HL2Local.m_nZoomType; }
#endif
	bool				IsSprinting( void ) { return m_HL2Local.m_bitsActiveDevices & bits_SUIT_DEVICE_SPRINT; }
	bool				IsFlashlightActive( void ) { return m_HL2Local.m_bitsActiveDevices & bits_SUIT_DEVICE_FLASHLIGHT; }
	bool				IsBreatherActive( void ) { return m_HL2Local.m_bitsActiveDevices & bits_SUIT_DEVICE_BREATHER; }

	virtual int			DrawModel( int flags );
	virtual	void		BuildTransformations( CStudioHdr *hdr, Vector *pos, Quaternion q[], const matrix3x4_t& cameraTransform, int boneMask, CBoneBitList &boneComputed );

	LadderMove_t		*GetLadderMove() { return &m_HL2Local.m_LadderMove; }
	virtual void		ExitLadder();
	bool				IsSprinting() const { return m_fIsSprinting; }
	
	// Input handling
	virtual bool		CreateMove( float flInputSampleTime, CUserCmd *pCmd );
	void				PerformClientSideObstacleAvoidance( float flFrameTime, CUserCmd *pCmd );
	void				PerformClientSideNPCSpeedModifiers( float flFrameTime, CUserCmd *pCmd );

	bool				IsWeaponLowered( void ) { return m_HL2Local.m_bWeaponLowered; }

public:

	C_HL2PlayerLocalData		m_HL2Local;
	EHANDLE				m_hClosestNPC;
	float				m_flSpeedModTime;
	bool				m_fIsSprinting;

private:
	C_BaseHLPlayer( const C_BaseHLPlayer & ); // not defined, not accessible
	
	bool				TestMove( const Vector &pos, float fVertDist, float radius, const Vector &objPos, const Vector &objDir );

	float				m_flZoomStart;
	float				m_flZoomEnd;
	float				m_flZoomRate;
	float				m_flZoomStartTime;

	bool				m_bPlayUseDenySound;		// Signaled by PlayerUse, but can be unset by HL2 ladder code...
	float				m_flSpeedMod;
	float				m_flExitSpeedMod;
#ifdef DARKINTERVAL
	ClientCCHandle_t	m_CCUnderWaterHandle; // underwater colour correction
#endif
friend class CHL2GameMovement;
};

#ifdef PORTAL_COMPILE
inline C_BaseHLPlayer *ToBaseHLPlayer(CBaseEntity *pEntity)
{
	if (!pEntity || !pEntity->IsPlayer())
		return NULL;

	return dynamic_cast<C_BaseHLPlayer*>(pEntity);
}

inline C_BaseHLPlayer *GetBaseHLPlayer(void)
{
	return static_cast<C_BaseHLPlayer*>(C_BasePlayer::GetLocalPlayer());
}
#endif

#endif
