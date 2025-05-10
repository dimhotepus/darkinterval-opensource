#include "cbase.h"
#include "ioptionspanel_video.h"
#include <vgui/Ivgui.h>
#include <vgui_controls/button.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Divider.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Tooltip.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/URLLabel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Slider.h>
#include "vgui/ISurface.h"
#include "igameuifuncs.h"
#include "estranged_system_caps.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
using namespace vgui;

COptionsPanel_Video::COptionsPanel_Video(vgui::Panel *parent) : PropertyPage(parent, NULL) //(VPANEL parent) : BaseClass(NULL, "DI_Options_Video")
{
	m_pModelDetailBox = NULL;
	m_pTextureSizeBox = NULL;
	m_pScreenSpaceReflectionsBox = NULL;
	m_pSpecularityBox = NULL;
	m_pAntialiasingBox = NULL;
	m_pFiltrationBox = NULL;
	m_pShadowDetailBox = NULL;
	m_pHDRSettingsBox = NULL;

	m_pMulticoreBox = NULL;
	m_pMotionBlurButton = NULL;
	m_pColorCorrectionButton = NULL;
	m_pVignetteButton = NULL;
	m_pChromaticButton = NULL;
	m_pDOFButton = NULL;

	DrawOptionsItems();
	m_bRequireRestart = false;
	LoadControlSettingsAndUserConfig("resource/ui/di_options_video.res");

	if (ScreenHeight() < 600)
		LoadControlSettingsAndUserConfig("resource/ui/di_options_video_640.res");
//	ivgui()->AddTickSignal(GetVPanel(), 100);
}

void COptionsPanel_Video::DrawOptionsItems()
{
	DrawModelDetailBox();
	DrawTextureSizeBox();
	DrawScreenSpaceReflectionsBox();
	DrawSpecularityBox();
	DrawAntialiasingBox();
	DrawFiltrationBox();
	DrawShadowDetailBox();
	DrawHDRSettingsBox();
	DrawMulticoreBox();
	DrawMotionBlurCheckButton();
	DrawColorCorrectionCheckButton();
//	DrawVignetteCheckButton();
	DrawChromaticCheckButton();
//	DrawDOFCheckButton();
}

void COptionsPanel_Video::DrawMenuLabels()
{
// pfft
}

void COptionsPanel_Video::DrawModelDetailBox()
{
	m_pModelDetailBox = new ComboBox(this, "ModelDetailBox", 6, false);
	m_pModelDetailBox->AddItem("#DI_Options_ModelDetail_Low", NULL);
	m_pModelDetailBox->AddItem("#DI_Options_ModelDetail_Medium", NULL);
	m_pModelDetailBox->AddItem("#DI_Options_ModelDetail_High", NULL);
	m_pModelDetailBox->GetTooltip()->SetText("#DI_Options_ModelDetail_TT");
}

void COptionsPanel_Video::DrawTextureSizeBox()
{
	m_pTextureSizeBox = new ComboBox(this, "TextureSizeBox", 6, false);
	m_pTextureSizeBox->AddItem("#DI_Options_TextureSize_Small", NULL);
	m_pTextureSizeBox->AddItem("#DI_Options_TextureSize_Medium", NULL);
	m_pTextureSizeBox->AddItem("#DI_Options_TextureSize_Large", NULL);
	m_pTextureSizeBox->GetTooltip()->SetText("#DI_Options_TextureSize_TT");
}

void COptionsPanel_Video::DrawScreenSpaceReflectionsBox()
{
	m_pScreenSpaceReflectionsBox = new ComboBox(this, "ScreenSpaceReflectionsBox", 8, false);
	m_pScreenSpaceReflectionsBox->AddItem("#DI_Options_SSReflections_None", NULL);
	m_pScreenSpaceReflectionsBox->AddItem("#DI_Options_SSReflections_Simple", NULL);
	m_pScreenSpaceReflectionsBox->AddItem("#DI_Options_SSReflections_Normal", NULL);
	m_pScreenSpaceReflectionsBox->AddItem("#DI_Options_SSReflections_Full", NULL);
	m_pScreenSpaceReflectionsBox->GetTooltip()->SetText("#DI_Options_SSReflections_TT");
}

