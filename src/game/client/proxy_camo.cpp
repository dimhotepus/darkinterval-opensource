//========================================================================//
//
// Purpose: 
// DI NOTE: this is old abandoned attempt at fixing the proxy. Messy and fruitless.
// $NoKeywords: $
//=============================================================================//


#include "cbase.h"
// identifier was truncated to '255' characters in the debug information
#pragma warning(disable: 4786)
#ifdef DARKINTERVAL
#include "filesystem.h"
#include "debugoverlay_shared.h"
#endif
#include "proxyentity.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/itexture.h"
#include "bitmap/tgaloader.h"
#include "view.h"
#include "datacache/idatacache.h"
#include "materialsystem/imaterial.h"
#include "vtf/vtf.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#ifdef DARKINTERVAL
#define USE_INSTANCEDATA 1

ConVar di_camoproxy_debug("di_camoproxy_debug", "0");
#endif
class CCamoMaterialProxy;

class CCamoTextureRegen : public ITextureRegenerator
{
public:
	CCamoTextureRegen( CCamoMaterialProxy *pProxy ) : m_pProxy(pProxy) {} 
	virtual void RegenerateTextureBits( ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect );
	virtual void Release() {}

private:
	CCamoMaterialProxy *m_pProxy;
};

class CCamoMaterialProxy : public CEntityMaterialProxy
{
public:
	CCamoMaterialProxy();
	virtual ~CCamoMaterialProxy();
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind(C_BaseEntity *pC_BaseEntity );
	virtual IMaterial *GetMaterial();

	// This is made public because the regenerator has to access it from outside the proxy.
	void GenerateCamoTexture(ITexture* pTexture, IVTFTexture *pVTFTexture);

protected:
#if USE_INSTANCEDATA
	virtual void SetInstanceDataSize( int size );
	virtual void *FindInstanceData( C_BaseEntity *pEntity );
	virtual void *AllocateInstanceData( C_BaseEntity *pEntity );
#endif

private:
	void LoadCamoPattern( void );
	void GenerateRandomPointsInNormalizedCube( void );
	void GetColors( Vector &lighting, Vector &base, int index, 
						  const Vector &boxMin, const Vector &boxExtents, 
						  const Vector &forward, const Vector &right, const Vector &up,
						  const Vector& entityPosition );
	// this needs to go in a base class

private:
#if USE_INSTANCEDATA
	// stuff that needs to be in a base class.
	struct InstanceData_t
	{
		C_BaseEntity *pEntity;
		void *data;
		struct InstanceData_s *next;
	};
	
	struct CamoInstanceData_t
	{
		int dummy;
	};
#endif
	
	unsigned char *m_pCamoPatternImage;

#if USE_INSTANCEDATA
	int m_InstanceDataSize;
	InstanceData_t *m_InstanceDataListHead;
#endif
#ifdef DARKINTERVAL
	C_BaseEntity *BindArgToEntity(void *pArg);
#endif
	IMaterial *m_pMaterial;
	IMaterialVar *m_pCamoTextureVar;
	IMaterialVar *m_pCamoPatternTextureVar;
	Vector *m_pointsInNormalizedBox; // [m_CamoPatternNumColors]
#ifdef DARKINTERVAL
	char m_pCamoPatternTGAName[_MAX_PATH];
#endif
	int m_CamoPatternNumColors;
	int m_CamoPatternWidth;
	int m_CamoPatternHeight;
#if 0
	cache_user_t m_camoImageDataCache;
#endif
	unsigned char m_CamoPalette[256][3];
	// these represent that part of the entitiy's bounding box that we 
	// want to cast rays through to get colors for the camo
	Vector m_SubBoundingBoxMin; // normalized
	Vector m_SubBoundingBoxMax; // normalized

	CCamoTextureRegen m_TextureRegen;
	C_BaseEntity *m_pEnt;
};

void CCamoTextureRegen::RegenerateTextureBits(ITexture *pTexture, IVTFTexture *pVTFTexture, Rect_t *pSubRect)
{
	m_pProxy->GenerateCamoTexture(pTexture, pVTFTexture);
}

