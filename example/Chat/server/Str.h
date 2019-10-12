#pragma once

#include <iostream>
#include <vector>
#include <string>

std::vector<std::string*>  SpiltStr(const std::string& TotalStr, const char& spiltChar);
bool MatchStr(const std::string& Match_Str, const std::string& Source_Str);
int SerchID(const std::vector<std::string> str, const std::string& ID);
