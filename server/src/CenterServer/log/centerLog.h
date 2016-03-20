#pragma once

#include <map>
#include <vector>
#include <string>
using namespace std;

class LogProtocol
{
public:
	enum LogFieldType
	{
		kLogFieldInt,
		kLogFieldString,
	};

	struct LogField
	{
		string mName;
		int    mType;
	};


	LogProtocol() {}
	void addField(int type, const char* name) 
	{
		LogField field;
		field.mType = type;
		field.mName = name;
		mFields.push_back(field);
	}

	const vector<LogField>& getFields() {
		return mFields;
	}

	const string& getName() {
		return mName;
	}

	vector<LogField> mFields;
	string mName;
};


class CenterLog
{
public:
	CenterLog(LogProtocol* protocol);
	~CenterLog();

	void setField(const char* name, int val) 
	{
		mIntFields[name] = val;
	}
	void setField(const char* name, const string& val)
	{
		mStrFields[name] = val;
	}

	void save();

private:

	bool mSaved;
	LogProtocol* mProtocol;

	map<string, int> mIntFields;
	map<string, string> mStrFields;
};