#pragma warning (disable:4355)

//-----------------------------------------------------------------------------
// Constructor.
//-----------------------------------------------------------------------------

CCamoMaterialProxy::CCamoMaterialProxy() : m_TextureRegen(this)
{
#if USE_INSTANCEDATA
	m_InstanceDataSize = 0;
#endif

#if 0
	memset( &m_camoImageDataCache, 0,sizeof( m_camoImageDataCache ) );
#endif

	m_pointsInNormalizedBox = NULL;

#if USE_INSTANCEDATA
	m_InstanceDataListHead = NULL;
#endif

	m_pCamoPatternImage = NULL; 
#ifdef DARKINTERVAL
	m_pCamoPatternTGAName[0] = 0;
#endif
	m_pMaterial = NULL;
	m_pCamoTextureVar = NULL;
	m_pCamoPatternTextureVar = NULL;
	m_pointsInNormalizedBox = NULL;
	m_pEnt = NULL;
}

#pragma warning (default:4355)

//-----------------------------------------------------------------------------
// Destructor.
//-----------------------------------------------------------------------------

CCamoMaterialProxy::~CCamoMaterialProxy()
{
#if USE_INSTANCEDATA
	InstanceData_t *curr = m_InstanceDataListHead;
	while( curr )
	{
		InstanceData_t *next;
#ifdef DARKINTERVAL
		next = (InstanceData_t*)(curr->next);
#else
		next = curr->next;
#endif
		delete curr;
		curr = next;
	}
	m_InstanceDataListHead = NULL;
#endif

	// Disconnect the texture regenerator...
	if (m_pCamoTextureVar)
	{
		ITexture *pCamoTexture = m_pCamoTextureVar->GetTextureValue();
		if (pCamoTexture)
			pCamoTexture->SetTextureRegenerator( NULL );
	}

	delete m_pCamoPatternImage;
	delete m_pointsInNormalizedBox;
}

#if USE_INSTANCEDATA
void CCamoMaterialProxy::SetInstanceDataSize( int size )
{
	m_InstanceDataSize = size;
}

void *CCamoMaterialProxy::FindInstanceData( C_BaseEntity *pEntity )
{
	InstanceData_t *curr = m_InstanceDataListHead;
	while( curr )
	{
		if( pEntity == curr->pEntity )
		{
			return curr->data;
		}
#ifdef DARKINTERVAL
		curr = (InstanceData_t*)(curr->next);
#else
		curr = curr->next;
#endif
	}
	return NULL;
}

void *CCamoMaterialProxy::AllocateInstanceData( C_BaseEntity *pEntity )
{
	InstanceData_t *newData = new InstanceData_t;
	newData->pEntity = pEntity;
#ifdef DARKINTERVAL
	newData->next = (InstanceData_s*)(m_InstanceDataListHead);
#else
	newData->next = m_InstanceDataListHead;
#endif
	m_InstanceDataListHead = newData;
	newData->data = new unsigned char[m_InstanceDataSize];
	return newData->data;
}
#endif

//-----------------------------------------------------------------------------
// Initialisation checks for correct VMT parameters.
//-----------------------------------------------------------------------------

bool CCamoMaterialProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	return false;
#if USE_INSTANCEDATA
	// set how big our instance data is.
	SetInstanceDataSize( sizeof( CamoInstanceData_t ) );
