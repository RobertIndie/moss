#pragma once
#include "Server.h"

class ProRequest {
	friend Data* fun(Data* const);
public:
	ProRequest() = default;
	virtual ~ProRequest() {}
public:
	//协助mHandle的一些函数
protected:
	//解析请求
	int Analysis(const std::string& request);

	Data* MakeErrorData();
	Data* MakeRespoData(const std::string& name);
	Data* MakeSucData();
protected:
	std::vector<std::string> ID_List;
	std::vector<std::string> Context_List;

};