#include "SearchOps.h"
#include <stdio.h>
#include <limits.h>
#include <ctype.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <libgen.h>
#include <fcntl.h>

using namespace std;

bool fnmatch (string pattern, string name, int dummy)
{
        if (pattern == "*")
        {
                return true;
        }
        if (pattern.substr(0,2) == "*.")
        {
                string extToFind = pattern.substr(2, pattern.size() - 2);
                if ((name.size() > extToFind.size()) &&(extToFind == name.substr(name.size() - extToFind.size(), extToFind.size())))
                {
                        return 0; // match
                }
                else
                {
                        return 1; // don't match
                }
        }
        return false;
}

struct _find_search_t
{
    char *pattern;
    char *curfn;
    char *directory;
    int dirlen;
    DIR *dirfd;
};

long _findfirst(const char *pattern, struct _finddata_t *data)
{
    _find_search_t *fs = new _find_search_t;
    fs->curfn = NULL;
    fs->pattern = NULL;

    // Separate the mask from directory name
    const char *mask = strrchr (pattern, '/');
    if (mask)
    {
        fs->dirlen = mask - pattern;
        mask++;
        fs->directory = (char *)malloc (fs->dirlen + 1);
        memcpy (fs->directory, pattern, fs->dirlen);
        fs->directory [fs->dirlen] = 0;
    }
    else
    {
        mask = pattern;
        fs->directory = strdup (".");
        fs->dirlen = 1;
    }

    fs->dirfd = opendir (fs->directory);
    if (!fs->dirfd)
    {
        _findclose ((long)fs);
        return -1;
    }

    /* Hack for "*.*" -> "*' from DOS/Windows */
    if (strcmp (mask, "*.*") == 0)
        mask += 2;
    fs->pattern = strdup (mask);

    /* Get the first entry */
    if (_findnext ((long)fs, data) < 0)
    {
        _findclose ((long)fs);
        return -1;
    }

    return (long)fs;
}

int _findnext(long id, struct _finddata_t *data)
{
    struct stat stat_buf;
    _find_search_t *fs = (_find_search_t *)id;

    /* Loop until we run out of entries or find the next one */
    dirent *entry;
    for (;;)
    {
        if (!(entry = readdir (fs->dirfd)))
            return -1;

        /* See if the filename matches our pattern */
        if (fnmatch (fs->pattern, entry->d_name, 0) == 0)
            break;
    }

    if (fs->curfn)
        free (fs->curfn);
    data->name = fs->curfn = strdup (entry->d_name);

    size_t namelen = strlen (entry->d_name);
    char *xfn = new char [fs->dirlen + 1 + namelen + 1];
    sprintf (xfn, "%s/%s", fs->directory, entry->d_name);

    /* stat the file to get if it's a subdir and to find its length */
    if (stat (xfn, &stat_buf))
    {
        // Hmm strange, imitate a zero-length file then
        data->attrib = _A_NORMAL;
        data->size = 0;
    }
    else
    {
        if (S_ISDIR(stat_buf.st_mode))
            data->attrib = _A_SUBDIR;
        else
            /* Default type to a normal file */
            data->attrib = _A_NORMAL;

        data->size = (unsigned long)stat_buf.st_size;
    }

    delete [] xfn;

    /* Files starting with a dot are hidden files in Unix */
    if (data->name [0] == '.')
        data->attrib |= _A_HIDDEN;

    return 0;
}

int _findclose(long id)
{
    int ret;
    _find_search_t *fs = (_find_search_t *)id;

    ret = fs->dirfd ? closedir (fs->dirfd) : 0;
    free (fs->pattern);
    free (fs->directory);
    if (fs->curfn)
        free (fs->curfn);
    delete fs;

    return ret;
}

int GetCurrentDirectory(size_t maxlen, char* buffer)
{
    int count = readlink( "/proc/self/exe", buffer, maxlen );
    return count;
}