#endif

	// remember what material we belong to.
	m_pMaterial = pMaterial;

	// get pointers to material vars.
	bool found;

	m_pCamoTextureVar = m_pMaterial->FindVar( "$baseTexture", &found );
	if( !found )
	{
		m_pCamoTextureVar = NULL;
		return false;
	}
	ITexture *pCamoTexture = m_pCamoTextureVar->GetTextureValue();
	if (pCamoTexture)
		pCamoTexture->SetTextureRegenerator( &m_TextureRegen );
	
	// Need to get the palettized texture to create the procedural texture from
	// somewhere.
	m_pCamoPatternTextureVar = m_pMaterial->FindVar( "$camoPatternTexture", &found );
	if( !found )
	{
		m_pCamoTextureVar = NULL;
		return false;
	}
	
	IMaterialVar *subBoundingBoxMinVar, *subBoundingBoxMaxVar;

	subBoundingBoxMinVar = m_pMaterial->FindVar( "$camoBoundingBoxMin", &found, false );
	if( !found )
	{
		m_SubBoundingBoxMin = Vector( 0.0f, 0.0f, 0.0f );
	}
	else
	{
		subBoundingBoxMinVar->GetVecValue( m_SubBoundingBoxMin.Base(), 3 );
	}

	subBoundingBoxMaxVar = m_pMaterial->FindVar( "$camoBoundingBoxMax", &found, false );
	if( !found )
	{
		m_SubBoundingBoxMax = Vector( 1.0f, 1.0f, 1.0f );
	}
	else
	{
		subBoundingBoxMaxVar->GetVecValue( m_SubBoundingBoxMax.Base(), 3 );
	}
	
	LoadCamoPattern();
	GenerateRandomPointsInNormalizedCube();

	return true;
}

void CCamoMaterialProxy::GetColors(Vector &diffuseColor, Vector &baseColor, int index,
	const Vector &boxMin, const Vector &boxExtents,
	const Vector &forward, const Vector &right, const Vector &up,
	const Vector& entityPosition)
{
	Vector position, transformedPosition;
#ifdef DARKINTERVAL
	Vector vRight = MainViewRight();
	Vector vFwd = MainViewForward();
	Vector vUp = MainViewUp();
#endif
	position[0] = m_pointsInNormalizedBox[index][0] * boxExtents[0] + boxMin[0];
	position[1] = m_pointsInNormalizedBox[index][1] * boxExtents[1] + boxMin[1];
	position[2] = m_pointsInNormalizedBox[index][2] * boxExtents[2] + boxMin[2];
#ifdef DARKINTERVAL
	transformedPosition[0] = vRight[0] * position[0] + vFwd[0] * position[1] + vUp[0] * position[2];
	transformedPosition[1] = vRight[1] * position[0] + vFwd[1] * position[1] + vUp[1] * position[2];
	transformedPosition[2] = vRight[2] * position[0] + vFwd[2] * position[1] + vUp[2] * position[2];
#else
	transformedPosition[ 0 ] = right[ 0 ] * position[ 0 ] + forward[ 0 ] * position[ 1 ] + up[ 0 ] * position[ 2 ];
	transformedPosition[ 1 ] = right[ 1 ] * position[ 0 ] + forward[ 1 ] * position[ 1 ] + up[ 1 ] * position[ 2 ];
	transformedPosition[ 2 ] = right[ 2 ] * position[ 0 ] + forward[ 2 ] * position[ 1 ] + up[ 2 ] * position[ 2 ];
#endif
	transformedPosition = transformedPosition + entityPosition;
#ifdef DARKINTERVAL
	Vector direction = transformedPosition - MainViewOrigin();
#else
	Vector direction = transformedPosition - CurrentViewOrigin();
#endif
	VectorNormalize(direction);
	direction = direction * (COORD_EXTENT * 1.74f);
	Vector endPoint = position + direction;

	if( di_camoproxy_debug.GetBool())
	NDebugOverlay::Line(m_pEnt->WorldSpaceCenter(), transformedPosition, 255, 150, 10, true, 0.01f);

	// baseColor is already in gamma space
#if 0
	engine->TraceLineMaterialAndLighting( NULL, endPoint, diffuseColor, baseColor );
#else
	engine->TraceLineMaterialAndLighting(transformedPosition, endPoint, diffuseColor, baseColor);
#endif

	// Convert from linear to gamma space
	diffuseColor[0] = pow(diffuseColor[0], 1.0f / 2.2f);
	diffuseColor[1] = pow(diffuseColor[1], 1.0f / 2.2f);
	diffuseColor[2] = pow(diffuseColor[2], 1.0f / 2.2f);

#if 0
	Msg("%f %f %f\n",
		diffuseColor[0],
		diffuseColor[1],
		diffuseColor[2]);
#endif

#if 0
	float max;
	max = diffuseColor[0];
	if (diffuseColor[1] > max)
	{
		max = diffuseColor[1];
	}
	if (diffuseColor[2] > max)
	{
		max = diffuseColor[2];
	}
	if (max > 1.0f)
	{
		max = 1.0f / max;
		diffuseColor = diffuseColor * max;
	}
#else
	if (diffuseColor[0] > 1.0f)
	{
		diffuseColor[0] = 1.0f;
	}
	if (diffuseColor[1] > 1.0f)
	{
		diffuseColor[1] = 1.0f;
	}
	if (diffuseColor[2] > 1.0f)
	{
		diffuseColor[2] = 1.0f;
	}
#endif
}

