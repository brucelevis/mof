//
//  DataBase.h
//  GameSrv
//
//  Created by 麦_Mike on 12-12-31.
//
//

#ifndef __GameSrv__DataBase__
#define __GameSrv__DataBase__
#include "Defines.h"
#include <map>
#include <vector>
#include <algorithm>
#include "log.h"
#include "Utils.h"
#include "DataCfgActivity.h"
#include "ItemHelper.h"
#if defined(CLIENT)
#include "inifile.h"
#include "ThreadMessage.h"
#endif
using namespace std;
class LivingObject;

/**
 可配置的------------------------------------------------------
 */
class Cfgable {
public:
    virtual ~ Cfgable(){}
#if defined(CLIENT)
    static void loadAll(ThreadDelegateProtocol* delegate, string msg, int startPercent, int endPercent);
#else
    static void loadAll();
#endif


    static string pathPrefix;

    static string getFullFilePath(string fileName);
};
/*
 ---------------------------------------------------------------
 */
//struct GameCfgDef {
////    std::string host;   // 服务器ip
////    int port;           // 连接端口
////    std::string user;   // 用户名
////    std::string passwd; // 密码
////    int showFPS;       // ＦＰＳ
////    int testScene;     // 测试场景
////    int showDebug;   // 调试输出
////    int mikeTest;    // test mike
//};
class GameCfg : public Cfgable {
public:
    GameCfg(){}
    virtual ~GameCfg(){}

    static void load(std::string fullPath);

    static bool isSkipLoginUI;//使用测试服和测试账号，跳过登录界面，
    static std::string testServerHost;   // 测试服，服务器ip
    static int testServerPort;           // 测试服，连接端口
    static std::string testAccount;   // 用户名
    static std::string testPassword; // 密码
    static int showFPS;       // ＦＰＳ
    static int testScene;     // 测试场景
    static int showDebug;   // 调试输出
    static ClientMode clientMode;    // 客户端模式
    static int silentmode;
    static bool isOpenAutoUpdater;
    static bool isUpdateApkInGame;
    static bool isEnableMemoryCrypt;    //是否开启内存加密
    static bool vipmod;
    static bool isOpenNetworkTimeout;    //前端是否开启检测掉线

    static std::string clientPlatform;//安卓还是ios
    static std::string clientVersion;   //客户端版本
    static std::string clientChannel;   //客户端渠道
    static std::string clientChildrenChannel;//客户端子渠道号，安卓版特有
    static std::string checkServerURL;  //检测客户端版本的服务器地址
    static std::string bakCheckServerURL;//检测客户端版本的备用服务器地址---linshusen
    static std::string serverMgrURL;    //获取所有服务器的地址
    static std::string curAutoUpdateVer;  //当前游戏安装包里的自动更新版本号
    //static std::string platformNoticeURL; //获取平台公告地址

    static std::string xinyouServerURL;
    static std::string xinyouKey;
    
    static std::string xinyouCollectURL;//新游收集数据url
    static int xinyouGameID;//新游收集数据ID
    
    //进入游戏过程跟踪的url，用于统计玩家在哪一步出问题导致无法登录游戏---linshusen
    static std::string enterGameCollectURL;
    ///是否开启包围盒
    static bool isOpenAtkBox;
};

//--------客户端需要在自动更新前加载的各个配置项----------------------
class ClientPreloadCfg : public Cfgable
{
public:
    ClientPreloadCfg(){}
    virtual ~ClientPreloadCfg(){}
    
    static void load(std::string fullPath);
    
    //是否严格检测网络。
    //严格：获取自动更新结果有各种异常，不能进入游戏。不严格：能进入游戏，但由于自动更新失败，资源错误可能会导致闪退。
    static bool isStrictCheckNetwork;
    static int loadingSize;//下载大小高于多少b就弹提示
};

#pragma mark- 客户端预加载资源表
class PreloadResCfg : public Cfgable
{
public:
    PreloadResCfg(){}
    virtual ~PreloadResCfg(){};
    
    static void load(std::string fullPath);
    
    static PreloadResCfg* getCfg(int resId);
    
    int mID;                    //资源ID
    std::string mName;          //资源名
    ResourceType mType;         //资源类型
    bool mIsNeedPreload;        //是否需要提前加载
    bool mIsNeedAsynload;       //是否需要异步加载
    int mOrient;                //原始朝向 0：向左，1：向右
    std::string mResName;       //美术资源名
    std::string mPath;          // resource path
    std::string mPlist;
    std::vector<std::string> mAnimationNames;    //动作名
    float mAnchorOffsetX;       //锚点的偏移量
    float mAnchorOffsetY;       //锚点的偏移量
    ResScaleType mScaleType;    //资源的缩放类型
    float mScaleX;              //缩放值
    float mScaleY;              //缩放值
    float mBoundingBoxX;        // 资源的包围框
    float mBoundingBoxY;        // 资源的包围框
    int mRealHeight;            //资源的帧数高度
    int mAttackLightID;         // 角色的攻击光效资源ID
    
    std::string getAnimationName();

    //新游动画logo
    static int VxinYouLogoResoures;
    //自动更新 光效 id
    static int LoadingEffectResId;
};

class Md5VerifyCfg : public Cfgable
{
public:
    Md5VerifyCfg(){}
    virtual ~Md5VerifyCfg(){}
    
    static void load(std::string fullPath);
    static std::string getCfgFileMd5(std::string fileName);
private:
    static std::map<std::string,std::string> allFileAndMd5s;
};

//--------美术资源表，包括角色，怪物，npc，光效等-----------------------
class ResCfg : public Cfgable
{
public:
    ResCfg(){}
    virtual ~ResCfg(){}

    static void load(std::string fullPath);
    static std::map<int, ResCfg*> sAllRes;
    
    static ResCfg* getCfg(int resId);

    int mID;                    //资源ID
    std::string mName;          //资源名
    ResourceType mType;         //资源类型
    bool mIsNeedPreload;        //是否需要提前加载
    bool mIsNeedAsynload;       //是否需要异步加载
    int mOrient;                //原始朝向 0：向左，1：向右
    std::string mResName;       //美术资源名
    std::string mPath;          // resource path
    std::string mPlist;
    std::vector<std::string> mAnimationNames;    //动作名
    float mAnchorOffsetX;      //锚点的偏移量(为了兼容美术制作时原点的不统一)。如果是人物，这个值就是人物影子中心点与（0，0）的差值
    float mAnchorOffsetY;      //锚点的偏移量(为了兼容美术制作时原点的不统一)。如果是人物，这个值就是人物影子中心点与（0，0）的差值
    ResScaleType mScaleType;    //资源的缩放类型
    float mScaleX;             //缩放值
    float mScaleY;             //缩放值
    float mBoundingBoxX;       // 资源的包围框
    float mBoundingBoxY;       // 资源的包围框
    int mRealHeight;            //资源的帧数高度
    int mAttackLightID;         // 角色的攻击光效资源ID

    std::string getAnimationName();

};


//--------音乐音效-----------------------
class AudioCfg : public Cfgable
{
public:
    AudioCfg(){}
    virtual ~ AudioCfg(){}

    static void load(std::string fullPath);
    static std::map<int, AudioCfg*> sAllAudios;

    int mID;
    std::string mName;      //名字
    std::string mDes;       //描述
    std::string mFileName;  //声音资源文件名
    AudioType mType;        //声音类型（背景乐，音效）
    bool mIsLoop;           //是否循环播放
    float mVolume;          //音量 0--1

};

//--------状态冲突表-----------------------
class StateMutexCfg : public Cfgable
{
public:
    StateMutexCfg(){}
    virtual ~StateMutexCfg(){}

    static void load(std::string fullPath);
    static ObjState stringToObjState(std::string str);

    static std::map<ObjState, std::map<ObjState, bool> > sObjStateMutexTable;

    static void setStateMutexTable(ObjState fromState, ObjState toState, bool canChange);

    std::string state; //状态

    bool none; //空状态
    bool idle; //站立
    bool walk; //行走
    bool attack; //攻击
    bool hitted; //被攻击
    bool cast; //吟唱
    bool skill; //放技能
    bool dead; //死亡
};

//--------角色模板-----------------------
class RoleTpltCfg : public Cfgable
{
public:
    RoleTpltCfg(){}
    virtual ~ RoleTpltCfg(){}

    static void load(std::string fullPath);
    static RoleTpltCfg* getCfg(int roleType);
    static std::map<int, RoleTpltCfg*> sAllRoleTplts;

    static ObjSex stringToObjSex(std::string str);
    static ObjJob stringToObjJob(std::string str);
    static string objJobToString(ObjJob job);


    int mID;                //
    std::string mName;      //名字
    std::string              mHeadImage;  //人物头像
    std::string              mBagheadImage; //半身像
    std::string               mJobName;
    ObjSex mSex;            //性别
    ObjJob mJob;            //职业
    int mResID;             //资源ID
    std::vector<float> mAttackTimes;    //每个攻击动作发起攻击的时间点
    std::vector<float> mAnimationTimes;

    float mLastAttackAddRate;           //最后一击增加的攻击百分比
    std::vector<int> mAttackAudios;     //攻击音效
    std::vector<int> mHittedAudios;     //受击
    std::vector<int> mAttackScreams;    //攻击呐喊音效
    std::vector<int> mHittedScreams;    //受伤呐喊音效
    int mWalkAudio;
    int mDeadAudio;                     //死亡音效
    int mKOSreamAudio;                  //被ko的死亡长啸
#ifdef CLIENT
    ReadWrite_Crypt(int, mMaxAnger, MaxAnger) //最大怒气值
#else
    READWRITE(int, mMaxAnger, MaxAnger)
#endif
    float mAttackMoveTime;                //普通攻击前进的时间
    int mAttackMoveDistance;            //普通攻击前进的距离
    float mLastAttackHitBackTime;         //普通攻击最后一击击退敌人的时间
    int mLastAttackHitBackDistance;     //普通攻击最后一击击退敌人的距离
    vector<int> mSkillInitialvalue;     //角色的技能初始列表

};

//--------零碎的资源-----------------------
class SeparateResIDs : public Cfgable
{
public:
    SeparateResIDs(){}
    virtual ~ SeparateResIDs(){}
    static void load(std::string fullPath);

    static int sBattleNumber;
    static int sBruiseEffect;
    static int sBattleKO;
    static int sMonsterDead;
    static int sRoleDead;
    static int sRoleLevelUp;
    static int sGetQuest;
    static int sSubmitQuest;
    static int sOpenBox;
    static int sOpenGift;
    static int sOpenStrengthen;
    static int sOpenMention;
    static int sOpenMosaic;
    static int sUseItem;
    static int sActivity;
    static int sMainUIcircle;
    static int sGodHit;
    static int sTitleGet;
	static int sBattleEnd;
    //VIP一级幻宠
    static int sFirstChargePetAnimation;
    static string sFirstcharge_skillname;
    static vector<int> sQualityEffect;
    static int sStarUp;   //升星
    static int sSynthetic; //合成
    static int sSettlement; //结算
    static int sQualityBleat;
    static int sPetCollectTip;
    static int sPetCollectAward ;
    static int sEscapeEffect;
    static int sWritingNetWork;
    static int sHelperGoing;
    static vector<int> sWaveCounts;
    // 首充奖励幻兽动画
    static string sFirstChargeAnims;
    static string sFirstChargePetSkills;
    // 新功能开启
    static int sNewFunction;
    static int sHelperNotice;
    static int  sResetGrowvalue;
    //升星和技能可升级 播放的特效
    static int  sStarAndSkillUpAnimation;
    // 骰子
    static int sDiceRollAnim;
    static int sDiceAwardAnim;
    static int sEquip_starlvlup;
    //女王祝福
    static int sQueenBlessingBefore;
    static int sQueenBlessingBack;
    //幻兽升阶升阶成功特效
    static int sPet_stageup;
    //幻兽每次合体光效
    static int sPet_stageup_everytime;
    //幻兽每次吞噬光效
    static int sPet_eating_everytime;
    static int sCapsuleToy;
    //任务光效
    static int sQuestAcceptable;
    static int sQuestFinished;
    static int sQuestDoing;
    //娱乐活动光效
    static int sFunnyActEffectID;
    static int sEnableEvolveID;
    //自动学习4主动技能 指引光效
    static int sAutoLearnSkillEffectID;
    // 附魔升级光效
    static int sEnchantLvlUp;
    //每日日程宝箱光效
    static int sDailySchedule;
    static int sFunnytoy_activityBtn;
    
    //语音请求时的菊花效果
    static int sVoiceReqLoading;
    
    //语音请求时的自己的播放效果
    static int sVoicePlayMyselfAction;
    
    //语音请求时的其他人的播放效果
    static int sVoicePlayOtherAction;
};

//--------零碎的数值-----------------------
class SeparateValues : public Cfgable
{
public:
    SeparateValues(){}
    virtual ~ SeparateValues(){}
    static void load(std::string fullPath);

    static int sRoleMaxCombo;   //角色最多连招数
    static float sCombeTime;    //连击的检测时间，两次点击小于这个时间就开始连击
    static float sDefaultAttackMoveDistance;//普通攻击的默认移动距离
    static float sDefaultAttackMoveTime;    //普通攻击的默认移动时间
    static float sDeadMoveDistance;         //通关全部死亡时的移动距离
    static float sDeadMoveTime;             //通关全部死亡时的移动时间
    static float sKOSlowMotionTime; //通关KO时，慢动作的持续时间
    static float sKOSlowMotionScale;//通关KO时，慢动作的放慢比率
    static int sWorldbossWaitTime;//世界boss等待时间
    static string sOfficalBBSUrl;//官方论坛地址
	static int sWorldChatCd;//世界聊天冷却时间
    static int sIncrHpFrequency;  //回血频率
    static int sPaihangFreshPeriod; //排行榜刷新周期
    static std::string platformNoticeURL; //获取平台公告地址
    static std::string jumpWebsiteURL;  
    static vector<int> lvlUpCollect;
    static int isShowAd;
    static std::string chargeURL;
    static float sJoystickResetTime;
    static float sJoystickLeftBorder;  //当手移动到屏幕左边的某个边界  认为划出屏幕了
    static float sJoystickBelowtBorder;  //当手移动到屏幕下边的某个边界  认为划出屏幕了
    static float sTipsIntervalTime;
    static int sPetEliteLimitLv[2];     // 幻兽试炼场开放幻兽选择等级
    static int sEachFrameCreateObj;     //每帧创建多少个对象
    static float sLoadingShortestTime;   //加载界面最短时间
    static bool  sIsShowTouristsLoginImage;  //是否显示游客登录界面
    static vector<int> mNoLoagingScenes;
    static vector< eSceneType> mOpenChatInCopy;
    static vector<string> sInvalidCurrencyCode;
    static int mStoreSkillActionCount;  //技能效果大于这个值的时候 开始删除没用的技能效果
};

/*
 ---------------------------------------------------------------
 */
//#define MAX_SCENE_MONSTER 100
//struct SceneMonster{
//    float _id;
//    float x;
//    float y;
//};
//struct SceneCfgDef {
//    SceneMonster monsters[MAX_SCENE_MONSTER];
//    int monNum;
//
//};
//class SceneCfg : public Cfgable {
//public:
//    virtual ~ SceneCfg(){}
//
//    static void load(std::string fullPath);
//    static SceneCfgDef& getCfg(int _id,bool& exist){
//        if( cfg.find(_id) == cfg.end() ){
//            printf("SceneCfg getCfg error id: %d", _id);
//            exist=false;
//            return cfg[0];
//        }else{
//            exist = true;
//            return cfg[_id];
//        }
//    }
//
//private:
//    static map<int,SceneCfgDef> cfg;
//};
/*
 ---------------------------------------------------------------
 */
struct RoleCfgDef{
    
    RoleCfgDef();
#ifdef CLIENT
    ReadWrite_Crypt(int, mWalkSpeed, WalkSpeed)
    ReadWrite_Crypt(int, mAtkSpeed, AtkSpeed)
   
    ReadWrite_Crypt(int, mExp, Exp)
    
    ReadWrite_Crypt(int, mStre, Stre) //力量
    ReadWrite_Crypt(int, mInte, Inte) //智力
    ReadWrite_Crypt(int, mPhys, Phys) //体力
    ReadWrite_Crypt(int, mCapa, Capa) //耐力
    
    ReadWrite_Crypt(int, mHp, Hp)
    ReadWrite_Crypt(int, mMp, Mp)
    ReadWrite_Crypt(int, mAtk, Atk)
    ReadWrite_Crypt(int, mDef, Def)
    
    ReadWrite_Crypt(float, mHit, Hit)
    ReadWrite_Crypt(float, mDodge, Dodge)
    ReadWrite_Crypt(float, mCri, Cri)
    ReadWrite_Crypt(float, mCrip, Crip)
    ReadWrite_Crypt(int, mMaxfriendly, Maxfriendly);// 亲密度上限
    
    ReadWrite_Crypt(int, mEnerge, Energe)
    ReadWrite_Crypt(float, mHpper, Hpper)
    ReadWrite_Crypt(int, mHpnum, Hpnum)
    ReadWrite_Crypt(int, mMpnum, Mpnum)
    ReadWrite_Crypt(int, mDungfreeTimes, DungfreeTimes)
    
    ReadWrite_Crypt(int, mElitefreeTimes, ElitefreeTimes)
    ReadWrite_Crypt(int, mTowertimes, Towertimes)
    ReadWrite_Crypt(int, mFriendDungeTimes, FriendDungeTimes)
    ReadWrite_Crypt(int, mPetElitefreeTimes, PetElitefreeTimes)
    ReadWrite_Crypt(int, mPetAdventurefreeTimes, PetAdventurefreeTimes)
    ReadWrite_Crypt(int, mPetAdventureRobotPetfreeRentedTimes, PetAdventureRobotPetfreeRentedTimes)
    
    ReadWrite_Crypt(int, mCrossServiceWarWorShipFreeTimes, CrossServiceWarWorShipFreeTimes); //跨服战膜拜免费次数
    ReadWrite_Crypt(int, mCrossServiceWarFreeTimes, CrossServiceWarFreeTimes); //跨服战膜拜免费次数

#else
    READWRITE(int, mWalkSpeed, WalkSpeed)
    READWRITE(int, mAtkSpeed, AtkSpeed)
    
    
    READWRITE(int, mExp, Exp)
    
    READWRITE(int, mStre, Stre) //力量
    READWRITE(int, mInte, Inte) //智力
    READWRITE(int, mPhys, Phys) //体力
    READWRITE(int, mCapa, Capa) //耐力
    
    
    READWRITE(int, mHp, Hp)
    READWRITE(int, mMp, Mp)
    READWRITE(int, mAtk, Atk)
    READWRITE(int, mDef, Def)
    
    
    READWRITE(float, mHit, Hit)
    READWRITE(float, mDodge, Dodge)
    READWRITE(float, mCri, Cri)
    READWRITE(float, mCrip, Crip)
    
    READWRITE(int, mMaxfriendly, Maxfriendly);// 亲密度上限
    
    

    READWRITE(int, mEnerge, Energe)
    READWRITE(float, mHpper, Hpper)
    READWRITE(int, mHpnum, Hpnum)
    READWRITE(int, mMpum, Mpnum)
    READWRITE(int, mDungfreeTimes, DungfreeTimes)
    
    READWRITE(int, mElitefreeTimes, ElitefreeTimes)
    READWRITE(int, mPetAdventurefreeTimes, PetAdventurefreeTimes)
    READWRITE(int, mPetAdventureRobotPetfreeRentedTimes, PetAdventureRobotPetfreeRentedTimes)
    
    READWIRTE(int, mCrossServiceWarWorShipFreeTimes, CrossServiceWarWorShipFreeTimes); // 跨服战膜拜免费次数
    READWIRTE(int, mCrossServiceWarFreeTimes, CrossServiceWarFreeTimes); // 跨服战免费次数
    

    READWRITE(int, mTowertimes, Towertimes)
    READWRITE(int, mFriendDungeTimes, FriendDungeTimes)
    READWRITE(int, mPetElitefreeTimes, PetElitefreeTimes)
    
#endif

};
class RoleCfg : public Cfgable {
public:
    RoleCfg(){}
    virtual ~ RoleCfg(){}

    static void load(std::string fullPath);
    static RoleCfgDef& getCfg(int job,int lvl);
    
    static int getMaxRoleLvl();
    static int getCrossServiceWarIsWorShipTimes();
    
private:
    
    static int maxRoleLvl;
    static int crossServiceWarIsWorShipTimes;
    static RoleCfgDef cfg[MAX_ROLE_JOB][MAX_ROLE_LVL + 1];
};

struct AnimationCfgDef
{
    string mName;
    float mDuration;
};

struct ResAnimationCfgDef
{
    vector<AnimationCfgDef> mAnimations;
};

class ResAnimationCfg
{
public:
    static void load(const char* path);
    
    static void addResAnimation(const char* resName);
    
    static float getAnimationDuration(const char* resName, const char* aniName);
    
    static const int fps;
    static map<string, ResAnimationCfgDef> cfg;
};

/*
 ---------------------------------------------------------------
 */

struct SceneInfoXML
{
    std::vector<int> mMonsters;
};


struct SceneNPCDef
{
    int x;
    int y;
    int npcId;
    SceneNPCDef()
    {
        x = y = 0;
        npcId = 0;
    }
};


#ifndef CLIENT

struct Point
{
    int x;
    int y;
};

struct Rect
{
    int x;
    int y;
    int width;
    int height;
};

inline bool isPointInRect(const Point& pt, const Rect& rect)
{
    return ((pt.x > rect.x && pt.x < rect.x + rect.width) &&
            (pt.y > rect.y && pt.y < rect.y + rect.height));
}

