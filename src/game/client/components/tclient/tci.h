#ifndef GAME_CLIENT_COMPONENTS_TCI_H
#define GAME_CLIENT_COMPONENTS_TCI_H
#include <game/client/component.h>

class CTCI : public CComponent
{
public:
	virtual int Sizeof() const override { return sizeof(*this); }
	// virtual void OnRender() override;

private:
	std::shared_ptr<CHttpRequest> m_pHttpRequest = nullptr;

	void Init(int Dummy);

};

#endif
