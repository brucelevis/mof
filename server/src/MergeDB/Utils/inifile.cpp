#include "inifile.h"
#include "iniparser.h"
#include <fstream>
#include <stdexcept>
#include "assert.h"



#if defined(CLIENT)
#include "CryptUtils.h"
#include "cocos2d.h"
using namespace cocos2d;
#endif

namespace
{
  class IniFileEvent : public IniParser::Event
  {
      IniFile& iniFile;

      std::string section;
      std::string key;

    public:
      IniFileEvent(IniFile& iniFile_)
        : iniFile(iniFile_)
        { }

      bool onSection(const std::string& section);
      bool onKey(const std::string& key);
      bool onValue(const std::string& value);
  };

    bool IniFileEvent::onSection(const std::string& section_)
    {
      section = section_;
      return false;
    }

    bool IniFileEvent::onKey(const std::string& key_)
    {
      key = key_;
      return false;
    }

    bool IniFileEvent::onValue(const std::string& value)
    {
      iniFile.setValue(section, key, value);
      return false;
    }

}

IniFile::IniFile(const std::string& filename)
{
#if defined(CLIENT)
    TargetPlatform form = CCApplication::sharedApplication()->getTargetPlatform();
    if (form == kTargetAndroid)
    {
        unsigned long size;
        unsigned char* data = CCFileUtils::sharedFileUtils()->getFileData(filename.data(), "r", &size);
        CCLOG("load ini:%s",filename.c_str());
        std::vector<unsigned char> file = CryptUtils::decryptBuf(data, size);
        IniFileEvent ev(*this);
        IniParser(ev).parse(file);
        return;
    }
#endif
    
    
    
    try
    {
#if defined(Crypt)
//        std::vector<unsigned char> file = CryptUtils::decryptFile(filename);
        unsigned long size;
        unsigned char* data = CCFileUtils::sharedFileUtils()->getFileData(filename.data(), "r", &size);
        std::vector<unsigned char> file = CryptUtils::decryptBuf(data, size);
        IniFileEvent ev(*this);
        IniParser(ev).parse(file);
#else
        std::ifstream in(filename.c_str());
        if (!in)
        {
            printf("could not open ini file:%s\n", filename.c_str());
                
        }
        IniFileEvent ev(*this);
        IniParser(ev).parse(in);
#endif
    }
    catch(...)
    {
        printf("some error happend when parse ini file:%s\n", filename.c_str());
        assert(0);
    }
}

IniFile::IniFile(std::istream& in)
{
  IniFileEvent ev(*this);
  IniParser(ev).parse(in);
}

std::ostream& operator << (std::ostream& out, const IniFile& ini)
{
  for (IniFile::MapType::const_iterator si = ini.data.begin();
       si != ini.data.end(); ++si)
  {
    out << '[' << si->first << "]\n";
    for (IniFile::MapType::mapped_type::const_iterator it = si->second.begin();
         it != si->second.end(); ++it)
      out << it->first << '=' << it->second << '\n';
  }
  return out;
}


std::vector<std::string> IniFile::getTable(const std::string& section,
                                   const std::string& token) const
{
    std::vector<std::string> _tokens;
    std::string str = getValue(section,token,"");
    if(str.empty())
        return  _tokens;
    
    const char* src = str.c_str();
    const char* sepstr = ";";
    int seplen = strlen(sepstr);
    
    while (*src){
        const char* pos = strstr(src, sepstr);
        if (pos == NULL){
            _tokens.push_back(src);
            break;
        }
        _tokens.push_back(std::string(src, pos - src));
        src = pos + seplen;
    }
    return _tokens;
}




