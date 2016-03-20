
#include "Utils.h"

typedef void (*HTTP_REQ_FUN)(PersistID id, string& cmd);

namespace NanoWeb
{
	class DllLoader
	{
	public:
		DllLoader()
		{
		    string app_path;
		    W_GetAppPath(app_path);
		    app_path += "/libNanoWebs.so";

			m_hMod = dlopen(app_path.c_str(), RTLD_LAZY);
			if (!m_hMod)
			{
				printf("无法加载libNanoWebs.so[err: %s]\n", dlerror());
			}
		}
		~DllLoader()
		{
			if (m_hMod)
				dlclose(m_hMod);
			m_hMod = NULL;
		}
		static DllLoader& Inst(){
			static DllLoader load;
			return load;
		}
		void* GetProcAddress(const char* lpProcName)
		{
			if (!m_hMod)
				return 0;
			return dlsym(m_hMod,lpProcName);
		}
	public:
		void* m_hMod;
	};

    // 开启网页服务器
    void Open(unsigned short port)
    {
        typedef void (*FUNC)(unsigned short port);
		static FUNC func = (FUNC)DllLoader::Inst().GetProcAddress("Open");
		if (func)
        {
            func(port);
        }
    }

    // 关闭网页服务器
    void Close()
    {
        typedef void (*FUNC)();
		static FUNC func = (FUNC)DllLoader::Inst().GetProcAddress("Close");
		if (func)
        {
            func();
        }
    }

    // 设置http请求回调
    void SetCallBack(HTTP_REQ_FUN fun)
    {
        typedef void (*FUNC)(HTTP_REQ_FUN fun);
		static FUNC func = (FUNC)DllLoader::Inst().GetProcAddress("SetCallBack");
		if (func)
        {
            func(fun);
        }
    }

    // 写入
    void Write(PersistID id, const char* str)
    {
        typedef void (*FUNC)(PersistID id, const char* str);
		static FUNC func = (FUNC)DllLoader::Inst().GetProcAddress("Write");
		if (func)
        {
            func(id, str);
        }
    }

    // 写入行
    void WriteLine(PersistID id, const char* str)
    {
        typedef void (*FUNC)(PersistID id, const char* str);
		static FUNC func = (FUNC)DllLoader::Inst().GetProcAddress("WriteLine");
		if (func)
        {
            func(id, str);
        }
    }

    // 设置table title 和 列名
    void SetTableTitle(PersistID id, const char* title, const char* name, const char* val)
    {
        typedef void (*FUNC)(PersistID id, const char* title, const char* name, const char* val);
		static FUNC func = (FUNC)DllLoader::Inst().GetProcAddress("SetTableTitle");
		if (func)
        {
            func(id, title, name, val);
        }
    }

    // 添加table行
    void AddTableItem(PersistID id, const char* name, const char* val)
    {
        typedef void (*FUNC)(PersistID id, const char* name, const char* val);
		static FUNC func = (FUNC)DllLoader::Inst().GetProcAddress("AddTableItem");
		if (func)
        {
            func(id, name, val);
        }
    }

    // 发送HTTP 回应
    void Flush(PersistID id)
    {
        typedef void (*FUNC)(PersistID id);
		static FUNC func = (FUNC)DllLoader::Inst().GetProcAddress("Flush");
		if (func)
		{
			func(id);
		}
    }

    // 发送文件
    void SendFile(PersistID id, const char* path)
    {
        typedef void (*FUNC)(PersistID id, const char* path);
		static FUNC func = (FUNC)DllLoader::Inst().GetProcAddress("SendFile");
		if (func)
		{
			func(id, path);
		}
    }

    // 关闭http链接
    void Disconnect(PersistID id, int secs)
    {
        typedef void (*FUNC)(PersistID id, int secs);
		static FUNC func = (FUNC)DllLoader::Inst().GetProcAddress("Disconnect");
		if (func)
		{
			func(id, secs);
		}
    }

    //获取客户端地址
	static const char* GetClientIP(PersistID id)
	{
		typedef const char* (*FUNC)(PersistID id);
		static FUNC func = (FUNC)DllLoader::Inst().GetProcAddress("GetClientIP");
		if (func)
		{
			return func(id);
		}
		return NULL;
	}
};
