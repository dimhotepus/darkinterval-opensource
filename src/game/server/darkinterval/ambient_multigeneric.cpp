// A kind of ambient_generic that can play multiple sounds each from a target location/entity, and have inputs to tune 
// their volumes. For example, used in Scrapland to have external and internal submarine music functionality.

#include "cbase.h"
#include "soundenvelope.h"

class CAmbientMultiGeneric : public CPointEntity
{
public:
	DECLARE_CLASS(CAmbientMultiGeneric, CPointEntity);
	DECLARE_DATADESC();

	CAmbientMultiGeneric();
	~CAmbientMultiGeneric();

	void	Precache(void);
	void	Spawn();
	int		UpdateTransmitState();
	void	OnSave(IEntitySaveUtils *pUtils);
	void	OnRestore();
	void	MultiGenericThink();

	void InputSetSound1Volume(inputdata_t &inputdata);
	void InputSetSound2Volume(inputdata_t &inputdata);
	void InputSetSound3Volume(inputdata_t &inputdata);
	void InputSetSound4Volume(inputdata_t &inputdata);

private:
	CSoundPatch *m_pSound1Patch;
	CSoundPatch *m_pSound2Patch;
	CSoundPatch *m_pSound3Patch;
	CSoundPatch *m_pSound4Patch;
	string_t m_sound1_soundname;
	string_t m_sound2_soundname;
	string_t m_sound3_soundname;
	string_t m_sound4_soundname;
	float m_sound1_volume;
	float m_sound2_volume;
	float m_sound3_volume;
	float m_sound4_volume;
	/*static*/ float m_sound1_transitionlength;
	/*static*/ float m_sound2_transitionlength;
	/*static*/ float m_sound3_transitionlength;
	/*static*/ float m_sound4_transitionlength;
	string_t m_Sound1_source_name;
	string_t m_Sound2_source_name;
	string_t m_Sound3_source_name;
	string_t m_Sound4_source_name;
	EHANDLE m_Sound1_source_ent;
	EHANDLE m_Sound2_source_ent;
	EHANDLE m_Sound3_source_ent;
	EHANDLE m_Sound4_source_ent;
	int		m_Sound1_source_entindex;
	int		m_Sound2_source_entindex;
	int		m_Sound3_source_entindex;
	int		m_Sound4_source_entindex;
};
/*
float CAmbientMultiGeneric::m_sound1_transitionlength = 0.5f;
float CAmbientMultiGeneric::m_sound2_transitionlength = 0.5f;
float CAmbientMultiGeneric::m_sound3_transitionlength = 0.5f;
float CAmbientMultiGeneric::m_sound4_transitionlength = 0.5f;
*/
LINK_ENTITY_TO_CLASS(ambient_multigeneric, CAmbientMultiGeneric);

BEGIN_DATADESC(CAmbientMultiGeneric)

	DEFINE_KEYFIELD(m_sound1_soundname, FIELD_SOUNDNAME, "sound1name"),
	DEFINE_KEYFIELD(m_sound2_soundname, FIELD_SOUNDNAME, "sound2name"),
	DEFINE_KEYFIELD(m_sound3_soundname, FIELD_SOUNDNAME, "sound3name"),
	DEFINE_KEYFIELD(m_sound4_soundname, FIELD_SOUNDNAME, "sound4name"),
	DEFINE_KEYFIELD(m_sound1_volume, FIELD_FLOAT, "sound1volume"), // max volume when looping sound1
	DEFINE_KEYFIELD(m_sound2_volume, FIELD_FLOAT, "sound2volume"), // max volume when looping sound2
	DEFINE_KEYFIELD(m_sound3_volume, FIELD_FLOAT, "sound3volume"), // max volume when looping sound3
	DEFINE_KEYFIELD(m_sound4_volume, FIELD_FLOAT, "sound4volume"), // max volume when looping sound4
	DEFINE_KEYFIELD(m_sound1_transitionlength, FIELD_FLOAT, "sound1transition"), // length of volume transition (delta) for sound1
	DEFINE_KEYFIELD(m_sound2_transitionlength, FIELD_FLOAT, "sound2transition"), // length of volume transition (delta) for sound2
	DEFINE_KEYFIELD(m_sound3_transitionlength, FIELD_FLOAT, "sound1transition"), // length of volume transition (delta) for sound3
	DEFINE_KEYFIELD(m_sound4_transitionlength, FIELD_FLOAT, "sound2transition"), // length of volume transition (delta) for sound4
	DEFINE_SOUNDPATCH(m_pSound1Patch),
	DEFINE_SOUNDPATCH(m_pSound2Patch),
	DEFINE_SOUNDPATCH(m_pSound3Patch),
	DEFINE_SOUNDPATCH(m_pSound4Patch),
	DEFINE_KEYFIELD(m_Sound1_source_name, FIELD_STRING, "sound1ent"),
	DEFINE_KEYFIELD(m_Sound2_source_name, FIELD_STRING, "sound2ent"),
	DEFINE_KEYFIELD(m_Sound3_source_name, FIELD_STRING, "sound3ent"),
	DEFINE_KEYFIELD(m_Sound4_source_name, FIELD_STRING, "sound4ent"),

	DEFINE_FIELD(m_Sound1_source_ent, FIELD_EHANDLE),
	DEFINE_FIELD(m_Sound2_source_ent, FIELD_EHANDLE),
	DEFINE_FIELD(m_Sound3_source_ent, FIELD_EHANDLE),
	DEFINE_FIELD(m_Sound4_source_ent, FIELD_EHANDLE),
		
	DEFINE_FUNCTION(MultiGenericThink),

	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetSound1Volume", InputSetSound1Volume),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetSound2Volume", InputSetSound2Volume),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetSound3Volume", InputSetSound3Volume),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetSound4Volume", InputSetSound4Volume),