void COptionsPanel_Video::DrawSpecularityBox()
{
	m_pSpecularityBox = new ComboBox(this, "SpecularityBox", 8, false);
	m_pSpecularityBox->AddItem("#DI_Options_Specularity_None", NULL);
	m_pSpecularityBox->AddItem("#DI_Options_Specularity_Cubemaps", NULL);
	m_pSpecularityBox->AddItem("#DI_Options_Specularity_Phong", NULL);
	m_pSpecularityBox->AddItem("#DI_Options_Specularity_Full", NULL);
	m_pSpecularityBox->GetTooltip()->SetText("#DI_Options_Specularity_TT");
}

void COptionsPanel_Video::DrawAntialiasingBox()
{
	//	m_pAntialiasingBox = new ComboBox(this, "AntialiasingBox", 8, false);
	//	m_pAntialiasingBox->AddItem("#DI_Options_Antialiasing_2xMSAA", NULL);
	//	m_pAntialiasingBox->AddItem("#DI_Options_Antialiasing_4xMSAA", NULL);
	//	m_pAntialiasingBox->AddItem("#DI_Options_Antialiasing_8xMSAA", NULL);

		// Build list of MSAA and CSAA modes, based upon those which are supported by the device
		//
		// The modes that we've seen in the wild to date are as follows (in perf order, fastest to slowest)
		//
		//								2x	4x	6x	8x	16x	8x	16xQ
		//		Texture/Shader Samples	1	1	1	1	1	1	1
		//		Stored Color/Z Samples	2	4	6	4	4	8	8
		//		Coverage Samples		2	4	6	8	16	8	16
		//		MSAA or CSAA			M	M	M	C	C	M	C
		//
		//	The CSAA modes are nVidia only (added in the G80 generation of GPUs)
		//
	m_nNumAAModes = 0;
	m_pAntialiasingBox = new ComboBox(this, "AntialiasingBox", 10, false);
	m_pAntialiasingBox->AddItem("#DI_Options_Antialiasing_None", NULL);
	m_nAAModes[m_nNumAAModes].m_nNumSamples = 1;
	m_nAAModes[m_nNumAAModes].m_nQualityLevel = 0;
	m_nNumAAModes++;

	m_pAntialiasingBox->GetTooltip()->SetText("#DI_Options_Antialiasing_TT");

	if (!materials->SupportsMSAAMode(2))
	{
		m_pAntialiasingBox->AddItem("#DI_Options_Antialiasing_None", NULL);
		m_pAntialiasingBox->SetEnabled(false);
	}
	else
	{
		if (materials->SupportsMSAAMode(2))
		{
			m_pAntialiasingBox->AddItem("#DI_Options_Antialiasing_2xMSAA", NULL);
			m_nAAModes[m_nNumAAModes].m_nNumSamples = 2;
			m_nAAModes[m_nNumAAModes].m_nQualityLevel = 0;
			m_nNumAAModes++;
		}

		if (materials->SupportsMSAAMode(4))
		{
			m_pAntialiasingBox->AddItem("#DI_Options_Antialiasing_4xMSAA", NULL);
			m_nAAModes[m_nNumAAModes].m_nNumSamples = 4;
			m_nAAModes[m_nNumAAModes].m_nQualityLevel = 0;
			m_nNumAAModes++;
		}

		if (materials->SupportsMSAAMode(6))
		{
			m_pAntialiasingBox->AddItem("#DI_Options_Antialiasing_6xMSAA", NULL);
			m_nAAModes[m_nNumAAModes].m_nNumSamples = 6;
			m_nAAModes[m_nNumAAModes].m_nQualityLevel = 0;
			m_nNumAAModes++;
		}
		
		if (materials->SupportsMSAAMode(8))
		{
			m_pAntialiasingBox->AddItem("#DI_Options_Antialiasing_8xMSAA", NULL);
			m_nAAModes[m_nNumAAModes].m_nNumSamples = 8;
			m_nAAModes[m_nNumAAModes].m_nQualityLevel = 0;
			m_nNumAAModes++;
		}

		if (materials->SupportsCSAAMode(4, 2))							// nVidia CSAA			"8x"
		{
			m_pAntialiasingBox->AddItem("#DI_Options_Antialiasing_8xÑSAA", NULL);
			m_nAAModes[m_nNumAAModes].m_nNumSamples = 4;
			m_nAAModes[m_nNumAAModes].m_nQualityLevel = 2;
			m_nNumAAModes++;
		}

		if (materials->SupportsCSAAMode(4, 4))							// nVidia CSAA			"16x"
		{
			m_pAntialiasingBox->AddItem("#DI_Options_Antialiasing_16xÑSAA", NULL);
			m_nAAModes[m_nNumAAModes].m_nNumSamples = 4;
			m_nAAModes[m_nNumAAModes].m_nQualityLevel = 4;
			m_nNumAAModes++;
		}

		if (materials->SupportsCSAAMode(8, 2))							// nVidia CSAA			"16xQ"
		{
			m_pAntialiasingBox->AddItem("#DI_Options_Antialiasing_16xQÑSAA", NULL);
			m_nAAModes[m_nNumAAModes].m_nNumSamples = 8;
			m_nAAModes[m_nNumAAModes].m_nQualityLevel = 2;
			m_nNumAAModes++;
		}
	}
}