struct SceneAreaDef
{
    Rect mRect;
    vector<Point> mLandPoints;
    vector<SceneNPCDef> mMonsters;
};
#endif

struct SceneCfgDef {
    
    std::string name; //名字
    int preCopy;
    int sceneIndex; //场景序列
    int bornX,bornY; //出生点x;y
    
    bool hasPortal; //是否通过传说门
    std::string townInfo; //对应主城*x;y
    int is_boss; //是否boss本
    int pet_exp;
    int copy_gold; //金币奖励
    int copy_exp; //经验奖励
    int copy_battlepoint; //战功奖励
    int copy_protoss; //星灵奖励
    int copy_petExp; //宠物经验
    int energy_consumption; //进入消耗精力
    int hang_time; //挂机消耗时间
    int is_attack; //是否允许攻击
    int min_vip; //最低VIP扫荡等级
    int fightpoint; //推荐战斗力
    int townid;
    int wipeOutLvl;
    int fightTime; //推荐战斗力下的标准战斗时间，单位秒
    
    string  monsterHead;    //在副本时怪物头像
    vector<int> copy_passtime; //通关时间
    vector<int> copy_maxhits;   //最高连击
    vector<int> copy_hitby;     //受击次数
    string fall_items;           //精英副本可能掉落的物品
    int incr_friendly;  // 好友亲密度增加值
	int incr_prestige; // 好友声望增加值
    std::string friendaward;

    vector<int> musicIDs; //场景可能有多个背景乐，随机播放一个
    int getMusicID();
    vector<int> resetCost;
    
    PetAdventureType petAdventureType; //幻兽大冒险类型
    
public:
    int getResetCost(int times)
    {
        if (resetCost.size() == 0) {
            return 0x7FFFFFFF;
        }
        if ( times < 0 || times >= resetCost.size()) {
            return resetCost[resetCost.size() - 1];
        }

        return resetCost[times];
    }
#if defined(CLIENT)
    //关键几个变量内存加密
    ReadWrite_Crypt(int, sceneId, SceneID)
    ReadWrite_Crypt(int, sceneType, SceneType)
    ReadWrite_Crypt(int, minLv, MinLevel)
    ReadWrite_Crypt(int, maxLv, MaxLevel)
#else
    int sceneId; //场景id
    int sceneType; //场景类型
    int minLv; //进入等级
    int maxLv; //进入的最大等级
    
    vector<Point> mLandPoses;
    vector<int> mMonsters;
    vector<SceneAreaDef> mAreas;
#endif
};

class SceneCfg:public Cfgable {
public:
    SceneCfg(){}
    virtual ~SceneCfg(){}

    static void load(std::string fullPath);
    static SceneCfgDef* getCfg(int _id);
    static int findSceneForIndex(int _index);
    static void findSceneForType(int _sceneType,std::vector<int> &_sceneIDs);
    static int checkCopyOpen(int _copyID,int _lastCopyID);
    static std::string SceneToString(int _sceneID);
#if defined(CLIENT)
#else
    static int getMonsterCount(SceneCfgDef* scene);
    static bool isMonsterExist(SceneCfgDef* scene, int monsterid);
#endif

private:
    static map<int,SceneCfgDef* > cfg;
};

/*
 ---------------------------------------------------------------
 */
struct PortalCfgDef {
    int portalID; //传送门id
    int res_id; //资源id
    int isToCopy; //是否去副本
    int isShow; //是否显示
    int sceneID; //所在地图
    std::vector<int> targetID; //目标地图
    int targetX; //目标坐标x
    int targetY; //目标坐标y
};

class PortalCfg:public Cfgable {
public:
    PortalCfg(){}
    virtual ~PortalCfg();

    static void load(std::string fullPath);
    static PortalCfgDef* getCfg(int _id);
    static int findPortalForSceneID(int _sceneID,int _targetID);
    static void findPortalsForSceneID(int _sceneID, vector<int>* _outPortals);
private:
    static map<int,PortalCfgDef*> cfg;
};
/*
 --------------------------------------------------------------
 */
struct NewbieCfgDef {
    int id; //jobid
    int newbieSceneID; //场景
    int level; //等级
    int dialogID; //对话id
    int hp; //血量
    int atk; //攻击
    int def; //防御
    float hit;  //命中
    float dodge;//闪避
    float cri;//暴击
    float hpper;//回血
    float hpper_cd;//回血间隔
    std::vector<int> skills;
//    int skill1; //技能1
//    int skill2; //技能2
//    int skill3; //技能3
//    int skill4; //技能4
};

class NewbieCfg:public Cfgable {
public:
    NewbieCfg(){}
    virtual ~NewbieCfg(){}

    static void load(std::string fullPath);
    static NewbieCfgDef* getCfg(int _id,bool &exist){
        if( cfg.find(_id) == cfg.end() ){
            exist=false;
            return NULL;
        }else{
            exist = true;
            return cfg[_id];
        }
    }
    static NewbieCfgDef* getNewbiePetCfg();
private:
    static map<int,NewbieCfgDef*> cfg;
};

/*
 ---------------------------------------------------------------
 */
struct NpcCfgDef {
    int id; //npcid
    int res_id; //资源ID
    std::string name; //名称
    int scene; //所在场景
    std::string conversation; //默认对话
    std::string npc_half;
    int npcType;   //  npc 类型
    int npcShowLvl;//  npc 显示等级（主角达到多少级npc才能显示）
    
};

class NpcCfg:public Cfgable {
public:
    NpcCfg(){}
    virtual ~NpcCfg(){}

    static void load(std::string fullPath);
    static NpcCfgDef* getCfg(int _id,bool &exist){
        if( cfg.find(_id) == cfg.end() ){
            exist=false;
            return NULL;
        }else{
            exist = true;
            return cfg[_id];
        }
    }
private:
    static map<int,NpcCfgDef*> cfg;
};


/*
 ---------------------------------------------------------------
 */
struct MonsterSkills
{
    MonsterSkills()
    {
        setSkillid(0);
        setRol(0);
    }
    
#ifdef CLIENT
    ReadWrite_Crypt(int, mSkillid, Skillid);
    ReadWrite_Crypt(int, mRol, Rol);
#else
    READWRITE(int, mSkillid, Skillid);
    READWRITE(int, mRol, Rol);
#endif
};
struct MonsterCfgDef{
    string name;
    //    //怪物模板相关
    int res_id;
    bool isArmor;
    
    MonsterCfgDef();
#ifdef CLIENT
    ReadWrite_Crypt(int, mLvl, Lvl)
    ReadWrite_Crypt(int, mWalkSpeed, WalkSpeed)
    ReadWrite_Crypt(int, mHp, Hp)
    ReadWrite_Crypt(float, mHit, Hit)
    ReadWrite_Crypt(int, mAtk, Atk)
    
    ReadWrite_Crypt(int, mDef, Def)
    ReadWrite_Crypt(int, mDodge, Dodge)

    ReadWrite_Crypt(int, mPerAnger, PerAnger)  //角色杀死这个怪物之后获取的怒气值
    ReadWrite_Crypt(float, mAtkProbality, AtkProbality)
    ReadWrite_Crypt(float, mAtkPeriod, AtkPeriod)
    
    ReadWrite_Crypt(int, mAtkSpeed, AtkSpeed)
    ReadWrite_Crypt(float, mAtkRangeX, AtkRangeX)
    ReadWrite_Crypt(float, mAtkRangeY, AtkRangeY)
    ReadWrite_Crypt(float, mAtkRectOrigX, AtkRectOrigX)
    ReadWrite_Crypt(float, mAtkRectOrigY, AtkRectOrigY)
    
    
    ReadWrite_Crypt(float, mBeAtkRangeX, BeAtkRangeX)
    ReadWrite_Crypt(float, mBeAtkRangeY, BeAtkRangeY)
    
    ReadWrite_Crypt(float, mSkillRangeX, SkillRangeX)
    ReadWrite_Crypt(float, mSkillRangeY, SkillRangeY)
    
     ReadWrite_Crypt(int, mFollowProbality, FollowProbality)
    
    
    
    ReadWrite_Crypt(float, mPatrolLength, PatrolLength)
    ReadWrite_Crypt(float, mVision, Vision)
    
    ReadWrite_Crypt(float, mChangingRoundX, ChangingRoundX)
    ReadWrite_Crypt(float, mChangingRoundY, ChangingRoundY)
    ReadWrite_Crypt(float, mActionperiod, Actionperiod)
    ReadWrite_Crypt(int, mHangAround, HangAround)
    
    ReadWrite_Crypt(float, mDeath_duration, Death_duration)
    
    ReadWrite_Crypt(int, mGrowthType, GrowthType)  //这个值 影响幻兽的成长值
    
#else
    READWRITE(int, mLvl, Lvl)
    READWRITE(int, mWalkSpeed, WalkSpeed)
    READWRITE(int, mHp, Hp)
    READWRITE(float, mHit, Hit)
    READWRITE(int, mAtk, Atk)
    
    READWRITE(int, mDef, Def)
    READWRITE(int, mDodge, Dodge)
    
    READWRITE(int, mPerAnger, PerAnger)  //角色杀死这个怪物之后获取的怒气值
    READWRITE(float, mAtkProbality, AtkProbality)
    READWRITE(float, mAtkPeriod, AtkPeriod)
    
    READWRITE(int, mAtkSpeed, AtkSpeed)
    READWRITE(float, mAtkRangeX, AtkRangeX)
    READWRITE(float, mAtkRangeY, AtkRangeY)
    READWRITE(float, mAtkRectOrigX, AtkRectOrigX)
    READWRITE(float, mAtkRectOrigY, AtkRectOrigY)
    
    
    READWRITE(float, mBeAtkRangeX, BeAtkRangeX)
    READWRITE(float, mBeAtkRangeY, BeAtkRangeY)
    
    READWRITE(float, mSkillRangeX, SkillRangeX)
    READWRITE(float, mSkillRangeY, SkillRangeY)
    
    READWRITE(int, mFollowProbality, FollowProbality)
    
    
    
    READWRITE(float, mPatrolLength, PatrolLength)
    READWRITE(float, mVision, Vision)
    
    READWRITE(float, mChangingRoundX, ChangingRoundX)
    READWRITE(float, mChangingRoundY, ChangingRoundY)
    
    READWRITE(float, mActionperiod, Actionperiod)
    READWRITE(int, mHangAround, HangAround)

    READWRITE(float, mDeath_duration, Death_duration)
    
    READWRITE(int, mGrowthType, GrowthType)  //这个值 影响幻兽的成长值
    
#endif

    
    std::vector<float> attackDelayTimes;  //怪物发起普通攻击的延迟时间
    vector<MonsterSkills*> skills;
    int castSkillRol;
    MonsterType monsterType;
    string monster_img;
    string monsterHead_img;
    int monster_degree;   //用于区分 普通、精英、Boss, 稀有

    //怪物音效资源
    vector<int> mAttackAudios;  //攻击音效
    vector<int> mHittedAudios;
    vector<int> mAttackScreams; //攻击呐喊
    vector<int> mHittedScreams; //受击呐喊
    int mDeadAudio;             //死亡声音
    int mKOScreamAudio;         //被ko的死亡长啸
 
};
class MonsterCfg : public Cfgable {
public:
    MonsterCfg(){}
    virtual ~ MonsterCfg(){}
    static MonsterType stringToMonsterType( std::string str );
    static void load(std::string propFilePath, std::string tpltFilePath);
    static MonsterCfgDef* getCfg(int _id);
    static vector<string> getAllMonsterHeadIcon();  //将所有类型的幻兽的头像存起来

private:
    static map<int,MonsterCfgDef*> cfg;
};
/*
 ---------------------------------------------------------------
 */
#define MIN_ID 400000
#define MAX_ID 460000

struct SkillEffectVal
{
    SkillEffectVal()
    {
        mEffectName = mCharacterAnimation = "";
//        isForSelf = true;
        mTargetType = eForSelf;
        
        mIsFullScreen = mIsBehindCharacter = mCharacterAnimationLoop =false;
        mResPosType = eOnFoot;
        
        setAffectValue(0);
        setAffectFloat(0);
        setCastTime(0);
        setDelayTime(0);
        setLastTime(0);

        setSkillEffectId(0);
        setEffectResId(0);
        setAppearOffset(0);
        setSpeed(0);
        setMoveLength(0);
        setAttackCount(0);
        setAttackIntervalTime(0);
        setHitBackDistance(0);
        setHitBackTime(0);
        setAudioID(0);
        setTargetNum(0);
    }

#ifdef CLIENT
    ReadWrite_Crypt(int, affectValue, AffectValue)
    ReadWrite_Crypt(float, affectfloat, AffectFloat)
    ReadWrite_Crypt(float, castTime, CastTime) //施法时间
    ReadWrite_Crypt(float, delayTime, DelayTime)
    ReadWrite_Crypt(float, lastTime, LastTime)

    ReadWrite_Crypt(int, mSkillEffectId, SkillEffectId)
    ReadWrite_Crypt(int, mEffectResId, EffectResId)
    ReadWrite_Crypt(float, mAppearOffset, AppearOffset)
    ReadWrite_Crypt(float, mSpeed, Speed)
    ReadWrite_Crypt(float, mMoveLength, MoveLength)
    ReadWrite_Crypt(int, mTargetNum, TargetNum)  //攻击目标的个数，只用于锁定目标类的技能效果
    ReadWrite_Crypt(int, mAttackCount, AttackCount)
    ReadWrite_Crypt(float, mAttackIntervalTime, AttackIntervalTime)
    ReadWrite_Crypt(int, mHitBackDistance, HitBackDistance)
    ReadWrite_Crypt(float, mHitBackTime, HitBackTime)
    ReadWrite_Crypt(int, mAudioID, AudioID)
#else
    READWRITE(int, affectValue, AffectValue)
    READWRITE(float, affectfloat, AffectFloat)
    READWRITE(float, castTime, CastTime)
    READWRITE(float, delayTime, DelayTime)
    READWRITE(float, lastTime, LastTime)

    READWRITE(int, mSkillEffectId, SkillEffectId)
    READWRITE(int, mEffectResId, EffectResId)
    READWRITE(float, mAppearOffset, AppearOffset)
    READWRITE(float, mSpeed, Speed)
    READWRITE(float, mMoveLength, MoveLength)
    READWRITE(int, mTargetNum, TargetNum)       //攻击目标的个数，只用于锁定目标类的技能效果
    READWRITE(int, mAttackCount, AttackCount)
    READWRITE(float, mAttackIntervalTime, AttackIntervalTime)
    READWRITE(int, mHitBackDistance, HitBackDistance)
    READWRITE(float, mHitBackTime, HitBackTime)
    READWRITE(int, mAudioID, AudioID)
#endif
    SkillAffect affectType;
    string affect;
//    bool isForSelf;
    SkillAffectTargetType mTargetType;

    ResPosType mResPosType; //资源相对于角色出现的位置
    
    string mEffectName;
    bool mIsFullScreen;
    string mCharacterAnimation;
    bool mCharacterAnimationLoop;

    bool mIsBehindCharacter;
};
struct SkillCfgDef
{
    SkillCfgDef()
    {
        setSkillID(0);
        setMaxLvl(0);
        setSkillCD(0);
        setComCD(0);
        setPrevID(0);
        setLearnLvl(0);
        setRange(0);
        setNeedGold(0);
        setBattlePoint(0);

        setAttackRectWidth(0);
        setAttackRectHeight(0);
        setAttackRectX(0);
        setAttackRectY(0);
        setCastRangeX(0);
        setCastRangeY(0);
        setSKillType(0);
        
        setPetSkillType(0);
        setNeedPages(0);
        setIsControl(false);
        setUpFailAddProb(0.0);
        setTopTotalProb(0.0);
    }

    string skillname;
    string desc;
    SkillOwner owner;
//    SkillTarget target;
    //ObjType targetType;
#ifdef CLIENT
    ReadWrite_Crypt(int, mSkillID, SkillID)
    ReadWrite_Crypt(int, mMaxLvl, MaxLvl)
    ReadWrite_Crypt(float, skillCD, SkillCD)
    ReadWrite_Crypt(float, comCd, ComCD)
    ReadWrite_Crypt(int, prevID, PrevID)
    ReadWrite_Crypt(int, learnLvl, LearnLvl)
    ReadWrite_Crypt(int, range, Range)
    ReadWrite_Crypt(int, needGold, NeedGold)
    ReadWrite_Crypt(int, battlepoint, BattlePoint)

    ReadWrite_Crypt(float, attackRectWidth, AttackRectWidth)
    ReadWrite_Crypt(float, attackRectHeight, AttackRectHeight)
    ReadWrite_Crypt(float, attackRectX, AttackRectX)
    ReadWrite_Crypt(float, attackRectY, AttackRectY)
    ReadWrite_Crypt(int, castRangeX, CastRangeX)
    ReadWrite_Crypt(int, castRangeY, CastRangeY)
    ReadWrite_Crypt(int, skillType, SKillType)
    
    ReadWrite_Crypt(int, petSkillType, PetSkillType)  //宠物技能类型
    ReadWrite_Crypt(int, needPages, NeedPages)    //升级需要技能书的页,宠物技能一定要>0
    ReadWriteBool_Crypt(bool, isControl, IsControl)  //幻兽试炼场里面  幻兽的某些技能是AI自己释放
    ReadWrite_Crypt(float, topTotalProb, TopTotalProb) // 技能升级，最大概率，超过最大概率，成功
    ReadWrite_Crypt(float, upFailAddProb, UpFailAddProb) // 升级失败所得概率补偿
#else
    READWRITE(int, mSkillID, SkillID)
    READWRITE(int, mMaxLvl, MaxLvl)
    READWRITE(float, skillCD, SkillCD)
    READWRITE(float, comCd, ComCD)
    READWRITE(int, prevID, PrevID)
    READWRITE(int, learnLvl, LearnLvl)
    READWRITE(int, range, Range)
    READWRITE(int, needGold, NeedGold)
    READWRITE(int, battlepoint, BattlePoint)

    READWRITE(float, attackRectWidth, AttackRectWidth)
    READWRITE(float, attackRectHeight, AttackRectHeight)
    READWRITE(float, attackRectX, AttackRectX)
    READWRITE(float, attackRectY, AttackRectY)
    READWRITE(int, castRangeX, CastRangeX)
    READWRITE(int, castRangeY, CastRangeY)
    READWRITE(int, skillType, SKillType)
    
    READWRITE(int, petSkillType, PetSkillType)  //宠物技能类型
    READWRITE(int, needPages, NeedPages)    //升级需要技能书的页,宠物技能一定要>0
    READWRITE(bool, isControl, IsControl)
    READWRITE(float, topTotalProb, TopTotalProb) // 技能升级，最大概率，超过最大概率，成功
    READWRITE(float, upFailAddProb, UpFailAddProb) // 升级失败所得概率补偿
#endif
    bool isBothSide;
//    int awakeLvl;                           //所需的觉醒等级，在加载觉醒表的时候初始化，主要服务器用到
    vector<SkillEffectVal*> effectList;
    string skillIcon;
};

//技能对战斗力的加成
struct SkillIncrBattle
{
    BaseProp   baseProp;       // 基础加法加成
    BattleProp battleProp;     // 战斗加法加成
    BasePropMultiFactor basePropFactor;    // 基础乘法加成
    BattlePropMultiFactor battlePropFactor;    // 战斗乘法加成
    
    SkillIncrBattle& operator+=( SkillIncrBattle& incr)
    {
        baseProp += incr.baseProp;
        battleProp += incr.battleProp;
        basePropFactor += incr.basePropFactor;
        battlePropFactor += incr.battlePropFactor;
        return *this;
    }
};

class SkillCfg : public Cfgable {
public:
    SkillCfg(){}
    virtual ~ SkillCfg(){}

    static void load(std::string skillPath, std::string skillEffectsPath);
    
    //客户端需要节省内存，不需要所有的技能数据。所以单独的加载方式。---linshusen
    static void clientLoad(std::string skillPath, std::string skillEffectsPath);

    //一个技能允许的最多效果数
    static const int sMaxSkillEffectNum;

    static vector<int> roleTotal(SkillOwner owner);
    static SkillCfgDef* getCfg(int _id,bool& exist );
    static SkillCfgDef* getCfg(int _id);
    static bool getCfgByRoleJob(int Job ,vector< int >& outData);
    
    static SkillCfgDef* loadOneSkillFromFile(int skillID);
//    static bool GetEffectFuncByTargetType(int skillId, bool isForSelf, vector<SkillEffectVal*> &effects);
    static vector<SkillEffectVal*> GetEffectFuncByTargetType(int skillId, int targetType);
    static void calcBattlePropFromSkill(int skillid, SkillAffectTargetType targettype, SkillIncrBattle& out);
private:
    static map<int,SkillCfgDef*> cfg;
    static GameInifile* sSkillFile;
    static GameInifile* sSkillEffectFile;
    // 用于多语言
    static GameInifile *mMLIniFile;
    //从文件解析不随等级改变的字段---linshusen
    static void parseUnchangeFields(SkillCfgDef* def, string idInFile_str, GameInifile* skillFile, GameInifile* skillEffectFile);
    
    //从文件解析随等级改变的字段。必须确保SkillCfgDef* def已经调用了parseUnchangeFields()---linshusen
    static void parseChangedFields(SkillCfgDef* def, string idInFile_str, int level, GameInifile* skillFile, GameInifile* skillEffectFile);
};

/*
-------------------------------------------------------------------------------
 物品配置
 */
