#include "iniparser.h"
#include <cctype>
#include <iostream>
#include <stdexcept>
#ifdef CLIENT
#include "cocos2d.h"
using namespace cocos2d;
#endif

bool IniParser::Event::onSection(const std::string& section)
{
  return false;
}

bool IniParser::Event::onKey(const std::string& key)
{
  return false;
}

bool IniParser::Event::onValue(const std::string& key)
{
  return false;
}

bool IniParser::Event::onComment(const std::string& comment)
{
  return false;
}

bool IniParser::Event::onError()
{
#ifdef CLIENT
//    CCLog("+++++++++++++++IniParser::Event::onError");
#endif
    // throw std::runtime_error("parse error in ini-file");
  return true;
}

void IniParser::parse(std::istream& in)
{
  char ch;
  while (in.get(ch) && !parse(ch))    ;
  end();
}

void IniParser::parse(std::vector<unsigned char> in)
{
    char ch;
    for (int i=0; i<in.size(); ++i)
    {
        ch = in[i];
        if(!parse(ch))
            continue;
    }
    end();
}

bool IniParser::parse(char ch)
{
  bool ret = false;
  switch (state)
  {
    case state_0:
      if (ch == '[')
      {
        data = "";
        state = state_section;
      }
      else if (std::isalnum(ch))
      {
        data = ch;
        state = state_key;
      }
      else if (ch == '#' || ch == ';')
      {
        state = state_comment;
      }
      else if (std::isspace(ch))
        ;
      else
      {
          //这里的报错没有用啊，根本没错。---linshusen
//        ret = event.onError();
          ;
      }
      break;

    case state_section:
      if (ch == ']')
      {
        ret = event.onSection(data);
        data.clear();
        state = state_0;
      }
      else
        data += ch;
      break;

    case state_key:
      if (ch == '=')
      {
        ret = event.onKey(data);
        state = state_value0;
      }
      else if (std::isspace(ch))
      {
        ret = event.onKey(data);
        state = state_key_sp;
      }
      else
        data += ch;
      break;

    case state_key_sp:
      if (ch == '=')
        state = state_value0;
      else if (!std::isspace(ch))
      {
        ret = event.onError();
      }
      break;

    case state_value0:
      if (ch == '\n'||ch == '\r')
      {
        ret = event.onValue(std::string());
        state = state_0;
      }
      else if (!std::isspace(ch))
      {
        data = ch;
        state = state_value;
      }
      break;

    case state_value:
      if (ch == '\n'||ch == '\r')
      {
        ret = event.onValue(data);
        data.clear();
        state = state_0;
      }
      else
        data += ch;
      break;

    case state_comment:
      if (ch == '\n'||ch == '\r')
        state = state_0;
      break;
  }

  return ret;
}

void IniParser::end()
{
  switch (state)
  {
    case state_0:
    case state_comment:
      break;

    case state_section:
    case state_key:
    case state_key_sp:
      event.onError();
      break;

    case state_value0:
      event.onValue(std::string());
      break;

    case state_value:
      event.onValue(data);
      break;
  }
}