//-----------------------------------------------------------------------------
// Generates the camouflage texture that's placed into the procedural-flagged
// VTF basetexture on each tick.
//-----------------------------------------------------------------------------
void CCamoMaterialProxy::GenerateCamoTexture(ITexture* pTexture, IVTFTexture *pVTFTexture)
{
	if (!m_pEnt)
	{
#ifdef DARKINTERVAL
		Warning("No m_pEnt - no camo can be generated!\n");
#endif
		return;
	}

#if USE_INSTANCEDATA
	CamoInstanceData_t *pInstanceData;
	pInstanceData = (CamoInstanceData_t *)FindInstanceData(m_pEnt);
	if (!pInstanceData)
	{
		pInstanceData = (CamoInstanceData_t *)AllocateInstanceData(m_pEnt);
		if (!pInstanceData)
		{
#ifdef DARKINTERVAL
			Warning("No instance data for m_pEnt!\n");
#endif
			return;
		}
		// init the instance data
	}
#endif

	Vector entityPosition;
	entityPosition = m_pEnt->GetLocalOrigin();

	QAngle entityAngles;
	entityAngles = m_pEnt->GetLocalAngles();

	// Get the bounding box for the entity
	Vector mins, maxs;
#ifdef DARKINTERVAL
	mins = m_pEnt->WorldAlignMins() * Vector(2, 2, 1);
	maxs = m_pEnt->WorldAlignMaxs() * Vector(2, 2, 1);

	if ( di_camoproxy_debug.GetBool() )
		NDebugOverlay::Box(entityPosition, mins, maxs, 15, 255, 15, 0, 0.01f);
#else
	mins = m_pEnt->WorldAlignMins();
	maxs = m_pEnt->WorldAlignMaxs();
#endif

	Vector traceDirection;
	Vector traceEnd;
	trace_t	traceResult;

	Vector forward, right, up;
	AngleVectors(entityAngles, &forward, &right, &up);

	Vector position, transformedPosition;
	Vector maxsMinusMins = maxs - mins;

	Vector diffuseColor[256];
	Vector baseColor;

	unsigned char camoPalette[256][3];
	// Calculate the camo palette
	DevMsg("Calculating camo palette\n");
	int i;
	for (i = 0; i < m_CamoPatternNumColors; i++)
	{
		GetColors(diffuseColor[i], baseColor, i,
			mins, maxsMinusMins, forward, right, up, entityPosition);
#if 1
		camoPalette[i][0] = diffuseColor[i][0] * baseColor[0] * 255.0f;
		camoPalette[i][1] = diffuseColor[i][1] * baseColor[1] * 255.0f;
		camoPalette[i][2] = diffuseColor[i][2] * baseColor[2] * 255.0f;
#endif
#if 0
		camoPalette[ i ][ 0 ] = baseColor[ 0 ] * 255.0f;
		camoPalette[ i ][ 1 ] = baseColor[ 1 ] * 255.0f;
		camoPalette[ i ][ 2 ] = baseColor[ 2 ] * 255.0f;
#endif
#if 0
		camoPalette[ i ][ 0 ] = diffuseColor[ i ][ 0 ] * 255.0f;
		camoPalette[ i ][ 1 ] = diffuseColor[ i ][ 1 ] * 255.0f;
		camoPalette[ i ][ 2 ] = diffuseColor[ i ][ 2 ] * 255.0f;
#endif
	}

	int width = pVTFTexture->Width();
	int height = pVTFTexture->Height();
	if (width != m_CamoPatternWidth || height != m_CamoPatternHeight)
	{
		return;
	}

	unsigned char *imageData = pVTFTexture->ImageData(0, 0, 0);
	enum ImageFormat imageFormat = pVTFTexture->Format();
	if (imageFormat != IMAGE_FORMAT_RGB888)
	{
#ifdef DARKINTERVAL
		Warning("Basetexture for Camo is not RBG888, it's %i!\n", (int)imageFormat);
#endif
		return;
	}

	// optimize
#if 1
	int x, y;
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			int offset = 3 * (x + y * width);
			assert(offset < width * height * 3);
			int paletteID = m_pCamoPatternImage[x + y * width];
			assert(paletteID < 256);
#if 1
			imageData[offset + 0] = camoPalette[paletteID][0];
			imageData[offset + 1] = camoPalette[paletteID][1];
			imageData[offset + 2] = camoPalette[paletteID][2];
#else
			imageData[offset] = 255;
			imageData[offset + 1] = 0;
			imageData[offset + 2] = 0;
#endif
		}
	}
