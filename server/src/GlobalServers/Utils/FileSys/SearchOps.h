#ifndef SEARCHOPS_H
#define SEARCHOPS_H

#include <sys/types.h>

struct _finddata_t
{
     char *name;
     int attrib;
     unsigned long size;
};

#ifdef LINUX_
#define _A_NORMAL 0x00  /* Normalfile-Noread/writerestrictions */
#define _A_RDONLY 0x01  /* Read only file */
#define _A_HIDDEN 0x02  /* Hidden file */
#define _A_SYSTEM 0x04  /* System file */
#define _A_ARCH   0x20  /* Archive file */
#define _MAX_PATH   128
#endif
#define _A_SUBDIR 0x10  /* Subdirectory */

long _findfirst(const char *pattern, struct _finddata_t *data);

int _findnext(long id, struct _finddata_t *data);

int _findclose(long id);

int GetCurrentDirectory(size_t maxlen, char* buffer);

int DeleteFile(const char* name);

int GetModuleFileName(int pid, char* buffer, size_t length);

bool CopyFile(const char* src, const char* dest, bool fail_if_exists);

long GetFileAttributes(const char* name);

bool SetCurrentDirectory(const char* pathname);

typedef struct _SECURITY_ATTRIBUTES
{
    size_t length;
    void* desc;
    bool inherit;
}SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

bool CreateDirectory(const char* path_name, LPSECURITY_ATTRIBUTES attr);

bool SetFileAttributes(const char* name, long attrib);

void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext);

bool RemoveDirectory(const char* path);

char* strupr(char* ioString);

char* strlwr(char* ioString);

int stricmp(const char* src, const char* dst);

#define _vsnwprintf vsnwprintf
#define _vsnprintf   vsnprintf
#define _snprintf   snprintf
#define _snwprintf  snwprintf
#define _vsnwprintf vsnwprintf
#define _wtoi   wtoi
#define _isnan  isnan
#define HANDLE  long

#endif // SEARCHOPS_H
