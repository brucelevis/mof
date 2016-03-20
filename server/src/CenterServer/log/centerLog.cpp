#include "centerLog.h"
#include "log.h"
#include "MsgBuf.h"
#include "sendMessage.h"
#include "MQ.h"
#include "Utils.h"


static map<string, LogProtocol*>&
getProtocolMap() {
	static map<string, LogProtocol*> protoMap;
	return protoMap;
}

static int 
registerProtocol(const char* name, LogProtocol* protocol) {
	getProtocolMap()[name] = protocol;
	return 0;
}

static LogProtocol* 
getProtocol(const char* name) {
	map<string, LogProtocol*>::iterator iter = getProtocolMap().find(name);
	if (iter == getProtocolMap().end()) {
		return NULL;
	}
	return iter->second;
}


///日志创建函数
#define begin_log(name) \
CenterLog create##name##Log() { \
	LogProtocol* proto = getProtocol(#name); \
	return CenterLog(proto);
#define def_int(name) 
#define def_string(name)
#define end_log() }

#include "log_protocol.h"

#undef begin_log
#undef end_log
#undef def_int
#undef def_string

//日志定义
#define begin_log(name) \
struct name##LogProtocol : public LogProtocol {\
	name##LogProtocol() { \
		mName = #name;
#define def_int(name)    addField(LogProtocol::kLogFieldInt, #name);
#define def_string(name) addField(LogProtocol::kLogFieldString, #name);
#define end_log()       } \
};

#include "log_protocol.h"

#undef begin_log
#undef end_log
#undef def_int
#undef def_string

//日志注册
#define begin_log(name) static struct name##LogProtocol __tmp##name##LogRegister; \
		                static int __tmp##name##LogRegisterRet = registerProtocol(#name, &__tmp##name##LogRegister);
#define def_int(name) 
#define def_string(name)
#define end_log() 

#include "log_protocol.h"

#undef begin_log
#undef end_log
#undef def_int
#undef def_string


CenterLog::CenterLog(LogProtocol* protocol) {
	mProtocol = protocol;
	mSaved = false;
}


void
CenterLog::save() {
	if (mProtocol == NULL) {
		return;
	}

	mSaved = true;
	CMsgTyped msg;
	msg.SetInt(0);
	msg.SetInt(1);
	msg.SetInt(0);
	msg.SetString(mProtocol->getName().c_str());
    string now = Utils::currentDate();
    now.append(" ");
    now.append(Utils::currentTime().c_str()) ;
	msg.SetString(now.c_str());
	const vector<LogProtocol::LogField>& fields = mProtocol->getFields();
	for (int i = 0; i < fields.size(); i++) {
		LogProtocol::LogField field = fields[i];
		switch (field.mType) {
			case LogProtocol::kLogFieldInt: {
				int val = mIntFields[field.mName];
				msg.SetInt(val);
				break;
			}
			case LogProtocol::kLogFieldString: {
				string val = mStrFields[field.mName];
				msg.SetString(val.c_str());
				break;
			}
		}
	}

	skynet_message logMsg;
	logMsg.sz = msg.GetLength();
	logMsg.data = new char[msg.GetLength()];
	memcpy(logMsg.data, msg.GetData(), msg.GetLength());
	logMsg.source = 0;
	logMsg.session = 0;
	sendMessageToLog(&logMsg);
}

CenterLog::~CenterLog() {
	if (!mSaved) {
		save();
	}
}