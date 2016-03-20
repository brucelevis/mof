#if !defined(CYCLE_MSG_H_)
#define CYCLE_MSG_H_
#include <vector>
#include "NetPacket.h"


#define begin_cyclemsg(name) class name { \
	public: \
    name();  \
	void encode(ByteArray &byteArray); \
	void decode(ByteArray &byteArray); \
    std::string tostring();

#define end_cyclemsg() };


#define def_int(name) int name;
#define def_int64(name) int64_t name;
#define def_float(name) float name;
#define def_string(name) std::string name;

#define def_int_arr(name) std::vector<int> name;
#define def_float_arr(name) std::vector<float> name;
#define def_string_arr(name) std::vector<std::string> name;
#define def_object_arr(obj,name) std::vector<obj> name;

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

#endif
