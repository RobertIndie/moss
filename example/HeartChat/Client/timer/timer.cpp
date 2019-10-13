#include "Timer.h"

Timer::Timer(Run* instance)
	:m_second(0), m_u_second(0), exec(instance) {}
Timer::Timer(Run* instance, const long& second, const long& u_second)
	: m_second(second), m_u_second(u_second), exec(instance) {}
Timer::~Timer() { this->Stop(); }
void Timer::SetTime(const long& second, const long& u_second)
{
	this->m_second = second;
	this->m_u_second = u_second;
}
void Timer::Start()
{
	pthread_create(&(this->TimerThread), NULL, TimerCore, this);
}
void Timer::Stop()
{
	pthread_cancel(TimerThread);
	pthread_join(TimerThread, NULL);
}
void* Timer::TimerCore(void* obejct_self)
{
	(static_cast<Timer*>(obejct_self))->ProThread();
}
void Timer::ProThread()
{
	while (true)
	{
		this->exec->start();
		pthread_testcancel();
		struct timeval tmpVarTime;
		tmpVarTime.tv_sec = this->m_second;
		tmpVarTime.tv_usec = this->m_u_second;
		select(0, NULL, NULL, NULL, &tmpVarTime);
	}
}