int DeleteFile(const char* name)
{
    return remove(name);
}

int GetModuleFileName(int pid, char* buffer, size_t length)
{
    char szTmp[32];
    if (0 == pid)
        pid = getpid();
    sprintf(szTmp, "/proc/%d/exe", getpid());
    int bytes = readlink(szTmp, buffer, length);
    if (bytes >= 0)
        buffer[bytes] = '\0';
    return bytes;
}

bool CopyFile(const char* src, const char* dest, bool fail_if_exists)
{
    int read_fd;
    int write_fd;
    struct stat stat_buf;
    off_t offset = 0;

    read_fd = open (src, O_RDONLY);

    fstat (read_fd, &stat_buf);

    write_fd = open (dest, O_WRONLY | O_CREAT, stat_buf.st_mode);

    sendfile (write_fd, read_fd, &offset, stat_buf.st_size);

    close (read_fd);

    close (write_fd);

    return true;
}

long GetFileAttributes(const char* name)
{
    _finddata_t fd;
    long handle = _findfirst(name, &fd);
    return fd.attrib;
}

bool SetCurrentDirectory(const char* pathname)
{
    return chdir(pathname);
}

bool CreateDirectory(const char* path_name, LPSECURITY_ATTRIBUTES attr)
{
    mkdir(path_name, 0);
    return true;
}

bool SetFileAttributes(const char* name, long attrib)
{
    return chmod(name, attrib);
}

void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext)
{
    char *tmp_path;
    int conut;
    char tmp[128], tmp_name[128], tmp_ext[128];
    int  check=0;
    int check_1=0;
     int i,j=0,k=0;
     int conut_1,conut_2, conut_3;

     tmp_path = NULL;
     if( ( realpath(path, tmp_path) == NULL) )
     {
      tmp_path = (char*) path;
     }

     conut=0;
     do
     {
      if( tmp_path[conut] == ':')
      {
       check=1;
       conut_1 = conut;
      }
      if (tmp_path[conut] == '/')
      {
       conut_2 = conut;
      }
      if (tmp_path[conut] == '.')
      {
       check_1 = 1;
       conut_3 = conut;
      }
      conut++;
     }while(tmp_path[conut] != '\0');

     if ( check == 1)
     {
      fname = basename(tmp_path);
      dir = dirname(tmp_path);
      for( i = 0 ; i< conut; i++)
      {
       if ( dir[i] != ':')
       {
        if( i>=2 )
        {
         tmp[i-2]=dir[i];
        }
       }
       else
       {
        drive=&dir[i-1];

       }
      }

      memset(dir,0,sizeof(dir));
      strcpy(dir,tmp);

     }
     else
     {
      fname = basename(tmp_path);
      dir = dirname(tmp_path);

     }

     if (check_1 == 1)
     {
      while( fname[j] != '\0')
      {
    if ( fname[j] == '.')
       {
        k=j;
       }

       if ( j >= k && k !=0 )
       {
         tmp_ext[j-k]=fname[j ];
       }
       else
       {
        tmp_name[j]= fname[j];


       }
       j++;
      }
      memset(ext,0,sizeof(ext));
      memset(fname, 0, sizeof(fname));
      strcpy(fname, tmp_name);
      strcpy(ext,tmp_ext);

     }
}

bool RemoveDirectory(const char* path)
{
    return rmdir(path);
}

char* strupr(char* ioString)
{
    int i;
    int theLength = strlen(ioString);
    for (i=0; i<theLength; ++i)
    {
        ioString[i] = toupper(ioString[i]);
    }
    return ioString;
}

char* strlwr(char* ioString)
{
    int i;
    int theLength = strlen(ioString);
    for (i=0; i<theLength; ++i)
    {
        ioString[i] = tolower(ioString[i]);
    }
    return ioString;
}

int stricmp(const char* src, const char* dst)
{
    return strcasecmp(src, dst);
}
