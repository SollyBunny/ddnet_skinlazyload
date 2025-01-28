#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <game/generated/protocol.h>

#include "../chat.h"
#include "../emoticon.h"

#include <game/client/animstate.h>
#include <game/client/render.h>
#include <game/client/ui.h>

#include "tater.h"
#include <game/client/gameclient.h>

CTater::CTater()
{
	OnReset();
}

void CTater::ConRandomTee(IConsole::IResult *pResult, void *pUserData) {}

void CTater::ConchainRandomColor(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	CTater *pThis = static_cast<CTater *>(pUserData);
	// resolve type to randomize
	// check length of type (0 = all, 1 = body, 2 = feet, 3 = skin, 4 = flag)
	bool RandomizeBody = false;
	bool RandomizeFeet = false;
	bool RandomizeSkin = false;
	bool RandomizeFlag = false;

	if(pResult->NumArguments() == 0)
	{
		RandomizeBody = true;
		RandomizeFeet = true;
		RandomizeSkin = true;
		RandomizeFlag = true;
	}
	else if(pResult->NumArguments() == 1)
	{
		const char *Type = pResult->GetString(0);
		int Length = Type ? str_length(Type) : 0;
		if(Length == 1 && Type[0] == '0')
		{ // Randomize all
			RandomizeBody = true;
			RandomizeFeet = true;
			RandomizeSkin = true;
			RandomizeFlag = true;
		}
		else if(Length == 1)
		{
			// randomize body
			RandomizeBody = Type[0] == '1';
		}
		else if(Length == 2)
		{
			// check for body and feet
			RandomizeBody = Type[0] == '1';
			RandomizeFeet = Type[1] == '1';
		}
		else if(Length == 3)
		{
			// check for body, feet and skin
			RandomizeBody = Type[0] == '1';
			RandomizeFeet = Type[1] == '1';
			RandomizeSkin = Type[2] == '1';
		}
		else if(Length == 4)
		{
			// check for body, feet, skin and flag
			RandomizeBody = Type[0] == '1';
			RandomizeFeet = Type[1] == '1';
			RandomizeSkin = Type[2] == '1';
			RandomizeFlag = Type[3] == '1';
		}
	}

	if(RandomizeBody)
		RandomBodyColor();
	if(RandomizeFeet)
		RandomFeetColor();
	if(RandomizeSkin)
		RandomSkin(pUserData);
	if(RandomizeFlag)
		RandomFlag(pUserData);
	pThis->m_pClient->SendInfo(false);
}

void CTater::OnInit()
{
	TextRender()->SetCustomFace(g_Config.m_ClCustomFont);
	m_pGraphics = Kernel()->RequestInterface<IEngineGraphics>();
}

bool LineShouldHighlight(const char *pLine, const char *pName)
{
	const char *pHL = str_utf8_find_nocase(pLine, pName);
	if(pHL)
	{
		int Length = str_length(pName);
		if(Length > 0 && (pLine == pHL || pHL[-1] == ' ') && (pHL[Length] == 0 || pHL[Length] == ' ' || pHL[Length] == '.' || pHL[Length] == '!' || pHL[Length] == ',' || pHL[Length] == '?' || pHL[Length] == ':'))
			return true;
	}
	return false;
}

bool CTater::SendNonDuplicateMessage(int Team, const char *pLine) 
{
	if(str_comp(pLine, m_PreviousOwnMessage) != 0)
	{
		GameClient()->m_Chat.SendChat(Team, pLine);
		return true;
	}
	str_copy(m_PreviousOwnMessage, pLine);
	return false;
}

