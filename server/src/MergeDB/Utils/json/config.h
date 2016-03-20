#ifndef JSON_CONFIG_H_INCLUDED
# define JSON_CONFIG_H_INCLUDED


/// If defined, indicates that Json use exception to report invalid type manipulation
/// instead of C assert macro.
# define JSON_USE_EXCEPTION 1

# if defined(JSON_DLL_BUILD)
#  define JSON_API __declspec(dllexport)
# elif defined(JSON_DLL)
#  define JSON_API __declspec(dllimport)
# else
#  define JSON_API
# endif

#endif // JSON_CONFIG_H_INCLUDED
