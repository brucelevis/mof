#include "cyclemsg.h"
#include "Qcheck.h"
#include <sstream>
#include <iterator>


//init
#define begin_cyclemsg(name) name::name(){ 

#define end_cyclemsg() };

#define def_int(name) name = 0;
#define def_int64(name) name = 0;
#define def_float(name) name = 0.0f;

//原来这里没初始化string,暂时不知道加上去后会不会有问题(by wangzhigang)
#define def_string(name) name = "";

#define def_int_arr(name)
#define def_float_arr(name)
#define def_string_arr(name)

#define def_object_arr(obj,name)

#include "cyclemsgdefine.h"

#undef begin_cyclemsg
#undef end_cyclemsg


#undef def_int
#undef def_int64
#undef def_float
#undef def_string

#undef def_int_arr
#undef def_float_arr
#undef def_string_arr
#undef def_object_arr


//encode
#define begin_cyclemsg(name) void name::encode(ByteArray &byteArray){
#define end_cyclemsg() };


#define def_int(name)  byteArray.write_int(name);
#define def_int64(name)  byteArray.write_int64(name);
#define def_float(name)  byteArray.write_float(name);
#define def_string(name)  byteArray.write_string(name);

#define def_int_arr(name) byteArray.write_int(name.size()); \
            do{for(int i=0;i<name.size();++i){byteArray.write_int(name[i]);};}while(false);
#define def_float_arr(name) byteArray.write_int(name.size()); \
            do{for(int i=0;i<name.size();++i){byteArray.write_float(name[i]);};}while(false);
#define def_string_arr(name) byteArray.write_int(name.size()); \
            do{for(int i=0;i<name.size();++i){byteArray.write_string(name[i]);};}while(false);

#define def_object_arr(obj,name)  byteArray.write_int(name.size()); \
            do{for(int i=0;i<name.size();++i){name[i].encode(byteArray);};}while(false);


#include "cyclemsgdefine.h"

#undef begin_cyclemsg
#undef end_cyclemsg
#undef def_int
#undef def_int64
#undef def_float
#undef def_string

#undef def_int_arr
#undef def_float_arr
#undef def_string_arr
#undef def_object_arr

//decode

#define begin_cyclemsg(name)  void name::decode(ByteArray &byteArray){
#define end_cyclemsg() };


#define def_int(name) name = byteArray.read_int();
#define def_int64(name) name = byteArray.read_int64();
#define def_float(name) name = byteArray.read_float();
#define def_string(name) name = byteArray.read_string();

#define def_int_arr(name) do{\
	int len = byteArray.read_int();\
	for(int i=0;i<len;++i){\
		name.push_back(byteArray.read_int());\
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

#include "cyclemsgdefine.h"

#undef begin_cyclemsg
#undef end_cyclemsg
#undef def_int
#undef def_int64
#undef def_float
#undef def_string

#undef def_int_arr
#undef def_float_arr
#undef def_string_arr
#undef def_object_arr

//tostring
#define begin_cyclemsg(name) std::string name::tostring(){ \
    std::ostringstream ostr; \
    ostr<<"[";

#define end_cyclemsg() ostr<<"]";return ostr.str();};

#define def_int(name) ostr<<name<<",";
#define def_int64(name) ostr<<name<<",";
#define def_float(name) ostr<<name<<",";
#define def_string(name) ostr<<"'"<<name<<"',";

#define def_int_arr(name) ostr<<"[";\
    std::copy(name.begin(),name.end(),std::ostream_iterator<int>(ostr,","));ostr<<"],";
#define def_float_arr(name) ostr<<"[";\
    std::copy(name.begin(),name.end(),std::ostream_iterator<float>(ostr,","));ostr<<"],";
#define def_string_arr(name) ostr<<"[";\
    std::copy(name.begin(),name.end(),std::ostream_iterator<std::string>(ostr,","));ostr<<"],";


#include "cyclemsgdefine.h"

#undef begin_cyclemsg
#undef end_cyclemsg


#undef def_int
#undef def_int64
#undef def_float
#undef def_string

#undef def_int_arr
#undef def_float_arr
#undef def_string_arr
#undef def_object_arr