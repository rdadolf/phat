#include "json.hh"

class RPC_Msg { 
  private:
    static const int MESSAGE_ = 1;
    static const int REPLY_ = -1;
    Json data_;
  public:
    RPC_Msg() {
      data_ = Json::array(MESSAGE_,Json::null,Json::null);
    }
    RPC_Msg(Json content) {
      data_ = Json::array(MESSAGE_,Json::null,content);
    }
    RPC_Msg(Json content, RPC_Msg in_reply_to) {
      data_ = Json::array(REPLY_,in_reply_to.data_[1].as_i(),content);
    }
    bool is_reply() { return data_[0].as_i()<0; }
    bool validate() {
      return data_.is_array() && data_[0].is_int() && data_[1].is_int() && data_[2].is_array();
    }

    Json& json() { return data_; }
    operator Json() { return data_; }
    bool operator!() { return !data_; }
    operator bool() { return data_; }
    Json& content() { return data_[2]; }
};
