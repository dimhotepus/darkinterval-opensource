#include "cbase.h"
#include "inewmenubase.h"
#include "ioptionsmaindialogue.h"
#include <vgui/Ivgui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include "ienginevgui.h"
#include "igenericmenuutil.h"
#include <vgui/ISurface.h>
#include "vgui/IInput.h"
#include "iclientmode.h"
#include "vgui_video.h"
#include "engine/ienginesound.h"
#include "IGameUIFuncs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// See interface.h/.cpp for specifics:  basically this ensures that we actually Sys_UnloadModule the dll and that we don't call Sys_LoadModule 
//  over and over again.
static CDllDemandLoader g_GameUI("GameUI");

INewMenuBase::INewMenuBase(vgui::VPANEL parent) : BaseClass(NULL, "NewMainMenuBase")
{
	SetParent(parent);
	SetCloseButtonVisible(false);
	SetDragEnabled(false);
	SetShowDragHelper(false);
	SetVisible(false);
	SetSizeable(false);
	SetTitleBarVisible(false);
	SetKeyBoardInputEnabled(true);

	AddMenuPanels();

	CreateInterfaceFn gameUIFactory = g_GameUI.GetFactory();
	if (gameUIFactory)
	{
		m_pGameUI = (IGameUI*)gameUIFactory(GAMEUI_INTERFACE_VERSION, NULL);
	}

	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);
}

void INewMenuBase::AddMenuPanels()
{
	menuList = new INewMenuMainList(this);
	gameTitleGlow = new vgui::Label(this, "gameTitle", "#darkinterval_Title");
	gameTitle2Glow = new vgui::Label(this, "gameTitle2", "#darkinterval_Title2");
	gameTitle = new vgui::Label(this, "gameTitle", "#darkinterval_Title");
	gameTitle2 = new vgui::Label(this, "gameTitle2", "#darkinterval_Title2");
	SetActiveMenu("NewMainMenuList");
}

void INewMenuBase::SetActiveMenu(const char* newMenu)
{
	if (!Q_strcmp(menuList->GetName(), newMenu))
	{
		menuList->SetVisible(true);
		menuList->RequestFocus();
	}
	else
	{
		menuList->SetVisible(false);
	}

	Activate();
}

void INewMenuBase::OnKeyCodePressed(vgui::KeyCode code)
{
	BaseClass::OnKeyCodePressed(code);
}

void INewMenuBase::OnKeyCodeTyped(vgui::KeyCode code)
{
	BaseClass::OnKeyCodeTyped(code);

	// HACK: Allow F key bindings to operate even here
	if (IsPC() && code >= KEY_F1 && code <= KEY_F12)
	{
		// See if there is a binding for the FKey
		const char *binding = gameuifuncs->GetBindingForButtonCode(code);
		if (binding && binding[0])
		{
			// submit the entry as a console commmand
			char szCommand[256];
			Q_strncpy(szCommand, binding, sizeof(szCommand));
			engine->ClientCmd_Unrestricted(szCommand);
		}
	}
}

void INewMenuBase::PerformLayout()
{
	BaseClass::PerformLayout();
	SetWide(ScreenWidth() * 0.25);
	SetTall(ScreenHeight() * 0.75f);
//	SetPos(ScreenWidth() / 2 - GetWide() / 2, ScreenHeight() / 4); // central positioning
	SetPos(ScreenWidth() / 16, ScreenHeight() / 4); // left side positioning
				
	gameTitle->SizeToContents();
	gameTitle->SetPos(0, 0);
	gameTitle->SetContentAlignment(vgui::Label::a_west);
	gameTitle->SetSize((ScreenWidth()), ScreenHeight() * 0.25);

	gameTitle2->SizeToContents();
	gameTitle2->SetPos(0, 0);
	gameTitle2->SetContentAlignment(vgui::Label::a_west);
	gameTitle2->SetSize((ScreenWidth()), ScreenHeight() * 0.4);
	
	gameTitleGlow->SizeToContents();
	gameTitleGlow->SetPos(0, 0);
	gameTitleGlow->SetContentAlignment(vgui::Label::a_west);
	gameTitleGlow->SetSize((ScreenWidth()), ScreenHeight() * 0.25);

	gameTitle2Glow->SizeToContents();
	gameTitle2Glow->SetPos(0, 0);
	gameTitle2Glow->SetContentAlignment(vgui::Label::a_west);
	gameTitle2Glow->SetSize((ScreenWidth()), ScreenHeight() * 0.4);

	menuList->SetPos(0, 0);
	menuList->SetSize(GetWide(), GetTall() * 0.5);
}

