#ifndef MSG_HELPER_CONSTRUCTOR
#define MSG_HELPER_CONSTRUCTOR

#ifdef msg_begin
#undef msg_begin
#endif
#define msg_begin(name)  \
name::name() \
{\
    MsgVariant var;\
    (void)var; \


#ifdef object_begin
#undef object_begin
#endif
#define object_begin(name)  \
MsgObject* name::clone(void* addr) const\
{\
    name* val = (name*)addr;\
    *val = *this;\
    return (MsgObject*)val;\
}\
MsgObject* name::clone() const\
{\
    name* val = new name;\
    *val = *this;\
    return (MsgObject*)val;\
}\
name::name() \
{\
    MsgVariant var;\
    (void)var; \


#ifdef msg_member_int
#undef msg_member_int
#endif
#define msg_member_int(name) \
    name = 0;\
    var = MsgVariant((int*)&name);\
    addMember(var);

#ifdef msg_member_int64
#undef msg_member_int64
#endif
#define msg_member_int64(name) \
    name = 0; \
    var = MsgVariant((int64_t*)&name);\
    addMember(var);

#ifdef msg_member_float
#undef msg_member_float
#endif
#define msg_member_float(name) \
    name = 0.0f;\
    var = MsgVariant((float*)&name);\
    addMember(var);

#ifdef msg_member_string
#undef msg_member_string
#endif
#define msg_member_string(name) \
    var = MsgVariant((string*)&name);\
    addMember(var);

#ifdef msg_member_object
#undef msg_member_object
#endif
#define msg_member_object(type, name) \
    var = MsgVariant((MsgObject*)&name);\
    addMember(var);


#ifdef msg_member_int_arr
#undef msg_member_int_arr
#endif
#define msg_member_int_arr(name) \
    var = MsgVariant(&name); \
    addMember(var);



#ifdef msg_member_int64_arr
#undef msg_member_int64_arr
#endif
#define msg_member_int64_arr(name) \
    var = MsgVariant(&name); \
    addMember(var);



#ifdef msg_member_float_arr
#undef msg_member_float_arr
#endif
#define msg_member_float_arr(name) \
    var = MsgVariant(&name); \
    addMember(var);



#ifdef msg_member_string_arr
#undef msg_member_string_arr
#endif
#define msg_member_string_arr(name) \
    var = MsgVariant(&name); \
    addMember(var);


#ifdef msg_member_object_arr
#undef msg_member_object_arr
#endif
#define msg_member_object_arr(type, name) \
    var = MsgVariant((void*)&name, (MsgObject*)&msgobject_##name); \
    addMember(var);


#ifdef object_end
#undef object_end
#endif
#define object_end() \
}

#ifdef msg_end
#undef msg_end
#endif
#define msg_end() \
}

#endif