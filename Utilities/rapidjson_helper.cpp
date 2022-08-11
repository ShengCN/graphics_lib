#include "rapidjson_helper.h"

bool rapidjson_get_int(
        rapidjson::Value &document, 
        const std::string key, 
        int &v) {
    bool ret=true;
    if (document.HasMember(key.c_str()) && document[key.c_str()].IsInt()) {
        v = document[key.c_str()].GetInt();
    }else {
        ERROR("Cannot find {} or {} type is wrong", key, key);
        ret = ret & false;
    }
    return ret;
}

bool rapidjson_get_bool(
        rapidjson::Value &document, 
        const std::string key, 
        bool &v) {
    bool ret=true;
    if (document.HasMember(key.c_str()) && document[key.c_str()].IsBool()) {
        v = document[key.c_str()].GetBool();
    } else {
        ERROR("Cannot find {} or {} type is wrong", key, key);
        ret = ret & false;
    }
    return ret;
}

bool rapidjson_get_string(
        rapidjson::Value &document, 
        const std::string key, 
        std::string &v) {
    bool ret=true;
    if (document.HasMember(key.c_str()) && document[key.c_str()].IsString()) {
        v = document[key.c_str()].GetString();
    } else {
        ERROR("Cannot find {} or {} type is wrong", key, key);
        ret = ret & false;
    }
    return ret;
}

bool rapidjson_get_double(
        rapidjson::Value &document, 
        const std::string key, 
        double &v) {
    bool ret=true;
    if (document.HasMember(key.c_str()) && document[key.c_str()].IsDouble()) {
        v = document[key.c_str()].GetDouble();
    } else {
        ERROR("Cannot find {} or {} type is wrong", key, key);
        ret = ret & false;
    }
    return ret;
}

bool rapidjson_get_double(
        rapidjson::Value &document, 
        const std::string key, 
        float &v) {
    bool ret=true;
    if (document.HasMember(key.c_str()) && document[key.c_str()].IsDouble()) {
        v = (float)document[key.c_str()].GetDouble();
    } else {
        ERROR("Cannot find {} or {} type is wrong", key, key);
        ret = ret & false;
    }
    return ret;
}

bool rapidjson_get_vec3(
        rapidjson::Value &document, 
        const std::string key, 
        glm::vec3 &v) {
    bool ret=true;
    if (document.HasMember(key.c_str()) && document[key.c_str()].IsString()) {
        v = purdue::string_vec3(document[key.c_str()].GetString());
    } else {
        ERROR("Cannot find {} or {} type is wrong", key, key);
        ret = ret & false;
    }
    return ret;
}

bool rapidjson_get_vec4(
        rapidjson::Value &document, 
        const std::string key, 
        glm::vec4 &v) {
    bool ret=true;
    if (document.HasMember(key.c_str()) && document[key.c_str()].IsString()) {
        v = purdue::string_vec4(document[key.c_str()].GetString());
    } else {
        ERROR("Cannot find {} or {} type is wrong", key, key);
        ret = ret & false;
    }
    return ret;
}

bool rapidjson_get_mat4(
        rapidjson::Value &document, 
        const std::string key, 
        glm::mat4 &v) {
    bool ret=true;
    if (document.HasMember(key.c_str()) && document[key.c_str()].IsString()) {
        v = purdue::string_mat4(document[key.c_str()].GetString());
    } else {
        ERROR("Cannot find {} or {} type is wrong", key, key);
        ret = ret & false;
    }
    return ret;
}


std::string get_obj_string(rapidjson::Value &obj) {
    using namespace rapidjson;
    StringBuffer sb;
    Writer<StringBuffer> writer(sb);
    obj.Accept(writer);
    return sb.GetString();
}

