#include <iosfwd>
#include <string>
#include <vector>


/**
 * Parser for files in ini-format
 */
class IniParser
{
  public:
    class Event
    {
      public:
        // events return true, if parsing should be stopped
        virtual bool onSection(const std::string& section);
        virtual bool onKey(const std::string& key);
        virtual bool onValue(const std::string& key);
        virtual bool onComment(const std::string& comment);
        virtual bool onError();
    };

  private:
    Event& event;
    std::string data;
    enum
    {
      state_0,
      state_section,
      state_key,
      state_key_sp,
      state_value0,
      state_value,
      state_comment

    } state;

  public:
    IniParser(Event& event_)
      : event(event_),
        state(state_0)
      { }

    bool parse(char ch);
    void end();
    void parse(std::istream& in);
    void parse(std::vector<unsigned char> in);
};

