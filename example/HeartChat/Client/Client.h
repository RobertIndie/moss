#pragma once

#include "moss/src/common/common.h"
#include <iostream>
#include <string>
#include "timer/Timer.h"
#include "common/common.h"


enum CONST_ { RecvBuff = 1024, SEN = 5 };



class Client {
	friend class TimerSend;
public:
	Client(const std::string& name = "User00",
		const std::string& ServerIP = "47.94.89.84",
		const unsigned short& ServePort = 9421);
	~Client();

public:
	void run();

private:
	void core();
	void SelfControlSend(const std::string& DirID);
	void autoRequest();
	Data* Packag(const std::string& str);

private:
	ssize_t SendAndRecv();


private:
	UDPClientChannel channel;

	//服务器的信息
private:
	std::string ServerAddress;
	unsigned short ServerPort;

private:
	std::string UserID;
private:
	Data* recv_data = nullptr;
	Data* send_data = nullptr;
};
