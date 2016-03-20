#if !defined(MSG_H_)
#define MSG_H_
#include "cyclemsg.h"

class lua_State;

#define begin_msg(name,proto_type,msgid) struct name : public INetPacket { \
	public: \
    name(); \
    ~name(){};  \
    static int _proto_type(){return proto_type;}\
    static int _msgid(){return msgid;}\
    const char* PacketName(){return #name;}\
	void encode(ByteArray &byteArray);\
	void decode(ByteArray &byteArray);\
	void build(ByteArray &byteArray);\
    std::string tostring();

#define end_msg() };


#define def_err(name) int name;
#define def_int(name) int name;
#define def_int64(name) int64_t name;
#define def_float(name) float name;
#define def_string(name) std::string name;
#define def_blob(name) std::vector<char> name;
#define def_object(objtype,name) objtype name;

#define def_int_arr(name) std::vector<int> name;
#define def_int64_arr(name) std::vector<int64_t> name;
#define def_float_arr(name) std::vector<float> name;
#define def_string_arr(name) std::vector<std::string> name;
#define def_object_arr(obj,name) std::vector<obj> name;

#include "msgdefine.h"

#undef begin_msg
#undef end_msg

#undef def_err
#undef def_int
#undef def_int64
#undef def_float
#undef def_string
#undef def_blob
#undef def_object

#undef def_int_arr
#undef def_int64_arr
#undef def_float_arr
#undef def_string_arr
#undef def_object_arr

#endif
