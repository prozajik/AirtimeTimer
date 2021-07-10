#pragma once
#pragma comment(lib, "pluginsdk.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"

class AirtimeTimer : public BakkesMod::Plugin::BakkesModPlugin
{
public:
	float m_secondsElapsedStart = 0;
	float m_secondsElapsedEnd = 0;
	bool m_finishedCounting = true;
	bool m_skipNextChange = false;
	bool m_logOnce = false;
	virtual void onLoad();
	virtual void onUnload();
	void OnGroundChanged(CarWrapper cw, void* params, std::string eventName);
	void OnRigidBodyCollision(std::string eventName);
	void OnReset(std::string eventName);
	void Render(CanvasWrapper canvas);
};
