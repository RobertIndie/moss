#pragma once

#include <pthread.h>
#include <sys/time.h>
#include <sys/select.h>

class Run {
public:
	void start()
	{
		this->CorePro();
	}
private:
	virtual void CorePro() = 0;
};

class Timer
{
	static void* TimerCore(void* obejct_self);
public:
	Timer(Run * instance);
	Timer(Run * insatance ,const long& second,const long& u_second=0);
	virtual ~Timer();
public:
	void SetTime(const long& second,const long& u_second=0);
	void Start();
	void Stop();
private:
	void ProThread();
private:
	long m_second;
	long m_u_second;
private:
	Run* exec;
private:
    //Ïß³ÌID
	pthread_t TimerThread;
};
