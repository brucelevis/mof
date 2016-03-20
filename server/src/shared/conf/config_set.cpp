#include "config_set.h"
#include "inifile.h"

#include <string>
#include <vector>
#include "GameIniFile.h"
using namespace std;

ConfigData* ConfigData::createFromIni(const char *filename)
{
    ConfigData* configData = NULL;
    try {
        GameInifile ini(filename);
        configData = new ConfigData;
        vector<string> sections;
        ini.getSections(inserter(sections, sections.begin()));
        for (vector<string>::iterator iter = sections.begin(); iter != sections.end(); iter++) {
            PropertySet* propertySet = new PropertySet;
            
            string section = *iter;
            
            vector<string> keys;
            ini.getKeys(section, inserter(keys, keys.begin()));
            for (vector<string>::iterator keyIter = keys.begin(); keyIter != keys.end(); keyIter++) {
                string key = *keyIter;
                string value = ini.getValue(section, key);
                propertySet->set(key.c_str(), value.c_str());
            }
            
            configData->addPropertySet(section.c_str(), propertySet);
        }
    } catch (...) {
        if (configData) {
            delete configData;
            configData = NULL;
        }
    }
    return configData;
}