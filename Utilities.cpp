/*
 *  Utilities.cpp
 *  
 *  Marc DelaCruz
 *  CS 6378-5U1 AOS Project 2 Summer 2010 
 *
 */

#include "Utilities.h"

void parse(std::string const& rStr, std::vector<std::string>& rCmds, char delim)
{
    std::string::size_type lastPos = rStr.find_first_not_of(delim, 0);
    std::string::size_type pos     = rStr.find_first_of(delim, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos)
    {
        rCmds.push_back(rStr.substr(lastPos, pos - lastPos));
        lastPos = rStr.find_first_not_of(delim, pos);
        pos = rStr.find_first_of(delim, lastPos);
    }
}
