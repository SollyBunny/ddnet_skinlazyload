#include <engine/graphics.h>
#include <engine/shared/config.h>

#include <game/client/animstate.h>
#include <game/client/render.h>
#include <game/generated/client_data.h>
#include <game/generated/protocol.h>

#include <game/client/gameclient.h>

#include "rainbow.h"

template <typename T>
T color_lerp(T a, T b, float c) {
	T result;
	for(size_t i = 0; i < 4; ++i)
		result[i] = a[i] + c * (b[i] - a[i]);
	return result;
}

bool CRainbow::PlayerDeath(vec2 Pos, int ClientId, float Alpha)
{
	if(!g_Config.m_ClRainbow && !g_Config.m_ClRainbowOthers)
		return false;
	if(g_Config.m_ClRainbowMode == 0)
		return false;
	for(int i = 0; i < 64; i++)
	{
		CParticle p;
		p.SetDefault();
		p.m_Spr = SPRITE_PART_SPLAT01 + (rand() % 3);
		p.m_Pos = Pos;
		p.m_Vel = random_direction() * (random_float(0.1f, 1.1f) * 900.0f);
		p.m_LifeSpan = random_float(0.3f, 0.6f);
		p.m_StartSize = random_float(24.0f, 40.0f);
		p.m_EndSize = 0;
		p.m_Rot = random_angle();
		p.m_Rotspeed = random_float(-0.5f, 0.5f) * pi;
		p.m_Gravity = 800.0f;
		p.m_Friction = 0.8f;
		p.m_Color = m_pClient->m_aClients[ClientId].m_RenderInfo.m_BloodColor.WithAlpha(0.75f);
		p.m_StartAlpha = Alpha;
		int Id = m_pClient->m_Particles.Add(CParticles::GROUP_GENERAL, &p);
		if(Id >= 0)
			m_aRainbowParticles.push_back(Id);
	}
	return true;
}

void CRainbow::OnRender()
{
	if(!g_Config.m_ClRainbow && !g_Config.m_ClRainbowOthers)
		return;
	if(g_Config.m_ClRainbowMode == 0)
		return;

	static float Time = 0.0f;
	Time += Client()->RenderFrameTime() * ((float)g_Config.m_ClRainbowSpeed / 100.0f);
	float DefTick = std::fmod(Time, 1.0f);
	static ColorRGBA Col;

	switch(g_Config.m_ClRainbowMode)
	{
	case COLORMODE_RAINBOW:
		Col = color_cast<ColorRGBA>(ColorHSLA(DefTick, 1.0f, 0.5f));
		break;
	case COLORMODE_PULSE:
		Col = color_cast<ColorRGBA>(ColorHSLA(std::fmod(std::floor(Time) * 0.1f, 1.0f), 1.0f, 0.5f + std::fabs(DefTick - 0.5f)));
		break;
	case COLORMODE_DARKNESS:
		Col = ColorRGBA(0.0f, 0.0f, 0.0f);
		break;
	case COLORMODE_RANDOM:
		static ColorHSLA Col1 = ColorHSLA(0.0f, 0.0f, 0.0f, 0.0f), Col2 = ColorHSLA(0.0f, 0.0f, 0.0f, 0.0f);
		if(Col2.a == 0.0f) // Create first target
			Col2 = ColorHSLA((float)rand() / (float)RAND_MAX, 1.0f, (float)rand() / (float)RAND_MAX, 1.0f);
		static float LastSwap = -INFINITY;
		if(Time - LastSwap > 1.0f) // Shift target to source, create new target
		{
			LastSwap = Time;
			Col1 = Col2;
			Col2 = ColorHSLA((float)rand() / (float)RAND_MAX, 1.0f, (float)rand() / (float)RAND_MAX, 1.0f);
		}
		Col = color_cast<ColorRGBA>(color_lerp(Col1, Col2, DefTick));
		break;
	}

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_pClient->m_Snap.m_aCharacters[i].m_Active)
			continue;

		// check if local player
		bool Local = m_pClient->m_Snap.m_LocalClientId == i;
		CTeeRenderInfo *RenderInfo = &m_pClient->m_aClients[i].m_RenderInfo;

		// check if rainbow is enabled
		if(Local ? g_Config.m_ClRainbow : g_Config.m_ClRainbowOthers)
		{
			RenderInfo->m_BloodColor = Col;
			RenderInfo->m_ColorBody = Col;
			RenderInfo->m_ColorFeet = Col;
			RenderInfo->m_CustomColoredSkin = true;
		}
	}

	// Update particles
	Col = Col.WithAlpha(0.75f);
	m_aRainbowParticles.erase(
		std::remove_if(
			m_aRainbowParticles.begin(),
			m_aRainbowParticles.end(),
			[this](int i) {
				CParticle *p = m_pClient->m_Particles.Get(i);
				if (!p || p->m_Spr < SPRITE_PART_SPLAT01 || p->m_Spr >= SPRITE_PART_SPLAT01 + 3)
					return true; // Mark for removal
				p->m_Color = Col;
				return false; // Keep
			}
		),
		m_aRainbowParticles.end()
	);
}
