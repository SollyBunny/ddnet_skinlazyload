/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <engine/textrender.h>

#include <game/generated/client_data.h>
#include <game/generated/protocol.h>

#include <game/client/gameclient.h>
#include <game/client/prediction/entities/character.h>

#include "camera.h"
#include "controls.h"
#include "nameplates.h"

void CNamePlates::RenderNameplate(
	const CNetObj_Character *pPrevChar,
	const CNetObj_Character *pPlayerChar,
	const CNetObj_PlayerInfo *pPlayerInfo)
{
	int ClientId = pPlayerInfo->m_ClientId;

	vec2 Position;
	if(ClientId >= 0 && ClientId < MAX_CLIENTS)
		Position = m_pClient->m_aClients[ClientId].m_RenderPos;
	else
		Position = mix(vec2(pPrevChar->m_X, pPrevChar->m_Y), vec2(pPlayerChar->m_X, pPlayerChar->m_Y), Client()->IntraGameTick(g_Config.m_ClDummy));

	RenderNameplatePos(Position, pPlayerInfo, 1.0f);
}

void CNamePlates::RenderNameplatePos(vec2 Position, const CNetObj_PlayerInfo *pPlayerInfo, float Alpha, bool ForceAlpha)
{
	int ClientId = pPlayerInfo->m_ClientId;

	bool OtherTeam = m_pClient->IsOtherTeam(ClientId);

	float FontSize = 18.0f + 20.0f * g_Config.m_ClNameplatesSize / 100.0f;
	float FontSizeClan = 18.0f + 20.0f * g_Config.m_ClNameplatesClanSize / 100.0f;

	TextRender()->SetRenderFlags(ETextRenderFlags::TEXT_RENDER_FLAG_NO_FIRST_CHARACTER_X_BEARING | ETextRenderFlags::TEXT_RENDER_FLAG_NO_LAST_CHARACTER_ADVANCE);
	float YOffset = Position.y - 38;
	ColorRGBA rgb = ColorRGBA(1.0f, 1.0f, 1.0f);

	// render players' key presses
	int ShowDirection = g_Config.m_ClShowDirection;
#if defined(CONF_VIDEORECORDER)
	if(IVideo::Current())
		ShowDirection = g_Config.m_ClVideoShowDirection;
#endif
	if((ShowDirection && ShowDirection != 3 && !pPlayerInfo->m_Local) || (ShowDirection >= 2 && pPlayerInfo->m_Local) || (ShowDirection == 3 && Client()->DummyConnected() && Client()->State() != IClient::STATE_DEMOPLAYBACK && ClientId == m_pClient->m_aLocalIds[!g_Config.m_ClDummy]))
	{
		Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		Graphics()->QuadsSetRotation(0);

		vec2 ShowDirectionPos = vec2(Position.x - 11.0f, YOffset - FontSize - 15.0f);

		bool DirLeft = m_pClient->m_Snap.m_aCharacters[pPlayerInfo->m_ClientId].m_Cur.m_Direction == -1;
		bool DirRight = m_pClient->m_Snap.m_aCharacters[pPlayerInfo->m_ClientId].m_Cur.m_Direction == 1;
		bool Jump = m_pClient->m_Snap.m_aCharacters[pPlayerInfo->m_ClientId].m_Cur.m_Jumped & 1;

		if(pPlayerInfo->m_Local && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		{
			DirLeft = m_pClient->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Direction == -1;
			DirRight = m_pClient->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Direction == 1;
			Jump = m_pClient->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Jump == 1;
		}
		if(Client()->DummyConnected() && Client()->State() != IClient::STATE_DEMOPLAYBACK && pPlayerInfo->m_ClientId == m_pClient->m_aLocalIds[!g_Config.m_ClDummy])
		{
			DirLeft = m_pClient->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_Direction == -1;
			DirRight = m_pClient->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_Direction == 1;
			Jump = m_pClient->m_Controls.m_aInputData[!g_Config.m_ClDummy].m_Jump == 1;
		}
		if(DirLeft)
		{
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ARROW].m_Id);
			Graphics()->QuadsSetRotation(pi);
			Graphics()->RenderQuadContainerAsSprite(m_DirectionQuadContainerIndex, 0, ShowDirectionPos.x - 30.f, ShowDirectionPos.y);
		}
		else if(DirRight)
		{
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ARROW].m_Id);
			Graphics()->RenderQuadContainerAsSprite(m_DirectionQuadContainerIndex, 0, ShowDirectionPos.x + 30.f, ShowDirectionPos.y);
		}
		if(Jump)
		{
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ARROW].m_Id);
			Graphics()->QuadsSetRotation(pi * 3 / 2);
			Graphics()->RenderQuadContainerAsSprite(m_DirectionQuadContainerIndex, 0, ShowDirectionPos.x, ShowDirectionPos.y);
		}
		Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		Graphics()->QuadsSetRotation(0);
	}

	// render name plate
	if((!pPlayerInfo->m_Local || g_Config.m_ClNameplatesOwn) && g_Config.m_ClNameplates)
	{
		float a = 1;
		if(g_Config.m_ClNameplatesAlways == 0)
			a = clamp(1 - std::pow(distance(m_pClient->m_Controls.m_aTargetPos[g_Config.m_ClDummy], Position) / 200.0f, 16.0f), 0.0f, 1.0f);

		const char *pName = m_pClient->m_aClients[pPlayerInfo->m_ClientId].m_aName;
		if(str_comp(pName, m_aNamePlates[ClientId].m_aName) != 0 || FontSize != m_aNamePlates[ClientId].m_NameTextFontSize)
		{
			str_copy(m_aNamePlates[ClientId].m_aName, pName);
			m_aNamePlates[ClientId].m_NameTextFontSize = FontSize;

			CTextCursor Cursor;
			TextRender()->SetCursor(&Cursor, 0, 0, FontSize, TEXTFLAG_RENDER);
			Cursor.m_LineWidth = -1;

			// create nameplates at standard zoom
			float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
			Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);
			RenderTools()->MapScreenToInterface(m_pClient->m_Camera.m_Center.x, m_pClient->m_Camera.m_Center.y);

			m_aNamePlates[ClientId].m_NameTextWidth = TextRender()->TextWidth(FontSize, pName, -1, -1.0f);

			TextRender()->RecreateTextContainer(m_aNamePlates[ClientId].m_NameTextContainerIndex, &Cursor, pName);
			Graphics()->MapScreen(ScreenX0, ScreenY0, ScreenX1, ScreenY1);
		}

		if(g_Config.m_ClNameplatesClan)
		{
			const char *pClan = m_pClient->m_aClients[ClientId].m_aClan;
			if(str_comp(pClan, m_aNamePlates[ClientId].m_aClanName) != 0 || FontSizeClan != m_aNamePlates[ClientId].m_ClanNameTextFontSize)
			{
				str_copy(m_aNamePlates[ClientId].m_aClanName, pClan);
				m_aNamePlates[ClientId].m_ClanNameTextFontSize = FontSizeClan;

				CTextCursor Cursor;
				TextRender()->SetCursor(&Cursor, 0, 0, FontSizeClan, TEXTFLAG_RENDER);
				Cursor.m_LineWidth = -1;

				// create nameplates at standard zoom
				float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
				Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);
				RenderTools()->MapScreenToInterface(m_pClient->m_Camera.m_Center.x, m_pClient->m_Camera.m_Center.y);

				m_aNamePlates[ClientId].m_ClanNameTextWidth = TextRender()->TextWidth(FontSizeClan, pClan, -1, -1.0f);

				TextRender()->RecreateTextContainer(m_aNamePlates[ClientId].m_ClanNameTextContainerIndex, &Cursor, pClan);
				Graphics()->MapScreen(ScreenX0, ScreenY0, ScreenX1, ScreenY1);
			}
		}

		float tw = m_aNamePlates[ClientId].m_NameTextWidth;
		if(g_Config.m_ClNameplatesTeamcolors && m_pClient->m_Teams.Team(ClientId))
			rgb = m_pClient->GetDDTeamColor(m_pClient->m_Teams.Team(ClientId), 0.75f);

		ColorRGBA TColor;
		ColorRGBA TOutlineColor;

		if(OtherTeam && !ForceAlpha)
		{
			TOutlineColor = ColorRGBA(0.0f, 0.0f, 0.0f, 0.2f * g_Config.m_ClShowOthersAlpha / 100.0f);
			TColor = ColorRGBA(rgb.r, rgb.g, rgb.b, g_Config.m_ClShowOthersAlpha / 100.0f);
		}
		else
		{
			TOutlineColor = ColorRGBA(0.0f, 0.0f, 0.0f, 0.5f * a);
			TColor = ColorRGBA(rgb.r, rgb.g, rgb.b, a);
		}
		if(g_Config.m_ClNameplatesTeamcolors && m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags & GAMEFLAG_TEAMS)
		{
			if(m_pClient->m_aClients[ClientId].m_Team == TEAM_RED)
				TColor = ColorRGBA(1.0f, 0.5f, 0.5f, a);
			else if(m_pClient->m_aClients[ClientId].m_Team == TEAM_BLUE)
				TColor = ColorRGBA(0.7f, 0.7f, 1.0f, a);
		}

		TOutlineColor.a *= Alpha;
		TColor.a *= Alpha;

		if(m_aNamePlates[ClientId].m_NameTextContainerIndex.Valid())
		{
			YOffset -= FontSize;
			if((g_Config.m_ClPingNameCircle || (m_pClient->m_Scoreboard.Active() && !pPlayerInfo->m_Local)) && !(Client()->State() == IClient::STATE_DEMOPLAYBACK))
			{
				Graphics()->TextureClear();
				Graphics()->QuadsBegin();
				rgb = color_cast<ColorRGBA>(ColorHSLA((300.0f - clamp(m_pClient->m_Snap.m_apPlayerInfos[ClientId]->m_Latency, 0, 300)) / 1000.0f, 1.0f, 0.5f, 0.8f));
				Graphics()->SetColor(rgb);
				float CircleSize = 7.0f;
				Graphics()->DrawCircle(Position.x - tw / 2.0f - CircleSize, YOffset + FontSize / 2.0f + 1.4f, CircleSize, 24);
				Graphics()->QuadsEnd();
			}
			TextRender()->RenderTextContainer(m_aNamePlates[ClientId].m_NameTextContainerIndex, TColor, TOutlineColor, Position.x - tw / 2.0f, YOffset);
		}

		if(g_Config.m_ClNameplatesClan)
		{
			YOffset -= FontSizeClan;
			if(m_aNamePlates[ClientId].m_ClanNameTextContainerIndex.Valid())
				TextRender()->RenderTextContainer(m_aNamePlates[ClientId].m_ClanNameTextContainerIndex, TColor, TOutlineColor, Position.x - m_aNamePlates[ClientId].m_ClanNameTextWidth / 2.0f, YOffset);
		}

		if(g_Config.m_ClShowSkinName)
		{
			YOffset -= FontSizeClan;
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "%s", m_pClient->m_aClients[pPlayerInfo->m_ClientId].m_aSkinName);
			float XOffset = TextRender()->TextWidth(FontSize, aBuf, -1, -1.0f) / 2.0f;
			TextRender()->TextColor(rgb);
			TextRender()->Text(Position.x - XOffset, YOffset, FontSize, aBuf, -1.0f);
		}

		if(g_Config.m_ClNameplatesFriendMark && m_pClient->m_aClients[ClientId].m_Friend)
		{
			YOffset -= FontSize;
			char aFriendMark[] = "♥";

			ColorRGBA Color;

			if(OtherTeam && !ForceAlpha)
				Color = ColorRGBA(1.0f, 0.0f, 0.0f, g_Config.m_ClShowOthersAlpha / 100.0f);
			else
				Color = ColorRGBA(1.0f, 0.0f, 0.0f, a);

			Color.a *= Alpha;

			TextRender()->TextColor(Color);
			float XOffSet = TextRender()->TextWidth(FontSize, aFriendMark, -1, -1.0f) / 2.0f;
			TextRender()->Text(Position.x - XOffSet, YOffset, FontSize, aFriendMark, -1.0f);
		}

		if(g_Config.m_Debug || g_Config.m_ClNameplatesIds) // render client id when in debug as well
		{
			YOffset -= FontSize;
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "%d", pPlayerInfo->m_ClientId);
			float XOffset = TextRender()->TextWidth(FontSize, aBuf, -1, -1.0f) / 2.0f;
			TextRender()->TextColor(rgb);
			TextRender()->Text(Position.x - XOffset, YOffset, FontSize, aBuf, -1.0f);
		}
	}

	if((g_Config.m_Debug || g_Config.m_ClNameplatesStrong) && g_Config.m_ClNameplates)
	{
		bool Following = (m_pClient->m_Snap.m_SpecInfo.m_Active && !GameClient()->m_MultiViewActivated && m_pClient->m_Snap.m_SpecInfo.m_SpectatorId != SPEC_FREEVIEW);
		if(m_pClient->m_Snap.m_LocalClientId != -1 || Following)
		{
			int SelectedId = Following ? m_pClient->m_Snap.m_SpecInfo.m_SpectatorId : m_pClient->m_Snap.m_LocalClientId;
			const CGameClient::CSnapState::CCharacterInfo &Selected = m_pClient->m_Snap.m_aCharacters[SelectedId];
			const CGameClient::CSnapState::CCharacterInfo &Other = m_pClient->m_Snap.m_aCharacters[ClientId];
			if(Selected.m_HasExtendedData && Other.m_HasExtendedData)
			{
				if(SelectedId == ClientId)
					TextRender()->TextColor(rgb);
				else
				{
					float ScaleX, ScaleY;
					const float StrongWeakImgSize = 40.0f;
					Graphics()->TextureClear();
					Graphics()->TextureSet(g_pData->m_aImages[IMAGE_STRONGWEAK].m_Id);
					Graphics()->QuadsBegin();
					ColorRGBA StrongWeakStatusColor;
					int StrongWeakSpriteId;
					if(Selected.m_ExtendedData.m_StrongWeakId > Other.m_ExtendedData.m_StrongWeakId)
					{
						StrongWeakStatusColor = color_cast<ColorRGBA>(ColorHSLA(6401973));
						StrongWeakSpriteId = SPRITE_HOOK_STRONG;
					}
					else
					{
						StrongWeakStatusColor = color_cast<ColorRGBA>(ColorHSLA(41131));
						StrongWeakSpriteId = SPRITE_HOOK_WEAK;
					}

					float ClampedAlpha = 1;
					if(g_Config.m_ClNameplatesAlways == 0)
						ClampedAlpha = clamp(1 - std::pow(distance(m_pClient->m_Controls.m_aTargetPos[g_Config.m_ClDummy], Position) / 200.0f, 16.0f), 0.0f, 1.0f);

					if(OtherTeam && !ForceAlpha)
						StrongWeakStatusColor.a = g_Config.m_ClShowOthersAlpha / 100.0f;
					else
						StrongWeakStatusColor.a = ClampedAlpha;

					StrongWeakStatusColor.a *= Alpha;
					Graphics()->SetColor(StrongWeakStatusColor);
					RenderTools()->SelectSprite(StrongWeakSpriteId);
					RenderTools()->GetSpriteScale(StrongWeakSpriteId, ScaleX, ScaleY);
					TextRender()->TextColor(StrongWeakStatusColor);

					YOffset -= StrongWeakImgSize * ScaleY;
					RenderTools()->DrawSprite(Position.x, YOffset + (StrongWeakImgSize / 2.0f) * ScaleY, StrongWeakImgSize);
					Graphics()->QuadsEnd();
				}
				if(g_Config.m_Debug || g_Config.m_ClNameplatesStrong == 2)
				{
					YOffset -= FontSize;
					char aBuf[12];
					str_format(aBuf, sizeof(aBuf), "%d", Other.m_ExtendedData.m_StrongWeakId);
					float XOffset = TextRender()->TextWidth(FontSize, aBuf, -1, -1.0f) / 2.0f;
					TextRender()->Text(Position.x - XOffset, YOffset, FontSize, aBuf, -1.0f);
				}
			}
		}
	}

	TextRender()->TextColor(TextRender()->DefaultTextColor());
	TextRender()->TextOutlineColor(TextRender()->DefaultTextOutlineColor());

	TextRender()->SetRenderFlags(0);
}