void COptionsPanel_Video::DrawFiltrationBox()
{
	m_pFiltrationBox = new ComboBox(this, "FiltrationBox", 8, false);
	m_pFiltrationBox->AddItem("#DI_Options_Filtration_Bi", NULL);
	m_pFiltrationBox->AddItem("#DI_Options_Filtration_Tri", NULL);
	m_pFiltrationBox->AddItem("#DI_Options_Filtration_Aniso_2", NULL);
	m_pFiltrationBox->AddItem("#DI_Options_Filtration_Aniso_4", NULL);
	m_pFiltrationBox->AddItem("#DI_Options_Filtration_Aniso_8", NULL);
	m_pFiltrationBox->AddItem("#DI_Options_Filtration_Aniso_16", NULL);
	m_pFiltrationBox->GetTooltip()->SetText("#DI_Options_Filtration_TT");
}

void COptionsPanel_Video::DrawShadowDetailBox()
{
	m_pShadowDetailBox = new ComboBox(this, "ShadowDetailBox", 6, false);
	m_pShadowDetailBox->AddItem("#DI_Options_ShadowDetail_None", NULL);
	m_pShadowDetailBox->AddItem("#DI_Options_ShadowDetail_Simple", NULL);
	m_pShadowDetailBox->AddItem("#DI_Options_ShadowDetail_Full", NULL);
	m_pShadowDetailBox->GetTooltip()->SetText("#DI_Options_ShadowDetail_TT");
}

void COptionsPanel_Video::DrawHDRSettingsBox() 
{
	m_pHDRSettingsBox = new ComboBox(this, "HDRSettingsBox", 6, false);
	m_pHDRSettingsBox->AddItem("#DI_Options_HDRSettings_None", NULL);
	m_pHDRSettingsBox->AddItem("#DI_Options_HDRSettings_OnlyBloom", NULL);
	m_pHDRSettingsBox->AddItem("#DI_Options_HDRSettings_Full", NULL);
	m_pHDRSettingsBox->GetTooltip()->SetText("#DI_Options_HDRSettings_TT");
}

void COptionsPanel_Video::DrawMulticoreBox() 
{
	m_pMulticoreBox = new CheckButton(this, "MulticoreBox", "#DI_Options_Multicore");
	m_pMulticoreBox->GetTooltip()->SetText("#DI_Options_Multicore_TT");
}

void COptionsPanel_Video::DrawMotionBlurCheckButton() 
{
	m_pMotionBlurButton = new CheckButton(this, "MotionBlurButton", "#DI_Options_MotionBlur");
	m_pMotionBlurButton->GetTooltip()->SetText("#DI_Options_MotionBlur_TT");
}

void COptionsPanel_Video::DrawColorCorrectionCheckButton() 
{
	m_pColorCorrectionButton = new CheckButton(this, "ColorCorrectionButton", "#DI_Options_ColorCorrection");
	m_pColorCorrectionButton->GetTooltip()->SetText("#DI_Options_ColorCorrection_TT");
}