//物品类型枚举，对应物品的type字段
enum EnumItemType
{
    kItemEquip = 1, //装备
    kItemConsumable =2 ,//消耗品
    kItemMaterial = 3,  //材料
    kItemPackage = 4,   //礼包
    kItemPropStone = 5, //附文石
    kItemMakePrint = 6, //图纸
    kItemRandBox = 7,   //宝箱
    kItemPetEgg = 8,//宠物蛋
    KItemPetDebris = 9,  //宠物碎片
    KItemProps = 10,    //活动道具
    KItemActivityBox = 11, //活动宝箱
    kItemSkillBook = 12, //宠物技能书
    kItemGrowthPill = 13, //宠物成长丸
    
    KItemTicket = 14,       //门票类型
    KItemRenameCard = 16,   //改名卡
    KItemShopProps = 17 ,   //神秘商店道具货币
  
    KItemLuckystone = 18,
    KItemFashion = 19,      //时装
    KItemFashionCollectMaterial = 20,             //2.5 新加字段： 时装武器搜集 合成所需材料
    kItemPartedByJobSex = 21,   // 按职业和性别发礼包
    kItemEquipStone = 22, //装备材料
    KItemRetinuePieces = 23, //侍魂碎片
    KitemRetinueSkillPieces = 24, //侍魂技能
	kItemCustomGift = 99	//自定义礼包
};


enum EnumSkillBookType  //技能书类型
{
    eUpSkillType = 0,   //升级技能
    eStudySkillType = 1 //学习技能
};

//因为物品分类比较多，通过字段来获取物品相应属性值
struct ItemCfgDef
{
    int itemid;
    int qua;
    int stack;
    map<string, string> props;

    int ReadInt(const char* propname, int def = 0, bool *suc = NULL);
    string ReadStr(const char* propname, string def = "", bool *suc = NULL);
    float ReadFloat(const char* propname, float def = 0, bool *suc = NULL);
    //id_name
    string getFullName();
   // string getIniName();  //得到物品所在ini的名称
    READWRITE(string, mIniName, IniName);
};

//这个表用来存储多语言的内容
struct MultiLgItemCfgDef
{
    map<int, string> desc;
    string getMultiLgDesc(int multiLgId);
};

class ItemCfg : public Cfgable
{
public:
    ItemCfg(){}
    virtual ~ItemCfg(){}

    static void load(std::string fullPath);

    static ItemCfgDef* getCfg(int _id);
    
    static MultiLgItemCfgDef * getMulitLgCfg(string iniName);

    static bool IsEquip(int _id);
    
    static int readInt(int itemId, const char* propname, int def = 0);
    
    //通过品质得到幻兽技能书的物品id
    static int getSkillBooksByQua(int qua, bool isOrdinary,int skillBookType);
    static int getPotionID();
   // static string getItemType(int type);
    
    static int getNewItemId(const int &lvl, const int &part, const int &qua, const int &job);
    static vector<int> getShopPropsVec();
    static map<int, ItemCfgDef*> & getAllItemDatas();
    //通过物品类型获取同种类型物品
    static vector<int> getItemsByType(int type);
private:
    static map<int, ItemCfgDef*> cfg;
    static vector<int> shopPropsCfg; //神秘商店货币道具
    static map<string ,MultiLgItemCfgDef *>multiLgCfg;
};


/*
 -------------------------------------------------------------------------------
 装备升星
 */
#include "DataCfgEquip.h"
    
    
/*
 -------------------------------------------------------------------------------
 星座配置
 */
// 星座一颗星的属性配置
struct ConstellStepDef
{
    ConstellStepDef();
    
    
	//需要的星灵值
#ifdef CLIENT
    ReadWrite_Crypt(int, mCons, Cons)
    
    ReadWrite_Crypt(int, mHp, Hp)
    ReadWrite_Crypt(int, mAccHp, AccHp)
    ReadWrite_Crypt(int, mDef, Def)
    ReadWrite_Crypt(int, mAccDef, AccDef)
    
    ReadWrite_Crypt(int, mAtk, Atk)
    ReadWrite_Crypt(int, mAccAtk, AccAtk)
   
    
    ReadWrite_Crypt(float, mHit, Hit)
    
    
    ReadWrite_Crypt(float, mAccHit, AccHit)
    ReadWrite_Crypt(float, mDodge, Dodge)
    ReadWrite_Crypt(float, mAccDodge, AccDodge)
    ReadWrite_Crypt(float, mCri, Cri)
    ReadWrite_Crypt(float, mAccCri, AccCri)
    // 成功机率
    ReadWrite_Crypt(float, mConPro, ConPro)
    //点亮失败后所要加成功概率
    ReadWrite_Crypt(float, mConProAdd, ConProAdd)
    // added by jianghan for 星灵命中概率改进
    ReadWrite_Crypt(float, mConProMaxFail, ConProMaxFail)
    
#else
    READWRITE(int, mCons, Cons)
    
    READWRITE(int, mHp, Hp)
    READWRITE(int, mAccHp, AccHp)
    READWRITE(int, mDef, Def)
    READWRITE(int, mAccDef, AccDef)
    
    READWRITE(int, mAtk, Atk)
    READWRITE(int, mAccAtk, AccAtk)
    
    
    READWRITE(float, mHit, Hit)
    
    
    READWRITE(float, mAccHit, AccHit)
    READWRITE(float, mDodge, Dodge)
    READWRITE(float, mAccDodge, AccDodge)
    READWRITE(float, mCri, Cri)
    READWRITE(float, mAccCri, AccCri)
    // 成功机率
    READWRITE(float, mConPro, ConPro)
    //点亮失败后所要加成功概率
    READWRITE(float, mConProAdd, ConProAdd)
    // added by jianghan for 星灵命中概率改进
    READWRITE(float, mConProMaxFail, ConProMaxFail)
#endif
};
	
// 星座配置定义
struct ConstellCfgDef
{
    
    ConstellCfgDef();
    
	// 星座id
    int consid;
	// 星座序号 1-12
    int index;
	// 星座点数
	int step;
	// 星阶
	int starStep;
	// 星座名字
    string name;
	// 星座里点亮一颗星新增属性和增加的总属性
    vector<ConstellStepDef> steps;
    
#ifdef CLIENT
    
    ReadWrite_Crypt(int, mPageHp, PageHp)
    ReadWrite_Crypt(int, mPageDef, PageDef)
    ReadWrite_Crypt(int, mPageAtk, PageAtk)

    ReadWrite_Crypt(float, mPageHit, PageHit)
    ReadWrite_Crypt(float, mPageDodge, PageDodge)
    ReadWrite_Crypt(float, mPageCri, PageCri)
  
#else
    READWRITE(int, mPageHp, PageHp)
    READWRITE(int, mPageDef, PageDef)
    READWRITE(int, mPageAtk, PageAtk)
    
    READWRITE(float, mPageHit, PageHit)
    READWRITE(float, mPageDodge, PageDodge)
    READWRITE(float, mPageCri, PageCri)

#endif
};

class ConstellationCfg : public Cfgable
{
public:
    ConstellationCfg(){};
    virtual ~ConstellationCfg(){};

    static void load(std::string fullPath);

    static ConstellCfgDef* getCfg(int id_); // 根据星座id获取星座配置
    static ConstellCfgDef* getCfgByIndex(int index,int starStep);// 根据星座的序号获取星座配置
    static ConstellCfgDef* getCfg(string name); // 根据星座名字获取星座配置

	static int getIndexMax() {return indexMax;}
    static int getStarStepMax(){ return star_stepMax;}
private:
    static std::map<int, ConstellCfgDef*> cfg;
	static int indexMax;
    static int star_stepMax;
};

// 新建角色属性配置，如体力值，背包物品等
class GlobalCfg
{
public:
    GlobalCfg(){};
    virtual ~GlobalCfg(){};

    static void load(std::string fullPath);

    static int ReadInt(const char* propname, int def = 0, bool *suc = NULL);
    static string ReadStr(const char* propname, string def = "", bool *suc = NULL);
    static float ReadFloat(const char* propname, float def = 0, bool *suc = NULL);

private:
    static map<string, string> props;

public:
    static vector<int> mMassRefreshHours;
    static int mMassRefreshPoint;
    static int mRefreshPoint;
    static int mRefreshPeriod;
    static int sCityPetNum;
    static int  sCityRetinueNum;
    static int sBackbagRmbExtendSize;
};


// vip 暂未实现
struct VipPrivilegeCfgDef
{
    int needexp;

    int moneyTreeTimes; //开宝箱次数
    int eliteBuyTimes;  //精英副本次数购买次数
    int petAdventureBuyTimes;               //幻兽大冒险副本次数购买次数
    int petAdventureRobotPetRentBuyingTimesLimit;      //幻兽大冒险租用幻兽购买次数限制
    
    int crossServiceWarWorshipBuyTimes; //跨服战膜拜购买次数
    int crossServiceWarBuyTimes; //跨服战购买次数
    
    
    int fatBuyTimes;    //精力值购买次数
    int teamCopyBuyTimes;   //好友（组队）副本次数购买次数

    int hangupElite;    //精英本是否可挂机
    int friendCount;    //可添加好友上限
    int pvpbuytimes;
    int dungeonsTimes;  //地下城重设次数
    int giftPackage;    //每日可领取的vip礼包等级
    int viplvlAward;    //每一级的礼包，一次性领取

    int propStoneLvl;   //可购买的石头最大等级
    int upgZhanHun;     //一键唤醒战魂什么的

    int freelotterytime;//免费抽奖次数
    int buylotterytime; //收费抽奖次数

    float expExtra;     //可获取的额外经验
    
    int petEliteBuytimes;  //操作幻兽次数购买次数
    std::vector<std::string> mPrivilegeDesciption;  //该等级所拥有的特权及其描述
    
};
class VipPrivilegeCfg
{
public:
    VipPrivilegeCfg(){}
    ~VipPrivilegeCfg(){}
    static void load(std::string fullPath);

    static VipPrivilegeCfgDef* getCfg(int viplvl);
    static int getMaxLvl();

private:
    static vector<VipPrivilegeCfgDef*> cfg;
    static int mMaxLvl;
};


//登录奖励
class LoginAwardCfg
{
public:
    LoginAwardCfg(){}
    ~LoginAwardCfg(){}

    static void load(std::string fullPath);
    static int getAwardId(int days);//获取第n天奖励的登录奖励礼包id
    static int getNormalAwardId(int days);
    static int getAfterOpenServerDays();
    static vector<int> & getActivityAwards();
    static vector<int> & getNormalAwards();
    
private:
    static vector<int> activity_awards; //运营活动每天登陆奖励
    static vector<int> normal_awards; //没有运营活动时的奖励
    static int after_openserver_days;
};

// 韩国留言奖励
class KoreaLevelAwardCfg
{
public:
    static void load(std::string fullpath);
    
    static int mLevel;          // 领取等级
    static std::string mAward;  // 奖励内容
    static std::string mMsgUrl; // 留言URL
};

//摇钱树
class MoneyTreeCfg
{
public:
    MoneyTreeCfg(){}
    ~MoneyTreeCfg(){}

    static void load(std::string fullPath);
    static int getGold(int lvl, int openTimes);  //根据等级获取一次获取的金币
    static int getFreeTimes(int lvl);  //根据等级获取免费次数
    static int getConsRmb(int times);  //根据已增加次数获取消耗的金钻数量

private:
    static vector<int> golds;
    static vector<int> freetimes;
    static vector<int> consrmbs;
    static vector<float> timesAddition;
};


struct Newguidance
{
    int level;
    string type;
    string image;
    string content;
    string sInfo;
    int prompt;    //系统提示 或者箭头提示
    int direction; //箭头的方向
    int taskID;
    string parentType;  //父节点类型
    int guidenLevel;    //指引级别
    int targetTag;      //目标节点tag值
    bool isOpen;        //指引是否开启
};

class NewguidanceCfg : public Cfgable {
public:
    NewguidanceCfg(){}
    virtual ~ NewguidanceCfg(){}

    static void load(std::string fullPath);

    //得到是通过升级等到的功能(不包括任务)
    static vector<Newguidance *> getNewguidanceData(int level);
    //通过完成任务开放的功能
    static vector<Newguidance *> getNewDateFromTask();
    static vector<Newguidance *> getAllNewguildanceData(int level);
    static Newguidance * getCfg(int step);
    static Newguidance * getCfgByName(string name);
    static Newguidance * getCfgByTypeAndImg(string type ,string img);
    //得到通过等级开发的新功能
    static vector<Newguidance *> getSystemPrompt(int level);
    //得到通过等级开发的新功能
    static vector<Newguidance *> getArrowsPrompt(int level);
    //通过递归求出级别为1的指引
    static Newguidance * getParentGuidance(Newguidance *newguidance);
    static vector<Newguidance *>  getNewguidanceData(int level,int  guidanceLevel,string parentType);
    static Newguidance*  getNewguidanceDataByTag(int level,int guidanceLevel,string parentType,int tag);
    //根据开放的功能得到所需要的等级
    static int getLevelByName(std::string name);
    //根据父级指引找出子级指引
    static vector<Newguidance *>  getCfgByParentType(string parentType);
private:
    static map<int,Newguidance*> cfg;
};

class WordStockCfg : public Cfgable
{
public:
    WordStockCfg(){}
    virtual ~ WordStockCfg(){}
    static void load(std::string fullPath);
    static vector<string> getWordStockData();
private:
    static vector<string> wordData;
};
struct BuyCfgDef
{
    vector<int> cost;
    int costType;
    int addpoint;
    vector<float> doublelottery;//消费抽奖掉两件概率
    vector<int> times;
};

enum BuyType
{
    eBuyNon,
    eFatBuy,
    eEliteBuy,
    eDungBuy,
    ePvpTimesBuy,
    eTeamCopyReset,
    eLotteryBuy,    // 轮盘抽奖
    eDailyQuestRefresh,
    eDiceLotteryBuy,    // 色子抽奖
    eOperateBuy,          //购买操作幻兽的副本
    ePetAdventureBuy,  //幻兽大冒险副本
    ePetAdventureRobotPetRent,
    eCrossServiceWarWorShipBuy, //跨服战膜拜次数
    eCrossServiceWarBuy, //跨服战次数

};

enum CostType
{
    eCostNon,
    eRmbCost,
    eGoldCost,
};

class BuyCfg : public Cfgable {
public:
    static void load(std::string fullPath);
    static int getCost(int buytype, int times, int& costType);
    static int getCost(int buytype, int times, string& moneyname);

//    static int getRmbCost(int type, int times);
//    static int getGoldCost(int type, int times);
    static int getAddpoint(int type);
    static int getSize(int type);

    static int getLvlTimes(int type, int lvl);

    static float getDouLottery(int times);
    static string costTypeToMoneyName(int costType);
private:
    static std::map<int, BuyCfgDef*> cfg;
};

inline string costName(int costType)
{
    return BuyCfg::costTypeToMoneyName(costType);
}

#define PVP_AWARD_TOTAL 5
#define PVP_AWARD_PART_ONE 100
#define PVP_AWARD_PART_TWO 150
#define PVP_AWARD_PART_THREE 200
#define PVP_AWARD_PART_FOUR 300

struct PvpAward
{
    PvpAward()
    {
        protoss = gold = 0;
    }
    int protoss;
    int gold;
    string itemAward;
};
struct PvpAwardDef
{
    PvpAwardDef()
    {
        mMinRank = mMaxRank = 0;
        mIsShuzu = false;
    }
    int mMinRank;
    int mMaxRank;
    bool mIsShuzu;
    vector<PvpAward*> mAwards;

};

struct PvpRankDataDef
{
    int mMax;
    int mMin;
    int space;
};


class PvpCfg : public Cfgable {
public:
    static void load(std::string fullPath);
    static PvpAward* getRankAward(int rank);
    static PvpAward* getResultAward(bool win);
    static int getfreeTimes();
    static int getOpenLvl();
    static void getRankRule(int myrank, PvpRankDataDef& output);
    static int getCoolDownTime(int times);
    static int getFreshCost(int times);
    static int getTimeout();
    static int getRobotCount();
    static int getAwardPointPeriod(){return mAwardPointPeriod;}
private:
    static vector<int> mRanges;
    static std::map<int, PvpAwardDef*> mRankAward;
    static vector<PvpAward*> mPvpResultAward;
    static int mfreetimes;
    static int mOpenlvl;
    static int mTimeout;
    static vector<PvpRankDataDef*> mRankDataRule;
    static vector<int> mCooldownTime;
    static vector<int> mFreshCost;
    static int mRobotCount;
    static int mAwardPointPeriod;
};

struct FriendRewardDef
{  
    // 翻牌奖励比例
    float mReward;
    // 金币奖励比例
    float mRewardGold;
    // 经验奖励比例
    float mRewardExp;
    // 战功奖励比例
    float mRewardBattlePoint;
    // 宠物奖励比例
    float mRewardPetExp;
    // 下限
    int mMin;
    // 上限
    int mMax;
};

class FriendRewardCfg : public Cfgable
{
public:
    static void load(std::string fullPath);
    static FriendRewardDef* getRankAward(int intimacy);
private:
    static vector<int> mRanges;
    static std::map<int, FriendRewardDef*> mRankAward;
};

#pragma mark - 神秘商人系统配置 activitymerchant.ini activitygoods.ini
struct MysteriousExchangeDef
{
	// 序号ID
	int index;
	// 物品ID
    int goodsId;
	// 限制等级区间
    int lvlMin;
	int lvlMax;
	//　需消耗物品
	int exchangeId;
	int exchangeNum;
	// 打折
	string sale;
	string desc;
    // 上限次数
	int limit_num;
	// 排序
	int rank;
	
	int start;
	int end;
    int after_openserver_days; //开服多少天后才可以出售
};
	
struct MysteriousTimeGoodsDef //特卖
{
	// 序号ID
	int index;
	// 类型
	int type;
	// 价格
	int consume;
	// 物品ID
    int goodsId;
	// 打折
	string sale;
	// 描述
	string desc;
	
	int start;
	int end;
	
	int limit_num;
	int rank;
    
    int after_openserver_days; //开服多少天后才可以出售
};
	
struct MysteriousGoodsDef
{
	// 序号ID
	int index;
	// 物品ID
    int goodsId;
    // 限制等级区间
    int lvlMin;
	int lvlMax;
    //限制VIP等级区间
    int viplvlMin;
    int viplvlMax;
    // 权重
    int pro;
	// 消耗类型
	int type;
	// 消耗值区间
    int consumeMin;
    int consumeMax;
	// 描述
	string desc;
	// 打折
	string sale;
};
	
class MysteriousCfg : public RandGoodsCfgBase
{
public:
	void read();
	int getFreeGoldNum() {return mGoldFreeNum;}
	int getUpdateNeedGold() {return mUpdateGold;}
    MysteriousGoodsDef * getMysteriousGoodsDef(int index);
	// 根据权重随机物品
    void randGoods(int level, int viplevel, vector<MysteriousGoodsDef*> &vec);
	virtual bool checkIsIndex(int index, int goodsId);
	
	MysteriousTimeGoodsDef * getMysteriousTimeGoodsDef(int index);
    
    map<int, MysteriousTimeGoodsDef> getMysteriousTimeGoodsMap();
	void getMysteriousTimeGoodsMap(vector<MysteriousTimeGoodsDef*> &vec);
	
	
    std::string getTimeGoodsDesc();
    bool isUseViplvlRefreshLimit() const;
	
	// 根据Id 获取 道具商品详细 信息
	const MysteriousExchangeDef * getExchangeGoods(int index);
    // 道具商品 等级筛选，
    vector<MysteriousExchangeDef *> getPropsExchangeByLvl(int lvl,int time);
    int  getPropsExchangeDataSize();
    vector<MysteriousTimeGoodsDef *> getMyMysteriousTimeGoodsInLimitTime(int time);
	
	void getExchangeGoods(vector<MysteriousExchangeDef*> &vec);
	
	bool checkSpecialTm();
	bool checkExchangeTm();
	bool specialIsDelay(int tm);
	bool exchangeIsDelay(int tm);
	int getExchangeEnd() {return mExchangeEnd;}
	int getSpecialEnd() {return mTimeEnd;}
private:
	void readMerchant(GameInifile &inifile);
	void readGoods(GameInifile &inifile);
	void readTimeActivaityGoods(GameInifile &inifile);
	void readExchangeGoods(GameInifile &inifile);
	
	// ----- 商店 -----
	typedef map<int, MysteriousGoodsDef> MysteriousGoodsTd;
	MysteriousGoodsTd mMysteriousMap;
	
	// 金币刷新消耗值
	int mUpdateGold;
	// 金币免费次数
	int mGoldFreeNum;
	
	// ----- 限时特卖 -----
	typedef map<int, MysteriousTimeGoodsDef> MysteriousTimeGoodsTd;
	MysteriousTimeGoodsTd mMysteriousTimeGoodsMap;
	
	// 限时特价商品说明
	string mTimeGoodsDesc;
	
	int mTimeStart;
	int mTimeEnd;
	int mTimeIsDelay;
	
	// ----- 物品兑换 -----
	typedef map<int, MysteriousExchangeDef> MysteriousExchangeGoodsTd;
	MysteriousExchangeGoodsTd mMysteriousExchangeGoodsMap;
    vector<MysteriousExchangeDef *> mMysteriousExchangePropsVec;
	
	int mExchangeStart;
	int mExchangeEnd;
	int mExchangeIsDelay;
};

class MysteriousCfgMgr
{
public:
	static void load();
	static MysteriousCfg sCfg;
};

#pragma mark -

#pragma mark - 图鉴系统配置 PetCollect.ini
struct IllustrationsDef
{
    // 模板ID
    int templetId;
    // 模板类型
    IllustrationsType type;
    string name;
    std::vector<string> singleRewardStr;
    string allRewardStr;
    std::vector<string> singleRewardInfor;
    string allRewardInfor;
    std::vector<string> singleGetWay;
    string petDescStr;

};

