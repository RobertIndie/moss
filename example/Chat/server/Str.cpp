
std::vector<std::string*>  SpiltStr(const std::string& TotalStr, const char& spiltChar)
{
	std::vector<std::string*> SpiliterResult;
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
int SerchID(const std::vector<std::string> str, const std::string& ID)
{
	for (unsigned int i = 0; i < str.size(); ++i)
	{
		if (MatchStr(ID, str[i]))
		{
			return i;
		}
	}
	return -1;
}
