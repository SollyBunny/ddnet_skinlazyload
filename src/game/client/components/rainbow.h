
#ifndef GAME_CLIENT_COMPONENTS_RAINBOW_H
#define GAME_CLIENT_COMPONENTS_RAINBOW_H
#include <game/client/component.h>

#include <vector>

class CRainbow : public CComponent
{
private:
	std::vector<int> m_aRainbowParticles;
public:
	virtual int Sizeof() const override { return sizeof(*this); }
	virtual void OnRender() override;
	bool PlayerDeath(vec2 Pos, int ClientId, float Alpha);
	
	enum COLORMODE
	{
		COLORMODE_RAINBOW = 1,
		COLORMODE_PULSE,
		COLORMODE_DARKNESS,
		COLORMODE_RANDOM
	};
};

#endif