void COptionsPanel_Video::DrawVignetteCheckButton() 
{
	m_pVignetteButton = new CheckButton(this, "VignetteButton", "#DI_Options_Vignette");
	m_pVignetteButton->GetTooltip()->SetText("#DI_Options_Vignette_TT");
}

void COptionsPanel_Video::DrawChromaticCheckButton() 
{
	m_pChromaticButton = new CheckButton(this, "ChromaticButton", "#DI_Options_Chromatic");
	m_pChromaticButton->GetTooltip()->SetText("#DI_Options_Chromatic_TT");
}

void COptionsPanel_Video::DrawDOFCheckButton()
{
	m_pDOFButton = new CheckButton(this, "DOFButton", "#DI_Options_DOF");
	m_pDOFButton->GetTooltip()->SetText("#DI_Options_DOF_TT");
}

void COptionsPanel_Video::PerformLayout()
{
	BaseClass::PerformLayout();
}

void COptionsPanel_Video::CheckRequiresRestart()
{
	m_bRequireRestart = false;
	
	ConVarRef reflections("mat_planar_reflections_level");
	int reflectionssetting = m_pScreenSpaceReflectionsBox->GetActiveItem();
	Assert(reflections.IsValid());
	if (reflectionssetting < 2 && reflectionssetting != reflections.GetInt())
		m_bRequireRestart = true;

	ConVarRef r_shadows("r_shadows");
	ConVarRef shadows("r_shadowrendertotexture");
	int shadowssetting = m_pShadowDetailBox->GetActiveItem();
	Assert(r_shadows.IsValid());
	Assert(shadows.IsValid());
	if (shadowssetting == 1 && !r_shadows.GetBool())
		m_bRequireRestart = true;
	if (shadowssetting == 2 && !shadows.GetBool())
		m_bRequireRestart = true;
}

void COptionsPanel_Video::OnApplyChanges()
{
	SaveVideoSettings();
/*
	if (RequiresRestart())
	{
	//	engine->ClientCmd_Unrestricted("mat_reloadallmaterials\n");
	}

	engine->ClientCmd_Unrestricted("mat_reloadallmaterials\n");
	engine->ClientCmd_Unrestricted("mat_savechanges\n");
*/
}

void COptionsPanel_Video::ApplyChangesToConVar(const char *pConVarName, int value)
{
	Assert(cvar->FindVar(pConVarName));
	char szCmd[256];
	Q_snprintf(szCmd, sizeof(szCmd), "%s %d\n", pConVarName, value);
	engine->ClientCmd_Unrestricted(szCmd);
}

int COptionsPanel_Video::FindMSAAMode(int nAASamples, int nAAQuality)
{
	// Run through the AA Modes supported by the device
	for (int nAAMode = 0; nAAMode < m_nNumAAModes; nAAMode++)
	{
		// If we found the mode that matches what we're looking for, return the index
		if ((m_nAAModes[nAAMode].m_nNumSamples == nAASamples) && (m_nAAModes[nAAMode].m_nQualityLevel == nAAQuality))
		{
			return nAAMode;
		}
	}
	return 0;	// Didn't find what we're looking for, so no AA
}

