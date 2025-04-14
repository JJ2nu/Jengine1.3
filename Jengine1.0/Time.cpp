#include "pch.h"
#include "Time.h"

Engine::Time* Engine::Time::m_Instance = nullptr;
float Engine::Time::timeScale = 1.0f;

Engine::Time::Time()
	: mSecondsPerCount(0.0), mDeltaTime(-1.0), mBaseTime(0),
	mPausedTime(0), mPrevTime(0), mCurrTime(0), mStopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&countsPerSec));
	mSecondsPerCount = 1.0 / static_cast<double>(countsPerSec);

	if (m_Instance == nullptr)
	{
		m_Instance = this;
	}

}

Engine::Time* Engine::Time::GetInstance()
{
	return m_Instance;
}

void Engine::Time::DestroyInstance()
{


}


float Engine::Time::TotalTime()const
{
	if (mStopped)
	{
		return static_cast<float>(mSecondsPerCount * (mStopTime - mPausedTime - mBaseTime));
	}
	else
	{
		return static_cast<float>(mSecondsPerCount * (mCurrTime - mPausedTime - mBaseTime));
	}
}

float Engine::Time::DeltaTime()const
{
	return static_cast<float>(mDeltaTime)*timeScale;
}

void Engine::Time::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));

	mBaseTime = currTime;
	mPrevTime = currTime;
	mStopTime = 0;
	mStopped = false;
}

void Engine::Time::Start()
{
	__int64 startTime;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&startTime));

	if (mStopped)
	{
		mPausedTime += (startTime - mStopTime);

		mPrevTime = startTime;
		mStopTime = 0;
		mStopped = false;
	}
}

void Engine::Time::Stop()
{
	if (!mStopped)
	{
		__int64 currTime;
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));

		mStopTime = currTime;
		mStopped = true;
	}
}

void Engine::Time::Tick()
{
	if (mStopped)
	{
		mDeltaTime = 0.0;
		return;
	}
	__int64 currTime;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currTime));
	mCurrTime = currTime;

	mDeltaTime = (mCurrTime - mPrevTime) * mSecondsPerCount;
	if (mDeltaTime > 1.f) mDeltaTime = 0.f;
	mPrevTime = mCurrTime;

	if (mDeltaTime < 0.0)
	{
		mDeltaTime = 0.0;
	}
}




