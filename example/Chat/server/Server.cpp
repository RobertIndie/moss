#include "Server.h"

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
	std::cout << reStr << std::endl;
	const int respon = funs.Analysis(reStr);
	std::vector<std::string*> result;
	switch (respon)
	{
	case Respond:
		result = SpiltStr(reStr, ':');
		std::cout << "Who::" << *(result[1]) << std::endl;
		if (SerchID(funs.ID_List, *result[1]) == -1)
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