class IllustrationsCfg : public Cfgable
{
public:
    static void load(std::string fullPath, std::string configPath="");
    static IllustrationsDef* getIllustrationsDef(int id);
    static std::vector<IllustrationsDef *> getIllustrationsDefsbyType(IllustrationsType type);
    
    static string getIllQuaReward(int id, IllustrationsQuality qua);
	
	typedef std::map<int, IllustrationsDef*>::iterator Iterator;
	static Iterator begin() {return mIllustrationsMap.begin();}
	static Iterator end() {return mIllustrationsMap.end();}
    
    static bool isBagUpdateVer(int ver);
private:
    static std::map<int, IllustrationsDef*> mIllustrationsMap;
    static int mVer; // 图鉴背包更新版本
};

    
#pragma mark - 问答活动
    struct Question
    {
        int id;         //题目ID
        int answer;     //正确答案1/2/3
        int difficulty; //难度
        int score;      //题目分数
        string desc;    //题目描述
        vector<string> selects; //选择选项描述，对应为0、1、2
        Question()
        {
            id = 0;
            answer = 0;
            difficulty = 1;
        }
    };
    
    //存放不同分数段奖励值
    struct ScoreItem
    {
        int minScore; //最小分数
        int maxScore;  //最大分数
        string rewards; //对应的奖励
        ScoreItem(const int &_minScore, const int &_maxScore, const string &_rewards)
        {
            minScore = _minScore;
            maxScore = _maxScore;
            rewards = _rewards;
        }
    };
    
    struct TotalawardItem
    {
        string awarditem;   //奖励物品
        int  totaldays;     //需要连续几天
        string desc;
        TotalawardItem(const string &_desc="", const string &_awarditem="", const int _totaldays=0):
        desc(_desc), awarditem(_awarditem), totaldays(_totaldays){}
    };
    
    class AnswerActivityCfg: public Cfgable
    {
    public:
        typedef std::map<int, Question> QuestionMap;
        typedef std::vector<int> QuestionidVec;
        //加载配置表
        static void load(const std::string &questionsFilePath, const std::string &answerActivityFilePath);
        static void loadQuestionsFile(const std::string &filepath);
        static void loadAnswerActivityFile(const std::string &filepath);
        //获取所有题目
        inline static QuestionMap getAllQuestions()
        {
            return _all_questions_map;
        }
        //获取简单题目的ID
        inline static QuestionidVec getEasyQuestionsId()
        {
            return _easy_questions_id;
        }
        //获取困难题目的ID
        inline static QuestionidVec getHardQuestionsId()
        {
            return _hard_questions_id;
        }
        // 根据ID获取题目
        static Question getQuestion(const int &id);

        // 根据ID获取答案

        
        //----------------------------------------------------------
        //根据Id获取题目 4个选项
        static vector<string> getQuestion4Selection(const int &id);
        //打乱 4个选项 的次序 ，用于客户端表现
        static vector<string> randomQuestion4Selection(const int &id);
        //查找 打乱后 4个选项 对应 配置表 原始位置。返回给服务器验证
        static int findOriginalSelection(const int &id, string selection);
        //----------------------------------------------------------
        
        //根据ID获取答案

        static int getAnswer(const int &id);
        // 抽题
        static vector<int> getRandomQuestions();

        static int chargeQuestion(const int &id, const vector<int> &vec);
        
        // 根据分数返回相应的奖励
        static string getScoreRewards(const int &score);
        // 判断答案是否正确
        static bool isRightAnswer(const int &questionId, const int &roleAnswer);
        static int getMaxChangeCount() { return _max_change_count; }
        
        static int getQuestionScore(const int &id);
        static int getAnswerTm(){return _answer_time;}
        static int getChangeQuestionCost(int num);
        static TotalawardItem getTotalawardItem(const int &key);
    private:
        // 更换简单的问题
        static int changeEasyQuestion(const vector<int> &easyId);
        // 更换困难的问题
        static int chargeHardQuestion(const vector<int> &hardId);
        
        static std::map<int , Question> _all_questions_map;//所有的题目
        static std::vector<int> _easy_questions_id;
        static std::vector<int> _hard_questions_id;
        static int _max_questions_count;   //抽得题目个数
        static std::vector<int> _cost_rmb;   //换题所需金钻
        static int _max_change_count;   //最大换题次数
        static int _easy_question_count;//简单题目个数
        static int _hard_question_count;//困难题目个数
        static int _answer_time;        //答题时间
        static vector<ScoreItem> _score_rewards; //不同分数段奖励
        static map<int, TotalawardItem> _totalaward_map; //连续奖励
    };

    //
class TemplateCfg: public Cfgable
{
public:
    typedef map<string, string> KV;
    typedef map<string, string>::iterator KVIterator;
    typedef map<string, KV> SecKV;
    typedef map<string, KV>::iterator SecKVIterator;
public:
    static void load(const string &filepath);
    static string getValue(const string &section, const string &key);
protected:
    static SecKV _map;
};
    
//vip等级影响精力恢复数
class MassFatLimitCfg: public Cfgable
{
public:
    typedef map<int, int> Map;
    typedef Map::iterator MapIterator;
    
    static void load(const string &filepath);
    static int getFatLimitByViplvl(const int &viplvl, const int &def = 5);
    static int getExtFatLimitByViplvl(const int &viplvl, const int &def = 50);
private:
    static Map mLimitMap;
    static Map mExtLimitMap;
};

struct EquipChangeMappingTable
{
    int level;
    vector<int> quality;
    vector<int> star;
};

//装备数值转换
class EquipChangeCfg: public Cfgable
{
public:
    typedef map<int, map<int, EquipChangeMappingTable> > TopMap;
    typedef TopMap::iterator TopMapIterator;
    typedef map<int, EquipChangeMappingTable> Map;
    typedef Map::iterator MapIterator;
    
    static void load(const string &filepath);
    //通过等级，原来武器颜色，原来武器星级获取新的武器的品质颜色。通过job,武器lvl,武器qua,武器type获取唯一武器装备
    static int getNewEquipQua(const int &roleLvl, const int &oldQua, const int &equipStar);
    static int getNewEquipStar(const int &roleLvl, const int &oldQua, const int &equipStar);
    static int getNewEquipLvl(const int &roleLvl, const int &oldQua, const int &equipStar);
private:
    //人物等级(1-19)->颜色(green,blue...)->对应的映射表(lvl,qua,star)
    static TopMap mData;
    //map<level, map<old_qua, maptable>>
};
    
//
struct ItemToGoldDef
{
    int index;
    int proto_id;
    int compensation_gold;
};
class ItemToGoldCfg: public Cfgable
{
public:
    static void load(const string &filepath);
    static ItemToGoldDef *getItemToGoldDef(const int &itemid);
private:
    //map<id, itemdef>
    static map<int, ItemToGoldDef> mData;
};

#pragma mark -
#pragma mark 女王祝福
struct QueenBlessTimeItem
{
    QueenBlessTimeItem()
    {
        index = 0;
        award_fatVec.clear();
        starttime.tm_hour = 0;
        starttime.tm_min = 0;
        endtime.tm_hour = 0;
        endtime.tm_min = 0;
        receiveStatus = eNotCanReceive;
    }
    int index;
    vector<int> award_fatVec;
    struct tm starttime;
    struct tm endtime;
    QueenBlessingReceiveStatus receiveStatus;
};
    
class QueenBlessCfg: public Cfgable
{
public:
    typedef vector<QueenBlessTimeItem> TimeItem;
    typedef TimeItem::iterator TimeItemIterator;
    
    static void load(const string &filepath);
    
    static QueenBlessTimeItem *getTimeItemByIndex(const int &index);
    //是否是活动时间
    static bool isActivityTime();
    
    static QueenBlessTimeItem *isDuringTime(const time_t &nowtime);
    //根据vip 等级取得当前能得到的精力值
    static int getQueenBlessFat(const time_t &nowtime, int vipLvl = 0);
    
    //获取在什么时候通知客户端倒计时
    static int getRemainTime();
    
    //获取领取精力列表
    static std::vector<QueenBlessTimeItem> getQueenBlessTimes();
    
private:
    static TimeItem mTime;
};

    
#pragma mark -
#pragma mark 幻兽升级（合并）
//某个品质所有阶宠物属性
struct PetMergeItem
{
    int cost_gold;                  //同品质宠物合并需要花费的金钱
    vector<int> merge_uplvl_exp;    //同品质升阶所要的经验（100，200，300，400，500），100表示升级到1级要100经验
    vector<double> incr_stre;        //力量增加百分比
    vector<double> incr_inte;        //智力增加百分比
    vector<double> incr_phys;        //体力增加百分比
    vector<double> incr_capa;        //耐力增加百分比
};

//某个星阶宠物的增加的属性
struct PetMergeLvlItem
{
    double incr_stre;
    double incr_inte;
    double incr_phys;
    double incr_capa;
};
    
class PetMergeCfg: public Cfgable
{
public:
    typedef map<int, PetMergeItem> Map;
    typedef Map::iterator MapIterator;
    
    static void load(const string &filepath);
    
    static int getMaxStage();
    
    static PetMergeItem *getPetMergeItem(const int &pet_qua);
    //通过宠物品质和阶等级得到相应的属性    merge_lvl: 1-mMaxStage
    static PetMergeLvlItem getPetMergeLvlItem(const int &pet_qua, const int &merge_lvl);
    //获得升级到X阶所需的经验      lvl: 1-mMaxStage
    static int getUplvlExp(const int &pet_qua, const int &lvl);
    //获得升到X阶所需要的总经验    lvl: 1-mMaxStage
    static int getUplvlTotalExp(const int &pet_qua, const int &lvl);
    //传入幻兽当前等级以及获取经验，得到宠物新等级以及剩余经验
    //                                  宠物品质     [in out]宠物等级  [in]获得经验+当前经验     [out]剩余经验
    static void getUplvlByExp(const int &pet_qua, int &petlvl, const int &exp, int &remain_exp);

private:
    static int mQuanum;
    static int mMaxStage;
    //map<pet qua, PetMergeItem>
    static Map mData;
};
    

#pragma mark -
#pragma mark 扭蛋机（台湾版本）
//扭蛋机 gashapon machines
    
struct GashaponMachinesQuaItem
{
    int qua_type;
    int only_sell;      
    int refresh_time;
    int can_everyday_free_get;
    int everyday_free_times;
    int sell_price;
    int buy_times_limit;
    vector<int> viplvl_buy_times_limit;
    int is_have_special_offers;
    int special_offers_buy_count;  //买x
    int special_offers_buy_present; //送y
    string icon;
    string desc_id;
    bool is_struct_enable;
    GashaponMachinesQuaItem()
    {
        is_struct_enable = false;
    }
};

struct GashaponMachinesAwardItem
{
    int item_id;
    int item_count;
    vector<int> item_prop;
};
    
class GashaponMachinesCfg: public Cfgable
{
public:
    typedef map<int, GashaponMachinesQuaItem> Map;
    typedef Map::iterator MapIterator;
    //          qua     lvl     award items
    typedef map<int, map<int, vector<GashaponMachinesAwardItem> > > ItemMap;
    typedef ItemMap::iterator ItemMapIterator;
    
    
    static void load(const string &cfg_filepath, const string award_filepath);
    
    //
    static bool isOpen();
    static int getQuaCount();
    static Map &getQuaItems();
    static ItemMap &getQuaAwardItems();
    //
    static GashaponMachinesQuaItem getGashaponMachinesQuaItem(const int &qua_type);
    // 返回-1 表示没有购买次数限制
    static int getViplvlBuyTimesLimit(const int &qua_type, const int &viplvl);
    // 获取某个类型，某个级别区间的奖励信息
    static vector<GashaponMachinesAwardItem> getGashaponMachinesAwardItemByLvl(const int &qua_type, const int &lvl);
    
	// 计算当前次数, 对应的区间位置
	static int getTimeNumIndex(int timeNum);
private:
    static bool mIsOpen;
    static int mQuaCount;
	// 不同的次数权限
	static vector<int> mTimeNumProp;
    static Map mQuaItems;       
    static ItemMap mAwardItems;
};

    
#pragma mark - 跑马灯公告
class SystemBroadcastMLCfg: public TemplateCfg
{
};

class SystemBroadcastCfg: public Cfgable
{
public:
    static void load(const std::string &filepath);
    static std::string getValue(const string &section, const std::string &key);
    typedef std::map<std::string, std::map<std::string, std::string> > Map;
    typedef std::map<std::string, std::map<std::string, std::string> >::iterator MapIterator;
protected:
    // static std::map <std::string, std::string> _broadcast_info;
    static Map _broadcast_info;
};
    

    class GameTextCfg: public TemplateCfg
    {
    public:
        GameTextCfg(){_map.clear();}
        static string getFormatString(const char *id, ...);
        static string getString(const string &id);
    };
    
    
#pragma mark - 
    class RandboxGiftMLCfg: public SystemBroadcastCfg
    {
    };
    
    
#pragma mark - url
    class UrlCfg: public TemplateCfg
    {
    public:
        static string get_offical_bbs_url()
        {
            return getUrl("offical_bbs_url");
        }
        static string get_register_order_url()
        {
            return getUrl("register_order_url");
        }
        static string get_activation_url()
        {
            return getUrl("activation_url");
        }
        
        static string get_pushnotify_url()
        {
            return getUrl("pushnotify_url");
        }
		
		static string getUrl(const char *name)
		{
			string url = getValue("root", name);
            if (url.empty()){
                log_info("get " << name << "'s" << " URL is empty");
            }
            return url.c_str();
		}
    };

#pragma mark - 装备共鸣系统配置 equip_resonance.ini
// 装备共鸣系统配置
struct EquipResonanceDef
{
#ifdef CLIENT 
    // 攻击
    ReadWrite_Crypt(int, mAtkup, Atkup)
    // 防御力
     ReadWrite_Crypt(int, mDefup, Defup)
	// 血量
     ReadWrite_Crypt(int, mHpup, Hpup)
    // 命中
     ReadWrite_Crypt(float, mHitup, Hitup)
    // 闪避
     ReadWrite_Crypt(float, mDodgeup, Dodgeup)
    // 暴击率
     ReadWrite_Crypt(float, mCriup, Criup)

#else
    // 攻击
    READWRITE(int, mAtkup, Atkup)
    // 防御力
    READWRITE(int, mDefup, Defup)
	// 血量
    READWRITE(int, mHpup, Hpup)
    // 命中
    READWRITE(float, mHitup, Hitup)
    // 闪避
    READWRITE(float, mDodgeup, Dodgeup)
    // 暴击率
    READWRITE(float, mCriup, Criup)
#endif
   // 说明
    string desc;
    
    EquipResonanceDef& operator+=(const EquipResonanceDef& def)
    {
        setAtkup(getAtkup() + def.getAtkup());
        setDefup(getDefup() + def.getDefup());
        setHpup(getHpup() + def.getHpup());
        setHitup(getHitup() + def.getHitup());
        setDodgeup(getDodgeup() + def.getDodgeup());
        setCriup(getCriup()+ def.getCriup());
        return *this;
    }
    
    EquipResonanceDef()
    {
        clear();
    }
    
    void clear()
    {
        setAtkup(0);
        setDefup(0);
        setHpup(0);
        setHitup(0.0);
        setDodgeup(0.0);
        setCriup(0.0);
        desc = "";
    }
};

class EquipResonanceCfg : public Cfgable
{
public:
    static void load(std::string fullPath);
    
    // 获得共鸣加成
    static void getEquipResonanceCount(EquipResonanceDef &def, int deepen, int quality);
    
    // 获取加成数据（装备星级）
    static EquipResonanceDef* getDeepenDef(int deepen);
    // 获取加成数据（装备品质）
    static EquipResonanceDef* getQualityDef(int quality);
    static int getDeepenMapSize();
    static int getQualityMapSize();
    static vector<int> mDeepenRanges;
private:
    static std::map<int, EquipResonanceDef*> mDeepenMap;
    static std::map<int, EquipResonanceDef*> mQualityMap;
};
#pragma mark -

#pragma mark - 特殊活动配置 promotion_activity.ini
// 特殊活动配置文件
struct PromotionActivityDef
{
    map<string, string> props;
    int ReadInt(const char* propname, int def = 0, bool *suc = NULL);
    string ReadStr(const char* propname, string def = "", bool *suc = NULL);
};
    
class PromotionActivityCfg : public Cfgable
{
public:
    static void load(std::string fullPath);
    static PromotionActivityDef * getDef(int id);
private:
    static map<int, PromotionActivityDef*> mMap;
};

#pragma mark -
#pragma mark - 分享奖励配置 

class ShareAwardCfg : public Cfgable
{
public:
	static void load(std::string fullPath);

	static string sfirstAward;
	static bool sIsWeekly;
	static string sWeeklyAward;
	static int sCycleTime;
	static string sDailyAward;
};
#pragma mark -
//  宠物配置文件
struct PetCfgDef
{
    
    PetCfgDef();
    std::string name;    //名字
  
    int growth[2];    //成长值的随机范围
    int batk[2];     //攻击的随机范围
    int bdef[2];     //防御随机范围
    int bhp[2];      //血量随机范围
    float bdodge[2]; //防御随机范围
    float bhit[2];   //命中随机范围
    
    int totalStageUpStep;                 //满星级所需要的星量
   
    vector<int>   starUpStep;             //升星价值量,不同等级不同
    vector<float> starUpSuccRatio;        //升星基础成功率
    vector<int>   starUpConsumeFactor;    //升星消耗金币系数
    
   
    vector<int>   stageInherit;           //可以继承部分阶量
    vector<int>   stageUpStep;            //某级升阶需要的阶量

    vector<int> skills;  //宠物技能列表
    vector<float> skillprops;//宠物获取相应技能的概率
    vector<float> skilllockprops;//宠物获取相应技能后，技能又被锁住的概率
    
    vector<float> lvlPoints;
    vector<float> ownPoints;
  
    vector<int> petMergeList;  //可合体幻兽id列表
    
    int getSkillIndex(int skillid)
    {
        int skillType = skillid / 100;
        
        for (int i = 0; i < skills.size(); i++) {
            if ((skills[i]/100) == skillType) {
                return i;
            }
        }
        
        return -1;
    }

#ifdef CLIENT
 //   ReadWrite_Crypt(int, mType, Type); //类型，1 远攻 2 近攻 3 远防
    ReadWrite_Crypt(int, mMaxlvl, Maxlvl); //最高等级限制
    ReadWrite_Crypt(int, mExtlvl, Extlvl); //最高高过主人的等级
    ReadWrite_Crypt(int, mMonsterTpltID, MonsterTpltID); //怪物模版id
    ReadWrite_Crypt(int, mQua, Qua);  //品质，1 2 3 4 5 分别对应 绿 蓝 紫 粉 金
    ReadWrite_Crypt(int, mExp, Exp);  //吞噬时额外增加的经验值,先天的
    ReadWrite_Crypt(float, mCrip, Crip);  //爆机伤害百分比
    ReadWrite_Crypt(float, mCri, Cri);  //爆机发生概率
    ReadWrite_Crypt(int, mMinGrowthRefine, MinGrowthRefine);  //最小成长洗练
    ReadWrite_Crypt(int, mMaxGrowthRefine, MaxGrowthRefine);  //最大成长洗练
    ReadWrite_Crypt(int, mNeedGrowthPill, NeedGrowthPill);  //需要成长丸个数
    ReadWrite_Crypt(float, mGrowthSuccFactor, GrowthSuccFactor);  //洗练成功系数
    ReadWrite_Crypt(int, mCollectqua, Collectqua);  //宠物图鉴的品质
    ReadWrite_Crypt(float,mGrowthRate,GrowthRate); //成长值的计算比率
    
    ReadWrite_Crypt(int,mStarBorn,StarBorn); //先天星量
    ReadWrite_Crypt(int, mStageBorn, StageBorn)//先天阶量
    ReadWrite_Crypt(int, mMaxbaseprop, Maxbaseprop)
    ReadWrite_Crypt(float, mExtraProp, ExtraProp)//额外属性概率
    ReadWrite_Crypt(int, mExtraPropPoint, ExtraPropPoint)
    
    
    ReadWrite_Crypt(int, mStre, Stre)//力量
    ReadWrite_Crypt(int, mInte, Inte)//智力
    ReadWrite_Crypt(int, mPhys, Phys)//体力
    ReadWrite_Crypt(int, mCapa, Capa)//耐力
  
    
    ReadWrite_Crypt(float, mPhystohp, Phystohp)
    ReadWrite_Crypt(float, mCapatodef, Capatodef)
    ReadWrite_Crypt(float, mIntetoatk,Intetoatk)
    ReadWrite_Crypt(float, mStretoatk, Stretoatk)
    