void INewMenuBase::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	IScheme *pClientScheme = vgui::scheme()->GetIScheme(vgui::scheme()->GetScheme("ClientScheme"));

	gameTitle->SetFont(pClientScheme->GetFont("ClientTitleFont", true));
	gameTitle2->SetFont(pClientScheme->GetFont("ClientTitleFont", true));

	gameTitleGlow->SetFont(pClientScheme->GetFont("ClientTitleFontGlow", true));
	gameTitle2Glow->SetFont(pClientScheme->GetFont("ClientTitleFontGlow", true));

	gameTitle->SetAlpha(100);
	gameTitle2->SetAlpha(100);

	gameTitleGlow->SetAlpha(50);
	gameTitle2Glow->SetAlpha(50);

	SetAlpha(0);

	m_pScheme = pScheme;
}

void INewMenuBase::OnCommand(const char *command)
{
	if (!Q_strcmp(command, "Close")) return;

	SetActiveMenu(command);

	IGenericMenuUtil::CloseGameFrames(GetVPanel());
}

void INewMenuBase::OnThink()
{
	BaseClass::OnThink();

	if (engine->IsDrawingLoadingImage() && IsVisible())
	{
		SetActiveMenu("NewMainMenuList");
		SetVisible(false);
	}

	if (IsVisible() && GetAlpha() < 200)
	{
		SetAlpha(GetAlpha() + 1);
	}
	
	SetBgColor(m_pScheme->GetColor("Orange", BLANK));
}

void INewMenuBase::OnTick()
{
	BaseClass::OnTick();

	if (!engine->IsDrawingLoadingImage() && !IsVisible())
	{
		Activate();
		SetVisible(true);
	}
}

//========================================================================//
//
// Purpose: Menu background video player
//
//=============================================================================

INewMenuVideoPanel::INewMenuVideoPanel(unsigned int nXPos, unsigned int nYPos, unsigned int nHeight, unsigned int nWidth, bool allowAlternateMedia) :
	BaseClass(NULL, "INewMenuVideoPanel"),
	m_VideoMaterial(NULL),
	m_nPlaybackWidth(0),
	m_nPlaybackHeight(0),
	m_bAllowAlternateMedia(allowAlternateMedia)
{
	vgui::VPANEL pParent = enginevgui->GetPanel(PANEL_GAMEUIDLL);
	SetParent(pParent);
	SetVisible(false);
	
	m_bBlackBackground = false;
	m_bLooped = false;

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	SetProportional(false);
	SetVisible(true);
	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);

	// Set us up
	SetTall(nHeight);
	SetWide(nWidth);
	SetPos(nXPos, nYPos);

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/INewMenuVideoPanelScheme.res", "INewMenuVideoPanelScheme"));
	LoadControlSettings("resource/UI/INewMenuVideoPanel.res");
}

