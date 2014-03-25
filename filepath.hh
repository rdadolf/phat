#include <iostream>
#include "json.hh"

#ifndef _FILE_PATH_HH_
#define _FILE_PATH_HH_

void print_json(Json& j) {
    std::cout << j << std::endl;
}

Json parse_filepath(const char* str) {
    Json ret = Json::make_array();
    String* tmp = new String();
    while (*str) {
        if (*str == '/') {
            if (tmp->length() > 0) {
                ret.push_back(tmp->c_str());
                delete tmp;
                tmp = new String();
            } else {
                ret.push_back("/");
            }
        } else if (*str =='\\' && *(str + 1) == ' ') {
            *tmp += ' ';
            str++;
        } else
            *tmp += *str;
        str++;
    }

    if (tmp->length() > 0)
        ret.push_back(tmp->c_str());
    
    return ret;
}

const char* join_filepath(Json path) {
    String str;
    for (int i = 0; i < path.size(); ++i) {
        assert(path[i].is_s());
        str += path[i].as_s();
        if (i != 0)
            str += "/";
    }
    return str.c_str();
}

#endif // _FILE_PATH_HH_