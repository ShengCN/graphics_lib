#pragma once
#include <string>
#include <common.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

bool rapidjson_get_int(
        rapidjson::Value &doc, 
        const std::string key, 
        int &v);
bool rapidjson_get_bool(
        rapidjson::Value &doc, 
        const std::string key, 
        bool &v);
bool rapidjson_get_string(
        rapidjson::Value &doc, 
        const std::string key, 
        std::string &v);
bool rapidjson_get_double(
        rapidjson::Value &doc, 
        const std::string key, 
        double &v);
bool rapidjson_get_double(
        rapidjson::Value &doc, 
        const std::string key, 
        float &v);
bool rapidjson_get_vec3(
        rapidjson::Value &doc, 
        const std::string key, 
        glm::vec3 &v);
bool rapidjson_get_vec4(
        rapidjson::Value &doc, 
        const std::string key, 
        glm::vec4 &v);
bool rapidjson_get_mat4(
        rapidjson::Value &doc, 
        const std::string key, 
        glm::mat4 &v);

std::string get_obj_string(rapidjson::Value &obj);