//-----------------------------------------------------------------------------
// Properly shutdown out materials
//-----------------------------------------------------------------------------
INewMenuVideoPanel::~INewMenuVideoPanel(void)
{
	SetParent((vgui::Panel *) NULL);

	// Shut down this video, destroy the video material
	if (g_pVideo != NULL && m_VideoMaterial != NULL)
	{
		g_pVideo->DestroyVideoMaterial(m_VideoMaterial);
		m_VideoMaterial = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Begins playback of a movie
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool INewMenuVideoPanel::BeginPlayback(const char *pFilename, bool bLooped)
{
	// need working video services
	if (g_pVideo == NULL)
		return false;

	// Create a new video material
	if (m_VideoMaterial != NULL)
	{
		g_pVideo->DestroyVideoMaterial(m_VideoMaterial);
		m_VideoMaterial = NULL;
	}

	m_VideoMaterial = g_pVideo->CreateVideoMaterial("VideoMaterial", pFilename, "GAME",
		VideoPlaybackFlags::DEFAULT_MATERIAL_OPTIONS,
		VideoSystem::DETERMINE_FROM_FILE_EXTENSION, m_bAllowAlternateMedia);

	if (m_VideoMaterial == NULL)
		return false;

	m_VideoMaterial->SetLooping(bLooped);
	
	// We want to be the sole audio source
	// FIXME: This may not always be true!
	enginesound->NotifyBeginMoviePlayback();

	int nWidth, nHeight;
	m_VideoMaterial->GetVideoImageSize(&nWidth, &nHeight);
	m_VideoMaterial->GetVideoTexCoordRange(&m_flU, &m_flV);
	m_pMaterial = m_VideoMaterial->GetMaterial();


	float flFrameRatio = ((float)GetWide() / (float)GetTall());
	float flVideoRatio = ((float)nWidth / (float)nHeight);

	if (flVideoRatio > flFrameRatio)
	{
		m_nPlaybackWidth = GetWide();
		m_nPlaybackHeight = (GetWide() / flVideoRatio);
	}
	else if (flVideoRatio < flFrameRatio)
	{
		m_nPlaybackWidth = (GetTall() * flVideoRatio);
		m_nPlaybackHeight = GetTall();
	}
	else
	{
		m_nPlaybackWidth = GetWide();
		m_nPlaybackHeight = GetTall();
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void INewMenuVideoPanel::Activate(void)
{
//	MoveToFront();
//	RequestFocus();
	SetVisible(true);
	SetEnabled(false);

	vgui::surface()->SetMinimized(GetVPanel(), false);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void INewMenuVideoPanel::DoModal(void)
{
//	MakePopup();
	Activate();

//	vgui::input()->SetAppModalSurface(GetVPanel());
//	vgui::surface()->RestrictPaintToSinglePanel(GetVPanel());
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void INewMenuVideoPanel::OnClose(void)
{
	enginesound->NotifyEndMoviePlayback();
	BaseClass::OnClose();

	if (vgui::input()->GetAppModalSurface() == GetVPanel())
	{
		vgui::input()->ReleaseAppModalSurface();
	}

	vgui::surface()->RestrictPaintToSinglePanel(NULL);
	
	SetVisible(false);
	MarkForDeletion();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void INewMenuVideoPanel::GetPanelPos(int &xpos, int &ypos)
{
	xpos = ((float)(GetWide() - m_nPlaybackWidth) / 2);
	ypos = ((float)(GetTall() - m_nPlaybackHeight) / 2);
}

//-----------------------------------------------------------------------------
// Purpose: Update and draw the frame
//-----------------------------------------------------------------------------
void INewMenuVideoPanel::Paint(void)
{
	BaseClass::Paint();

	if (engine->IsDrawingLoadingImage() || engine->IsConnected()) return; // don't draw it on pause or during load

	if (m_VideoMaterial == NULL)
		return;

	if (m_VideoMaterial->Update() == false)
	{
		// Issue a close command
		OnVideoOver();
		OnClose();
	}

	// Sit in the "center"
	int xpos, ypos;
	GetPanelPos(xpos, ypos);

	// Black out the background (we could omit drawing under the video surface, but this is straight-forward)
	if (m_bBlackBackground)
	{
		vgui::surface()->DrawSetColor(0, 0, 0, 255);
		vgui::surface()->DrawFilledRect(0, 0, GetWide(), GetTall());
	}

	// Draw the polys to draw this out
	CMatRenderContextPtr pRenderContext(materials);

	pRenderContext->MatrixMode(MATERIAL_VIEW);
	pRenderContext->PushMatrix();
	pRenderContext->LoadIdentity();

	pRenderContext->MatrixMode(MATERIAL_PROJECTION);
	pRenderContext->PushMatrix();
	pRenderContext->LoadIdentity();

	pRenderContext->Bind(m_pMaterial, NULL);

	CMeshBuilder meshBuilder;
	IMesh* pMesh = pRenderContext->GetDynamicMesh(true);
	meshBuilder.Begin(pMesh, MATERIAL_QUADS, 1);

	float flLeftX = xpos;
	float flRightX = xpos + (m_nPlaybackWidth - 1);

	float flTopY = ypos;
	float flBottomY = ypos + (m_nPlaybackHeight - 1);

	// Map our UVs to cut out just the portion of the video we're interested in
	float flLeftU = 0.0f;
	float flTopV = 0.0f;

	// We need to subtract off a pixel to make sure we don't bleed
	float flRightU = m_flU - (1.0f / (float)m_nPlaybackWidth);
	float flBottomV = m_flV - (1.0f / (float)m_nPlaybackHeight);

	// Get the current viewport size
	int vx, vy, vw, vh;
	pRenderContext->GetViewport(vx, vy, vw, vh);

	// map from screen pixel coords to -1..1
	flRightX = FLerp(-1, 1, 0, vw, flRightX);
	flLeftX = FLerp(-1, 1, 0, vw, flLeftX);
	flTopY = FLerp(1, -1, 0, vh, flTopY);
	flBottomY = FLerp(1, -1, 0, vh, flBottomY);

	float alpha = ((float)GetFgColor()[3] / 255.0f);

	for (int corner = 0; corner<4; corner++)
	{
		bool bLeft = (corner == 0) || (corner == 3);
		meshBuilder.Position3f((bLeft) ? flLeftX : flRightX, (corner & 2) ? flBottomY : flTopY, 0.0f);
		meshBuilder.Normal3f(0.0f, 0.0f, 1.0f);
		meshBuilder.TexCoord2f(0, (bLeft) ? flLeftU : flRightU, (corner & 2) ? flBottomV : flTopV);
		meshBuilder.TangentS3f(0.0f, 1.0f, 0.0f);
		meshBuilder.TangentT3f(1.0f, 0.0f, 0.0f);
		meshBuilder.Color4f(1.0f, 1.0f, 1.0f, alpha);
		meshBuilder.AdvanceVertex();
	}

	meshBuilder.End();
	pMesh->Draw();

	pRenderContext->MatrixMode(MATERIAL_VIEW);
	pRenderContext->PopMatrix();

	pRenderContext->MatrixMode(MATERIAL_PROJECTION);
	pRenderContext->PopMatrix();
}

//-----------------------------------------------------------------------------
// Purpose: Create and playback a video in a panel
// Input  : nWidth - Width of panel (in pixels) 
//			nHeight - Height of panel
//			*pVideoFilename - Name of the file to play
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool NewMenuVideoPanel_Create(unsigned int nXPos, unsigned int nYPos,
	unsigned int nWidth, unsigned int nHeight,
	const char *pVideoFilename,
	bool bLooped)
{
	// Create the base video panel
	INewMenuVideoPanel *pNewMenuVideoPanel = new INewMenuVideoPanel(nXPos, nYPos, nHeight, nWidth, false);
	if (pNewMenuVideoPanel == NULL)
		return false;
	
	// Start it going
	if (pNewMenuVideoPanel->BeginPlayback(pVideoFilename, bLooped) == false)
	{
		delete pNewMenuVideoPanel;
		return false;
	}

	// Take control
	pNewMenuVideoPanel->DoModal();

	return true;
}
