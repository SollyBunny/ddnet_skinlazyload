/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_NAMEPLATES_H
#define GAME_CLIENT_COMPONENTS_NAMEPLATES_H
#include <base/vmath.h>

#include <engine/shared/protocol.h>
#include <engine/textrender.h>

#include <game/client/component.h>

class CNamePlateRenderData
{
public:
	bool m_InGame;
	vec2 m_Position;
	ColorRGBA m_Color;
	float m_Alpha;
	bool m_ShowName;
	const char *m_pName;
	bool m_ShowFriendMark;
	bool m_ShowClientId;
	int m_ClientId;
	float m_FontSizeClientId;
	bool m_ClientIdNewLine;
	float m_FontSize;
	bool m_ShowClan;
	const char *m_pClan;
	float m_FontSizeClan;
	bool m_ShowDirection;
	bool m_DirLeft;
	bool m_DirJump;
	bool m_DirRight;
	float m_FontSizeDirection;
	bool m_ShowHookStrongWeak;
	enum
	{
		HOOKSTRONGWEAK_WEAK,
		HOOKSTRONGWEAK_UNKNOWN,
		HOOKSTRONGWEAK_STRONG
	} m_HookStrongWeak;
	bool m_ShowHookStrongWeakId;
	int m_HookStrongWeakId;
	float m_FontSizeHookStrongWeak;
};

class CNamePlate;

class CNamePlates : public CComponent
{
private:
	CNamePlate *m_aNamePlates;
	void RenderNamePlate(CNamePlate &NamePlate, const CNamePlateRenderData &Data);

public:
	void RenderNamePlateGame(vec2 Position, const CNetObj_PlayerInfo *pPlayerInfo, float Alpha, bool ForceAlpha);
	void RenderNamePlatePreview(vec2 Position, int Dummy);
	void ResetNamePlates();
	virtual int Sizeof() const override { return sizeof(*this); }
	virtual void OnWindowResize() override;
	virtual void OnInit() override;
	virtual void OnRender() override;
	~CNamePlates();
};

#endif
