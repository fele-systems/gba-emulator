#pragma once

#include <string>

template<char space = ' '>
void left_trim(std::string& input)
{
    size_t i = 0;
    if (input.size() == 0) return;
    while(input[i] == space && i < input.size()) i++;
    input = input.substr(i);
}

template<char space = ' '>
void right_trim(std::string& input)
{
    if (input.size() == 0) return;
    int i = input.size() - 1;
    while(input[i] == space && i >= 0) i--;
    input.resize(i + 1);
}

template<char lspace = ' ', char rspace = ' '>
void lr_trim(std::string& input)
{
    left_trim<lspace>(input);
    right_trim<rspace>(input);
}

template<char space = ' '>
std::string left_trim_cp(const std::string& input)
{
    size_t i = 0;
    if (input.size() == 0) return "";
    while(input[i] == space && i < input.size()) i++;
    return input.substr(i);
}

template<char space = ' '>
std::string right_trim_cp(const std::string& input)
{
    if (input.size() == 0) return "";
    size_t i = input.size() - 1;
    while(input[i] == space && i >= 0) i--;
    return input.substr(0, i + 1);
}
template<char lspace = ' ', char rspace = ' '>
std::string lr_trim_cp(const std::string& input)
{
    auto s = left_trim_cp<lspace>(input); // cp or not cp, it will make a copy of the string
    right_trim<rspace>(s); // we don't need to copy it again, so lets use the reference version
    return s;
}
