/*
 * Interface for graphics lib
 * */
#pragma once
#include <string>

class ISerialize {
public:
    virtual std::string to_json()=0;  
    virtual int from_json(const std::string json_str)=0;  
};

//TODO
class IGUI {
public:
    virtual void draw_gui()=0;
};
