#include <iostream>
#include "common/common.h"
#include <iostream>
#include <string>
#include <time.h>



class Client {
	public:
		Client(const std::string& name ="User00",
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
		Data * recv_data = nullptr;
		Data * send_data = nullptr;
};
enum CONST_ {RecvBuff = 1024,SEN=5};
Client::Client(const std::string& UserID,
				const std::string& ServerIP,
				const unsigned short& Port)
				:ServerAddress(ServerIP),ServerPort(Port),UserID(UserID)
				{
					channel.Connect(this->ServerAddress,this->ServerPort);
					char * recv_buff = new char[RecvBuff]{'\0'};
					this->recv_data = new Data(recv_buff,RecvBuff);
					if(this->recv_data == nullptr)
					{
						std::cout<<"Creating Data Failed"<<std::endl;
					}
				}
Client::~Client()
{
	if(recv_data != nullptr)
		delete recv_data;
	if(send_data != nullptr)
		delete send_data;
}
ssize_t Client::SendAndRecv()
{
	channel.Send(this->send_data,this->recv_data);
}

Data* Client::Packag(const std::string & str)
{
	char * C_str = new char[str.length() + 1];
	for(int i = 0 ;i <= str.length() +1 ; ++i)
	{
		*(C_str + i) = str[i];
	}
	if(C_str != nullptr && (this->send_data = new Data(C_str,str.length()+1)) == nullptr)
	{
		std::cout<<"Creating Data failed!!"<<std::endl;
		return nullptr;
	}
	else {
		//std::cout<<this->recv_data<<std::endl;
		return this->send_data;
	}
	
}

void Client::SelfControlSend(const std::string& DirID)
{
	std::cout<<"please Enter context:"<<std::endl;
	std::string str;
	std::cin >> str;

	std::string Header = DirID + std::string("\t") + this->UserID + std::string("\t");
	std::string result = Header + str;

	std::cout<<result<<std::endl;

	Packag(result);
	//std::cout<<"this"<<std::endl;

	this->SendAndRecv();
}

void Client::autoRequest()
{
	std::cout<<"SendToAutoRequest"<<std::endl;
	this->Packag(std::string("Request:")+this->UserID);
	this->SendAndRecv();


		std::cout<<"OutPut:Message"<<std::endl;
		for (unsigned int i = 0; i <= this->recv_data->len &&
			*(this->recv_data->buff) != '\0'; ++i)
		{
		std::cout << *(this->recv_data->buff + i);
		}
		std::cout << std::endl;
	
}

void Client::run()
{
	bool flag = true;

	char c;
	while(flag)
	{
		std::cout<<"what do you want to do:\n"
				 <<"u : SendMessageToUser\n"
				 <<"a : SendRequest"
				 <<"q : quit\n"
				 <<std::endl;
		//this->autoRequest();
		std::cin>>c;
		switch(c)
		{
			case 'q':
			case 'Q':
					flag = false;
					break;
			case 'u':
			case 'U':
					this->core();
					break;
			case 'a':
			case 'A':
					this->autoRequest();
					break;
		}
	}
	
}

void Client::core()
{
	std::cout<<"please enter UserID:"<<std::endl;
	std::string DirID;
	std::cin>>DirID;
	this->SelfControlSend(DirID);
	//this->autoRequest();
}

int main(int argc,char** argv)
{
	InitLogger(argv);
	Client client("ID","47.94.89.84");
	client.run();
	return 0;
}