    ReadWrite_Crypt(int, mMergeExp, MergeExp)     //幻兽合并升阶额外经验
    ReadWrite_Crypt(int, mExtlvlAdd, ExtlvlAdd)   //等级差增加值，海外版
    ReadWrite_Crypt(int, mMaxlvlAdd, MaxlvlAdd)   //等级上限增加值，国内版
    ReadWrite_Crypt(int, mGrowthAdd, GrowthAdd)   //成长值上限增加值,影响宠物洗练成长上限
    ReadWrite_Crypt(int, mTypeForPetMerger, TypeForPetMerger);   //用于幻兽合体的类型
    ReadWrite_Crypt(int, mDisplayInCity, DisplayInCity)
	ReadWrite_Crypt(int, mCollectID, CollectID); // 图鉴使用的模板id
    ReadWrite_Crypt(float, mSpiritSpeakCoef, SpiritSpeakCoef);
#else
  //  READWRITE(int, mType, Type); //类型，1 远攻 2 近攻 3 远防
    READWRITE(int, mMaxlvl, Maxlvl); //最高等级限制
    READWRITE(int, mExtlvl, Extlvl); //最高高过主人的等级
    READWRITE(int, mMonsterTpltID, MonsterTpltID); //怪物模版id
    READWRITE(int, mQua, Qua);  //品质，1 2 3 4 5 分别对应 绿 蓝 紫 粉 金
    READWRITE(int, mExp, Exp);  //吞噬时额外增加的经验值,先天的
    READWRITE(float, mCrip, Crip);  //爆机伤害百分比
    READWRITE(float, mCri, Cri);  //爆机发生概率
    READWRITE(int, mMinGrowthRefine, MinGrowthRefine);  //最小成长洗练
    READWRITE(int, mMaxGrowthRefine, MaxGrowthRefine);  //最大成长洗练
    READWRITE(int, mNeedGrowthPill, NeedGrowthPill);  //需要成长丸个数
    READWRITE(float, mGrowthSuccFactor, GrowthSuccFactor);  //洗练成功系数
    READWRITE(int, mCollectqua, Collectqua);  //宠物图鉴的品质
    READWRITE(float,mGrowthRate,GrowthRate); //成长值的计算比率
    
    READWRITE(int,mStarBorn,StarBorn); //先天星量
    READWRITE(int, mStageBorn, StageBorn)//先天阶量
    READWRITE(int, mMaxbaseprop, Maxbaseprop)
    READWRITE(float, mExtraProp, ExtraProp)//额外属性概率
    READWRITE(int, mExtraPropPoint, ExtraPropPoint)//额外属性概率
    
    
    READWRITE(int, mStre, Stre)//力量
    READWRITE(int, mInte, Inte)//智力
    READWRITE(int, mPhys, Phys)//体力
    READWRITE(int, mCapa, Capa)//耐力
    
    
    READWRITE(float, mPhystohp, Phystohp)
    READWRITE(float, mCapatodef, Capatodef)
    READWRITE(float, mIntetoatk,Intetoatk)
    READWRITE(float, mStretoatk, Stretoatk)
    
    READWRITE(int, mMergeExp, MergeExp)     //  幻兽合并升阶额外经验
    READWRITE(int, mExtlvlAdd, ExtlvlAdd)   //等级差增加值，海外版
    READWRITE(int, mMaxlvlAdd, MaxlvlAdd)   //等级上限增加值，国内版
    READWRITE(int, mGrowthAdd, GrowthAdd)   //成长值上限增加值,影响宠物洗练成长上限
    READWRITE(int, mTypeForPetMerger, TypeForPetMerger);   //用于幻兽合体的类型
    
    READWRITE(int, mDisplayInCity, DisplayInCity)
    
	READWRITE(int, mCollectID, CollectID); // 图鉴使用的模板id
    
    READWRITE(float, mSpiritSpeakCoef, SpiritSpeakCoef);
#endif

};

struct PetGrowthVal
{
    float atk;
    float def;
    float hp;
    float hit;
    float dodge;
    float cri;
};

//宠物成长属性
struct PetGrowth
{
    vector<float> atk;
    vector<float> def;
    vector<float> hp;
    vector<float> hit;
    vector<float> dodge;
    vector<float> cri;
    string        petTypeImage;   //幻兽对应类型的图片
};

//宠物升级经验
struct PetExp
{
    vector<int> exp;      //宠物升级所需经验
    vector<int> totalExp;//宠物升级所需总经验
    int maxCommonSkillCount;  //一种品质最大可以学习的普通技能数
    int maxSpecialskillCount;  //一种品质最大可以学习的特殊技能数
};



//宠物升星参数
struct PetStarCfg
{
    vector<vector<float> >   percentConversion;    //星量转移比例
    vector<vector<float> >   propertyRatio;        //属性加强百分比
    //1维品质 2维星级
};
    
//指定品质宠物升阶参数
struct PetStageQua
{
    int                 quality;                    //品质
    vector<int>         stageUpConsumeFactor;       //指定等级升阶需要的消耗金币系数
    vector<float>       stagePropertyRatio;         //指定等级升阶对战斗力加成
};


class PetCfg
{
public:
    static bool load(std::string petPath, std::string petGrowthPath,std::string petStarPath);
    static PetCfgDef* getCfg(int pettypeid);    //宠物基本属性配置
    static PetGrowthVal getGrowthVal(int type, int lvl); //根据宠物类型和等级获取宠物的成长属性系数，系数*成长值＝宠物总成长
    static int getLvlExp(int qua, int lvl);  //根据宠物等级和宠物品质获取宠物升级所需经验
    static int getLvlTotalExp(int qua, int lvl);//根据宠物等级和宠物品质获取宠物升到该等级的总经验
    static int getMaxCommmonSkills(int qua); //根据宠物品质得到宠物可学习的最大普通技能数
    static int getMaxSpecialSkills(int qua);//根据宠物品质得到宠物可学习的最大特殊技能数


    //输入品质星级，返回星量转移比例及属性加强比 qua[0,4],lvl[0,9]
    static float getPercentConversion(int qua,int starlvl);
    static float getPropertyRatio(int qua,int starlvl);
    static bool checkAbsorbAmount(int amount);
    
    //输入品质星阶，返回升阶消耗金币系数及加成比 qua[0,4],lvl[0,+)
    static int   getStageUpConsumeFactor(int qua,int stage);
    static float getStagePropertyRatio(int qua,int stage);
    static int   getMaxStageLvl(int qua);
    static std::map<int, PetGrowth*> growthcfg;
    static std::map<int, PetExp*> expcfg;
    static std::map<int, PetCfgDef*> cfg;
    static PetStarCfg petStarCfg;
    
    static std::string getPetGrowthTypeImage(int petType);
    
    
    
    static int calcAbsorbedPropPoints(int absorbedProp, const vector<float>& ownPoints,
                                      int mainProp, const vector<float>& lvlPoints);
    static int calcAbsorbedPropPoints(float absorbedPropsNum, int mainProp, const vector<float>& lvlPoints); //根据 提供的 属性点 总和 计算 主幻兽 能升级多少属性
    static float calcAbsorbedPropPointsNum(int absorbedProp, const vector<float>& ownPoints);//计算 幻兽 被吞噬 提供 多少 属性点
    static int calcMinAbsorbedPropPoints(float absorbedPropsNum, int mainProp, const vector<float>& lvlPoints);//客户端 预览 最小升级 属性点
    static int calcMaxRateAbsorbedPropPoints(float absorbedPropsNum, int mainProp,
        const vector<float>& lvlPoints);//客户端 预览 最大升级 属性点
    static vector<PetStageQua> petStageCfg;
    

protected:
    static bool loadPetGrowCfg(const char* path);      //加载宠物成长
    static bool loadPetStarCfg(const char* path);      //加载宠物升星参数
    static bool loadPetStageCfg(const char* path);     //加载宠物升阶参数
};

#pragma mark - 幻兽助阵配置表管理

struct PetAssistAddPropDef
{
    string prop;
    string name;
    PetAssistAddPropDef()
    {
        prop = "";
        name = "";
    }
};

struct PetAssistDef
{
    int position;
    int lvl ;
    int awakeLvl;
    std::vector<PetAssistAddPropDef *> petAssistAddPropDefVec;
    vector< vector<float> >ratioDatas;
    PetAssistDef()
    {
        lvl = 0;
        awakeLvl = 0;
        petAssistAddPropDefVec.clear();
        ratioDatas.clear();
    }
};
    
class PetAssistCfg
{
public:
    static void  load(string path);
    static float getStagePropRatio(int pos,int qua, int stage);
    static string getPetAssistDesc();
    static int getMaxPos();
    static PetAssistDef* getPetAssistDefByPos(int pos);
    static map<int, PetAssistDef*> &getPetAssistDefMap();
    static int getPetAssistDefLvlByPos(int pos);
    // 需要的觉醒等级
    static int getPetAssistDefAwakeLvlByPos(int pos);
    static vector<PetAssistAddPropDef *> &getPetAssistAddPropDefVecByPos(int pos);
    // 获取开放的助阵列表
    static vector<PetAssistDef *> getOpenPetAssistDefVec(int lvl, int awakeLvl);
    static bool isPositionOpen(int rolelvl, int awakeLvl, int checkPos);
private:
    static string propDec;
    static int  starinterval;
    static map<int, PetAssistDef*> petAssistDefMap;
};


class QualityColorCfg
{
public:
    static bool load(std::string fullPath);
    static string getColor(string name);
private:
    static map<std::string,string> colorCfg;
};

//地下城
struct DungeonCfgDef
{
    int dungeonid;
    int difficulty;
    int floor;
    string desc;
    int firstmap;
    int minlvl;
    int energy_cosume;
    string award;
    vector<int> rmbcost;
    string icon;
};

class DungeonCfg : public Cfgable
{
public:
    static bool load(std::string fullPath);
    static int getResetCost(int index, int times);
    static int getEnergyConsumption(int index);
    static int getEnergyConsumption(string index, int tag = 0);
    static int getEnergyConsumption(eSceneType sceneType, int index = 0);
    static vector<DungeonCfgDef *> getDungeonDatasByType(string name);  //--lihengjin
    static vector<DungeonCfgDef *> getDungeonDatasByType(eSceneType sceneType);
    static DungeonCfgDef* getDungeonCfg(eSceneType sceneType, int index);
    static DungeonCfgDef* getDungeonCfg(int index);
private:
    static map<string, vector<DungeonCfgDef*> > cfg;
    static map<int, DungeonCfgDef*> cfgIdIndex;
};
    
#pragma mark - 保卫神像配置表 defendstatue.ini

    struct monsterWave
    {
        int monsterid;
        int sum;
        int x;
        int y;
    };

struct DefenseAwardDef
{
    DefenseAwardDef()
    {
        gold_coef = exp_coef = batpoint_coef = petexp_coef = 0;
    }
    int gold_coef;
    int exp_coef;
    int batpoint_coef;
    int petexp_coef;
    string giftName;
//    string mail_title;
//    string mail_contentHead;
};

struct DefendStatueCfgDef
{
    int wave;
    string desc;
    int m_standarTime;  //基准时间
    int m_standarPoints;    //能获得的基准积分
    int m_minPoints;
    int m_maxPoints;
    int m_coefficient;
    vector<monsterWave> monsterWaves;
    
    DefenseAwardDef mAward;
};

struct FriendAdditAward
{
    FriendAdditAward()
    {
        mMin = mMax = 0;
        mIsShuzu = false;
    }
    int mMin;
    int mMax;
    bool mIsShuzu;
    vector<float> m_coef;
};

class DefendStatueCfg: public Cfgable
{
public:
    static void load(std::string fullPath);
#pragma mark 根据活动的等级123级()lvl)和波数(_wave)得到配置
    static DefendStatueCfgDef* getDefendStatueCfg(int _lvl,int _wave);
#pragma mark 根据活动的等级123级(_lvl)得到该等级的总波数
    static int getWaveSumforDefengLvl(int _lvl);
private:
    static map<int,DefendStatueCfgDef*> cfg;
};
    
#pragma mark - 试炼配置表 guildtest.ini

struct ChallengeCfgDef
{
    int wave;
    int time;  //基准时间
    int addContribute;
    int addFeats;
    int addWealth;
    int addOverBuild;
    int failSum;    //大于多少个就失败
    vector<monsterWave> monsterWaves;
    int minGuildLvl;    //进入的最小公会等级
};

struct ChallengeGuildCfg
{
    int traintNum;  //  试炼次数
    int blessingNum; // 免费祝福次数
    int blessedNum; // 被祝次数
    float blessedAward; // 祝福1次加成
    int blessingRmb; // 金钻祝福消耗金钻
    int inspireNum; // 个人鼓舞次数上限，这个不能废了，防小号刷
    int inspiredNum; // 公会被鼓舞上限
    float inspiredfighting; // 鼓舞战斗力加成
    int inspiredRmb; // 鼓舞消耗金钻
    std::string blesser_get_award; //祝福成功后的奖励
};
        
class ChallengeCfg: public Cfgable
{
public:
    ChallengeCfg();
    static void load(std::string fullPath);
#pragma mark 根据活动的等级123级()lvl)和波数(_wave)得到配置
    static ChallengeCfgDef* getChallengeCfg(int _lvl,int _wave);
#pragma mark 根据活动的等级123级(_lvl)得到该等级的总波数
    static int getWaveSumforChallengeLvl(int _lvl);

    static const ChallengeGuildCfg& getGuildDef();
private:
    static map<int,ChallengeCfgDef*> cfg;
    static ChallengeGuildCfg guildcfg;
};
        
                                
#pragma mark - 本地推送表
    struct LocalNotificationCfgDef
    {
        int ID;
        string name;
        bool isInTime;
        int weekDay;
        int time;
        string body;
        string ps;
    };
    class LocalNotificationCfg: public Cfgable
    {
    public:
        static void load(std::string fullPath);
        static LocalNotificationCfgDef* getLocalNotificationCfg(int ID);
        static map<int,LocalNotificationCfgDef*>* getCfgList();
    private:
        static map<int,LocalNotificationCfgDef*> cfg;
    };
#pragma mark -

class FriendAwardCoefCfg
{
public:
    static void load(std::string fullPath);
    static float getFriendsCountCoef(int friendcount);
    static float getFriendsIntimacyCoef(int intimacysum);
private:
    static vector<FriendAdditAward*> friend_coef;
    static vector<FriendAdditAward*> intimacy_coef;
};



//IOS客户端版本类型
static const char* IOS_CHANNEL_APPLE   = "apple";
static const char* IOS_CHANNEL_91      = "91";
static const char* IOS_CHANNEL_PPS     = "pps";
static const char* IOS_CHANNEL_TONGBU  = "tongbu";
static const char *IOS_CHANEL_UC = "uc";
static const char *IOS_CHANEL_KUAIYONG = "kuaiyong";
static const char *IOS_CHANNEL_APPLE_FANTI = "apple_fanti";


    
    
    
//充值
struct ChargeCfgDef
{
    int index;
    string product_name;
    int diamond_num;
    float rmb_num;
    bool is_hot;
    string platform;
    string productID;
};
class ChargeCfg : public Cfgable
{
public:
    static bool load(std::string fullPath);
    static vector<ChargeCfgDef*> getChargeList(string _platform);

private:
    static map<int, ChargeCfgDef*> cfg;
};
class sysTipCfg : public Cfgable
{
public:
    static bool load(std::string fullPath);
    static string getSysDesc(int index);
private:
    static map<int, string> cfg;
};
//抽奖奖品Icon
struct IconCfgDef {
    int type;
    string IConImgName;
};
class IconCfg :public Cfgable {
public:
    static bool load(std::string fullPath);
    static string getImgName(enLotteryItemType index);
private:
    static std::vector<IconCfgDef*> iconf;
};


//活动----------(加入公会活动)----------
struct ActivityCfgDef
{
    int    activityId;
    string activityName;
    string activityTopName;
    string activityTime;
    int    minLvl;
    int    sort;
    int    is_flash;
    int    isblink;  //播放光效
    string activityIcon;
    string activityContent;
    string ActivityAward;
    int     isTicket;   //是不是门票活动
    //——--------------------属于公会特殊部分--------------------------
    int isguild;//标记是否是属于公会的活动
    int isChooseMode;//普通、中等和困难 难度 模式 或者是直接一个立即前往按钮
    int isNum;//是否有次数限制
    int guildLvl;
    //--------------------------------------------------------------
};

class ActivityCfg : public Cfgable
{
public:
    static bool load(std::string fullPath);
    static ActivityCfgDef * getActivityData(int id);
    static ActivityCfgDef * getGuildActivityData(int id);//公会活动数据
    static ActivityCfgDef * getCfg(int id);
    static int getMaxActivity();
    static int getGuildMaxActivity();//公会活动
    static std::vector<ActivityCfgDef *> getAllDataBySort(std::vector<int> data);//普通活动数据获取--过滤掉公会活动
    static std::vector<ActivityCfgDef *> getAllDataBySort(std::vector<int> data ,int lvl);//普通活动数据获取--过滤掉公会活动
    static std::vector<ActivityCfgDef *> getGuildAllDataBySort(std::vector<int>data);//
    static std::vector<int> getGuildActivityTotalId();
    
    static std::vector<ActivityCfgDef*> getSynPvpDataBySort(std::vector<int> data); // 过滤后的同步竞技场数据
    static ActivityCfgDef* getSynPvpData(int id);   // 同步竞技场数据
    static vector<string> getGuildTreasureFightOpenDays();
private:
    static std::vector<ActivityCfgDef*> cfg;//普通活动
    static std::vector<ActivityCfgDef*> guildcfg;//公会活动 vector
    static std::vector<ActivityCfgDef*> synPvpcfg;  // 同步竞技场活动
    static std::vector<int> guildActivityTotalIds;//存放配置表读取出来的所有公会活动id,用于活动界面的显示
    static std::vector<string> guildTreasureFightOpenDays; // 存放公会夺宝战 周几开放
};

//世界boss奖励区间
struct WorldBossBonusCfgDef {
    int rank_bonus;//[116]; 奖励基数
    int rank_range;//[116];//{100,200,300,400,99999} 排名规则

};
struct WorldBossCfgDef {//世界boss奖励
    int bossid;        // == monsterid
    int sceneid;       // sceneid
    int passtime;      // 过关时间 seconds
    int nextBoss;      // next
    int rollBackTime;  // 回滚上一个 boss时间
    int beforeBoss;    // 上一个boss
    
    float level_factor; //基础系数,等级越高，值越大
    float batpointaward; // 战功
    float goldaward;     // 金币
    float expaward;      // 经验
    float constellaward; // 星灵
    

};
//世界boss
class WorldBossCfg : public Cfgable {
public:
    static bool load(std::string fullPath);

    static WorldBossCfgDef* getBossCfg(int bossid){
        if (bosses.find(bossid)!=bosses.end()) {
            return bosses[bossid];
        }
        return NULL;
    }

    static int getAwardBonus(int rolerank){
        for (int i=0; i < ranges.size(); i++) {
            if (rolerank <= ranges[i]->rank_range) {
                return ranges[i]->rank_bonus;
            }
        }
        return 0;
    }

    static WorldBossCfgDef* getFirstBossCfg(){
        return bosses[firstBossID];
    }
    
    //前n名奖对应物品，没有奖的话，返回0
    static int getItemAward(int rank)
    {
        map<int,int>::iterator it = itemaward.find(rank);
        if(it == itemaward.end())
            return 0;
        return it->second;
    }
    
    static int preparetime;

    static int lasthit; // 最后一击奖励
    static int clientranknum;//客服端排行
    static int reborn_seconds; //
    static int enter_lvl;//进入等级
    static float syn_seconds;//同步
	static float round_refresh_seconds; //分段伤害刷新时间
    static int damage_bonus;//伤害奖励
    static int round_max_hurt_base;//分段最大伤害基础值
    
    static float hurt_batpointaward; //伤害战功奖
    static float hurt_goldaward;   //伤害金币奖
private:
    static map<int,WorldBossCfgDef*> bosses;
    static vector<WorldBossBonusCfgDef*> ranges;
    static map<int,int> itemaward;//前n名奖对应物品
    static int firstBossID;
};    
    
//money multi lang cfg
struct MoneyMLCfg: public Cfgable
{
public:
    static void load(const string &filepath);
    static string getValue(const string &section, const string &key);
    
    typedef map<string, map<string, string> > Map;
    typedef map<string, map<string, string> >::iterator MapIterator;
private:
    static map<string, map<string, string> > _map;
};
    
struct MoneyCfgDef
{
    int    id;
    int type;
    string name;
    string moneytype;
};

class MoneyCfg
{
public:
    static void load(const string &filepath);
    
    static string getName(const char* money);
    
private:
    static map<string, MoneyCfgDef> sMoneyNames;
};

/*
struct MailMLCfg: public Cfgable
{
public: 
    static void load(const string &filepath);
    static string getValue(const string &section, const string &key);
    
    typedef map<string, map<string, string> > Map;
    typedef map<string, map<string, string> >::iterator MapIterator;
private:
    static map<string, map<string, string> > _map;
};
*/

    
struct MailFormat
{
    string sendername;
    string title;
    string content;
};
    
class MailCfg
{
public:
    static void load(const char* fullPath);
    static MailFormat* getCfg(const char* name);
private:
    static std::map<string, MailFormat*> formats;
};

/*class MLCfg
{
public:
    void load(const char* fullpath);
    string getStr(const char* key);
    void clear();
    
private:
    map<string, string> strMapML;
};*/

class StrMapCfg
{
public:
    static bool load(const char* fullPath);
    static string getString(const char* name, const char* def = "");
    
private:
    static map<string, string> strMap;
};

/*************************************  神秘副本  **************************************/
struct MysticalCopyListCfgDef
{
    MysticalCopyListCfgDef():copylist(""),
                             act_icon(""),
                             listname(""),
                             day("")
    {
        
    }
    vector<int> getOpenDays();    
    string copylist;
    string act_icon;
    string listname;
    string day;
    int openLvl;
    int copyType;
};

class MysticalCopyListCfg
{
public:
    static void load(const char* fullPath);
    static MysticalCopyListCfgDef* getCfg(int index);
public:
    static int replacePeriod;
    static int listcount;
    static int activeGroup;
    static int copyCountPerGroup;
    static int existTime;
    static vector<int> getCopyID(int index);
    static std::map<int, MysticalCopyListCfgDef*> m_copylist;
    static vector<int> m_noOpenCopyList;//保存未开放的list
    static vector<int> getNoOpenList();
    static vector<int> mOpenDays;
    static int getNoOpenListSize();
    static void findNoOpenList(vector<int> &vec);
    
};

