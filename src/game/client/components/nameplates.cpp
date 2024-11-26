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
	vec2 Position,
	ColorRGBA Color, ColorRGBA OutlineColor, float Alpha,
	STextContainerIndex *Name, float FontSize,
	STextContainerIndex *Clan, float FontSizeClan,
	bool ShowFriendMark,
	bool ShowId, int Id,
	bool ShowDirection, bool DirLeft, bool Jump, bool DirRight,
	bool ShowHookWeakStrong, TRISTATE HookWeakStrong,
	bool ShowHookWeakStrongId, int HookWeakStrongId
)
{
	float YOffset = Position.y - 38.0f;

	TextRender()->SetRenderFlags(ETextRenderFlags::TEXT_RENDER_FLAG_NO_FIRST_CHARACTER_X_BEARING | ETextRenderFlags::TEXT_RENDER_FLAG_NO_LAST_CHARACTER_ADVANCE);

	// Render directions
	const float ShowDirectionImgSize = FontSize;
	YOffset -= ShowDirectionImgSize;
	// if (ShowDirection && (DirLeft || Jump || DirRight))
	if (1)
	{
		const vec2 ShowDirectionPos = vec2(Position.x - ShowDirectionImgSize, YOffset + ShowDirectionImgSize / 2.0f);
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ARROW].m_Id);
		Graphics()->QuadsBegin();
		RenderTools()->SelectSprite(IMAGE_ARROW);
		Graphics()->SetColor(1.0f, 1.0f, 1.0f, Alpha);
		if(DirLeft)
		{
			// Graphics()->QuadsSetRotation(pi);
			RenderTools()->DrawSprite(ShowDirectionPos.x - ShowDirectionImgSize * 1.5f, ShowDirectionPos.y, ShowDirectionImgSize);
		}
		if(Jump)
		{
			// Graphics()->QuadsSetRotation(pi * 1.5f);
			RenderTools()->DrawSprite(ShowDirectionPos.x, ShowDirectionPos.y - ShowDirectionImgSize / 2.0f, ShowDirectionImgSize);
		}
		if(DirRight)
		{
			// Graphics()->QuadsSetRotation(0.0f);
			RenderTools()->DrawSprite(ShowDirectionPos.x + ShowDirectionImgSize * 1.5f, ShowDirectionPos.y, ShowDirectionImgSize);
		}
		Graphics()->QuadsEnd();
		Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		Graphics()->QuadsSetRotation(0.0f);
	}

	if(ShowFriendMark && FriendMarkFontSize != FontSize)
	{
		FriendMarkFontSize = FontSize;
		CTextCursor Cursor;
		TextRender()->SetCursor(&Cursor, 0.0f, 0.0f, FriendMarkFontSize, TEXTFLAG_RENDER);
		TextRender()->RecreateTextContainer(FriendMarkTextContainerIndex, &Cursor, FriendMark);
		FriendMarkTextContainerWidth = TextRender()->GetBoundingBoxTextContainer(FriendMarkTextContainerIndex).m_W;
	}

	if(Name || ShowFriendMark || ShowId)
	{
		YOffset -= FontSize;
		float NameWidth = 0.0f;
		float IdWidth = 0.0f;
		static char IdBuf[15];
		if(ShowId)
		{
			const char *Format;
			if(Name && ShowFriendMark)
				Format = " %d: ";
			else if(Name)
				Format = "%d: ";
			else if(ShowFriendMark)
				Format = " %d";
			else
				Format = "%d";
			str_format(IdBuf, sizeof(IdBuf), Format, Id);
			IdWidth = TextRender()->TextWidth(FontSize, IdBuf);
		}
		if(Name)
		{
			NameWidth = TextRender()->GetBoundingBoxTextContainer(*Name).m_W;
			float X = Position.x - NameWidth / 2.0f;
			if(ShowFriendMark)
				X += FriendMarkTextContainerWidth / 2.0f;
			if(ShowId)
				X += IdWidth / 2.0f;
			TextRender()->RenderTextContainer(*Name, Color, OutlineColor, X, YOffset);
		}
		if(ShowId)
		{
			TextRender()->TextColor(Color);
			float X = Position.x- NameWidth / 2.0f - IdWidth / 2.0f;
			if(ShowFriendMark)
				X += FriendMarkTextContainerWidth / 2.0f;
			TextRender()->Text(X, YOffset, FontSize, IdBuf);
		}
		if(ShowFriendMark)
		{
			FriendMarkColor.a = Alpha;
			float X = Position.x - FriendMarkTextContainerWidth / 2.0f - NameWidth / 2.0f - IdWidth / 2.0F;
			TextRender()->RenderTextContainer(FriendMarkTextContainerIndex, FriendMarkColor, OutlineColor, X, YOffset);
		}
	}

	if(Clan)
	{
		YOffset -= FontSizeClan;
		TextRender()->RenderTextContainer(*Clan, Color, OutlineColor, Position.x - TextRender()->GetBoundingBoxTextContainer(*Clan).m_W / 2.0f, YOffset);
	}

	if(ShowHookWeakStrong || ShowHookWeakStrongId)
	{
		ColorRGBA HookWeakStrongColor;
		int StrongWeakSpriteId;
		if(HookWeakStrong == TRISTATE::ALL)
		{
			HookWeakStrongColor = color_cast<ColorRGBA>(ColorHSLA(6401973));
			StrongWeakSpriteId = SPRITE_HOOK_STRONG;
		}
		else if(HookWeakStrong == TRISTATE::SOME)
		{
			HookWeakStrongColor = ColorRGBA(1.0f, 1.0f, 1.0f);
			StrongWeakSpriteId = SPRITE_HOOK_ICON;
		}
		else // if(HookWeakStrong == TRISTATE::NONE)
		{
			HookWeakStrongColor = color_cast<ColorRGBA>(ColorHSLA(41131));
			StrongWeakSpriteId = SPRITE_HOOK_WEAK;
		}
		HookWeakStrongColor.a = Alpha;

		YOffset -= FontSizeClan;
		float ShowHookWeakStrongIdSize = 0.0f;
		if(ShowHookWeakStrongId)
		{
			char aBuf[12];
			str_format(aBuf, sizeof(aBuf), "%d", HookWeakStrongId);
			TextRender()->TextColor(HookWeakStrongColor);
			ShowHookWeakStrongIdSize = TextRender()->TextWidth(FontSizeClan, aBuf);
			float X = Position.x - ShowHookWeakStrongIdSize / 2.0f;
			if(ShowHookWeakStrong)
				X += FontSizeClan * 0.75f;
			TextRender()->Text(X, YOffset, FontSizeClan, aBuf);
		}
		if(ShowHookWeakStrong)
		{
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_STRONGWEAK].m_Id);
			Graphics()->QuadsBegin();

			Graphics()->SetColor(HookWeakStrongColor);
			RenderTools()->SelectSprite(StrongWeakSpriteId);

			const float StrongWeakImgSize = FontSizeClan * 1.5f;
			float X = Position.x;
			if(ShowHookWeakStrongId)
				X -= ShowHookWeakStrongIdSize / 2.0f;
			RenderTools()->DrawSprite(X, YOffset + StrongWeakImgSize / 4.0f, StrongWeakImgSize);
			Graphics()->QuadsEnd();
		}
		
	}

	TextRender()->TextColor(TextRender()->DefaultTextColor());
	TextRender()->TextOutlineColor(TextRender()->DefaultTextOutlineColor());

	TextRender()->SetRenderFlags(0);
}

