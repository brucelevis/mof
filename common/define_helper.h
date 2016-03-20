//
//  define_helper.h
//  GameSrv
//
//  Created by 丁志坚 on 11/25/14.
//
//

#ifndef GameSrv_define_helper_h
#define GameSrv_define_helper_h


#define NamedEnum(enumname) e##enumname

#ifndef NAMED_ENUM_IMP

#define BeginNamedEnum(enumname) enum NamedEnum(enumname) {
#define NamedEnumMember(enum, name) enum,
#define EndNamedEnum(enumname)  };\
int get##enumname##ByName(const char* name);\
const char* getNameBy##enumname(int type); \
int get##enumname##Count();

#else

#define BeginNamedEnum(enumname) static const char* s_##enumname[] = {
#define NamedEnumMember(enum, name) #name,
#define EndNamedEnum(enumname)  };                                      \
int get##enumname##ByName(const char* name)                             \
{                                                                       \
for (int i = 0; i < sizeof(s_##enumname) / sizeof(const char*); i++)\
{                                                                   \
if (strcmp(s_##enumname[i], name) == 0)                         \
{                                                               \
return i;                                                   \
}                                                               \
}                                                                   \
return -1;                                                          \
}                                                                       \
const char* getNameBy##enumname(int type)                               \
{                                                                       \
if (type < 0 || type >= sizeof(s_##enumname) / sizeof(const char*) )\
{\
return "";\
}\
return s_##enumname[type];\
}\
int get##enumname##Count() \
{\
return sizeof(s_##enumname) / sizeof(const char*);\
}

#endif

#endif