struct MysticalMonsterDef
{
    MysticalMonsterDef()
    {
        m_Monsterid = m_Existtime = x = y = talkingid = 0;
    }
    int m_Monsterid;
    int m_Existtime;
    int x;
    int y;
    int talkingid;
    int energy_consum;
};
    
struct MysticalCopyDef
{
    MysticalCopyDef()
    {
        monsternum = 0;
        m_Monsters.clear();
        m_AppearProps.clear();
        miniLvl = 1;
    }
    int monsternum;
    std::vector<MysticalMonsterDef> m_Monsters;
    std::vector<int> m_AppearProps;
    
    std::vector<std::string> m_award;
    std::vector<int> m_AwardProps;
    int bossExistProps;
    int miniLvl;
    
    MysticalMonsterDef m_boss;
    string bossAward;
    
    std::vector<float> m_MonsterAppearDiscount;
    std::vector<float> m_BossApearDiscount;
    
    void getMysticalMonsters(int times, std::vector<MysticalMonsterDef>& monsters, MysticalMonsterDef& boss)
    {
        float monsterDiscount = getMonsterAppearDiscount(times);
        float bossDiscount = getMonsterAppearDiscount(times);
    
        for (int i = 0; i < monsternum && i < m_AppearProps.size(); i++) {
            int rol = rand()%100;
            
            if (rol <= ( m_AppearProps[i] * monsterDiscount )) {
                monsters.push_back(m_Monsters[i]);
            }
            else
            {
                MysticalMonsterDef monster;
                monster.m_Monsterid = 0;
                monsters.push_back(monster);
            }
        }
        
        int rol2 = rand()%100;
        if (rol2 <= ( bossExistProps * bossDiscount) ) {
            boss = m_boss;
        }
        else
        {
            boss.m_Monsterid = 0;
        }
    }
    
    //百分百出现隐藏 BOSS 与 哥布林
    void getMysticalMonsters(std::vector<MysticalMonsterDef>& monsters, MysticalMonsterDef& boss)
    {
        for (int i = 0; i < monsternum && i < m_AppearProps.size(); i++) {
                monsters.push_back(m_Monsters[i]);
        }
        
        boss = m_boss;
    }
    
    string getMonsterKillAward()
    {
        int pros = 0;
        for (int i = 0; i < m_AwardProps.size() && i < m_award.size(); i++) {
            pros += m_AwardProps[i];
        }
        
        if (pros) {
            int randnum = rand()%pros;
            pros = 0;
            
            for (int i = 0; i < m_AwardProps.size() && i < m_award.size(); i++) {
                pros += m_AwardProps[i];
                if (randnum < pros) {
                    return m_award[i];
                }
            }
        }
        return "";
    }
    
    MysticalMonsterDef getMonsterCfg(int monsterid)
    {
        MysticalMonsterDef def;
        for (int i = 0; i < m_Monsters.size(); i++) {
            if (m_Monsters[i].m_Monsterid == monsterid) {
                def = m_Monsters[i];
            }
        }
        return def;
    }
    
    float getMonsterAppearDiscount(int times)
    {
        if(times < 0)
        {
            return 1.0f;
        }
        
        int size = m_MonsterAppearDiscount.size();
        if (!size) {
            return 1.0f;
        }
        
        if (times < m_MonsterAppearDiscount.size()) {
            return m_MonsterAppearDiscount[times];
        }
        else
        {
            return m_MonsterAppearDiscount[size-1];
        }
    }
    int getMysticalCopyMinLvl()
    {
        return miniLvl;
    }
};

class MysticalCopyCfg
{
public:
    static void load(const char* fullPath);
    static void getMysticalMonsters(int copyid, int times, std::vector<MysticalMonsterDef>& outdata, MysticalMonsterDef& boss);
    static void getMysticalMonsters(int copyid, std::vector<MysticalMonsterDef>& outdata, MysticalMonsterDef& boss);

    static MysticalCopyDef* getcfg(int copyid);
public:
    
    static std::map<int, MysticalCopyDef*> m_MysticalCopyData;
};

// 小助手
struct HelperCfgDef{
    int eventId;        // 活动id
    string eventName;   // 
    string eventRes;    // 
    string eventTime;   // 
    string eventDes;    // 
    string eventProp;   // 
    int eventType;      // 
    int eventStars;     // 
    int eventLvl;       // 
    int eventGoto;      //
    float eventSclae;   //
};
struct HelperMenuDef{
    string menuName;    // 
    int menuType;// 
};
class HelperCfg : public Cfgable{
public:
    static void load(std::string fullPath);
    static vector<HelperCfgDef*> getHelperCfgListByType(int nType);
    static vector<HelperMenuDef*> getHelperMenuList();
    static HelperMenuDef* getHelperMenuById(int id);
    static int getHelperMenuSize();
private:
    static vector<HelperCfgDef*> mHelperCfgs;
    static vector<HelperMenuDef*> mHelperMenus;
};

/************************************ petpvp cfg ***************************************/
string petPvpType2String(PetPvpType type);
    
struct PetPvpAwardDef
{
    PetPvpAwardDef()
    {
        mMinRank = mMaxRank = 0;
        mIsShuzu = false;
    }
    int mMinRank;
    int mMaxRank;
    bool mIsShuzu;
    vector<int> mAwards;
    
};

struct PetPvpSectionCfg
{
    PetPvpSectionCfg()
    {
        mFormationSize = mLostCups = mWinHigherCups = mWinLowerCups = mFixAwardCups = 0;
    }
    
    int mMinCups;           //最低的竞技积分，由于还有一个用来兑换的积分，所以用cups来称呼
    float mMinPerCent;      //可进行换段的最低排名百分比
    int mFormationSize;
    int mLostCups;
    int mWinHigherCups;
    int mWinLowerCups;
    int mFixAwardCups;
    int mType;
    int mfixAwardPoints;
    float mRankAdditionCoef;
    int mSearchEnemyCost;
    string mDesc;
    string mIconName;
    string mItemList;
    string mName;
};
struct PetPvpCfgDef
{
    int mLostGivePoints;
    int mWinGivePoints;
    int mAwardTimes;  //奖励次数
    int mEnemyGetPoints;
    int mOpenLvl;
	float mPetMaxHpCoef;
    int mLimitCups;       //分段是高于这个竞技积分才纳入统计
    
    vector<pair<int, int> > mPetCounts;  //roleLvl, petcounts
    int mEnterTime; //进入次数

};
class PetPvpCfg
{
public:
    static void load(std::string fullPath);
    static int getRankAward(int playerRank);
    static PetPvpSectionCfg* getPetPvpSectionCfg(PetPvpType type);
    static int getPetFormSize(int playerLvl);
    static int getPetFormLvl(int petNum);
    static PetPvpType calPlayerPetpvpType(int playerPoint, int playerGlobalRank, int playerCounts);
    static int getLimitCups();
    static int getEnterTimes(){return mCfgData.mEnterTime;}

private:
    static std::vector<PetPvpAwardDef*> mRankAward;
    static std::vector<PetPvpSectionCfg*> mSectionsInfo;
public:
    static PetPvpCfgDef mCfgData;
};
    
struct PetPvpItem
{
    PetPvpItem()
    {
        mItemid = mConsumPoints = 0;
    }
    int mItemid;
    string mItemName;
    int mConsumPoints;
    int mNeedDan;
    string mDesc;
};
    
class PetPvpRedemptionCfg
{
public:
    static void load(std::string fullpath);
    static int consumPoints(int itemid);
    static string getItemName(int itemid);
    static std::map<int, PetPvpItem*>& getAllItems();
    static std::vector<PetPvpItem*> getItems(PetPvpType type);
private:
    static std::map<int, PetPvpItem*> mItems;
    static std::map<int, std::vector<PetPvpItem*> > mSectionItems;
};
    
    

struct TlkMonsterCfgDef
{
public:
    int mMonsterMod;
    int mUnitNum;
    int mScore;
    float mScale;
};
    
struct TimeLimitKillCopyAward
{
public:
    int score;
    string award;
    string desc;
};

struct TimeLimitKillCopyCfgDef
{
public:
    int mLastTime;
    int mRefreshTime;
    int mMaxMonsterNum;
    vector<TimeLimitKillCopyAward> mAwards;
    vector<vector<TlkMonsterCfgDef> > mGroupMonsters;
    vector<int> mGroupScores;
};
    
class TimeLimitKillCopyCfg
{
public:
    static bool load(std::string fullpath);
    
    static TimeLimitKillCopyCfgDef* getCfg(int copyId)
    {
        map<int, TimeLimitKillCopyCfgDef*>::iterator iter = sCfg.find(copyId);
        if (iter == sCfg.end())
        {
            return NULL;
        }
        
        return iter->second;
    }
    
    static vector<int> getAllMonsterModByMap(int copyid);
    static TlkMonsterCfgDef * getTlkMonsterCfg(int copyId,int mod);
    
private:
    static map<int, TimeLimitKillCopyCfgDef*> sCfg;
};

    
#pragma mark - 名人堂配置表
struct FamousCfgDef
{
    FamousCfgDef()
    {
        _id = 0;
        default_lvl = default_roletplt = default_lvl = default_weapon = 0;
        default_name = "";
        hp = atk = def = hit = dodge = cri = 0;
    }
    int _id;
    string default_name;
    int default_lvl;
    int default_roletplt;
    int default_weapon;
    int hp;
    int atk;
    int def;
    int hit;
    int dodge;
    int cri;
    vector<int> skills;
};
    
struct FamousDungeCfgDef
{
    int wave;
    string dungeName;
    int limitTime;
    string award;
};

struct FamousHeroBaseDef
{
    FamousHeroBaseDef()
    :_id(0)
    ,_name("")
    ,_roleTplt(0)
    ,_weaponQue(0){}
    int _id;
    string _name;
    int _roleTplt;
    int _weaponQue;
};

class FamousCfg
{
public:
    static void load(std::string fullpath);
    FamousCfg(){}
    virtual ~FamousCfg(){}

    static FamousCfgDef* getFamousCfg(int _id);
    static FamousDungeCfgDef* getFamousDungeCfg(int _wave);
    static FamousCfgDef* getFamousCfgNew(int _job, int _startLayer, int _pos);
private:
    static map<int,FamousCfgDef*> cfg;
    static map<int,FamousDungeCfgDef*> famousdungecfg;
};

struct FamousDataCfgDef
{
    FamousDataCfgDef()
    {
        fameHallList = "";
        famesHallLayers = 0;
        famesChallengeLayers = 0;
        famesNeedPassLayers = 0;
        famesNextAddLayers = 0;
    }
    string fameHallList;
    int famesHallLayers;             // 总的名人堂层数
    int famesChallengeLayers;        // 每轮名人堂需挑战的层数
    int famesNeedPassLayers;         // 通过每轮需要挑战的层数
    int famesNextAddLayers;          // 下轮挑战从几层后开始
};

class FamousDataCfg
{
public:
    static void load(std::string fullpath);
public:
    static FamousDataCfgDef mData;
    static FamousHeroBaseDef* getFamousHeroBaseDef(int _job, int _pos);
    static FamousHeroBaseDef* getFamousHeroBaseDef(int _id);
private:
    static map<int, FamousHeroBaseDef*> mFamousHeroBaseDefs;
};

    
#pragma mark -------------------限时活动地图的配置表------------------
struct TicketTypeDef
{
    int activityid;
    string activityDesc;
    string activityName;
};
struct LimitActivityDef
{
    int mapid;
    string name;
    std::vector<std::string> costItmem;    //消耗物品的字符串
    int minLvl;
};
class LimitActivityCfg
{
public:
    static void load(std::string fullpath);
    static TicketTypeDef *  getTicketTypeByActivityID(int activityID);  //得到活动表述
    static vector<LimitActivityDef *>  getAllLimitActivityDatas(int activityID); //根据活动id得到对应的副本
    static LimitActivityDef * getLimitByMapID(int mapID);  //根据地图得到对应的信息
    static int  getActivityIdByTicketId(int ticketId);            //通过门票id 得到活动id
    
    static vector<TicketTypeDef *>  ticketTypeDatas;
    static map<int,vector<LimitActivityDef *> > limitActivityDatas;

};

#pragma mark -------------------防作弊相关配置表------------------
//-----------------------linshusen
class AntiClientCheatCfg
{
public:
    static void load(std::string fullpath);
public:
    static float sHeatBeatIntervalTime;     //心跳的间隔时间
    static float sCommunicationTime;     //客户端和服务器通信需要的最长时间
    static int sAcclerateKickSecond; // 客户端时间比服务器大x秒被踢
    static int sFoceClientSyncSecond; // 客户端时间比服务器小x秒,强制同步客户端时间
    static int sSyncClientInterval; // 同步客户端间隔时间
    static float sSyncNetworkStuckTime; //同步战斗的时候，当超过这个时间没有收到服务器消息时，认为客户端网络很卡（单位秒）
};

#pragma mark -----------------通关副本战斗力验证配置表-------------
//wangzhigang
struct CompatPowerVerifyCfgDef
{
    CompatPowerVerifyCfgDef()
    {
        minPercent = maxPercent = 0;
        timeMulti = cheatPercent = 0.0f;
    }
    int minPercent;     //低于推荐战斗力的最少百分比
    int maxPercent;     //低于推荐战斗力的最大百分比
    float timeMulti;    //参考时间相对于标准时间的倍数
    float cheatPercent; //判定为作弊的概率
};
    
class CompatPowerVerifyCfg
{
public:
    static void load(std::string fullpath);
    static CompatPowerVerifyCfgDef* getCfg(int cmpPercent); //根据低于推荐战斗力的百分比查找配置
private:
    static std::vector<CompatPowerVerifyCfgDef*> mCfg;
};
#pragma mark -----------------虚拟物品资源-------------
struct moneyRes
{
    moneyRes()
    :mId(0)
    ,mType(0)
    ,mName("")
    ,mIcon("")
    ,mMoneyType("")
    {
    
    }
    int mId;
    int mType;
    std::string mName;
    std::string mIcon;
    std::string mMoneyType;
};
class moneyResCfg
{
public:
    static void load(std::string fullpath);
    static std::string getIconByName(std::string sName);
    static moneyRes* getMoneyRes(std::string sName);
    typedef std::map<std::string, moneyRes*> Props;
    static std::string getMoneyName(std::string sName);
private:
    static Props mMoneyResMap;
};
class autoBattleCfg
{
public:
    static void load(std::string fullpath);
    static int getOpenBattleVipLvl(int sceneType); 
private:
      static map<int,int> autoBattleDatas;
};

struct GameFunctionCfgDef
{
    GameFunctionCfgDef():equipType(0),
                         mysticalcopyType(0),
                         vipEffectShopType(0),
                         krLvlRewardType(0),
                         leveldifferenceType(0),
                         activitychooseType(0),
                        bBSUsingType(0),
                        chargeInterface(0),
                        petCanMergeType(0),
                        weChatShare(""),
						limitTimeKilled(0),
                        friendInvitation(0)
    {
        
    }
    bool isEmpty()
    {
        return ( equipType == eNonType || mysticalcopyType == eNonType || vipEffectShopType == eNonType || krLvlRewardType == eNonType || leveldifferenceType == eNonType || activitychooseType == eNonType || bBSUsingType == eNonType || chargeInterface == eNonType || petCanMergeType == eNonType || weChatShare == "" || limitTimeKilled == eNonType || friendInvitation == eNonType);
    }
    int equipType;
    int mysticalcopyType;
    int vipEffectShopType;
    int krLvlRewardType;
    int leveldifferenceType;
    int activitychooseType;
    int bBSUsingType;
    int chargeInterface;
    int petCanMergeType;
    std::string weChatShare;
	int limitTimeKilled;
    int friendInvitation;
};
    
class GameFunctionCfg
{
public:
    static void load(std::string fullpath);
    
    static GameFunctionCfgDef getGameFunctionCfg()
    {
        return functionData;
    }
    static bool isWeChatShare();
private:
    static map<string, int> functionDatas;
    
    static GameFunctionCfgDef functionData;
    
};
    
class KoreaPlatformJump
{
public:
    static void load(std::string fullpath);
    static string getUrl(string platform);
private:
    static map<string, string> platformUrls;
};
// 2.0 普通活动 配置表 daily_activity.ini
struct OrdinaryActivityCfgDef
{
    int    activityId;
    string activityName;
    //string activityTopName;
    int    minLvl;
    int    sort;
    int    type;             // tpye : 1 装备活动 / 2 幻兽活动
    string activityIcon;
    string activityContent;
    string activityAward;
    int    is_ticket;        //是否为门票活动
    
};

class OrdinaryActivityCfg :public Cfgable
{
public:
    static bool load(std::string fullPath);
    static OrdinaryActivityCfgDef * getEquiqActivityData(int id);
    static OrdinaryActivityCfgDef * getPetActivityData(int id);
    static OrdinaryActivityCfgDef * getCfg(int id);
    static int getMaxEquipActivity();
    static int getMaxPetActivity();
    static std::vector<OrdinaryActivityCfgDef *> getSortedEquipActDatas(std::vector<int> data);
    static std::vector<OrdinaryActivityCfgDef *> getSortedPetActDatas(std::vector<int> data);
    static std::vector<int> getTotalEquipActIds();
    static std::vector<int> getTotalPetActIds();
private:
    static std::vector<OrdinaryActivityCfgDef *> equipCfg;
    static std::vector<OrdinaryActivityCfgDef *> petCfg;
    static std::vector<int> equipCfgIds;
    static std::vector<int> petCfgIds;
};


#pragma mark- 主界面 UI 活动图标 配置
struct UiIconSortCfgDef {
    int    id;
    string img;
    
    UiIconSortCfgDef()
    {
        id = 0;
        img  = "";
    }
};
class UiIconSortCfg {
private:
    static std::map<int,UiIconSortCfgDef*> uiIconData;
public:
    static bool load(std::string fullpath);
    static bool findUiIconByString(std::string name);
    static int getDataSize();
    static std::map<int,UiIconSortCfgDef*> getUiIconMap();
};

struct NetWorkStateDef
{
    int minPing;
    int maxPing;
    string desc;
    string colorStr;
    NetWorkStateDef()
    {
        minPing = 0;
        maxPing = 0;
        desc = " ";
        colorStr = "255,255,255";
    }
};
class NetWorkStateCfg
{
private:
    static vector<NetWorkStateDef *> cfg;
public:
    static bool load(std::string fullpath);
    static NetWorkStateDef *  getNetWorkStateDesc(float value);
};

#pragma mark- 娱乐活动 UI 界面图标 配置
struct FunnyActCfgDef{
    
    FunnyActCfgDef(){
        funnyActId = 0;
        minLvl = 0;
        funnyActType="";
        funnyActName="";
        funnyActImage="";
        funnyActDesc ="";
    }
    int funnyActId;
    int minLvl;
    string funnyActType;
    string funnyActName;
    string funnyActImage;
    string funnyActDesc;
    
};
class FunnyActCfg : public Cfgable{
public:
    static bool load(std::string fullPath);
    static vector<FunnyActCfgDef*> getFunnyActList();
    static bool checkMinLvl(string actType , int lvl);
    static FunnyActCfgDef* getFunnyActByIndex(int index);
    static int getFunnyActListSize();
private:
    static vector<FunnyActCfgDef*> mFunnyActCfgs;
};

class AndroidFantiCfg : public Cfgable {
public:
    static void load(const char* fullPath);
    static std::string googlePayURL;
    static std::string clientPartnerName;   //合作商
    static int openMyCardPayNeedLevel;
};

struct WardrobeCfgDef
{
    WardrobeCfgDef();
#ifdef CLIENT
    ReadWrite_Crypt(int, mNeedexp,Needexp)
    ReadWrite_Crypt(int, mHp, Hp)
    ReadWrite_Crypt(int, mDef, Def)
    ReadWrite_Crypt(int, mAtk, Atk)
    ReadWrite_Crypt(float, mHit, Hit)
    ReadWrite_Crypt(float, mDodge, Dodge)
    ReadWrite_Crypt(float, mCri, Cri)
//    ReadWrite_Crypt(int, mStre, Stre)
//    ReadWrite_Crypt(int, mInte, Inte)
//    ReadWrite_Crypt(int, mPhys, Phys)
//    ReadWrite_Crypt(int, mCapa, Capa)
#else
    READWRITE(int, mNeedexp,Needexp)
    READWRITE(int, mHp, Hp)
    READWRITE(int, mDef, Def)
    READWRITE(int, mAtk, Atk)
    READWRITE(float, mHit, Hit)
    READWRITE(float, mDodge, Dodge)
    READWRITE(float, mCri, Cri)
//    READWRITE(int, mStre, Stre)
//    READWRITE(int, mInte, Inte)
//    READWRITE(int, mPhys, Phys)
//    READWRITE(int, mCapa, Capa)
    
    READWRITE(int, mAccHp, AccHp)
    READWRITE(int, mAccDef, AccDef)
    READWRITE(int, mAccAtk, AccAtk)
    READWRITE(float, mAccHit, AccHit)
    READWRITE(float, mAccDodge, AccDodge)
    READWRITE(float, mAccCri, AccCri)
//    READWRITE(int, mAccStre, AccStre)
//    READWRITE(int, mAccInte, AccInte)
//    READWRITE(int, mAccPhys, AccPhys)
//    READWRITE(int, mAccCapa, AccCapa)
#endif
};
class WardrobeCfg : public Cfgable{
public:
    static bool load(std::string fullPath);
    static WardrobeCfgDef * getWardrobeCfg(int lvl);
    static int getMaxLvl();
private:
    static vector<WardrobeCfgDef*> cfg;
    static int maxLvl;
};