void CTater::OnMessage(int MsgType, void *pRawMsg)
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return;

	if(MsgType == NETMSGTYPE_SV_CHAT)
	{
		CNetMsg_Sv_Chat *pMsg = (CNetMsg_Sv_Chat *)pRawMsg;
		int ClientId = pMsg->m_ClientId;

		if(ClientId < 0 || ClientId > MAX_CLIENTS)
			return;
		int LocalId = GameClient()->m_Snap.m_LocalClientId;
		if(ClientId == LocalId)
			str_copy(m_PreviousOwnMessage, pMsg->m_pMessage);

		bool PingMessage = false;

		bool ValidIds = !(GameClient()->m_aLocalIds[0] < 0 || (GameClient()->Client()->DummyConnected() && GameClient()->m_aLocalIds[1] < 0));

		if(ValidIds && ClientId >= 0 && ClientId != GameClient()->m_aLocalIds[0] && (!GameClient()->Client()->DummyConnected() || ClientId != GameClient()->m_aLocalIds[1]))
		{
			PingMessage |= LineShouldHighlight(pMsg->m_pMessage, GameClient()->m_aClients[GameClient()->m_aLocalIds[0]].m_aName);
			PingMessage |= GameClient()->Client()->DummyConnected() && LineShouldHighlight(pMsg->m_pMessage, GameClient()->m_aClients[GameClient()->m_aLocalIds[1]].m_aName);
		}

		if(pMsg->m_Team == TEAM_WHISPER_RECV)
			PingMessage = true;

		if(!PingMessage)
			return;

		char aPlayerName[MAX_NAME_LENGTH];
		str_copy(aPlayerName, GameClient()->m_aClients[ClientId].m_aName, sizeof(aPlayerName));

		bool PlayerMuted = GameClient()->m_aClients[ClientId].m_Foe || GameClient()->m_aClients[ClientId].m_ChatIgnore;
		if(g_Config.m_ClAutoReplyMuted && PlayerMuted) 
		{
			char aBuf[256];
			if(pMsg->m_Team == TEAM_WHISPER_RECV)
			{
				str_format(aBuf, sizeof(aBuf), "/w %s %s", aPlayerName, g_Config.m_ClAutoReplyMutedMessage);
				SendNonDuplicateMessage(0, aBuf);
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "%s: %s", aPlayerName, g_Config.m_ClAutoReplyMutedMessage);
				SendNonDuplicateMessage(0, aBuf);
			}
			return;
		}

		bool WindowActive = m_pGraphics && m_pGraphics->WindowActive();
		if(g_Config.m_ClAutoReplyMinimized && !WindowActive && m_pGraphics)
		{
			char aBuf[256];
			if(pMsg->m_Team == TEAM_WHISPER_RECV)
			{
				str_format(aBuf, sizeof(aBuf), "/w %s %s", aPlayerName, g_Config.m_ClAutoReplyMinimizedMessage);
				SendNonDuplicateMessage(0, aBuf);
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "%s: %s", aPlayerName, g_Config.m_ClAutoReplyMinimizedMessage);
				SendNonDuplicateMessage(0, aBuf);
			}
			return;
		}
	}

	if(MsgType == NETMSGTYPE_SV_VOTESET)
	{
		CNetMsg_Sv_VoteSet *pMsg = (CNetMsg_Sv_VoteSet *)pRawMsg;
		OnReset();
		if(pMsg->m_Timeout)
		{

		}
	}
}

void CTater::OnConsoleInit()
{
	Console()->Register("tc_random_player", "s[type]", CFGFLAG_CLIENT, ConRandomTee, this, "Randomize player color (0 = all, 1 = body, 2 = feet, 3 = skin, 4 = flag) example: 0011 = randomize skin and flag [number is position] ");
	Console()->Chain("tc_random_player", ConchainRandomColor, this);
}

void CTater::RandomBodyColor()
{
	g_Config.m_ClPlayerColorBody = ColorHSLA((std::rand() % 100) / 100.0f, (std::rand() % 100) / 100.0f, (std::rand() % 100) / 100.0f, 1.0f).Pack(false);
}

void CTater::RandomFeetColor()
{
	g_Config.m_ClPlayerColorFeet = ColorHSLA((std::rand() % 100) / 100.0f, (std::rand() % 100) / 100.0f, (std::rand() % 100) / 100.0f, 1.0f).Pack(false);
}

void CTater::RandomSkin(void *pUserData)
{
	CTater *pThis = static_cast<CTater *>(pUserData);
	// get the skin count
	int SkinCount = (int)pThis->m_pClient->m_Skins.GetSkinsUnsafe().size();

	// get a random skin number
	int SkinNumber = std::rand() % SkinCount;

	// get all skins as a maps
	const std::unordered_map<std::string_view, std::unique_ptr<CSkin>> &Skins = pThis->m_pClient->m_Skins.GetSkinsUnsafe();

	// map to array
	int Counter = 0;
	std::vector<std::pair<std::string_view, CSkin *>> SkinArray;
	for(const auto &Skin : Skins)
	{
		if(Counter == SkinNumber)
		{
			// set the skin name
			const char *SkinName = Skin.first.data();
			str_copy(g_Config.m_ClPlayerSkin, SkinName, sizeof(g_Config.m_ClPlayerSkin));
		}
		Counter++;
	}
}

void CTater::RandomFlag(void *pUserData)
{
	CTater *pThis = static_cast<CTater *>(pUserData);
	// get the flag count
	int FlagCount = pThis->m_pClient->m_CountryFlags.Num();

	// get a random flag number
	int FlagNumber = std::rand() % FlagCount;

	// get the flag name
	const CCountryFlags::CCountryFlag *pFlag = pThis->m_pClient->m_CountryFlags.GetByIndex(FlagNumber);

	// set the flag code as number
	g_Config.m_PlayerCountry = pFlag->m_CountryCode;
}
