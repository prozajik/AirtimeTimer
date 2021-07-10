#include "AirtimeTimer.h"
#include "bakkesmod\wrappers\GameEvent\TutorialWrapper.h"
#include "bakkesmod\wrappers\GameObject\BallWrapper.h"
#include "bakkesmod\wrappers\GameObject\CarWrapper.h"

BAKKESMOD_PLUGIN(AirtimeTimer, "Airtime Timer", "0.1", PLUGINTYPE_FREEPLAY)

const float diff = 0.5f;

void AirtimeTimer::onLoad()
{
	/* 
		Some quick explanation:
			counter = start + end filled by GetSecondsElapsed()
			counter inited to 0
			start = GetSecondsElapsed(): called TAGame.Car_TA.OnGroundChanged
			end = GetSecondsElapsed() on first TAGame.RBActor_TA.OnRigidBodyCollision or consequent TAGame.Car_TA.OnGroundChanged
	*/
	gameWrapper->HookEventWithCaller<CarWrapper>("Function TAGame.Car_TA.OnGroundChanged", std::bind(&AirtimeTimer::OnGroundChanged, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	
	// From my understading TAGame.RBActor_TA.OnRigidBodyCollision gets called whenever car (or its part) comes into a contact with an object. 
	//	This is closest to what we are trying to achieve - ideally I would like to get a function which gets called once the car touches any object and keeps this state until it "lifts off" again, but I wasn't able to find one.
	gameWrapper->HookEventPost("Function TAGame.RBActor_TA.OnRigidBodyCollision", std::bind(&AirtimeTimer::OnRigidBodyCollision, this, std::placeholders::_1));
	gameWrapper->RegisterDrawable(std::bind(&AirtimeTimer::Render, this, std::placeholders::_1));
	gameWrapper->HookEventPost("Function Engine.Controller.Restart", std::bind(&AirtimeTimer::OnReset, this, std::placeholders::_1));
}

void AirtimeTimer::onUnload()
{
}

void AirtimeTimer::OnGroundChanged(CarWrapper cw, void* params, std::string eventName)
{
	if (!gameWrapper->IsInGame())
		return;

	if (m_skipNextChange)
	{
		m_skipNextChange = false;
		return;
	}

	if (m_finishedCounting)
	{
		m_secondsElapsedStart = gameWrapper->GetGameEventAsServer().GetSecondsElapsed();
		m_secondsElapsedEnd = 0;
		m_finishedCounting = false;
	}
	else
	{
		m_secondsElapsedEnd = gameWrapper->GetGameEventAsServer().GetSecondsElapsed();
		m_finishedCounting = true;
		m_logOnce = true;
	}
}

void AirtimeTimer::OnRigidBodyCollision(std::string eventName)
{
	if (!gameWrapper->IsInGame())
		return;

	auto time = gameWrapper->GetGameEventAsServer().GetSecondsElapsed();

	if (!m_finishedCounting)
	{
		m_secondsElapsedEnd = time;
		m_finishedCounting = true;
		m_skipNextChange = true;
		m_logOnce = true;
	}
}

void AirtimeTimer::OnReset(std::string eventName)
{
	m_secondsElapsedStart = 0;
	m_secondsElapsedEnd = 0;
	m_finishedCounting = true;
	m_skipNextChange = true;
	m_logOnce = false;
}

void AirtimeTimer::Render(CanvasWrapper canvas)
{
	if (!gameWrapper->IsInGame())
		return;

	auto tutorial = gameWrapper->GetGameEventAsServer();

	if (tutorial.GetCars().Count() == 0)
		return;

	float currentTime;

	if (m_finishedCounting)
	{
		currentTime = m_secondsElapsedEnd - m_secondsElapsedStart;
	}
	else
	{
		currentTime = gameWrapper->GetGameEventAsServer().GetSecondsElapsed() - m_secondsElapsedStart;
	}
	
	if (m_finishedCounting && m_logOnce)
	{
		gameWrapper->LogToChatbox("Managed to stay in air: " + std::to_string(currentTime) + "s");
		m_logOnce = false;
	}

	auto screenSize = canvas.GetSize();
	Vector2 drawLoc = { (screenSize.X * 0.35), 0 };
	canvas.SetPosition(drawLoc);
	canvas.DrawString(std::to_string(currentTime), 3, 3);
}