void CNamePlates::OnRender()
{
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;

	int ShowDirection = g_Config.m_ClShowDirection;
#if defined(CONF_VIDEORECORDER)
	if(IVideo::Current())
		ShowDirection = g_Config.m_ClVideoShowDirection;
#endif
	if(!g_Config.m_ClNameplates && ShowDirection == 0)
		return;

	// get screen edges to avoid rendering offscreen
	float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
	Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);
	// expand the edges to prevent popping in/out onscreen
	//
	// it is assumed that the nameplate and all its components fit into a 800x800 box placed directly above the tee
	// this may need to be changed or calculated differently in the future
	ScreenX0 -= 400;
	ScreenX1 += 400;
	//ScreenY0 -= 0;
	ScreenY1 += 800;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		const CNetObj_PlayerInfo *pInfo = m_pClient->m_Snap.m_apPlayerInfos[i];
		if(!pInfo)
		{
			continue;
		}

		vec2 *pRenderPos;
		if(m_pClient->m_aClients[i].m_SpecCharPresent)
		{
			// Each player can also have a spec char whose nameplate is displayed independently
			pRenderPos = &m_pClient->m_aClients[i].m_SpecChar;
			// don't render offscreen
			if(!(pRenderPos->x < ScreenX0) && !(pRenderPos->x > ScreenX1) && !(pRenderPos->y < ScreenY0) && !(pRenderPos->y > ScreenY1))
			{
                if(!g_Config.m_ClRenderNameplateSpec)
                {
                    RenderNameplatePos(m_pClient->m_aClients[i].m_SpecChar, pInfo, 0.4f, true);
                }
			}
		}
		if(!m_pClient->m_Snap.m_aCharacters[i].m_Active)
		{
			continue;
		}

		else if(m_pClient->m_Snap.m_aCharacters[i].m_Active)
		{
			// only render nameplates for active characters
			RenderNameplate(
				&m_pClient->m_Snap.m_aCharacters[i].m_Prev,
				&m_pClient->m_Snap.m_aCharacters[i].m_Cur,
				pInfo);
		}
	}
}

void CNamePlates::ResetNamePlates()
{
	for(auto &NamePlate : m_aNamePlates)
	{
		TextRender()->DeleteTextContainer(NamePlate.m_NameTextContainerIndex);
		TextRender()->DeleteTextContainer(NamePlate.m_ClanNameTextContainerIndex);

		NamePlate.Reset();
	}
}

void CNamePlates::OnWindowResize()
{
	ResetNamePlates();
}

void CNamePlates::OnInit()
{
	ResetNamePlates();

	// Quad for the direction arrows above the player
	m_DirectionQuadContainerIndex = Graphics()->CreateQuadContainer(false);
	RenderTools()->QuadContainerAddSprite(m_DirectionQuadContainerIndex, 0.f, 0.f, 22.f);
	Graphics()->QuadContainerUpload(m_DirectionQuadContainerIndex);
}
