#include "ProRequest.h"

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
	unsigned int g = SerchID(this->ID_List, name);
	std::string result = this->Context_List[g];
	char* buff = new char[result.length()];
	for (unsigned int i = 0; i <= result.length(); ++i)
	{
		*(buff + i) = result[i];
	}
	this->ID_List.erase(this->ID_List.begin() + (g - 1));
	this->Context_List.erase(this->Context_List.begin() + (g - 1));

	return new Data(buff, result.length());
}

int ProRequest::Analysis(const std::string& tmpStr)
{

	std::vector<std::string*> Spilter;
	std::cout << tmpStr << std::endl;
	bool MaBool = MatchStr("Request:", tmpStr);
	std::cout << ((MaBool) ? "Has" : "not") << std::endl;

	if (MaBool)
	{
		std::cout << "match request" << std::endl;
		return Respond;
	}
	else
	{
		Spilter = SpiltStr(tmpStr, '\t');
		if (Spilter.size() == 3)
		{
			//若检测到发送数据将他发送到请求表
			this->ID_List.push_back(*(Spilter[0]));
			this->Context_List.push_back(*(Spilter[1]) + *(Spilter[2]));
			return SucData;
		}
		else
		{

			std::cout << "Data Struct Failed!";
			return ErrData;
		}

	}

}