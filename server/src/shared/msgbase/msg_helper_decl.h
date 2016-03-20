#ifndef SHARED_MSG_HELPER_H
#define SHARED_MSG_HELPER_H

#ifndef msg_begin
#define msg_begin(name) \
class name : public MsgBase\
{ \
public: \
    name();
#endif


#ifndef object_begin
#define object_begin(name)  \
class name : public MsgObject\
{ \
public: \
    virtual MsgObject* clone() const;\
    virtual MsgObject* clone(void* addr) const;\
    name();
#endif

#ifndef msg_member_int
#define msg_member_int(name) \
int name;
#endif

#ifndef msg_member_int64
#define msg_member_int64(name) \
int64_t name;
#endif

#ifndef msg_member_float
#define msg_member_float(name) \
float name;
#endif


#ifndef msg_member_string
#define msg_member_string(name) \
string name;
#endif

#ifndef msg_member_object
#define msg_member_object(type, name) \
type name;
#endif

#ifndef msg_member_int_arr
#define msg_member_int_arr(name) \
vector<int> name;
#endif

#ifndef msg_member_int64_arr
#define msg_member_int64_arr(name) \
vector<int64_t> name;
#endif

#ifndef msg_member_float_arr
#define msg_member_float_arr(name) \
vector<float> name;
#endif

#ifndef msg_member_string_arr
#define msg_member_string_arr(name) \
vector<string> name;
#endif

#ifndef msg_member_object_arr
#define msg_member_object_arr(type, name) \
type msgobject_##name;\
MsgObjectArray<type> name;
#endif

#ifndef object_end
#define object_end() \
};
#endif

#ifndef msg_end
#define msg_end() \
};
#endif

#endif