void CNamePlates::RenderNameplate(vec2 Position, const CNetObj_PlayerInfo *pPlayerInfo, float Alpha, bool ForceAlpha)
{
	bool ShowFriendMark = false;
	bool ShowId = false;
	bool ShowHookWeakStrong = false; TRISTATE HookWeakStrong;
	bool ShowHookWeakStrongId = false; int HookWeakStrongId = -1;

	const auto &ClientData = m_pClient->m_aClients[pPlayerInfo->m_ClientId];
	const bool OtherTeam = m_pClient->IsOtherTeam(pPlayerInfo->m_ClientId);

	const float FontSize = 18.0f + 20.0f * g_Config.m_ClNameplatesSize / 100.0f;
	const float FontSizeClan = 18.0f + 20.0f * g_Config.m_ClNameplatesClanSize / 100.0f;

	if(!ForceAlpha)
	{
		if(g_Config.m_ClNameplatesAlways == 0)
			Alpha = clamp(1.0f - std::pow(distance(m_pClient->m_Controls.m_aTargetPos[g_Config.m_ClDummy], Position) / 200.0f, 16.0f), 0.0f, 1.0f);
		if(OtherTeam)
			Alpha *= (float)g_Config.m_ClShowOthersAlpha / 100.0f;
	}

	ColorRGBA Color = ColorRGBA(1.0f, 1.0f, 1.0f);
	ColorRGBA OutlineColor = ColorRGBA(0.0f, 0.0f, 0.0f);

	if(g_Config.m_ClNameplatesTeamcolors)
	{
		if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags & GAMEFLAG_TEAMS)
		{
		if(ClientData.m_Team == TEAM_RED)
			Color = ColorRGBA(1.0f, 0.5f, 0.5f);
		else if(ClientData.m_Team == TEAM_BLUE)
			Color = ColorRGBA(0.7f, 0.7f, 1.0f);
		}
		else
		{
			const int Team = m_pClient->m_Teams.Team(pPlayerInfo->m_ClientId);
			if(Team)
				Color = m_pClient->GetDDTeamColor(Team, 0.75f);
		}
	}

	OutlineColor.a = Alpha * (OtherTeam ? 0.2f : 0.5f);
	Color.a = Alpha;

	bool Local;
	if(Client()->DummyConnected() && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		Local = pPlayerInfo->m_ClientId == m_pClient->m_aLocalIds[g_Config.m_ClDummy];
	else
		Local = pPlayerInfo->m_Local;

	int ShowDirectionConfig = g_Config.m_ClShowDirection;
#if defined(CONF_VIDEORECORDER)
	if(IVideo::Current())
		ShowDirectionConfig = g_Config.m_ClVideoShowDirection;
#endif
	bool ShowDirection = false;
	bool DirLeft = false, Jump = false, DirRight = false;
	if(ShowDirectionConfig == 1) // others
		ShowDirection = !Local;
	else if(ShowDirectionConfig == 2) // everyone
		ShowDirection = true;
	else if(ShowDirectionConfig == 3) // only self
		ShowDirection = Local;
	if(ShowDirection)
	{
		if(pPlayerInfo->m_Local) // always render local input
		{
			const auto &InputData = m_pClient->m_Controls.m_aInputData[g_Config.m_ClDummy];
			DirLeft = InputData.m_Direction == -1;
			DirRight = InputData.m_Direction == 1;
			Jump = InputData.m_Jump == 1;
		}
		else
		{
			const auto &Character = m_pClient->m_Snap.m_aCharacters[pPlayerInfo->m_ClientId];
			DirLeft = Character.m_Cur.m_Direction == -1;
			DirRight = Character.m_Cur.m_Direction == 1;
			Jump = Character.m_Cur.m_Jumped & 1;
		}
	}

	STextContainerIndex *Name = nullptr;
	STextContainerIndex *Clan = nullptr;

	// render name plate
	if((!Local || g_Config.m_ClNameplatesOwn) && g_Config.m_ClNameplates)
	{
		SPlayerNamePlate &NamePlate = m_aNamePlates[pPlayerInfo->m_ClientId];
		if(str_comp(ClientData.m_aName, NamePlate.m_aName) != 0 || FontSize != NamePlate.m_NameTextFontSize)
		{
			str_copy(NamePlate.m_aName, ClientData.m_aName);
			NamePlate.m_NameTextFontSize = FontSize;

			CTextCursor Cursor;
			TextRender()->SetCursor(&Cursor, 0.0f, 0.0f, FontSize, TEXTFLAG_RENDER);

			// create nameplates at standard zoom
			float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
			Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);
			RenderTools()->MapScreenToInterface(m_pClient->m_Camera.m_Center.x, m_pClient->m_Camera.m_Center.y);
			TextRender()->RecreateTextContainer(NamePlate.m_NameTextContainerIndex, &Cursor, ClientData.m_aName);
			Graphics()->MapScreen(ScreenX0, ScreenY0, ScreenX1, ScreenY1);
		}

		if(g_Config.m_ClNameplatesClan)
		{
			if(str_comp(ClientData.m_aClan, NamePlate.m_aClan) != 0 || FontSizeClan != NamePlate.m_ClanTextFontSize)
			{
				str_copy(NamePlate.m_aClan, ClientData.m_aClan);
				NamePlate.m_ClanTextFontSize = FontSizeClan;

				CTextCursor Cursor;
				TextRender()->SetCursor(&Cursor, 0, 0, FontSizeClan, TEXTFLAG_RENDER);

				// create nameplates at standard zoom
				float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
				Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);
				RenderTools()->MapScreenToInterface(m_pClient->m_Camera.m_Center.x, m_pClient->m_Camera.m_Center.y);
				TextRender()->RecreateTextContainer(NamePlate.m_ClanTextContainerIndex, &Cursor, ClientData.m_aClan);
				Graphics()->MapScreen(ScreenX0, ScreenY0, ScreenX1, ScreenY1);
			}
		}

		if(NamePlate.m_NameTextContainerIndex.Valid())
			Name = &NamePlate.m_NameTextContainerIndex;
		if(Name && g_Config.m_ClNameplatesFriendMark && ClientData.m_Friend)
			ShowFriendMark = true;
		if(g_Config.m_ClNameplatesClan && NamePlate.m_ClanTextContainerIndex.Valid())
			Clan = &NamePlate.m_ClanTextContainerIndex;

		ShowId = g_Config.m_Debug || g_Config.m_ClNameplatesIds;
	}

	ShowHookWeakStrong = (g_Config.m_Debug || g_Config.m_ClNameplatesStrong) && g_Config.m_ClNameplates;
	HookWeakStrong = TRISTATE::SOME;
	if(ShowHookWeakStrong)
	{
		const bool Following = (m_pClient->m_Snap.m_SpecInfo.m_Active && !GameClient()->m_MultiViewActivated && m_pClient->m_Snap.m_SpecInfo.m_SpectatorId != SPEC_FREEVIEW);
		if(m_pClient->m_Snap.m_LocalClientId != -1 || Following)
		{
			const int SelectedId = Following ? m_pClient->m_Snap.m_SpecInfo.m_SpectatorId : m_pClient->m_Snap.m_LocalClientId;
			const CGameClient::CSnapState::CCharacterInfo &Selected = m_pClient->m_Snap.m_aCharacters[SelectedId];
			const CGameClient::CSnapState::CCharacterInfo &Other = m_pClient->m_Snap.m_aCharacters[pPlayerInfo->m_ClientId];
			if(Selected.m_HasExtendedData && Other.m_HasExtendedData)
			{
				if(SelectedId != pPlayerInfo->m_ClientId)
					HookWeakStrong = Selected.m_ExtendedData.m_StrongWeakId > Other.m_ExtendedData.m_StrongWeakId ? TRISTATE::ALL : TRISTATE::NONE;
				ShowHookWeakStrongId = g_Config.m_Debug || g_Config.m_ClNameplatesStrong == 2;
				if(ShowHookWeakStrongId)
					HookWeakStrongId = Other.m_ExtendedData.m_StrongWeakId;
			}
		}
	}

	RenderNameplate(
		Position,
		Color, OutlineColor, Alpha,
		Name, FontSize,
		Clan, FontSizeClan,
		ShowFriendMark,
		ShowId, pPlayerInfo->m_ClientId,
		ShowDirection, DirLeft, Jump, DirRight,
		ShowHookWeakStrong, HookWeakStrong,
		ShowHookWeakStrongId, HookWeakStrongId
	);
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

		if(m_pClient->m_aClients[i].m_SpecCharPresent)
		{
			// Each player can also have a spec char whose nameplate is displayed independently
			const vec2 RenderPos = m_pClient->m_aClients[i].m_SpecChar;
			// don't render offscreen
			if(in_range(RenderPos.x, ScreenX0, ScreenX1) && in_range(RenderPos.y, ScreenY0, ScreenY1))
			{
				RenderNameplate(RenderPos, pInfo, 0.4f, true);
			}
		}
		if(m_pClient->m_Snap.m_aCharacters[i].m_Active)
		{
			// Only render nameplates for active characters
			const vec2 RenderPos = m_pClient->m_aClients[i].m_RenderPos;
			// don't render offscreen
			if(in_range(RenderPos.x, ScreenX0, ScreenX1) && in_range(RenderPos.y, ScreenY0, ScreenY1))
			{
				RenderNameplate(RenderPos, pInfo, 1.0f, false);
			}
		}
	}
}

void CNamePlates::ResetNamePlates()
{
	for(auto &NamePlate : m_aNamePlates)
	{
		TextRender()->DeleteTextContainer(NamePlate.m_NameTextContainerIndex);
		TextRender()->DeleteTextContainer(NamePlate.m_ClanTextContainerIndex);

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
}