END_DATADESC()


CAmbientMultiGeneric::CAmbientMultiGeneric()
{
	m_sound1_transitionlength = 0.5f;
	m_sound2_transitionlength = 0.5f;
	m_sound3_transitionlength = 0.5f;
	m_sound4_transitionlength = 0.5f;
	m_sound1_soundname = NULL_STRING;
	m_sound2_soundname = NULL_STRING;
	m_sound3_soundname = NULL_STRING;
	m_sound4_soundname = NULL_STRING;
	m_sound1_volume = 1;
	m_sound2_volume = 1;
	m_sound3_volume = 1;
	m_sound4_volume = 1;
	m_Sound1_source_ent = NULL;
	m_Sound2_source_ent = NULL;
	m_Sound3_source_ent = NULL;
	m_Sound4_source_ent = NULL;
}

CAmbientMultiGeneric::~CAmbientMultiGeneric()
{
	if (m_pSound1Patch)
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.SoundDestroy(m_pSound1Patch);
		m_pSound1Patch = NULL;
	}

	if (m_pSound2Patch)
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.SoundDestroy(m_pSound2Patch);
		m_pSound2Patch = NULL;
	}

	if (m_pSound3Patch)
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.SoundDestroy(m_pSound3Patch);
		m_pSound3Patch = NULL;
	}

	if (m_pSound4Patch)
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.SoundDestroy(m_pSound4Patch);
		m_pSound4Patch = NULL;
	}
}

int CAmbientMultiGeneric::UpdateTransmitState()
{
	// ALWAYS transmit to all clients.
	return SetTransmitState(FL_EDICT_ALWAYS);
}

void CAmbientMultiGeneric::OnSave(IEntitySaveUtils *pUtils)
{
	BaseClass::OnSave(pUtils);
}

void CAmbientMultiGeneric::OnRestore()
{
	BaseClass::OnRestore();
}

void CAmbientMultiGeneric::Precache(void)
{
	PrecacheScriptSound(STRING(m_sound1_soundname));
	PrecacheScriptSound(STRING(m_sound2_soundname));
	PrecacheScriptSound(STRING(m_sound3_soundname));
	PrecacheScriptSound(STRING(m_sound4_soundname));

	BaseClass::Precache();
}

void CAmbientMultiGeneric::Spawn(void)
{
	Precache();
	SetSolid(SOLID_NONE);							// Remove model & collisions
	SetMoveType(MOVETYPE_NONE);
	SetModel(STRING(GetModelName()));		// Set size

	m_Sound1_source_entindex = -1;
	m_Sound2_source_entindex = -1;
	m_Sound3_source_entindex = -1;
	m_Sound4_source_entindex = -1;
	
	m_nRenderMode = kRenderNone;

	SetThink(&CAmbientMultiGeneric::MultiGenericThink);
	SetNextThink(CURTIME);
}

void CAmbientMultiGeneric::InputSetSound1Volume(inputdata_t &inputdata)
{
	m_sound1_volume = inputdata.value.Float();
}

void CAmbientMultiGeneric::InputSetSound2Volume(inputdata_t &inputdata)
{
	m_sound2_volume = inputdata.value.Float();
}

