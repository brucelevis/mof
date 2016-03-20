/*-------------------------------------------------------------------------
	created:	2010/08/23  14:11
	filename: 	e:\Project_SVN\Server\ServerDemo\GameObject\GameType.h
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/

#pragma once

#include "LuaHandle.h"

#include <string>
#include <map>
#include <list>
#include <queue>
using namespace std;

class IGame;

typedef struct tagRECT
{
    long    left;
    long    top;
    long    right;
    long    bottom;
} RECT;


enum GameType
{
	GAME_NONE,
	GAME_SCENE,
	GAME_PLAYER,
	GAME_NPC,
	GAME_HELPER,
	GAME_ITEM,
	GAME_SYSTEM = 99,   // 系统本身
};

enum SceneType
{
	SCENE_PUBLIC, // 公共场景
	SCENE_COPY,   // 副本场景
};

enum ProcessorIdEnum
{
	PROC_DISTRICT = 1, // 登录， 帐号状态用途， 访问 Nx_District 表
	PROC_STORAGE,   // 对象存储用， 访问Nx_Storage 表
	PROC_GLOBAL_MSG, // 公共消息, 访问TranCache 表
	PROC_LOG, // 日志
	PROC_MAIL,// 邮件
	PROC_SHOPPING,// 商城
	PROC_DETAILS,//对象细节，用于运营
	PROC_GM,
};

enum GLOBAL_SERVER_DEF
{
	IdServer = 0,//存储
	LoginServer, //登录
	MailServer,// 邮件
	LogServer,// 日志

	Reserve1,//保留1
	Reserve2,//保留2
	Reserve3,//保留3
	Reserve4,//保留3
	Reserve5,//保留3

	GLOBAL_SERVER_COUNTER,
};

// 判断浮点数是否为0
#define FLOAT_0(fval) (fval < 0.0001f && fval > -0.0001f)
#define FLOAT_EQUAL(fval1, fval2) FLOAT_0( (fval1 - fval2) )
#define FLOAT_ABOVE_0(fval) (fval > 0.0001f)
#define FLOAT_BELOW_0(fval) (fval < 0.0001f)
#define FLOAT_EABOVE_0(fval) ((FLOAT_0(fval))||(FLOAT_ABOVE_0(fval)))
#define FLOAT_EBELOW_0(fval) ((FLOAT_0(fval))||(FLOAT_BELOW_0(fval)))


// 对象坐标
struct OBJ_POS
{
public:
    OBJ_POS():x(0.0f),y(0.0f),z(0.0f),orient(0.0f){}
    void Reset()
    {
         x = 0.0f; y=0.0f; z=0.0f; orient = 0.0f;
    }
    float x;
    float y;
    float z;
    float orient;
};

// 对象速度
struct OBJ_SPEED
{
public:
    OBJ_SPEED():sx(0.0f),sy(0.0f),sz(0.0f){}
    bool IsMoving()
    {
        return !( FLOAT_0(sx) && FLOAT_0(sy) &&
			FLOAT_0(sz) );
    }
    void Stop()
    {
        sx = 0.0f; sy=0.0f; sz=0.0f;
    }
    float sx;
    float sy;
    float sz;
};

// 对象加速度
struct OBJ_ASPEED
{
public:
    OBJ_ASPEED():ax(0.0f),ay(0.0f),az(0.0f){}
    bool IsMoving()
    {
        return !( FLOAT_0(ax) && FLOAT_0(ay) &&
			FLOAT_0(az) );
    }
    void Stop()
    {
        ax = 0.0f; ay=0.0f; az=0.0f;
    }
    float ax;
    float ay;
    float az;
};

#define GUID_LEN 40

// 角色存档信息
class RoleSaveInfo
{
public:
	RoleSaveInfo()
	{
		Clean();
	}
	~RoleSaveInfo()
	{

	}

	void Clean()
	{
		szRoleName = "";
		szAccount = "";
		nRoleType = 0;
		createTime = 0;
		saveTime = 0;
		totleSecs = 0;
		roleIndex = 0;
		sceneTypeID = 0;
		pos.Reset();
		szSavePara = "";
		pSaveData = NULL;
		nSaveDataLen = 0;
		guid[0] = 0;
	}

    void SetGuid(const char* _guid)
	{
	    if (_guid && strlen(_guid) < GUID_LEN)
	    {
	        strcpy(guid, _guid);
	    }
	}

	void SetRoleBuffer(char* buffer, size_t nBuffernLen)
	{
		pSaveData = buffer;
		nSaveDataLen = nBuffernLen;

		if (!buffer)
		{
			nSaveDataLen = 0;
		}
	}

	string szRoleName; //name	nvarchar(32)
	string szAccount;   //account	nvarchar(32)
	int nRoleType;      // role_type int
	time_t createTime;  //create_time	datetime
	time_t saveTime;    //存档时间
	int totleSecs;      //total_sec	int
	int roleIndex;      //role_index	int
	int sceneTypeID;    //scene_id	int
	OBJ_POS pos;
	string szSavePara;  //save_para	nvarchar(2048)
	char* pSaveData;    //save_time	datetime
	size_t nSaveDataLen;//save_data	varbinary(MAX)
	char guid[GUID_LEN];         //
};

// UserInfo
struct UserInfo
{
	UI32 m_nPlayerId;				// 玩家id
	char m_szClientIP[32];			// 玩家IP
	UI32 m_nCurrentSceneId;			// 玩家当前所处场景id
	time_t m_loginTime;				// 玩家此次登陆时间
	time_t m_createTime;			// 玩家角色创建时间
	UI32 m_nTotleSeconds;			// 玩家总游戏时间(秒)
	UI32 m_nSecondsAfterLogin;		// 玩家此次登陆后累计时间(秒)
	char m_szUserAccount[64];       // 玩家账号
	UI32 m_nDelayMiniSecs;          // 客户端延迟毫秒
	char m_AccountState[64];        // 账号状态(比如防沉迷状态)
	time_t m_SaveTime;              // 上次存档时间
};

// FindPlayer
struct FindPlayerStruct
{
	UI32 id;
	char roleName[64];
	int level;
};

// 同步方式
enum SYNC_POSI_OPT
{
	SPO_NONE,  // 更新坐标但不同步客户端
	SPO_FORCE, // 强行同步所有移动参数
	SPO_CHANGE,   // 速度参数发生变化时同步
	SPO_CACHEFORCE, //带缓冲的强行同步所有移动参数
};

// 同步原因
enum SYNC_REASON
{
	SR_NONE,  //0. 无
	SR_ENTRY, //1. 进入场景
	SR_LEAVE, //2. 离开场景
	SR_DROP,  //3. 掉落(生产)
	SR_PICKUP,//4. 被拾取(消费)
	SR_DEAD,  //5. 死亡
	SR_RELIVE,//6. 复活
	SR_DELETE, //7. 销毁
	SR_HIDE,    //隐藏
	SR_DISPLAY, //显示(和隐藏相反)
};

// 对象位置信息
class OBJ_POSI_INFO
{
public:
	OBJ_POSI_INFO()
	{
		visible = true;         // 是否可见
		map_info = NULL;
		extend_para = NULL;
	}

	~OBJ_POSI_INFO()
	{
	    SAFE_DELETE_ARRAY(extend_para);
	}

	bool IsMoving()
	{
	    return speed.IsMoving();
	}

	void Stop()
	{
	    speed.Stop();
	    aspeed.Stop();
	}

	void Copy(OBJ_POSI_INFO* another)
	{
		if (!another)
		{
			return;
		}

		pos = another->pos;
		speed = another->speed;
		aspeed = another->aspeed;

		visible = another->visible;         // 是否可见
		map_info = another->map_info;
		SetExtendPara(another->extend_para);
	}

	void CopyExtend(OBJ_POSI_INFO* another)
	{
		if (!another)
		{
			return;
		}

		speed = another->speed;
		aspeed = another->aspeed;

		visible = another->visible;         // 是否可见
		SetExtendPara(another->extend_para);
	}

	void WriteToMsg(CMsgTyped& msg, bool use_z, bool use_a)
	{
	    // use_z 使用Z坐标
	    // use_a, 使用加速度

		msg.SetFloat(pos.x);
		msg.SetFloat(pos.y);
		if (use_z) msg.SetFloat(pos.z);
		msg.SetFloat(pos.orient);

		msg.SetFloat(speed.sx);
		msg.SetFloat(speed.sy);
		if (use_z)  msg.SetFloat(speed.sz);

		if (use_a)
		{
            msg.SetFloat(aspeed.ax);
            msg.SetFloat(aspeed.ay);
            if (use_z) msg.SetFloat(aspeed.az);
		}

		msg.SetInt(visible ? 1 : 0);
		msg.SetString(extend_para);
	}

	const char* GetExtendPara(){return extend_para;}

	// 直接设置
	void SetExtendParaDirect(char* ext_para){extend_para = ext_para;}

	// 设置ext_para
	void SetExtendPara(const char* ext_para)
	{
	    if (!ext_para)
	    {
	        SAFE_DELETE_ARRAY(extend_para);
	        return;
	    }

		// 需要的长度
		size_t need_len = strlen(ext_para) + 1;

		if (!extend_para)
		{
			extend_para = new char[need_len];
		}
		else if	(extend_para && strlen(extend_para) + 1 < need_len)
		{
			// 重新申请
			delete [] extend_para;
			extend_para = new char[need_len];
		}

		::memcpy(extend_para, ext_para, need_len);
	}

public:
    OBJ_POS pos;
    OBJ_SPEED speed;
    OBJ_ASPEED aspeed;
	bool visible;         // 是否可见
	void* map_info;

private:
	char* extend_para;          // 扩展状态
};

// 场景进入方式
enum EntryMod
{
    ES_LANDINGGAME,  // 登录游戏
    EESE,// 进行已经存在的场景
    EESM,// 创建并进入副本,
    ECCO,    // 从主场景创建并副本
    ES_COPY_RESET,     // 副本重置， 进入起始副本
    ES_TRANSPORT,      // 副本传送
};

// 副本进程参数
struct COPY_THREAD_PARAM
{
	UI32 entrySceneType;
	UI32 max_players;
	UI32 current_players;
	int harden;
	time_t beginTime;
};

// 传送门
struct SCENE_GATE
{
    float x;// 传送门坐标x
	float y;// 传送门坐标y
	float z;// 传送门坐标z
	UI32 dest_scene;// 目标场景类型
	float dest_x;// 目标传送点x
	float dest_y;// 目标传送点y
	float dest_z;// 目标传送点z
	UI32 mod;
	UI32 type;// 传送门类型
	UI32 gate_id;// 传送门ID
	bool is_open;// 门是否开启
	UI32 spring_range;// 触发范围
	char desc[256];// 传送门描述
};

// 游戏常用数值定义
// 游戏允许最多100万种对象类型
#define MAX_GAME_TYPE_COUNT 1000000
#define IS_TYPE_ID(id) ( id < MAX_GAME_TYPE_COUNT )

// 服务器端逻辑帧速
#define SERVER_FPS 20

// 服务器每帧耗时(ms)
#define MINISEC_PER_FRAME (int)(1000.0f / SERVER_FPS)

// 玩家id分配范围
#define PLAYER_INIT_ID 1000001
#define PLAYER_ID_COUNT 100000

// 场景id分配范围
#define SCENE_INIT_ID 1100001
#define SCENE_ID_COUNT 100000

// 场景NPC id分配范围
#define SCENE_OBJ_INIT_ID 1200001
#define MAX_SCENE_OBJECTS 10000
#define INIT_CACHE_ID (SCENE_ID_COUNT * MAX_SCENE_OBJECTS + 100000000)

// 判断实体 id 是否场景 id 或角色 id
#define IS_PLAYERID(id)  (id >=  PLAYER_INIT_ID && id < SCENE_INIT_ID)
#define IS_SCENERID(id)  (id >=  SCENE_INIT_ID && id < SCENE_OBJ_INIT_ID)

// 用于INI属性操作
#define CLONEABLE(type) \
	virtual go* Clone(){\
	type* p = new type;\
	(*p) = (*this);\
	return p;\
	}\

#define LOAD_INI(str)\
	CIniFile2 ini(str);\
	ini.LoadFromFile();\

#define GET_SECTIONS(sections)\
	ini.GetSections(sections);\

#define LOAD_PROP_STRING1(section, key, value)\
	value = ini.ReadString(section, key, "");\

// LOAD_PROP_XXX()
#define LOAD_PROP_INT(name)\
	name = (int)ini.ReadInteger("property", #name, 0);\

#define LOAD_PROP_INT2(name,prop)\
	name = (int)ini.ReadInteger("property", #prop, 0);\

#define LOAD_PROP_STRING(name)\
	name = ini.ReadString("property", #name, "");\

#define LOAD_PROP_STRING2(name, prop)\
	name = ini.ReadString("property", #prop, "");\

#define LOAD_PROP_RAND(name)\
	string str##name = ini.ReadString("property", #name, "");\
	name = str##name.c_str();\

#define LOAD_PROP_RAND2(name, prop)\
	string str##name = ini.ReadString("property", #prop, "");\
	name = str##name.c_str();\

#define LOAD_PROP_FLOAT(name)\
	name = ini.ReadFloat("property", #name, 0.0f);\

#define LOAD_PROP_FLOAT2(name,prop)\
	name = ini.ReadFloat("property", #prop, 0.0f);\

// LOAD_SEC_XXX()
#define LOAD_SEC_INT(sec, name)\
	name = ini.ReadInteger(sec, #name, 0);\

#define LOAD_SEC_INT2(sec, name,prop)\
	name = ini.ReadInteger(sec, #prop, 0);\

#define LOAD_SEC_STRING(sec, name)\
	name = ini.ReadString(sec, #name, "");\

#define LOAD_SEC_STRING2(sec, name, prop)\
	name = ini.ReadString(sec, #prop, "");\

#define LOAD_SEC_RAND(sec, name)\
	string str##name = ini.ReadString(sec, #name, "");\
	name = str##name.c_str();\

#define LOAD_SEC_RAND2(sec, name, prop)\
	string str##name = ini.ReadString(sec, #prop, "");\
	name = str##name.c_str();\

#define LOAD_SEC_FLOAT(sec, name)\
	name = ini.ReadFloat(sec, #name, 0.0f);\

#define LOAD_SEC_FLOAT2(sec, name, prop)\
	name = ini.ReadFloat(sec, #prop, 0.0f);\

#define COPY_PROP(name) name = another.name;\


// 获取指定场景第一个场景对象的id,用来作为初始化id
static UI32 GetSceneObjInitID(UI32 nSceneId)
{
	if (nSceneId < SCENE_INIT_ID)
	{
		return 0;
	}

	UI32 nSectorIndex = nSceneId - SCENE_INIT_ID; // 0 .... n

	return SCENE_OBJ_INIT_ID + nSectorIndex * MAX_SCENE_OBJECTS;
}

// 根据场景对象id 判定其所在场景
static UI32 GetSceneIDFromOBjId(UI32 objId)
{
	if (objId < SCENE_OBJ_INIT_ID)
	{
		return 0;
	}

	UI32 nSceneIndex = (objId - SCENE_OBJ_INIT_ID) / MAX_SCENE_OBJECTS;
	if (nSceneIndex >= SCENE_ID_COUNT)
	{
		return 0;
	}

	return SCENE_INIT_ID + nSceneIndex;
}

// 场景对象遍历函数 // 返回false 结束遍历
typedef
bool (*LPTRAVERSESCENEFUNC)(IGame* pScene, UI32 id, UI32 type, UI32 objType, OBJ_POSI_INFO* info, void* param);

// 属性更改回调
typedef
void (*LPPROPCHANGED)(IGame* pScene, UI32 self, const char* name, void* oldvalPtr, void* newvalPtr);

// 模块间注册
typedef void(*LPMODULEFUN)(void* param);

// 查询角色状态回调函数
typedef void(*LPQUERYROLEFUN)(IGame* pScene, UI32 self, int count, CMsgTyped* plist, const char* para);

// 创建对象回调
typedef void(*LPCREATEPUBOBJFUN)(IGame* pScene, UI32 self, UI32 targetId, UI32 targetType, const char* guid, const char* para);

// 遍历模板库回调
typedef void(*LPTRAVERSETYPEDBFUN)(IGame* pScene, UI32 typeId, void* param);

// 创建场景回调
typedef void(*LPCREATESCENE)(IGame* pScene, UI32 creator, UI32 mod, UI32 sceneId, UI32 threadId, const char* params);
