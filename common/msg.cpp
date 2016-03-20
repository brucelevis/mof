#include "msg.h"
#include "cyclemsg.h"
#include "Qcheck.h"
#include <sstream>
#include <algorithm>
#include <iterator>

//init
#define begin_msg(name,proto_type,msgid) name::name(){ _type=proto_type;_id=msgid;

#define end_msg() };

#define def_err(name) name = 0;
#define def_int(name) name = 0;
#define def_int64(name) name = 0;
#define def_float(name) name = 0.0f;
#define def_string(name)
#define def_blob(name)
#define def_object(objtype,name)

#define def_int_arr(name)
#define def_int64_arr(name)
#define def_float_arr(name)
#define def_string_arr(name)
#define def_object_arr(obj,name)

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

//encode
#define begin_msg(name,proto_type,msgid) void name::encode(ByteArray &byteArray){
#define end_msg() };


#define def_err(name)  byteArray.write_int(name);
#define def_int(name)  byteArray.write_int(name);
#define def_int64(name)  byteArray.write_int64(name);
#define def_float(name)  byteArray.write_float(name);
#define def_string(name)  byteArray.write_string(name);
#define def_blob(name)  byteArray.write_blob(name);
#define def_object(objtype,name) name.encode(byteArray);

#define def_int_arr(name) byteArray.write_int(name.size()); \
    do{for(int i=0;i<name.size();++i){byteArray.write_int(name[i]);};}while(false);
#define def_int64_arr(name) byteArray.write_int(name.size()); \
    do{for(int i=0;i<name.size();++i){byteArray.write_int64(name[i]);};}while(false);
#define def_float_arr(name) byteArray.write_int(name.size()); \
            do{for(int i=0;i<name.size();++i){byteArray.write_float(name[i]);};}while(false);
#define def_string_arr(name) byteArray.write_int(name.size()); \
            do{for(int i=0;i<name.size();++i){byteArray.write_string(name[i]);};}while(false);

#define def_object_arr(obj,name) byteArray.write_int(name.size()); \
            do{for(int i=0;i<name.size();++i){name[i].encode(byteArray);};}while(false);

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

//decode
#define begin_msg(name,proto_type,msgid)  void name::decode(ByteArray &byteArray){

#define end_msg() };


#define def_err(name) name = byteArray.read_int();
#define def_int(name) name = byteArray.read_int();
#define def_int64(name) name = byteArray.read_int64();
#define def_float(name) name = byteArray.read_float();
#define def_string(name) name = byteArray.read_string();
#define def_blob(name) name = byteArray.read_blob();
#define def_object(objtype,name) name.decode(byteArray);

#define def_int_arr(name) do{\
	int len = byteArray.read_int();\
	for(int i=0;i<len;++i){\
		name.push_back(byteArray.read_int());\
	} \
	}while(false);


#define def_int64_arr(name) do{\
    int len = byteArray.read_int();\
    for(int i=0;i<len;++i){\
    name.push_back(byteArray.read_int64());\
    } \
    }while(false);


#define def_float_arr(name) do{\
	int len = byteArray.read_int();\
	for(int i=0;i<len;++i){\
		name.push_back(byteArray.read_float()); \
	} \
	}while(false);
#define def_string_arr(name) do{\
	int len = byteArray.read_int();\
	for(int i=0;i<len;++i){\
		name.push_back(byteArray.read_string()); \
	} \
	}while(false);

#define def_object_arr(obj,name) do{\
	int len = byteArray.read_int();\
	for(int i=0;i<len;++i){\
		obj tmp;				\
		tmp.decode(byteArray);  \
		name.push_back(tmp);    \
	} \
	}while(false);

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

//build
#define begin_msg(name,proto_type,msgid)	void name::build(ByteArray &byteArray){ \
	_type = proto_type;	\
    _id = msgid;		\
    INetPacket::build(byteArray);\
	encode(byteArray);

#define end_msg() };


#define def_err(name)
#define def_int(name)
#define def_int64(name)
#define def_float(name)
#define def_string(name)
#define def_blob(name)
#define def_object(objtype, name)

#define def_int_arr(name)
#define def_int64_arr(name)
#define def_float_arr(name)
#define def_string_arr(name)

#define def_object_arr(obj,name)

	
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


//tostring
#define begin_msg(name,proto_type,msgid) std::string name::tostring(){ \
    std::ostringstream ostr; \
    ostr<<#name"{";

#define end_msg() ostr<<"}";return ostr.str();};


#define def_err(name) ostr<<name<<",";
#define def_int(name) ostr<<name<<",";
#define def_int64(name) ostr<<name<<",";
#define def_float(name) ostr<<name<<",";
#define def_string(name) ostr<<"'"<<name<<"',";
#define def_blob(name) ostr<<"[";std::copy(name.begin(),name.end(),std::ostream_iterator<int>(ostr,","));ostr<<"],";

#define def_object(objtype,name) ostr<<"{"<<name.tostring()<<"}";

#define def_int_arr(name) ostr<<"[";std::copy(name.begin(),name.end(),std::ostream_iterator<int>(ostr,","));ostr<<"],";
#define def_int64_arr(name) ostr<<"[";std::copy(name.begin(),name.end(),std::ostream_iterator<int>(ostr,","));ostr<<"],";
#define def_float_arr(name) ostr<<"[";std::copy(name.begin(),name.end(),std::ostream_iterator<float>(ostr,","));ostr<<"],";
#define def_string_arr(name) ostr<<"[";std::copy(name.begin(),name.end(),std::ostream_iterator<std::string>(ostr,","));ostr<<"],";

#define def_object_arr(obj,name) ostr<<"["; \
    for(std::vector<obj>::iterator obj##It = name.begin();obj##It != name.end();++obj##It){ostr<<obj##It->tostring()<<",";} \
    ostr<<"],";


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