void COptionsPanel_Video::LoadVideoSettings()
{
	ConVarRef r_rootlod("r_rootlod");

	if (m_pModelDetailBox)
	{
		m_pModelDetailBox->ActivateItem(2 - clamp(r_rootlod.GetInt(), 0, 2));
	}

	if (m_pTextureSizeBox)
	{
		ConVarRef texturesize("mat_picmip");
		m_pTextureSizeBox->ActivateItem(2 - clamp(texturesize.GetInt(), 0, 2));
	}

	if (m_pScreenSpaceReflectionsBox)
	{
		ConVarRef reflectionslevel("mat_planar_reflections_level");
		m_pScreenSpaceReflectionsBox->ActivateItem(reflectionslevel.GetInt());
	}

	if (m_pSpecularityBox)
	{
		ConVarRef specularity_envmap("mat_specular");
		ConVarRef specularity_phong("mat_phong");

		if (!specularity_envmap.GetBool() && !specularity_phong.GetBool())
			m_pSpecularityBox->ActivateItem(0); // Both disabled
		else if (specularity_envmap.GetBool() && !specularity_phong.GetBool())
			m_pSpecularityBox->ActivateItem(1); // only envmap
		else if (!specularity_envmap.GetBool() && specularity_phong.GetBool())
			m_pSpecularityBox->ActivateItem(2); // only Phong. Wtf?! Who does this?!
		else if (specularity_envmap.GetBool() && specularity_phong.GetBool())
			m_pSpecularityBox->ActivateItem(3); // Both enabled
	}

	if (m_pFiltrationBox)
	{
		ConVarRef mat_trilinear("mat_trilinear");
		ConVarRef mat_forceaniso("mat_forceaniso");

		switch (mat_forceaniso.GetInt())
		{
		case 2:
			m_pFiltrationBox->ActivateItem(2);
			break;
		case 4:
			m_pFiltrationBox->ActivateItem(3);
			break;
		case 8:
			m_pFiltrationBox->ActivateItem(4);
			break;
		case 16:
			m_pFiltrationBox->ActivateItem(5);
			break;
		case 0:
		default:
			if (mat_trilinear.GetBool())
			{
				m_pFiltrationBox->ActivateItem(1);
			}
			else
			{
				m_pFiltrationBox->ActivateItem(0);
			}
			break;
		}
	}

	if (m_pShadowDetailBox)
	{
		ConVarRef shadowquality("r_shadowrendertotexture");
		ConVarRef shadowsenabled("r_shadows");

		if (!shadowsenabled.GetBool())
			m_pShadowDetailBox->ActivateItem(0); // No shadows at all
		else
		{
			if (!shadowquality.GetBool())
				m_pShadowDetailBox->ActivateItem(1); // Blobby shadows
			else
				m_pShadowDetailBox->ActivateItem(2); // Fine shadows
		}
	}

	ConVarRef motionblur("mat_motion_blur_enabled");
	ConVarRef vignette("mat_vignette_enabled");
	ConVarRef cc("mat_colorcorrection");
	ConVarRef chromatic("mat_cubicdistortion_enabled");
	ConVarRef mat_queue_mode("mat_queue_mode");
	ConVarRef dof("mat_dof_enabled");

	if (m_pMotionBlurButton)
	{
		m_pMotionBlurButton->SetEnabled(CEstrangedSystemCaps::HasCaps(CAPS_SHADER_POSTPROCESS));
		m_pMotionBlurButton->SetSelected(m_pMotionBlurButton->IsEnabled() && motionblur.GetBool());
	}

	if (m_pVignetteButton)
	{
		m_pVignetteButton->SetEnabled(CEstrangedSystemCaps::HasCaps(CAPS_SHADER_POSTPROCESS));
		m_pVignetteButton->SetSelected(m_pVignetteButton->IsEnabled() && vignette.GetBool());
	}

	if (m_pColorCorrectionButton)
	{
		m_pColorCorrectionButton->SetEnabled(CEstrangedSystemCaps::HasCaps(CAPS_SHADER_POSTPROCESS));
		m_pColorCorrectionButton->SetSelected(m_pColorCorrectionButton->IsEnabled() && cc.GetBool());
	}

	if (m_pChromaticButton)
	{
		m_pChromaticButton->SetEnabled(CEstrangedSystemCaps::HasCaps(CAPS_SHADER_POSTPROCESS));
		m_pChromaticButton->SetSelected(m_pChromaticButton->IsEnabled() && chromatic.GetBool());
	}

	if (m_pDOFButton)
	{
		m_pDOFButton->SetEnabled(CEstrangedSystemCaps::HasCaps(CAPS_SHADER_POSTPROCESS));
		m_pDOFButton->SetSelected(m_pChromaticButton->IsEnabled() && dof.GetBool());
	}

	if (m_pMulticoreBox)
	{
		if (mat_queue_mode.GetInt() == 0)
			m_pMulticoreBox->SetSelected(0);
		else
			m_pMulticoreBox->SetSelected(1);
	}

	if (m_pAntialiasingBox)
	{
		// Map convar to item on AA drop-down
		ConVarRef mat_antialias("mat_antialias");
		ConVarRef mat_aaquality("mat_aaquality");

		int nAASamples = mat_antialias.GetInt();
		int nAAQuality = mat_aaquality.GetInt();
		int nMSAAMode = FindMSAAMode(nAASamples, nAAQuality);
		m_pAntialiasingBox->ActivateItem(nMSAAMode);
	}

	if (m_pHDRSettingsBox)
	{
		ConVarRef mat_hdr_override("mat_hdr_override");
		ConVarRef mat_hdr_level("mat_hdr_level");
		ConVarRef mat_disable_bloom("mat_disable_bloom");

		if (mat_hdr_level.GetInt() == 2)
		{
			if (mat_hdr_override.GetBool())
			{
				if (!mat_disable_bloom.GetBool())
					m_pHDRSettingsBox->ActivateItem(1);
				else
					m_pHDRSettingsBox->ActivateItem(0);
			}
			else
				m_pHDRSettingsBox->ActivateItem(2);
		}
		else
		{

		}
	}
}