void CAmbientMultiGeneric::InputSetSound3Volume(inputdata_t &inputdata)
{
	m_sound3_volume = inputdata.value.Float();
}

void CAmbientMultiGeneric::InputSetSound4Volume(inputdata_t &inputdata)
{
	m_sound4_volume = inputdata.value.Float();
}

void CAmbientMultiGeneric::MultiGenericThink(void)
{
	if (true) //(HasSpawnFlags(SF_PRECIPITATION_PLAY_RAIN_SOUNDS) && m_bPlayRainSound)
	{
		if (!m_pSound1Patch && m_sound1_volume > 0.0f)
		{
			if (m_Sound1_source_name != NULL_STRING) {
				m_Sound1_source_ent = gEntList.FindEntityByName(NULL, m_Sound1_source_name);
				if (m_Sound1_source_ent != NULL) m_Sound1_source_entindex = m_Sound1_source_ent->entindex();
			}

			if (m_Sound1_source_ent == NULL) {
				m_Sound1_source_ent = this;
				m_Sound1_source_entindex = entindex();
			}

			CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
			CPASAttenuationFilter filter(this);
			m_pSound1Patch = controller.SoundCreate(filter, m_Sound1_source_entindex, STRING(m_sound1_soundname));
			controller.Play(m_pSound1Patch, 0.001f, 100);
		}

		if (!m_pSound2Patch && m_sound2_volume > 0.0f)
		{
			if (m_Sound2_source_name != NULL_STRING) {
				m_Sound2_source_ent = gEntList.FindEntityByName(NULL, m_Sound2_source_name);
				if (m_Sound2_source_ent != NULL) m_Sound2_source_entindex = m_Sound2_source_ent->entindex();
			}

			if (m_Sound2_source_ent == NULL) {
				m_Sound2_source_ent = this;
				m_Sound2_source_entindex = entindex();
			}

			CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
			CPASAttenuationFilter filter(this);
			m_pSound2Patch = controller.SoundCreate(filter, m_Sound2_source_entindex, STRING(m_sound2_soundname));
			controller.Play(m_pSound2Patch, 0.001f, 100);
		}

		if (!m_pSound3Patch && m_sound3_volume > 0.0f)
		{
			if (m_Sound3_source_name != NULL_STRING) {
				m_Sound3_source_ent = gEntList.FindEntityByName(NULL, m_Sound3_source_name);
				if (m_Sound3_source_ent != NULL) m_Sound3_source_entindex = m_Sound3_source_ent->entindex();
			}

			if (m_Sound3_source_ent == NULL) {
				m_Sound3_source_ent = this;
				m_Sound3_source_entindex = entindex();
			}

			CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
			CPASAttenuationFilter filter(this);
			m_pSound3Patch = controller.SoundCreate(filter, m_Sound3_source_entindex, STRING(m_sound3_soundname));
			controller.Play(m_pSound3Patch, 0.001f, 100);
		}

		if (!m_pSound4Patch && m_sound4_volume > 0.0f)
		{
			if (m_Sound4_source_name != NULL_STRING) {
				m_Sound4_source_ent = gEntList.FindEntityByName(NULL, m_Sound4_source_name);
				if (m_Sound4_source_ent != NULL) m_Sound4_source_entindex = m_Sound4_source_ent->entindex();
			}

			if (m_Sound4_source_ent == NULL) {
				m_Sound4_source_ent = this;
				m_Sound4_source_entindex = entindex();
			}

			CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
			CPASAttenuationFilter filter(this);
			m_pSound4Patch = controller.SoundCreate(filter, m_Sound4_source_entindex, STRING(m_sound4_soundname));
			controller.Play(m_pSound4Patch, 0.001f, 100);
		}
	}

	if (m_pSound1Patch)
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.SoundChangeVolume(m_pSound1Patch, m_sound1_volume, m_sound1_transitionlength);
	}

	if (m_pSound2Patch)
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.SoundChangeVolume(m_pSound2Patch, m_sound2_volume, m_sound2_transitionlength);
	}

	if (m_pSound3Patch)
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.SoundChangeVolume(m_pSound3Patch, m_sound3_volume, m_sound3_transitionlength);
	}

	if (m_pSound4Patch)
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.SoundChangeVolume(m_pSound4Patch, m_sound4_volume, m_sound4_transitionlength);
	}

	SetNextThink(CURTIME + 0.1f);
}
