/**
 * Copyright 2019 Aaron Robert 
 change by YHTB
 * */

#include "common/common.h"
#include <iostream>
#include <vector>
#include <string>



using HandleFUN = Data * (*)(Data*const );

enum ERRORCODE { ErrData = -1, SucData, Respond };

bool MatchStr(const std::string& str, const std::string& str1);
std::vector<std::string*> SpiltStr(const std::string& ,const char &);
int SerchID(const std::vector<std::string> str,const std::string & ID);


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
Data* ProRequest::MakeSucData()
{
	
	char* buff = new char[8];
	static std::string  tmpStr("SucData");
	for (unsigned int i = 0; i <= tmpStr.length(); ++i)
	{
		*(buff + i) = tmpStr[i];
	}
	return new Data(buff, 8);
}
Data* ProRequest::MakeErrorData()
{
	char* buff = new char[16];
	static std::string  tmpStr("ErrorDataType");
	for (unsigned int i = 0; i <= tmpStr.length(); ++i)
	{
		*(buff + i) = tmpStr[i];
	}
	return new Data(buff, 16);
}
Data* ProRequest::MakeRespoData(const std::string& name)
{
	unsigned int g = SerchID(this->ID_List,name);
	std::string result = this->Context_List[g];
	char * buff = new char[result.length()];
	for(unsigned int i = 0;i <= result.length();++i)
	{
		*(buff + i) = result[i];
	}
	this->ID_List.erase(this->ID_List.begin()+(g-1));
	this->Context_List.erase(this->Context_List.begin() +(g-1));

	return new Data(buff,result.length());
}

int ProRequest::Analysis(const std::string& tmpStr)
{
	
	std::vector<std::string*> Spilter;
	std::cout<<tmpStr<<std::endl;
	bool MaBool = MatchStr("Request:",tmpStr);
	std::cout<<((MaBool) ? "Has":"not")<<std::endl;

	if(MaBool)
	{
		std::cout<<"match request"<<std::endl;
		return Respond;
	}
	else
	{
		Spilter = SpiltStr(tmpStr,'\t');
		if(Spilter.size() == 3)
		{
			//若检测到发送数据将他发送到请求表
			this->ID_List.push_back(*(Spilter[0]));
			this->Context_List.push_back(*(Spilter[1])+*(Spilter[2]));
			return SucData;
		}
		else
		{
			
			std::cout<<"Data Struct Failed!";
			return ErrData;
		}
		
	}

}

class Server {
public:
	explicit Server(const unsigned short& port);
	virtual  ~Server() {}
public:
	void Run(HandleFUN fun);
	//处理接受的请求的函数
private:
	ProRequest* Handle = nullptr;
private:
	UDPServerChannel Channel;
};
Server::Server(const unsigned short& port)
{
	Channel.Bind("0.0.0.0", port);
}
void Server::Run(HandleFUN fun)
{
	Channel.Serve(fun);
}


Data* fun(Data* const request)
{
	static ProRequest funs;
	std::string reStr(request->buff);
    std::cout<<reStr<<std::endl;
	const int respon = funs.Analysis(reStr);
	std::vector<std::string*> result;
	switch (respon)
	{
	case Respond:
		result = SpiltStr(reStr,':');
		std::cout<<"Who::"<<*(result[1])<<std::endl;
		if(SerchID(funs.ID_List,*result[1]) == -1)
		{
			return funs.MakeErrorData();
		}
		
		//std::cout<<"size:"<<result.size()<<*(result[0])<<std::endl;
		return funs.MakeRespoData(*(result[1]));
	
	case ErrData:
		return funs.MakeErrorData();
	
	case SucData:
		return funs.MakeSucData();
	}

}

int main(int argc , char ** argv)
{
  
  InitLogger(argv);
   Server server(9421);
   server.Run(fun);
  return 0;
}


std::vector<std::string*>  SpiltStr(const std::string& TotalStr, const char& spiltChar)
{
	std::vector<std::string *> SpiliterResult;
	std::string tmpStr;
	for (unsigned int i = 0; i <= TotalStr.length(); ++i)
	{
		if (TotalStr[i] != spiltChar)
		{
			tmpStr.push_back(TotalStr[i]);
		}
		else
		{
			SpiliterResult.push_back(new std::string(tmpStr));
			tmpStr.clear();
		}
	}
	if (tmpStr.length())
	{
		SpiliterResult.push_back(new std::string(tmpStr));
	}


	return SpiliterResult;
}
//匹配第个字符串时候包含第一个字符串
bool MatchStr(const std::string& Match_Str, const std::string& Source_Str)
{
	//unsigned int d = 0;
	for (unsigned int i = 0; i < Match_Str.length(); ++i)
	{
		if (Source_Str[i] == Match_Str[i])
		{
			continue;
		}
		else
		{
			return false;
		}
	}
	return true;
}
int SerchID(const std::vector<std::string> str,const std::string & ID)
{
	for (unsigned int i = 0; i < str.size(); ++i)
	{
		if (MatchStr(ID,str[i]))
		{
			return i;
		}
	}
	return -1;
}