#endif
}

#ifdef DARKINTERVAL
//-----------------------------------------------------------------------------
// Bind the proxy functions to the actual entity
//-----------------------------------------------------------------------------
C_BaseEntity *CCamoMaterialProxy::BindArgToEntity(void *pArg)
{
	IClientRenderable *pRend = (IClientRenderable *)pArg;
	return (C_BaseEntity*)pRend->GetIClientUnknown()->GetBaseEntity();
}
#endif
//-----------------------------------------------------------------------------
// Called when the texture is bound...
//-----------------------------------------------------------------------------
void CCamoMaterialProxy::OnBind(C_BaseEntity *pEntity)
{
#ifdef DARKINTERVAL
	if (!pEntity)
		return;
#endif
	if (!m_pCamoTextureVar)
	{
		return;
	}

#ifdef DARKINTERVAL
//	C_BaseEntity *pBindEntity = BindArgToEntity(pEntity);
//	if (!pBindEntity) { Msg("Cannot find base entity to bind camo proxy!\n"); return; }
#endif

	m_pEnt = pEntity;
	ITexture *pCamoTexture = m_pCamoTextureVar->GetTextureValue();
	pCamoTexture->Download();

	// Mark it so it doesn't get regenerated on task switch
	m_pEnt = NULL;
}

//-----------------------------------------------------------------------------
// Loads the TGA palette texture. 
// It must be 8-bit greyscale and it's filename's hardcoded, currently.
//-----------------------------------------------------------------------------

