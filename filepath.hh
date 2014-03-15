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
    while (strlen(str) > 0) {
        if (*str == '/') {
            if (tmp->length() > 0) {
                ret.push_back(tmp->c_str());
                delete tmp;
                tmp = new String();
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

#endif // _FILE_PATH_HH_