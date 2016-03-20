#ifndef _INIFILE_H_
#define _INIFILE_H_

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <string.h>

//ini文件读取类
class IniFile
{
    friend std::ostream& operator << (std::ostream& out, const IniFile& ini);

    typedef std::map<std::string, std::map<std::string, std::string> > MapType;
    MapType data;

  public:
    IniFile()  { }
    explicit IniFile(const std::string& filename);
    explicit IniFile(std::istream& in);

    /// Returns true, if section exists.
    bool exists(const std::string& section) const
    { return data.find(section) != data.end(); }

    /// Returns true, if key exists in section exists.
    bool exists(const std::string& section,
                const std::string& token) const
    {
      MapType::const_iterator si = data.find(section);
      return    si != data.end()
             && si->second.find(token) != si->second.end();
    }

    /// Returns associated value from section-key-pair or default-value.
    std::string getValue(const std::string& section,
       const std::string& token, const std::string& def = std::string()) const
    {
      // find section

      MapType::const_iterator si = data.find(section);

      if (si != data.end())
      {
        // find token

        MapType::mapped_type::const_iterator token_it = si->second.find(token);

        if (token_it != si->second.end())
          return token_it->second;
      }

      return def;
    }

    /// Get the value and convert it with istream-operator.
    /// The return-type is identified by the default-value-type.
    template <typename T>
     T getValueT(const std::string& section,
                const std::string& token,
                const T& def) const
    {
      // find section

      MapType::const_iterator si = data.find(section);

      if (si != data.end())
      {
        // find token

        MapType::mapped_type::const_iterator token_it = si->second.find(token);

        if (token_it != si->second.end())
        {
          // extract value with stream
          T value;
          std::istringstream s(token_it->second);
          s >> value;

          if (s)
            return value;
        }
      }

      return def;
    }

    std::vector<std::string> getTable(const std::string& section,
                                       const std::string& token) const;
    template <typename T>
    std::vector<T> getTableT(const std::string& section,
                const std::string& token ,const T& def) const
    {
        std::vector<T> result;
        std::string str = getValue(section,token,"");
        if(str.empty())
            return  result;

        std::vector<std::string> _tokens;
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

        for (int i = 0 , ni = _tokens.size(); i< ni ; ++i)
        {
            T value;
            std::istringstream s(_tokens[i]);
            s >> value;
            result.push_back(value);
        }
        return result;
    }



    /// setting a new value
    void setValue(const std::string& section, const std::string& key,
      const std::string& value)
    {
      data[section][key] = value;
    }

    /// setting a new value with a type (need output-operator for ostream).
    template <typename T>
    void setValueT(const std::string& section, const std::string& key,
      const T& value)
    {
      std::ostringstream v;
      v << value;
      data[section][key] = v.str();
    }

    /**
     * Get the names of sections.
     * example1:
     * <code>
     *  IniFile ini("my.ini");
     *
     *  // copy names of sections into a container
     *  std::set<std::string> keys;
     *  ini.getSections(std::inserter(s, s.begin());
     *
     *  // or printing the names:
     *  ini.getSections(std::ostream_iterator(std::cout, "\n"));
     * </code>
     */
    template <typename OutputIterator>
    void getSections(OutputIterator oi)
    {
      for (MapType::const_iterator it = data.begin(); it != data.end(); ++it, ++oi)
        *oi = it->first;
    }

    /**
     * Get the keys of a section.
     * example1:
     * <code>
     *  IniFile ini("my.ini");
     *
     *  // copy keys into a container
     *  std::set<std::string> keys;
     *  ini.getKeys("section2", std::inserter(s, s.begin());
     *
     *  // or printing the keys:
     *  ini.getKeys("section2", std::ostream_iterator(std::cout, "\n"));
     * </code>
     */
    template <typename OutputIterator>
    void getKeys(const std::string& section, OutputIterator oi)
    {
      MapType::const_iterator si = data.find(section);
      if (si != data.end())
      {
        for (MapType::mapped_type::const_iterator it = si->second.begin();
             it != si->second.end(); ++it, ++oi)
          *oi = it->first;
      }
    }

};

/// Outputs ini-file to a output-stream
std::ostream& operator << (std::ostream& out, const IniFile& ini);


#endif // _INIFILE_H_
