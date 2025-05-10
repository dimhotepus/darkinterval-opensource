#ifndef TGAIMAGEPANEL_H
#define TGAIMAGEPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Panel.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Displays a tga image
//-----------------------------------------------------------------------------
class CRAWTGAImagePanel : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CRAWTGAImagePanel, vgui::Panel);

public:
	CRAWTGAImagePanel(vgui::Panel *parent, const char *name);

	~CRAWTGAImagePanel();

	void SetTGA(const char *filename);
	void SetTGANonMod(const char *filename);

	virtual void Paint(void);

	void DrawNewGameTGAImage(int chapter);

private:
	int m_iTextureID;
	int m_iImageWidth, m_iImageHeight;
	bool m_bHasValidTexture, m_bLoadedTexture;
	char m_szTGAName[256];
	int iChapter;
};

#endif //TGAIMAGEPANEL_H