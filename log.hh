// used http://www.drdobbs.com/cpp/logging-in-c/201804215 as a starting point

#ifndef __LOG_H__
#define __LOG_H__

#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/time.h>
#include "json.hh"

typedef std::string _str;

inline String Now();

class Log {
protected:
    std::ostream& os;
public:
    Log(std::ostream& strm = std::cerr): os(strm) {
    }
    virtual ~Log();

    inline Log& operator<<(String str) {
        Get() << str << std::endl;
        return *this;
    }
    virtual std::ostream& Get();
private:
    Log& operator=(const Log&);
};

inline std::ostream& Log::Get(){
    os << "- " << Now();
    os << " : ";
    return os;
}

inline Log::~Log() {
    os.flush();
}

inline String Now() {
    char buffer[11];
    time_t t;
    time(&t);
    tm r = {0};
    strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
    struct timeval tv;
    gettimeofday(&tv, 0);
    char result[100] = {0};
    std::sprintf(result, "%s.%06ld", buffer, (long)tv.tv_usec); 
    return result;
}

class NameID_Log {
    std::fstream _fp;
    class Logger : public Log {
        const char* _name;
        int _id;
    public:
        Logger(std::ostream& fp, const char* name, int id): Log(fp) {
            _id = id;
            _name = name;
        }
        std::ostream& Get() {
            Log::Get() << _name << " - " << _id << " : ";
            return Log::os;
        }
    };
    Logger* log;
public:
    NameID_Log(const char* name,int id){
        String fn = "log_";
        fn.append(std::to_string(getpid()));
        fn.append(".txt");
        _fp.open(fn.c_str(),std::ofstream::out | std::ofstream::app);
        log = new Logger(_fp,name,id);
    }
    ~NameID_Log(){
        delete log;
        _fp.close();
    }
    inline NameID_Log& operator<<(String str) {
        *log << str;
        return *this;
    }
    inline NameID_Log& operator<<(Json j) {
        *log << j.unparse();
        return *this;
    }
    inline NameID_Log& operator<<(const char* str) {
        *log << String(str);
        return *this;
    }
};

#endif // __LOG_H__