#pragma mark - 月卡系统
class MonthCardCfgDef
{
public:
    MonthCardCfgDef();
#ifdef CLIENT
    ReadWrite_Crypt(int, mIndex, Index);
    ReadWrite_Crypt(int, mDayIdx, DayIdx);
    ReadWrite_Crypt(int, mAwardId, AwardIdx);
#else
    READWRITE(int, mIndex, Index);
    READWRITE(int, mDayIdx, DayIdx);
    READWRITE(int, mAwardId, AwardIdx);
#endif
};
class MonthCardCfg
{
    vector<MonthCardCfgDef*> monthCardAwards;
public:
    
    MonthCardCfg();
    
    typedef vector<MonthCardCfgDef* >::iterator Iterator;
    Iterator begin(){ return monthCardAwards.begin(); }
    Iterator end(){ return monthCardAwards.end(); }
    bool load(string fullPath);
    int getSize(){  return monthCardAwards.size(); }
    MonthCardCfgDef* getMonthCardDefByIdx(int idx);
#ifdef CLIENT
    ReadWrite_Crypt(int, mNeedRmb, NeedRmb);
    ReadWrite_Crypt(int, mTotalCount, TotalCount);
    ReadWrite_Crypt(int, mAwardNum, AwardNum);
    ReadWrite_Crypt(int, mStartTime, StartTime);
    ReadWrite_Crypt(int, mEndTime, EndTime);
#else
    READWRITE(int, mNeedRmb, NeedRmb);
    READWRITE(int, mTotalCount, TotalCount);
    READWRITE(int, mAwardNum, AwardNum);
    READWRITE(int, mStartTime, StartTime);
    READWRITE(int, mEndTime, EndTime);
#endif
    void pushMonthCardDef(MonthCardCfgDef* def);
};
class MonthCardCfgMgr
{
public:
    static bool load(string fullPath);
    static MonthCardCfg* getMonthCardCfgById(int id);
    static MonthCardCfg* getMonthCardCfgByTime(int tm);
private:
    static map<int, MonthCardCfg*> monthCardCfgs;
};

/********
 同步组队副本
 *******/
struct SyncTeamCopyCfgDef
{
    int scenemod;
    int constructionAward;       // 公会建设度
    int fortuneAward;            // 公会财富
    int goldAward;
    int battlepointAward;
};

//副本列表
class SyncTeamCopyCfg
{
public:
    static void load(string fullpath);
    static SyncTeamCopyCfgDef* getCfg(int sceneMod);
    static void getAllCopyList(vector<int>& outdata);
private:
    static map<int, SyncTeamCopyCfgDef*> mSyncCopys;
    
};

struct SyncTeamCopyFunctionCfgDef
{
    int teamMemberCount;
    int incomeTimes;
};

//功能相关
class SyncTeamCopyFunctionCfg
{
public:
    static void load(string fullpath);
    static int getTeamMemberCount();
    static int getIncomeTimes();
    static float getIncomeMultiple(int intimacysum);
private:
    static SyncTeamCopyFunctionCfgDef mCfg;
    static vector<int> mIntimacyBase;
    static vector<float> mAwardMultiple;
};
    
class ReservedNameMgr
{
public:
    static bool load(const char* fullpath);
    static bool add(const char* name);
    static bool find(const char* name);
    
    static string sDefaultRobotFriend;
private:
    
    static set<string> sNames;
    
};
    

//装备模型对应的颜色
class EquipmodeCorrespondedColor
{
public:
    static void load(string fullpath);
    static string getColorStrByEquipmode(int mode);
private:
    static map<int, string> datas;
};  

//检验与服务器连接状态
class LinkStateCheckCfg
{
public:
    static void load(string fullpath);
    static int mIsCheckON;
    static float mCheckReqDelta;
    static float mRespondWaitDelta;
};

struct PetEvolutionMaterial
{
    PetEvolutionMaterial(): tmpid(0),
                                lvl(0),
                                starlvl(0),
                                prop(0),
                                stage(0),
                                quality(0),
                                restitution(0)
    {

    }
    
    int tmpid;
    int lvl;            //等级
    int starlvl;        //星数
    int prop;           //属性--力+智+体+耐
    int stage;          //阶数
    int quality;        //品质
    int restitution;    //是否有进化补偿

    vector<int> skills;
};

struct PetEvolutionNode
{
    int tmpid;      //monster_tplid * 10 + qua;
    
    typedef pair<PetEvolutionMaterial*, PetEvolutionMaterial*> MaterialPair;
    
    //int = targetTmpId, pair->first:material self, pair->second:otherMaterial
    map<int, vector< MaterialPair > > targets;

    vector< MaterialPair > material;
    
    vector<pair<int,MaterialPair> > getAllEvolutionRelation();   //得到所有的映射的关系
    vector< MaterialPair > getMaterialByTarget(int targetid);  //通过目标id 得到需要的另外材料
    string  getWayDesc;  //获取方式的描述

};

class PetEvolutionCfgMgr
{
    struct NodeTmp
    {
        NodeTmp() :tmpid(0),
                   checked(false)
        {
            material.clear();
            targets.clear();
        }
        
        bool isTargetExist(int targetModId)
        {
            for (int i = 0; i < targets.size(); i++) {
                if (targets[i].first == targetModId) {
                    return true;
                }
            }
            
            return false;
        }
        
        int tmpid;      //monster_tplid * 10 + qua;
        vector<pair<int, int> > material;
        vector<pair<int, int> > targets;    //modid， typeid
        string  getWayDesc;  //获取方式的描述
        
        bool checked;
    };
    
    static bool checkOtherNodeExistTarget(int evolutionTypeid, int targetid, map<int, PetEvolutionCfgMgr::NodeTmp> &nodesTmp)
    {
        map<int, PetEvolutionCfgMgr::NodeTmp>::iterator iter = nodesTmp.find(evolutionTypeid);
        
        if (iter == nodesTmp.end()) {
            return false;
        }
    
        if (iter->second.isTargetExist(targetid))
        {
            return true;
        }
    
        return false;
    }
    
    static bool checkEvolutionCircuit(map<int, PetEvolutionCfgMgr::NodeTmp> &nodesTmp);
    
    static bool onCheckEvolutionCircuit(PetEvolutionCfgMgr::NodeTmp& node, map<int, PetEvolutionCfgMgr::NodeTmp> &nodesTmp);
public:
    static void load(string nodeFile, string materialFile);
    static PetEvolutionMaterial* getMaterial(int materialId);
    static PetEvolutionNode* getNode(int tmpid);
    
    static PetEvolutionNode::MaterialPair getMaterials(int targetid, int firstTypeid, int secondTypeid);

    static PetEvolutionNode *getPetEvolutionNodeByPetTmpid(int petTmpid);  //通过幻兽模板id  得到 PetEvolutionNode

private:
    static void makeNodes(map<int, NodeTmp>& nodesTmp);
    
    static map<int, PetEvolutionMaterial*> mMaterial;
    static map<int, PetEvolutionNode*> mNodes;
    
    static vector<int> mCircuit;
};

struct PetEvolutionRestitutionItem
{
    int itemId;
    int refValue;
};

struct PetEvolutionRestitutionCfgDef
{
    float mMergeExpRestitutionPercent;
    float mPagesRestitutionPercent;
    float mGoldRestitutionPercent;
    
    typedef vector<PetEvolutionRestitutionItem*> SkillBookRestitutionArray;
    
    map<int, PetEvolutionRestitutionItem*> mPetEgg;         //补偿经验
    map<int, PetEvolutionRestitutionItem*> mSkillBooks;
    map<int, PetEvolutionRestitutionItem*> mStagePetEggs;   //补偿星级哥布林
};

class PetEvolutionRestitutionCfg
{
public:
    static void load(string fullPath);
    static float getMergeExpPercent();
    static float getPagesPercent();
    static float getGoldPercent();
    static PetEvolutionRestitutionItem* getPetEggCfg(int quality);
    static PetEvolutionRestitutionItem* getSkillBookCfg(int quality);
    static PetEvolutionRestitutionItem* getStagePetEggCfg(int quality);
private:
    static PetEvolutionRestitutionCfgDef mCfg;
};

#pragma mark - 武器附魔系统
struct EnchantPropertyAddDef
{
    EnchantPropertyAddDef()
    {
        setHp(0);
        setAtk(0);
        setDef(0);
        setDodge(0.0f);
    }
#ifdef CLIENT
    ReadWrite_Crypt(int, mHp, Hp)        // 生命
    ReadWrite_Crypt(int, mAtk, Atk)      // 攻击
    ReadWrite_Crypt(int, mDef, Def)      // 防御
    ReadWrite_Crypt(float, mDodge, Dodge)// 防御
#else
    READWRITE(int, mHp, Hp)
    READWRITE(int, mAtk, Atk)
    READWRITE(int, mDef, Def)
    READWRITE(float, mDodge, Dodge)
#endif

};
struct WeaponEnchantCfgDef
{
    WeaponEnchantCfgDef()
    {
        typeId=0;
        maxLvl = 0;
        icon = "";
        costVec.clear();
        resourceMap.clear();
        skillIds.clear();
        probabilitys.clear();
        enchantPropertyAddDefVec.clear();
        desc="";
    }
    int typeId;                // typeId
    int maxLvl;                // 最高等级
    string icon;            // 附魔图标
    vector<int> costVec;       // 升每等级消耗的货币的个数
    map< ObjJob, vector<int> > resourceMap;
    vector<int> skillIds;      // 每级被动技能id
    vector<float> probabilitys;// 被动技能触发几率
    vector<EnchantPropertyAddDef *> enchantPropertyAddDefVec; // 每升一级的战斗属性的加成
    std::string desc;
    // 每一级的花费货币
    int getCost(int enchantLvl);
    // 当前级光效资源id
    int getResourcesId(int enchantLvl,ObjJob objJob);
    // 当前级技能id
    int getSkillid(int enchantLvl);
    // 当前级被动技能触发几率
    float getProbability(int enchantLvl);
    // 当前级战斗属性加成
    EnchantPropertyAddDef* getEnchantPropertyAddDef(int enchantLvl);
	
	bool isMax(int lvl) {return lvl >= maxLvl;}
};
class WeaponEnchantCfg:public Cfgable
{
public:
    static void load(std::string fullpath);
    // 获取一种武器附魔全部数据
    static WeaponEnchantCfgDef* getWeaponEnchantCfgDef(int typeId);
    // 获取某种武器附魔每一级花费的货币
    static int getCost(int typeId,int enchantLvl);
    // 获取某种武器附魔当前光效资源id
    static int getResourcesId(int typeId,int enchantLvl,ObjJob objJob);
    // 获取某种武器附魔当前级技能id
    static int getSkillid(int typeId,int enchantLvl);
    // 获取某种武器附魔当前级被动技能触发几率
    static float getProbability(int typeId,int enchantLvl);
    // 获取某种武器附魔当前级战斗属性加成
    static  EnchantPropertyAddDef* getEnchantPropertyAddDef(int typeId,int enchantLvl);
    // 获取某种武器附魔升到当前级战斗属性加成总和
    static void getAllEnchantPropertyAddDef(int typeId, int enchantLvl,int &hp,int &atk,int &def, float &Dodge);

    STATICREADWRITE(int, mTypeNum, TypeNum);            // 附魔种类
    STATICREADWRITE(int, mEffectValue,EffectValue);     // 生效数值
    STATICREADWRITE(float, mLowestPro,LowestPro);       // 最低概率
    STATICREADWRITE(int, mEnchantTimes,EnchantTimes);   // 连续执行此次
    
    static map<int,WeaponEnchantCfgDef *> &getEnchantCfgData();
private:
    static map<int,WeaponEnchantCfgDef *> cfg;
};

struct ItemCell
{
    ItemCell(): itemid(0),
                count(0),
                externDes("")
    {
        
    }
    
    ItemCell(const ItemCell& src)
    {
        this->itemid = src.itemid;
        this->count = src.count;
        this->externDes = src.externDes;
    }
    
    ItemCell(int itemid, int count):itemid(itemid),
                                    count(count)
    {
        
    }
    int itemid;
    int count;
    string externDes;
};

//时装搜集 类型

struct FashionCollectCfgDef
{
    FashionCollectType fashionType;
    int fashionObjectId;   // Id格式AABBxx：AA ：类型，BB ：不同品种 xx ：第几阶
    int stage;
    
    vector<ItemCell> fashionProductId;
    vector<ItemCell> awardGift;
    
    BattleProp mPropAward;
    BattleProp mMaterialPropAdded;
    
    vector<ItemCell> materialIds;
    vector<ItemCell> shareMaterialIds;
    
//    ItemCell getProduct(int rolejob)
//    {
//        if(fashionProductId.size() <= 0)
//        {
//            ItemCell nillCell;
//            return nillCell;
//        }
//        
//        if(rolejob < 0 || rolejob >= fashionProductId.size())
//        {
//            return fashionProductId[0];
//        }
//        
//        return fashionProductId[rolejob];
//    }

};

struct TargetOfFashionCollectMaterial
{
    int targetid;
    int count;
    int grid;
};

typedef vector<TargetOfFashionCollectMaterial*> FashionCollectMaterialInfo;

class FashionCollectCfg :public Cfgable
{

public:

    FashionCollectCfg(){}
    virtual ~FashionCollectCfg(){}
    
    static void load(std::string fullPath);
    
    static FashionCollectCfgDef * getCfg(int _id);
    
    static map<int, FashionCollectCfgDef*> & getAllFashionDatas();
    static vector<int> getFashionCollectItemByType(int type);//基础时装，热门武器，热门全身
    
    static FashionCollectMaterialInfo getMaterialTargets(int materialid);
    
    static TargetOfFashionCollectMaterial* getMaterialInfowithTarget(int materialid, int targetid);
    
    static vector<FashionCollectCfgDef*>& getFirstStageIds();
    
private:
    static void addMaterialInfo(int materialId, TargetOfFashionCollectMaterial* material);
    
private:
    static map<int , FashionCollectCfgDef*> cfg;
    static map<int, FashionCollectMaterialInfo> mMaterialInfo;
    static vector<FashionCollectCfgDef*> mFirstStageIds;
};

struct FashionMaterialRecycleCfgDef
{
    FashionMaterialRecycleCfgDef(): lvl(0),
                                    exp(0),
                                    parameter(0.0f)
    {
        
    }
    int lvl;
    int exp;
    float parameter;
};

class FashionMaterialRecycleCfg
{
public:
    static void load(std::string fullPath);
    static FashionMaterialRecycleCfgDef* getCfg(int lvl);
private:
    static map<int, FashionMaterialRecycleCfgDef*> mCfg;
};

/*******************************************/

struct SynthesisCfgDef
{
    int synthesisID;
    int needLvl;
    int needGold;
    vector<ItemCell> materials;
    vector<ItemCell> synthetics;
    void checkSameMaterial();
    void checkSameSynthetic();
};


class SynthesisCfg :public Cfgable
{
public:
    
    SynthesisCfg(){}
    virtual ~SynthesisCfg(){}
    static void load(std::string fullPath);
    static vector<SynthesisCfgDef *> getCfgByMaterial(int materialid);
    static vector<SynthesisCfgDef> getSortCfgByMaterial(int materialid);  //这个是排序后的返回值  为了不操作原始指针  返回对象
    static SynthesisCfgDef * getSynthesisCfg(int synthesisid);
    static vector<int>   getAllMaterialIDs();
    static vector<int>   getAllSynthesisByMaterialID(int materialid);
private:
    static void transformDatas(vector<SynthesisCfgDef *> & datas);
    static bool   SynthesisCfgIsSample(SynthesisCfgDef *def1 ,SynthesisCfgDef * def2);
    static void  checkSameMaterialAndSynthetic(vector<SynthesisCfgDef *> datas, SynthesisCfgDef * def);
    static void checkAndAddMaterialID(vector<int> & materialids, int itemid);
    static vector<ItemCell > sortSynthesis(SynthesisCfgDef * data, int materialid); //把所需要的材料 把materialid放到最前面
    static bool isFindMaterialid(SynthesisCfgDef * data, int materialid);
private:
    static vector<SynthesisCfgDef *> synthesisCfg;
    static map<int, vector<SynthesisCfgDef *> > cfg;
    
};

//-----幻兽大冒险------
struct PetAdventureCfgDef
{
    PetAdventureCfgDef()
    {
        setSceneid(0);
        setHp(0);
        setHpTimes(1.0);
        setAttack(0);
        setAttackTimes(1.0);
        setDefence(0);
        setDefenceTimes(1.0);
        setHit(0);
        setHitTimes(1.0);
        setDodge(0);
        setDodgeTimes(1.0);
        setSkill(0);
        setSkillTimes(1.0);
    }
    int begin_time;
    int end_time;
    vector<int> recommended_pet;
    PetAdventureType    pet_adventure_type;

    READWRITE(float, mHpTimes, HpTimes);
    READWRITE(float, mAttackTimes, AttackTimes);
    READWRITE(float, mDefenceTimes, DefenceTimes);
    READWRITE(float, mHitTimes, HitTimes);
    READWRITE(float, mDodgeTimes, DodgeTimes);
    READWRITE(float, mSkillTimes, SkillTimes);
    
#ifdef CLIENT
    ReadWrite_Crypt(int, mSceneid, Sceneid);
    ReadWrite_Crypt(int, mHp, Hp);
    ReadWrite_Crypt(int, mAttack, Attack);
    ReadWrite_Crypt(int, mDefence, Defence);
    ReadWrite_Crypt(int, mHit, Hit);
    ReadWrite_Crypt(int, mDodge, Dodge);
    ReadWrite_Crypt(int, mSkill, Skill);
#else
    READWRITE(int, mSceneid, Sceneid);
    READWRITE(int, mHp, Hp);
    READWRITE(int, mAttack, Attack);
    READWRITE(int, mDefence, Defence);
    READWRITE(int, mHit, Hit);
    READWRITE(int, mDodge, Dodge);
    READWRITE(int, mSkill, Skill);
#endif

//  额外奖励

    int extra_gold;
    int extra_exp;
    int extra_battlepoint;
    int extra_consval;
    int extra_petexp;
    float reward_rate;
    string extra_fall_items;

};

class PetAdventureCfg : public Cfgable
{
public:
    static void load(std::string fullPath);
    
    static PetAdventureCfgDef* getPetAdventureCfgDef(int id);

    static void getPetAdventureNormalCfgDef();
    
    static vector<int>  getCurPetAdventureData(int sceneId); //获取当前副本推荐幻兽的数据
    static void getPetAdventureCopy(); //获取所有的幻兽大冒险副本
    
    static vector<PetAdventureCfgDef*> getNormalCopy(); // 获取普通副本(一般)
    static vector<PetAdventureCfgDef *> getActivityCopy(); //获取活动副本(包含永久和限时)

private:
    static map<int, PetAdventureCfgDef*> cfg;

};


//---------------------------------------------------------
//幻兽大冒险机器幻兽配置
class PetAdventureRobotPetsForRentCfgDef
{
public:
    
    int growth;
    int sceneId;
    
    BattleProp batprop;
    
    vector<int> skillsLvl;
    
};

class PetAdventureRobotPetsForRentCfg : public Cfgable
{
public:
    
    static void load(std::string fullPath);
    
    static PetAdventureRobotPetsForRentCfgDef* getPetAdventurePetRobotCfgDef(int id);
    static vector<int> getPetAdventureRobotPetSceneIdList();
    
private:
    static map<int, PetAdventureRobotPetsForRentCfgDef*> cfg;
};

//---------------------------------------------------------


//---------跨服战-------------
struct CrossServiceWarWorshipCfgDef
{
    CrossServiceWarWorshipCfgDef()
    {
        setCrossServiceWarId(0);
    }
    double  specialbattlepointplus;
    double  commonbattlepointplus;
    
    std::string awards;
    std::string worshiptimes;
    std::string cspvptimes;
    READWIRTE(int, mCrossServiceWarId, CrossServiceWarId);
};

struct CrossServiceWarRankingserviceCfgDef
{
    string index;
    string awards;
};

struct CrossServiceWarRankingsimpleCfgDef
{
    string index;
    std::string  awards;

};

struct CsPvpPeriod
{
    int mWeekBegin;
    int mWeekEnd;
};

//膜拜
class CrossServiceWarWorshipCfg : public Cfgable
{
public:
     static void load(std::string fullPath);
public:
    static map<int, CrossServiceWarWorshipCfgDef*> worshipcfg;
    static int  getOpenLvl();


public:
    static bool isWorshipEnd(int wday, int hour, int min);
    static bool canWorship(int wday, int hour, int min);
    static bool isBattleEnd(int wday, int hour, int min);
    static bool canBattle(int wday, int hour, int min);

    static int genWeekTimeCmp(int wday, int hour, int min);

    static float sCommonWorshipPlus;
    static float sSpecialWorshipPlus;
    static CsPvpPeriod sWorshipPeriod;
    static CsPvpPeriod sBattlePeriod;
    static vector<string> sAwards;
    static int openLvl;
    
};
//服务器排名
class CrossServiceWarRankingserviceCfg : public Cfgable
{
public:
    static void load(std::string fullPath);
public:
    static vector<pair<int, int> > sRankIntervals;
    static vector<string> sAwards;
    
    static string getAwardByRank(int rank);
    
    static map<string, CrossServiceWarRankingserviceCfgDef*> rankingservicecfg;
    static int getAwardsCount();
};
//个人排名
class CrossServiceWarRankingsimpleCfg : public Cfgable
{
public:
    static void load(std::string fullPath);
public:
    static map<string, CrossServiceWarRankingsimpleCfgDef*> rankingsimplecfg;
    static int getRankingSimpleAwardCount();
    