void CCamoMaterialProxy::LoadCamoPattern(void)
{
#if 0
	// hack - need to figure out a name to attach that isn't too long.
	m_pCamoPatternImage =
		(unsigned char *)datacache->FindByName(&m_camoImageDataCache, "camopattern");

	if (m_pCamoPatternImage)
	{
		// is already in the cache.
		return m_pCamoPatternImage;
	}
#endif
#ifdef DARKINTERVAL
	Q_snprintf(m_pCamoPatternTGAName, sizeof(m_pCamoPatternTGAName), 
		"//MOD/materials/models/_Monsters/Combine/Combine_Camo/pattern_rgba.tga");
#endif
	enum ImageFormat indexImageFormat;
	int indexImageSize;
	float dummyGamma;

	if (!TGALoader::GetInfo(m_pCamoPatternTGAName,
		&m_CamoPatternWidth, &m_CamoPatternHeight, &indexImageFormat, &dummyGamma))
	{
#ifdef DARKINTERVAL
		Warning("Can't get tga info for camo material from texture %s !\n", m_pCamoPatternTGAName);
#endif
		m_pCamoTextureVar = NULL;
		return;
	}
#ifndef DARKINTERVAL
	if (indexImageFormat != IMAGE_FORMAT_I8)
	{
	//	Warning("Camo texture %s must be 8-bit greyscale but it's format %i!\n", m_pCamoPatternTGAName, (int)indexImageFormat );
		m_pCamoTextureVar = NULL;
		return;
	}
#endif

	indexImageSize = ImageLoader::GetMemRequired(m_CamoPatternWidth, m_CamoPatternHeight, 1, indexImageFormat, false);

#if 0
	m_pCamoPatternImage = (unsigned char *)
		datacache->Alloc(&m_camoImageDataCache, indexImageSize, "camopattern");
#endif

	m_pCamoPatternImage = (unsigned char *)new unsigned char[indexImageSize];
	if (!m_pCamoPatternImage)
	{
		m_pCamoTextureVar = NULL;
		return;
	}
#ifdef DARKINTERVAL
#if 1
	if (!TGALoader::Load(m_pCamoPatternImage, m_pCamoPatternTGAName,
		m_CamoPatternWidth, m_CamoPatternHeight, IMAGE_FORMAT_RGBA8888, dummyGamma, false))
#else
	CUtlMemory<unsigned char> tga;

	if( !TGALoader::LoadRGBA8888(m_pCamoPatternTGAName, tga, m_CamoPatternWidth, m_CamoPatternHeight ))
#endif
	{
		Warning("Cannot load %s or its format is not TGA greyscale!\n", m_pCamoPatternTGAName);
		m_pCamoTextureVar = NULL;
		return;
	}
#else
	if ( !TGALoader::Load(m_pCamoPatternImage, m_pCamoPatternTextureVar->GetStringValue(),
		m_CamoPatternWidth, m_CamoPatternHeight, IMAGE_FORMAT_I8, dummyGamma, false ) )
	{
		//Warning( "camo texture hl2/materials/models/combine_elite/camo7paletted.tga must be grey-scale" );
		m_pCamoTextureVar = NULL;
		return;
	}
#endif
	bool colorUsed[256];
	int colorRemap[256];
	// count the number of colors used in the image.
	int i;
	for (i = 0; i < 256; i++)
	{
		colorUsed[i] = false;
	}
	for (i = 0; i < indexImageSize; i++)
	{
		colorUsed[m_pCamoPatternImage[i]] = true;
	}
	m_CamoPatternNumColors = 0;
	for (i = 0; i < 256; i++)
	{
		if (colorUsed[i])
		{
			colorRemap[i] = m_CamoPatternNumColors;
			m_CamoPatternNumColors++;
		}
	}
	// remap the color to the beginning of the palette.
	for (i = 0; i < indexImageSize; i++)
	{
		m_pCamoPatternImage[i] = colorRemap[m_pCamoPatternImage[i]];
	}
}

void CCamoMaterialProxy::GenerateRandomPointsInNormalizedCube(void)
{
	m_pointsInNormalizedBox = new Vector[m_CamoPatternNumColors];
	if (!m_pointsInNormalizedBox)
	{
		m_pCamoTextureVar = NULL;
		return;
	}

	int i;
	for (i = 0; i < m_CamoPatternNumColors; i++)
	{
		m_pointsInNormalizedBox[i][0] = random->RandomFloat(m_SubBoundingBoxMin[0], m_SubBoundingBoxMax[0]);
		m_pointsInNormalizedBox[i][1] = random->RandomFloat(m_SubBoundingBoxMin[1], m_SubBoundingBoxMax[1]);
		m_pointsInNormalizedBox[i][2] = random->RandomFloat(m_SubBoundingBoxMin[2], m_SubBoundingBoxMax[2]);
	}
}

IMaterial *CCamoMaterialProxy::GetMaterial()
{
#ifdef DARKINTERVAL
	return m_pCamoPatternTextureVar->GetOwningMaterial();
#else
	return m_pMaterial;
#endif
}

EXPOSE_INTERFACE( CCamoMaterialProxy, IMaterialProxy, "Camo" IMATERIAL_PROXY_INTERFACE_VERSION );