void COptionsPanel_Video::SaveVideoSettings()
{
	if (m_pModelDetailBox)	ApplyChangesToConVar("r_rootlod", 2 - m_pModelDetailBox->GetActiveItem());
	if (m_pTextureSizeBox)	ApplyChangesToConVar("mat_picmip", 2 - m_pTextureSizeBox->GetActiveItem());
	if (m_pScreenSpaceReflectionsBox)	ApplyChangesToConVar("mat_planar_reflections_level", m_pScreenSpaceReflectionsBox->GetActiveItem());

	if (m_pScreenSpaceReflectionsBox)
	{
		ConVarRef reflections("mat_planar_reflections_level");
		switch (m_pScreenSpaceReflectionsBox->GetActiveItem())
		{
		case 0: // Only simplest water surface a la DX 7.
		{
			ApplyChangesToConVar("r_waterforcecheap", 1);
			ApplyChangesToConVar("r_waterdrawreflection", 0);
			ApplyChangesToConVar("r_waterforceexpensive", 0);
			ApplyChangesToConVar("r_waterforcereflectentities", 0);
		}
		break;
		case 1: // Cheap reflections - cubemap based.
		{
			ApplyChangesToConVar("r_waterforcecheap", 0);
			ApplyChangesToConVar("r_waterdrawreflection", 0);
			ApplyChangesToConVar("r_waterforceexpensive", 0);
			ApplyChangesToConVar("r_waterforcereflectentities", 0);
		}
		break;
		case 2: // Draw expensive water, but only reflect world.
		{
			ApplyChangesToConVar("r_waterforcecheap", 0);
			ApplyChangesToConVar("r_waterdrawreflection", 1);
			ApplyChangesToConVar("r_waterforceexpensive", 1);
			ApplyChangesToConVar("r_waterforcereflectentities", 0);
		}
		break;
		case 3: // Most expensive water, world and entities. 
		{
			ApplyChangesToConVar("r_waterforcecheap", 0);
			ApplyChangesToConVar("r_waterdrawreflection", 1);
			ApplyChangesToConVar("r_waterforceexpensive", 1);
			ApplyChangesToConVar("r_waterforcereflectentities", 1);
		}
		break;
		}
	}

	if (m_pSpecularityBox)
	{
		switch (m_pSpecularityBox->GetActiveItem())
		{
		case 0:
		{
			ApplyChangesToConVar("mat_specular", 0);
			ApplyChangesToConVar("mat_phong", 0);
		}
		break;
		case 1:
		{
			ApplyChangesToConVar("mat_specular", 1);
			ApplyChangesToConVar("mat_phong", 0);
		}
		break;
		case 2:
		{
			ApplyChangesToConVar("mat_specular", 0);
			ApplyChangesToConVar("mat_phong", 1);
		}
		break;
		case 3:
		{
			ApplyChangesToConVar("mat_specular", 1);
			ApplyChangesToConVar("mat_phong", 1);
		}
		break;
		}
	}

	if (m_pFiltrationBox)
	{
		ApplyChangesToConVar("mat_trilinear", false);
		ApplyChangesToConVar("mat_forceaniso", 1);
		switch (m_pFiltrationBox->GetActiveItem())
		{
		case 0:
			break;
		case 1:
			ApplyChangesToConVar("mat_trilinear", true);
			break;
		case 2:
			ApplyChangesToConVar("mat_trilinear", false);
			ApplyChangesToConVar("mat_forceaniso", 2);
			break;
		case 3:
			ApplyChangesToConVar("mat_trilinear", false);
			ApplyChangesToConVar("mat_forceaniso", 4);
			break;
		case 4:
			ApplyChangesToConVar("mat_trilinear", false);
			ApplyChangesToConVar("mat_forceaniso", 8);
			break;
		case 5:
			ApplyChangesToConVar("mat_trilinear", false);
			ApplyChangesToConVar("mat_forceaniso", 16);
			break;
		}
	}

	if (m_pShadowDetailBox)
	{
		switch (m_pShadowDetailBox->GetActiveItem())
		{
		case 0:
		{
			ApplyChangesToConVar("r_shadows", 0);
			ApplyChangesToConVar("r_shadowrendertotexture", 0);
		}
		break;
		case 1:
		{
			ApplyChangesToConVar("r_shadows", 1);
			ApplyChangesToConVar("r_shadowrendertotexture", 0);
		}
		break;
		case 2:
		{
			ApplyChangesToConVar("r_shadows", 1);
			ApplyChangesToConVar("r_shadowrendertotexture", 1);
		}
		break;
		}
	}

	if (m_pMotionBlurButton)	ApplyChangesToConVar("mat_motion_blur_enabled", m_pMotionBlurButton->IsSelected());
	if (m_pVignetteButton)	ApplyChangesToConVar("mat_vignette_enabled", m_pVignetteButton->IsSelected());
	if (m_pColorCorrectionButton)	ApplyChangesToConVar("mat_colorcorrection", m_pColorCorrectionButton->IsSelected());
	if (m_pChromaticButton)	ApplyChangesToConVar("mat_cubicdistortion_enabled", m_pChromaticButton->IsSelected());
	if (m_pDOFButton)	ApplyChangesToConVar("mat_dof_enabled", m_pDOFButton->IsSelected());

	if (m_pMulticoreBox)
	{
		if (m_pMulticoreBox->IsSelected())
			ApplyChangesToConVar("mat_queue_mode", -1);
		else
			ApplyChangesToConVar("mat_queue_mode", 0);
	}

	if (m_pAntialiasingBox)
	{
		// Set the AA convars according to the menu item chosen
		int nActiveAAItem = m_pAntialiasingBox->GetActiveItem();
		if (m_pAntialiasingBox->GetActiveItem() == 0)
			ApplyChangesToConVar("mat_antialias", 0);
		else
			ApplyChangesToConVar("mat_antialias", m_nAAModes[nActiveAAItem].m_nNumSamples);
		ApplyChangesToConVar("mat_aaquality", m_nAAModes[nActiveAAItem].m_nQualityLevel);
	}

	if (m_pHDRSettingsBox)
	{
		if (m_pHDRSettingsBox->GetActiveItem() < 2)
		{
			ApplyChangesToConVar("mat_hdr_override", 1);
			if (m_pHDRSettingsBox->GetActiveItem() < 1)
				ApplyChangesToConVar("mat_disable_bloom", 1);
			else
				ApplyChangesToConVar("mat_disable_bloom", 0);
		}
		else
		{
			ApplyChangesToConVar("mat_hdr_override", 0);
			ApplyChangesToConVar("mat_disable_bloom", 0);
		}
	}

	CheckRequiresRestart();
	if (m_bRequireRestart)
	{
		engine->ClientCmd_Unrestricted("mat_reloadallmaterials\n");
		engine->ClientCmd_Unrestricted("mat_savechanges\n");
	}
}

void COptionsPanel_Video::OnCommand(const char *command)
{
	if (!Q_strcmp(command, "cancel") || !Q_strcmp(command, "Close"))
	{
		SetVisible(false);
	}
	else if (!Q_strcmp(command, "save"))
	{
		SaveVideoSettings();
	}
	else if (!Q_strcmp(command, "apply"))
	{
		SaveVideoSettings();
	}

	BaseClass::OnCommand(command);
}
void COptionsPanel_Video::OnResetData()
{
	m_bRequireRestart = false;
	LoadVideoSettings();
}

/*
void COptionsPanel_Video::OnTextChanged(Panel *pPanel, const char *pszText)
{
	OnControlModified();
}
*/