    static string getAwardByRank(int rank);
    static vector<pair<int, int> > sRankIntervals;
    static vector<string> sAwards;
};


#define CROSSSERVICEWAR_AWARD_TOTAL 5
#define CROSSSERVICEWAR_AWARD_PART_ONE 100
#define CROSSSERVICEWAR_AWARD_PART_TWO 150
#define CROSSSERVICEWAR_AWARD_PART_THREE 200
#define CROSSSERVICEWAR_AWARD_PART_FOUR 300

struct CrossServiceWarAward
{
    CrossServiceWarAward()
    {
        protoss = gold = 0;
    }
    int protoss;
    int gold;
    string itemAward;
};
struct CrossServiceWarAwardDef
{
    CrossServiceWarAwardDef()
    {
        mMinRank = mMaxRank = 0;
        mIsShuzu = false;
    }
    int mMinRank;
    int mMaxRank;
    bool mIsShuzu;
    vector<CrossServiceWarAward*> mAwards;
    
};

struct CrossServiceWarRankDataDef
{
    int mMax;
    int mMin;
    int space;
};


class CrossServiceWarCfg : public Cfgable {
public:
    static void load(std::string fullPath);
    static CrossServiceWarAward* getRankAward(int rank);
    static CrossServiceWarAward* getResultAward(bool win);
    static int getfreeTimes();
    static int getOpenLvl();
    static void getRankRule(int myrank, CrossServiceWarRankDataDef& output);
    static int getCoolDownTime(int times);
    static int getFreshCost(int times);
    static int getTimeout();
    static int getRobotCount();
    static int getAwardPointPeriod(){return mAwardPointPeriod;}
private:
    static vector<int> mRanges;
    static std::map<int, CrossServiceWarAwardDef*> mRankAward;
    static vector<CrossServiceWarAward*> mCrossServiceWarResultAward;
    static int mfreetimes;
    static int mOpenlvl;
    static int mTimeout;
    static vector<CrossServiceWarRankDataDef*> mRankDataRule;
    static vector<int> mCooldownTime;
    static vector<int> mFreshCost;
    static int mRobotCount;
    static int mAwardPointPeriod;
};


class CsPvpGroupCfgDef
{
public:
    vector<int> mServerIds;
    vector<pair<int, int> > mCloseWeeks;
    int mGroupId;
};

class CsPvpGroupCfg
{
public:
    static void load(const char* fullpath);
    static bool isClose(int serverId, int testTime);
    static bool isGroupClose(int groupId, int testTime);
    static CsPvpGroupCfgDef* getCfgDefByServerId(int serverId);
    static CsPvpGroupCfgDef* getCfgDef(int groupId);

    static bool sAllServer;
    static map<int, CsPvpGroupCfgDef*> sGroups;
    static map<int, CsPvpGroupCfgDef*> sServerIdIndex;
};


//----------------------------------------

struct DailyScheduleCfgDef
{
    int cfgId;
    int type;
    string name;
    string desc;
    vector<int> points;
    int target;
    int progress;
    string icon;
    float iconScle;
    int activityID;
    string openCfg;
    string openType;
    string title;
    
    int getPoint(int day);
};

struct DailyScheduleAwardCfgDef
{
    int mId;
    int mPoint;
    string mAward;
    int mIndex;
    int mStartLvl;
    int mEndLvl;
};

class DailyScheduleCfg:public Cfgable
{
public:
    DailyScheduleCfg(){}
    virtual ~DailyScheduleCfg(){}
    static void load(const char* fullPath, const char* awardPath);
    
    static DailyScheduleCfgDef* getCfg(int id);
    static DailyScheduleCfgDef* getCfgByType(int type);
    static DailyScheduleAwardCfgDef* getAwardCfg(int id);
    static int getDailySheduleCfgListSize();
    static DailyScheduleCfgDef* getDailyScheduleCfgByName(string name);
    static vector<DailyScheduleAwardCfgDef*> getAwardListByLv(int lv);
private:
    static map<int, DailyScheduleCfgDef * > cfg;
    static vector<DailyScheduleCfgDef*> cfgTypeIdx;
    static map<int, DailyScheduleAwardCfgDef*> awards;
};

#pragma mark  ----------------loading小提示-------------------------
struct LoadingTipsDef
{
    int mId;
    string mTips;
};

class LoadingTipsDefCfg:public Cfgable
{
public:
    LoadingTipsDefCfg(){}
    virtual ~LoadingTipsDefCfg(){}
    static void load(string fullPath);
    static string getRandomTips();
private:
    static vector<LoadingTipsDef * > cfg;
};

#pragma mark  ---------------------------------------

class ActivateBagGridsCfg:public Cfgable
{
public:
    ActivateBagGridsCfg(){}
    virtual ~ActivateBagGridsCfg(){}
    static void load(const string &filepath);
    static int getCfg(int page);
    static int getMaxPage();
private:
    static vector<int> activateCost;
};
#pragma mark  ----------------角色奖励-------------------------
struct RoleAwardItemDef
{
    int itemid;         // 奖励id，匹配对应奖励
    int viplv;          // 奖励需要的vip等级
    std::string item;   // 奖励内容
    RoleAwardItemDef()
    {
        itemid = 0;
        viplv = 0;
        item= "";
    }
};

class RoleAwardCfgDef
{
public:
    RoleAwardCfgDef();
    ~RoleAwardCfgDef();
    RoleAwardItemDef* getRoleAwardItemDefByid(int itemid);
    
    READWRITE(int, cfgid, Cfgid);       // 角色奖励id
    READWRITE(int, days, Days);         // 累计登陆天数需求
    vector<RoleAwardItemDef*> &getRoleAwardItems();
    vector<RoleAwardItemDef*> items;    // 奖励内容
};

class RoleAwardCfgMgr
{
public:
    typedef vector<RoleAwardCfgDef*>::iterator Iterator;
    static Iterator Begin(){ return awards.begin(); }
    static Iterator End(){ return awards.end(); }
    static void load(const char* fullPath);
    static RoleAwardCfgDef* getRoleAwardCfgDefByid(int cfgid);
    static RoleAwardItemDef* getRoleAwardItemDefByRoleAwardItemId(int roleAwardItemId);
    static vector<RoleAwardCfgDef*> &getRoleAwardCfgDefs();
private:
    static vector<RoleAwardCfgDef*> awards;
};

class ColorCfg
{
public:
    static void load(const char* fullpath);
    static string getColor(int color);
    static string getColorByName(const char* name);
    
private:
    static map<int, string> sColors;
    static string sDefaultColor;
};

struct AwakeSkillReplace
{
    AwakeSkillReplace():preSkillId(0),
                        newSkillId(0)
    {
        
    }
    
    AwakeSkillReplace(const AwakeSkillReplace& src)
    {
        this->preSkillId = src.preSkillId;
        this->newSkillId = src.newSkillId;
        this->skillDescrip = src.skillDescrip;
    }
    int preSkillId;
    int newSkillId;
    
    string skillDescrip;
};

struct AwakePropAddtion
{
    AwakePropAddtion():propName(""),
                       addRatio(0.0f)
    {
        
    }
    
    AwakePropAddtion(const AwakePropAddtion& src)
    {
        this->propName = src.propName;
        this->addRatio = src.addRatio;
    }
    string propName;
    float addRatio;
};

typedef vector<AwakePropAddtion*> RoleAwakePropAddtion;
typedef vector<AwakeSkillReplace*> RoleAwakeSkillReplace;

struct RoleAwakeCfgDef
{
    int awakeLvl;
    int needLvl;
    int resid;
    
    float fashionCollectAdd;    //时装收集属性加成
    float weaponEnchantsAdd;    //武器附魔加成
    
    map<int, RoleAwakeSkillReplace> skillReplace;
    map<int, RoleAwakePropAddtion> propAddtion;
    
    string name;
    vector<ItemCell> needItem;
    std::map< ObjJob, int> lightResIds;
    
    void addSkillReplace(int job, string& skills, string& skillDes);
    void addPropAddtion(int job, string& propsInfo);
    void addNeedItemInfo(string& itemStr, string& externDes);
};

struct RoleAwakeAddtionInfo     //主要提供给前端使用的信息
{
    RoleAwakeAddtionInfo(): resid(0),
                            fashionCollectAdd(0.0f),
                            weaponEnchantsAdd(0.0f),
                            needLvl(0),
                            awakeLvl(0),
                            awakeName(""),
                            lightResId(0)
    
    {
        propAddtion.clear();
        skillsReplace.clear();
        needItem.clear();
    }
    vector<AwakePropAddtion> propAddtion;
    vector<AwakeSkillReplace> skillsReplace;
    
    vector<ItemCell> needItem;
    int resid;
    
    float fashionCollectAdd;    //时装收集属性加成
    float weaponEnchantsAdd;    //武器附魔加成
    
    string awakeName;
    int needLvl;
    int awakeLvl;
    int lightResId;
};


class RoleAwakeCfg
{
public:
    static void load(const char* fullPath);
    static bool getRoleAwakeInfo(int job, int awakeLvl, RoleAwakeAddtionInfo& outData);
    static void getAllAwakePropAddtion(ObjJob job,map< string, AwakePropAddtion> &awakePropAddtionMap);
    static void getAllAwakeSkillReplace(ObjJob job,vector<AwakeSkillReplace> &skillsReplace);
    static void getAlreadySkillReplace(ObjJob job,int roleAwakeLvl,vector<AwakeSkillReplace> &skillsReplace);
    static int  getRoleAwakeLightResId(ObjJob job,int awakeLvl);
private:
    static map<int, RoleAwakeCfgDef*> mAwakeCfgData;
};

/* 装备洗炼 */


struct EquipBaptizeSlot
{
    int qua;
    string quaName;
    int slotNum;
    std::string slotDesc;
    EquipBaptizeSlot()
    {
        qua = 0;
        quaName = "";
        slotNum = 0;
        slotDesc = "";
    }
};

struct EquipBaptizeAttri
{
    std::string attriValue;
    int attriPro;              //属性值名权重
    EquipBaptizeAttri()
    {
        attriValue = "";
        attriPro = 0;
    }
    
    string getValue()
    {
        float min = 0.0f;
        float max = 0.0f;
        
        sscanf(attriValue.c_str(), "%f-%f", &min, &max);
        
        float ret = range_randf(min, max);
        int tmp = ret * 10;
        
        ret = tmp * 1.0f / 10;
        
        string retStr = Utils::makeStr("%f", ret);
        return retStr;
    }
};

struct EquipBaptizeAttriDef
{
    std::string attriCnName;
    std::string attriName;
    int pro;                  //属性名权重
    std::vector<EquipBaptizeAttri*> equipBaptizeAttris;
    EquipBaptizeAttriDef()
    {
        attriCnName="";
        attriName = "";
        pro = 0;
        equipBaptizeAttris.clear();
    }
    
    pair<string, string> getProp()
    {
        int vsize = equipBaptizeAttris.size();
        
        int pros = 0;
        for (int i = 0; i < vsize; i++)
        {
            pros += equipBaptizeAttris[i]->attriPro;
        }
        
        if (pros <= 0)
        {
            return make_pair("", "");
        }
        
        int randnum = rand()%pros;
        pros = 0;
        for( int i = 0; i < vsize; i++)
        {
            pros += equipBaptizeAttris[i]->attriPro;
            if(randnum < pros)
            {
                return make_pair(attriName, equipBaptizeAttris[i]->getValue());
            }
        }
        
        return make_pair("", "");
    }
};

extern string getJsonValue(string& str, string key);

class EquipBaptizeCfg:public Cfgable
{
public:
    
    static void load(const char* equipBatizeSlotPath,const char* equipBatizeAttriPath);
    static EquipBaptizeSlot* getEquipBaptizeSlotByQua(int qua);

    static pair<string, string> makeBaptizeProp();
    static EquipBaptizeAttriDef* getEquipBaptizeAttriDefbyattriName(string _attriName);
    static vector<EquipBaptizeAttriDef*> &getEquipBaptizeAttriDefList(){ return equipBaptizeAttriDefData;};
    static map<int,EquipBaptizeSlot*>  &getEquipBaptizeSlotList(){ return equipBaptizeSlots;};
public:
    
    static string getBaptizeKeyName(){ return "Baptize"; }
    static string getBackupBaptizeKeyName(){ return "BackupBaptize";}
    
    static pair<string, string> getBaptizeProp(string& data, int qua, int index);
    static pair<string, string> getBackupBaptizeProp(string& data, int qua, int index);
    static void calcBaptizeProp(const char* propName, const char* propVal, BattleProp& battleProp);
    
public:
    STATICREADWRITE(int , mBatizeCost , BatizeCost)       //洗炼花费
    STATICREADWRITE(int , mBatizeStoneId , BatizeStoneId) //洗炼石
    STATICREADWRITE(int , mRecoveryCost , RecoveryCost)   //复原花费
private:
    static map<int,EquipBaptizeSlot*>  equipBaptizeSlots;
    static vector<EquipBaptizeAttriDef*> equipBaptizeAttriDefData;
    
};


/*
 *侍魂相关
 */


//侍魂的模型根据等级不一样 不一样

struct RetinueOutline
{
    int lvl;
    int resID;
    string retinueIamge;
    string retinueHeadImage;
    RetinueOutline()
    {
        lvl = 0;
        resID = 0;
        retinueIamge = "";
        retinueHeadImage = "";
    }
};

//侍魂模板
struct RetinueModDef
{
    int retinueId;
    int steps;
    int position;
    
    int bExp;
    int bStarExp;
    
    int maxLvl;
    int maxStar;
    
    int bAtk;
    int bDef;
    int bHp;
    float bHit;
    float bDodge;
    float bCri;
    string dropDesc;
    #ifdef CLIENT
    ReadWrite_Crypt(int, mPropStrength, PropStrength)  //属性强度
    #else
    READWRITE(int, mPropStrength, PropStrength)
    #endif
    
    vector<ItemCell > mNeedMaterials;       //合成所需材料，合成之后变成专用升级材料
    vector<int> uniMaterialForStarup;                //通用升星材料
    
    vector<int> specificMaterialForLvlup;  //特有升级材料
    vector<int> uniMaterialForLvlup; //通用升级材料
    
    vector<RetinueOutline>  outlineData;
    
    string name;
    
    RetinueModDef():bExp(0),
                    bStarExp(0),
                    maxLvl(0),
                    maxStar(0),
                    bAtk(0),
                    bDef(0),
                    bHp(0),
                    bHit(0.0f),
                    bDodge(0.0f),
                    bCri(0.0f)
    {
        retinueId = 0;
        steps = 0;
        position = 0;
        setPropStrength(0);
    }

    RetinueOutline * getOutlineData(int lvl);
    bool checkRetinueLvlupMaterial(int materialId);
    bool checkRetinueStarupMaterial(int materialId);
    
    int getStarUpMaterial(int lvl)
    {
        if(lvl <= 0)
        {
            return 0;
        }
        
        if(lvl >= mNeedMaterials.size() )
        {
            return mNeedMaterials[mNeedMaterials.size() - 1].itemid;
        }
        
        return mNeedMaterials[lvl].itemid;
    }
    
    int getUniStarupMaterial(int lvl)
    {
        if(lvl <= 0)
        {
            return 0;
        }
        
        if(lvl >= uniMaterialForStarup.size() )
        {
            return uniMaterialForStarup[uniMaterialForStarup.size() - 1];
        }
        
        return uniMaterialForStarup[lvl];
    }
};

//聚魂
struct RetinueSummonDef
{
    RetinueSummonDef(): index(0),
                        ordinarySummon(0),
                        rmbSummon(0),
                        rmbSummonCost(0),
                        roleLvl(0),
                        cooldown(0),
                        resetCost(0),
                        res_id_stand(0),
                        res_id_run(0)
    {
    }
    int index;
    int ordinarySummon;     //普通聚魂对应的翻牌id
    int rmbSummon;  //金钻聚魂对用的翻牌id
    int rmbSummonCost;            //金钻聚魂花费的金钻
    int roleLvl;            //开启等级
    int cooldown;           //冷却时间
    int resetCost;          //消除冷却时间花费
    int resetTimes;         //
    int res_id_stand;
    int res_id_run;
    string name;
    ItemArray mItems; //聚魂所获得的奖励
    string backgroundSpr;
};

//侍魂技能
struct RetinueSkillDef
{
    RetinueSkillDef():  skillId(0),
                        maxLvl(0),
                        powerConsum(0)
    {
        //setRol(0);
        setCoolingTime(0);
    }
    int skillId;            //对应的技能
    string name;
#ifdef CLIENT
    //ReadWrite_Crypt(int, mRol, Rol)  //技能释放概率
    ReadWrite_Crypt(float, mCoolingTime, CoolingTime)  //技能冷却时间
#else
    //READWRITE(int, mRol, Rol)  //技能释放概率
    READWRITE(float, mCoolingTime, CoolingTime)  //技能冷却时间
#endif
    string      skillIcon;
    vector<int> skillIds;  //一个大技能模板下面有几个小技能
    
    int maxLvl;
    int powerConsum;        //释放是消耗的能量点
    //string desc;            //描述
    
    ItemCell getUpgMaterial(int lvl)
    {
        if(lvl <= 0)
        {
            return ItemCell();
        }
        
        if(lvl >= upgradeMater.size() )
        {
            return upgradeMater[upgradeMater.size() - 1];
        }
        
        return upgradeMater[lvl];
    }
    
    ItemCell getUniUpgMaterial(int lvl)
    {
        if(lvl <= 0)
        {
            return ItemCell();
        }
        
        if(lvl >= uniUpgradeMater.size() )
        {
            return uniUpgradeMater[uniUpgradeMater.size() - 1];
        }
        
        return uniUpgradeMater[lvl];
    }
    
    //升级所需的材料
    vector<ItemCell> upgradeMater;
    
    //升级所需的通用材料
    vector<ItemCell> uniUpgradeMater;
    
    //用于得到每个等级的技能描述  需要存储的变量
    string canshu1_s;
    string  canshu2_s;
    string canshu3_s;
    string canshu4_s;
    string canshu5_s;
    string commonDesc;
    string getSkillDescBySkillLvl(int lvl);
    
    
    
};

//通灵
struct SpiritSpeakDef
{
    SpiritSpeakDef(): gribCount(0),
                      retinueStep(0)
    {
        
    }
    
    int retinueStep;
    int gribCount;      //位置数量
    string desc;        //描述
    int spiritSpeakid;
};

class RetinueCfg
{
public:
    static void load(string modPath, string skillPath, string summonPath, string spiritPath);
    
    static void getAllSpiritSpeakCfg(vector<SpiritSpeakDef *>& data);
    
    static void getAllRetinueSummonCfg(vector<RetinueSummonDef *>& data);
    static map<int, RetinueSkillDef *> getAllSkillsCfg();
    
    static RetinueSummonDef * getRetinueSummonCfg(int retinueIndex);
    static RetinueSummonDef * getRecentlySummonDefByLvl(int rolelvl, int * summonIndex = NULL);
    
    static int getRetinueSummonResetCost(int index);
    static int getRetinueSummonResetTimes(int index);
public:
    
    static SpiritSpeakDef * getSpiritSpeakCfg(int index);
    
    static RetinueModDef * getRetinueModCfg(int retinueId);
    
    static RetinueSkillDef * getRetinueSkillCfg(int skillId);
    
    //根据steps和postions得到对应的聚魂类型
    static vector<RetinueModDef *> getAllRetinueModDefsByStepsAndPos(int steps, int postions);
    static vector<RetinueModDef *> getAllRetinueModDefsByConfig();
    
protected:
    static void loadMod(string& path);
    static void loadSkills(string& path);
    static void loadSummon(string& path);
    static void loadSpiritSpeak(string& path);
private:
    
    typedef map<int, RetinueModDef *> RetinueModMap;
    typedef map<int, RetinueSkillDef *> RetinueSkillMap;
    typedef map<int, RetinueSummonDef *> RetinueSummonMap;
    typedef map<int, SpiritSpeakDef *> SpiritSpeakMap;
    
    static map<int, RetinueModDef *> mRetinueMods;
    static map<int, RetinueSkillDef *> mRetinueSkills;
    static map<int, RetinueSummonDef *> mRetinueSummon;
    static map<int, SpiritSpeakDef *> mSpiritSpeak;
};



const int DEFAULT_RANKLIST_REFRESHTIME = 3600;

struct ServerGroupCfgDef
{
	int             mGroupId;
	set<int>        mServerIds;
    
    //ranklist data
    int             mBeginTime;
    int             mEndTime;
    int             mSendAwardTime;
    vector<string>  mAwards;
    int             mRankListFreshPeriod;
    int             mRankListType;
};

class ServerGroupCfg
{
public:
    void load(string config, int type);
    
    typedef map<int, ServerGroupCfgDef*>::iterator Iterator;
    ServerGroupCfgDef* getServerGroupCfgDefByServerid(int serverId);
    ServerGroupCfgDef* getServerGroupCfgDefByGroupid(int groupid);
    int                getServerGroupId(int serverId);
    int                getGroupFreshPeriod(int serverId);
    void               getGroupList(vector<int> &output);
    bool               isLoaded(){return mLoaded;}
    
private:
    bool                         mLoaded;
    map<int, int>                mServer2Group;
    map<int, ServerGroupCfgDef*> mServerGroupCfgMap;
    vector<int>                  mGroupidList;
};

extern ServerGroupCfg g_RechargeRankListGroupCfg;
extern ServerGroupCfg g_ConsumeRankListGroupCfg;

#endif /* defined(__GameSrv__DataBase__) */


