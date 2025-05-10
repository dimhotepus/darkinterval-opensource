#include "cbase.h"
#include "itgaiconloader.h"
#include "bitmap/tgaloader.h"
#include "vgui/ISurface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

CRAWTGAImagePanel::CRAWTGAImagePanel(vgui::Panel *parent, const char *name) : BaseClass(parent, name)
{
	m_iTextureID = -1;
	m_bHasValidTexture = false;
	m_bLoadedTexture = false;
	m_szTGAName[0] = 0;
//	m_iImageWidth = 240;
//	m_iImageHeight = 240;
	iChapter = 0;

	SetPaintBackgroundEnabled(false);
}

CRAWTGAImagePanel::~CRAWTGAImagePanel()
{
	// release the texture memory
}

void CRAWTGAImagePanel::SetTGA(const char *filename)
{
	Q_snprintf(m_szTGAName, sizeof(m_szTGAName), "//MOD/%s", filename);
}

void CRAWTGAImagePanel::SetTGANonMod(const char *filename)
{
	Q_strcpy(m_szTGAName, filename);
}

void CRAWTGAImagePanel::DrawNewGameTGAImage(int chapter)
{
	iChapter = chapter;
}

void CRAWTGAImagePanel::Paint()
{
	if (!m_bLoadedTexture)
	{
		m_bLoadedTexture = true;
		// get a texture id, if we haven't already
		if (m_iTextureID < 0)
		{
			m_iTextureID = vgui::surface()->CreateNewTextureID(true);
			SetSize(450, 200);
		}

		char szTGA[_MAX_PATH];
		Q_snprintf(szTGA, sizeof(szTGA), "materials/vgui/chapters/chapter_%i.tga", iChapter);

		// load the file
		CUtlMemory<unsigned char> tga;
		if (TGALoader::LoadRGBA8888(szTGA, tga, m_iImageWidth, m_iImageHeight))
		{
			// set the textureID
			surface()->DrawSetTextureRGBA(m_iTextureID, tga.Base(), m_iImageWidth, m_iImageHeight, false, true);
			m_bHasValidTexture = true;
			// set our size to be the size of the tga
			SetSize(m_iImageWidth, m_iImageHeight);
		}
		else
		{
			m_bHasValidTexture = false;
		}
	}

	// draw the image
	int wide, tall;
	if (m_bHasValidTexture)
	{
		surface()->DrawGetTextureSize(m_iTextureID, wide, tall);
		surface()->DrawSetTexture(m_iTextureID);
		surface()->DrawSetColor(255, 255, 255, 255);
		surface()->DrawTexturedRect(0, 0, wide, tall);
	}
	else
	{
		// draw a black fill instead
		wide = 450, tall = 200;
		surface()->DrawSetColor(0, 0, 0, 255);
		surface()->DrawFilledRect(0, 0, wide, tall);
	}

	m_bLoadedTexture = false;
}
