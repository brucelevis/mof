//
//  Defines.h
//  GameSrv
//
//  Created by 麦_Mike on 12-12-28.
//
//

#ifndef GameSrv_Defines_h
#define GameSrv_Defines_h

//todo 拆分成多个定义文件，，，图形独立

#include <stdlib.h>
#include <math.h>
#include <string>
#include "json/value.h"
#include "Utils.h"
#include "MemoryCrypt.h"
//#include "cocos2d.h"
//
//using namespace cocos2d;

// 只读
#define READONLY(varType, varName, funName)                 \
public: varType varName;                                    \
private: void set##funName(varType var){ varName = var; }   \
public: varType get##funName(void) const { return varName; }


// 可读写
#define READWRITE READWIRTE
#define READWIRTE(varType, varName, funName)                \
public: varType varName;                                    \
public: varType get##funName(void) const { return varName; }\
public: void set##funName(varType var){ varName = var; }

#define VIRTUAL_READWRITE(varType, varName, funName)                \
public: varType varName;                                    \
public: virtual varType get##funName(void) const { return varName; }\
public: virtual void set##funName(varType var){ varName = var; }

// 静态只读
#define STATICREADONLY(varType, varName, funName)           \
public: static varType varName;                             \
private: static void set##funName(varType var){ varName = var; }   \
public: static varType get##funName(void) { return varName; }

//静态读写
#define STATICREADWRITE(varType, varName, funName)          \
public: static varType varName;                             \
public: static varType get##funName(void) { return varName; }\
public: static void set##funName(varType var) { varName = var;}


#define MAX_SKILL_COUNT 23
#define SKILL_EQUIT_NUM 4
#define PETARENASCENE 501222

enum ClientMode
{
    eOnline = 0,
    eOffline = 1,
};

// obj类型
enum ObjType
{
    eUnkown,
    /////////////LivingObject///////////////////
    eRole,                      //角色
    eMainRole,                  //主角
    eMonster,                   //怪物
    ePet,                       //宠物
    eStatue,                    //神像
    eNPC,                       //NPC
    eRetinue,                   //侍魂

    /////////////GameObject///////////////////
    ePortal,                    //传送圈
    eWordNumberEffect,          //文字特效
    eFlyEffect,                 //技能特效
    eHitEffect,                 //打击effect
    eSkillEffect,               //技能特效
    eUIEffect,                  //UI特效

    eObjectTypeNum
};
//sex
enum ObjSex
{
    eBoy=0,
    eGirl=1,
    eAllSex =2
};
// obj职业
enum ObjJob
{
    eWarrior=0  // 剑士
    ,eMage=1    // 法师
    ,eAssassin=2// 刺客
    ,MAX_ROLE_JOB=3
    ,eTest=5
    ,eAllJob=6

};
// obj状态
enum ObjState
{
    eNone,   // 不在使用中，仅供obj对象缓存使用
    eIDLE,   // 待机
    eWALK,   // 行走
    
//    eCLIMB,  // 爬
//    eCLIMB_PAUSE,//攀爬暂停状态
//    eJUMP,   // 跳
//    eFALL,   // 下落
    eATTACK, // 普通攻击
    eCAST,   // 呤唱
    eSKILL,  // 放技能
    eHITTED, // 被击中
    eDEAD,
    eObjStateNum,

    eNPC_Acceptable,    //NPC 可接任务
    eNPC_Accepted,      //NPC 已接任务
    eNPC_Commitable,    //NPC 可提交任务
    eNPC_NoTask,        //NPC 没任务
};
//obj控制方式。目前这个枚举只有客户端使用，服务器不要使用，以免概念混淆---linshusen
enum ObjCtrlMode
{
    eClientCtrl = 1,    //完全由客户端控制。例如：单机模式
    eServerCtrl = 2,    //完全由服务器端控制。例如：服务器控制的AI，同步pk时的对手
    eClientServerCtrl = 3, //由客户端发起，服务器同步。例如：同步pk时的主角
};

//obj朝向
enum ObjOrientation
{
    eOrientNone=0,
    eUp = 1,    //向上
    eDown,      //向下
    eLeft,      //向左
    eLeftUp,
    eLeftDown,
    eRight,     //向右
    eRightUp,
    eRightDown,
    eSlop
};

enum PointOutOfAreaType
{
    eNonOut = 0,
    eOutOfX,
    eOutOfY,
    eOutOfBoth,
};

//声音资源类型
enum AudioType
{
    eAudioNone = 0,
    eAudioBackground = 1,
    eAudioEffect = 2,
    eAudioUI = 3
};

//换装相关 ---linshusen
//可以换装的身体部位
enum BodyPart
{
    eBodyPartNone = 0,
    eHair,          //头发
    eClothes,       //上衣
    ePants,         //裤子
    eShoes,         //鞋子
    eWeapon,        //武器
    eWholeBody,     //全套时装
    eCityBody,      //主城时装
};


////摇杆方向
//enum JoystickOrient {
//    eNoOrient,            //没方向
//    eOrientLeft,
//    eOrientRight,
//    eOrientUp,
//    eOrientDown
//};


//按钮类型
enum ButtonType {
    eAttackBtn,
    eSkillBtn
};

//新手指导箭头的方向
enum direction
{
    eArrowLeft = 1,
    eArrowUp =2,
    eArrowRight = 3,
    eArrowDown = 4
};
//二维向量：可表示一个点或者一个方向
//struct CCPoint
//{
//    CCPoint():x(0),y(0)
//    {}
//    CCPoint(float _x, float _y)
//    {
//        this->x = _x;
//        this->y = _y;
//    }
//
//    void Zero(){x=0;y=0;}
//    float   x; // x
//    float   y; // y
//
//    /**
//     *	@brief	获取距离的平方
//     *
//     *	@return	距离的平方
//     */
//    float getSquraDistance()
//    {
//        return x*x + y*y;
//    }
//
//    void operator+=(CCPoint other)
//    {
//        x += other.x;
//        y += other.y;
//    }
//    void operator-=(CCPoint other)
//    {
//        x -= other.x;
//        y -= other.y;
//    }
//    CCPoint operator+(CCPoint other)
//    {
//        return CCPoint(x + other.x, y + other.y);
//    }
//    CCPoint operator-(CCPoint other)
//    {
//        return CCPoint(x - other.x, y - other.y);
//    }
//    CCPoint operator-()
//    {
//        return CCPoint(-x, -y);
//    }
//    bool operator==(CCPoint other)
//    {
//        return x == other.x && y == other.y;
//    }
//    bool operator!=(CCPoint other)
//    {
//        return x != other.x || y != other.y;
//    }
//    CCPoint operator*(float f)
//    {
//        return CCPoint(x * f, y * f);
//    }
//    CCPoint operator/(float f)
//    {
//        return CCPoint(x / f, y / f);
//    }
//};
// color 0－255  a->255为全不透明
//struct Color {
//    //Color(int _r,int _g,int _b,int _a){r=_r;g=_g;b=_b;a=_a;}
//    int r;
//    int g;
//    int b;
//    int a;
//};

//角色移动的信息
//struct RoleMoveInfo
//{
//    bool mIsNull;
//    CCPoint mPos;
//    CCPoint mSpeed;
//    ObjOrientation oritent;
//    int animation;
//    int mState;
//};

// 事件类型
enum EventType
{
    eStateChange,       //状态改变
    ePosChange,         //位置改变
    eOrientChange,      //朝向改变

    eArrivePos,             //到达目的地
    eClickNPC,              //点击NPC

    //按钮事件
    eBtnPress,                //按下
    eBtnRelease,            //松开

    eJoystick,
    eAttack,
    eAttackContinue,
    eAttackContinueEnd,
    eSkill,
    eCompletedAnimation, //

};

//通知界面更新的事件
enum UpateUIEventType
{
    uiet_AllMainRole = 0,   //所有主角色信息
    uiet_FrequentMainRoleInfo,  //经常需要更新的主角色信息
    uiet_Level,                 //等级
    uiet_Exp,                   //经验
    uiet_Rmb,                   //经验
    uiet_Gold,                  //设置金钱
    uiet_Constell,              //升星
    uiet_BatPoint,
    uiet_BagList,
    uiet_BagUpdate,            //背包更新
    uiet_EquipList,
    uiet_EquipUpdate,          //装备更新
    uiet_EquipCombine,          //材料合成
    uiet_EquipLuckyStone,       //幸运石数量更新
    uiet_EquipStreng,           //装备强化
    uiet_EquipBaptize,          //装备洗练
    uiet_EquipStrengError,      //装备强化错误处理

    uiet_SkillInfor,            //技能的信息
    uiet_SkillSucceedTip,       //技能学习或升级成功
    uiet_SkillFailTip,          //技能学习或升级失败

    //~~~任务界面
    eAddAcceptTask, //向已接列表中添加数据
    eAddUnaceptTask ,//向未接列表中添加数据
    eReduceAcceptTask ,    //向已接列表中删除数据
    eReduceUnacceptTask,    //向未接列表中删除数据
    eRefreshData,             //刷新数据
    eDeleteUnAcceptAllTask,       //删除未接的所有任务
    eRefreshAllTask,             //刷新已接列表的所有数据
    eDeleteAcceptAllTask,   //删除已接的所有任务
    //~~~附近人物界面
    eUIAddASceneRole,
    eUIClearSceneRoles,
    eUIDeleteASceneRole,
    //~~~查找人
    eUIAddASearchResult,
    //~~~好友界面
    eUIAddAFriend,
    eUIAFriendOnLine,       //好友上线或下线
    eUIAckFriend,               //请求好友
    eUIDeleteAFriend,

    eUIIntimateChange,      //好友亲密值发生变化

    eUIAckNewMail,         //有新的邮件
    
    eUINewTitle,            // 新称号
    eUIGuidApply,           //入会申请
    eUIRefreshQueenBlessings,         //女王祝福刷新
    eUIRecvQueenBlessingSuccess,     //领取成功
    
    eEquipFusionError,   //装备融合错误处理
    eUIDuleApply,       //决斗邀请
    eUIFriendState,       // 主城好友玩家状态
    eUIOtherRoleState,    // 主城其他玩家状态
    eUIEnchantCurrencyChange,    // 武器附魔币变化
    eUIEnchantList,              // 武器附魔附魔列表
    eUIIsEnchanting,             // 当前被附魔变化
    eUIet_MaterialBagList,       //材料背包列表
    eUIet_MaterialBagUpdate,     //材料背包更新
    eUI_RoleAwakeInfo,           //觉醒界面更新
 };

//任务寻路的状态
//enum Taskpathfinding
//{
//    AcceptTaskShow,                //接任务显示
//    UnacceptTaskShow,                  //交任务显示
//    LookforingNPC,             //寻找NPC
//    BattleShow                     //战斗
//};
//通知升级或学习是否成功
//enum SkillTipType
//{
//    uiet_SkillInfor,            //技能的信息
//    uiet_SkillSucceedTip,       //技能学习或升级成功
//    uiet_SkillFailTip           //技能学习或升级失败
//};
// 图层类型
enum LayerType
{
    eLayerGame,  // 静态逻辑层
    eLayerMainUI, // 主界面层
    eLayerMap,    // 地图层
    eLayerObj,   //角色层
    eLayerLogin,
    eLayerCreation,
};

////地图路线模式
//enum MapLineMode {
//    eRoad = 0,              //路
//    eRope = 3,              //吊绳
//    eBlock = 5,             //阻挡
//};
//
////线的类型
//enum LineType{
//    eHorizontalLine,        //水平线
//    eVerticlalLine,             //垂直线
//    eSlopeLine,                 //斜线
//};
//
////路径动作
//enum PathAction {
//    eRun = 1,                   //平路
//    eClimb = 2,                 //爬
//    //eSlope  = 3,                //走斜坡
//};

/**
 *	@brief	路径中的一段
 */
//struct PathSegment
//{
//    CCPoint     destination; /**< 目的点 */
//    float   speed; /**< 运动的速度 */
//    PathAction     action; /**< 运动的行为(走，爬，跳) */
//
//    PathSegment(){}
//    PathSegment(CCPoint _destination, float _speed, PathAction _action)
//    {
//        destination = _destination;
//        speed = _speed;
//        action = _action;
//    }
//};

////路径状态
//enum PathState
//{
//    eError = 0,                             //错误
//    eToPathInALine = 1,             //两个路径在同一条线上
//    eToPathInLinkPath = 2,      //两个路径都在主路径上
//    eStartPathInLink = 3,           //开始的路径在主路径上，结束路径在次路径上
//    eEndPathInLink = 4,             //开始的路径在次路径上，结束路径在主路径上
//};

// tilemap
enum GraphChildTag
{
    eTag_TileMap = 1,
    eTag_NoFlipx = 10, //不跟随父节点翻转的子节点
};

// sprite 动画 id
enum eAnimationID
{
    //各个职业人物的动作
    eAnim_bruise,
    eAnim_climb2,
    eAnim_climb3,
    eAnim_jump,
    eAnim_run,
    eAnim_stand,
    eAnim_sf,
    eAnim_physical_1,
    eAnim_physical_2,
    eAnim_physical_3,
    eAnim_physical_4,
    eAnim_physical_5,
    eAnim_ycphysical,
    eAnim_physical_10,
    eAnim_physical_20,
    eAnim_physical_30,
    eAnim_physical_40,
    eAnim_physical_50,
    eAnim_fell,
    eAnim_dead,

    eAnim_effect,


    animateNum  // 总数
};

//任务类型
enum eQuestType {
//    eNewGuideTask,                      //新手任务
//    eMainTask = 2,                      //主线任务
//    eSideTask = 3,                        //支线任务
//    eOccupationalTask = 4,          //职业任务
//    eDailyTask = 5,                        //日常任务
//    eMentorTask = 6,                    //师徒任务
//    eFamilyTask = 7,                        //家族任务
//    eActivityTask = 8,                       //活动任务
//    eVIPDailyTask = 9,                      //vip日常任务
//    eTimeTask = 10,                         //限时任务
//    eCycleTask = 11,                        //循环任务
//    eItemTask = 12,                         //道具任务
//    eChallengeTask = 13,                 //挑战任务
    qtMainQuest = 1,
    qtSideQuest = 2,
    qtDailyQuest = 3,
};

//任务状态
enum eClientQuestState{
    cqsVoid = 0,                                //无效的
    cqsAcceptable = 1,                     //可接的
    cqsInProgress = 2,                    //进行中的
    cqsFinished  = 3,                     //完成了(未提交)的
    cqsDialoging = 4,
    cqsFinishingCopy = 5,
    cqsDailyQuestSubmit = 6,        //日常任务提交
    cqsUpEquip = 7,
    cqsStudySkill = 8,
    cqsNextLvQuest = 9,
    cqsArenaBattle = 10,
    cqsCopyStar = 11,
    cqsCopyCombo = 12,
    cqsFinishingEliteCopy = 13,
    cqsConstellupg = 14,
    cqsStudypassiveskill = 15,
    cqsEnterFriendCopy,
    cqsEnterPetCamp,
    cqsFinishPetAdventure,
    cqsFinishFamousHall,
    cqsFashionCollectMaterial,
    cqsPetevolution,
//    tsFailed = 4,                             //失败的
//    tsClosed = 5,                            //结束的（完成并提交的）
//    tsVoid = 6,                                  //无效的
//    tsGivingItem= 99,                    //给物品进行中
//    tsTalking = 100,                         //对话进行中
};

enum eSceneType {
    stTown = 1, // 主城
    stCopy = 2, // 普通副本
    stEliteCopy = 3, // 精英副本
    stDungeon = 4, // 地下城
    stNewbie = 5, // 新手副本
    stPvp = 6, //竞技场
    stTeamCopy = 7, // 好友副本
    stFriendDunge = 8, // 好友地下城
    stPrintCopy = 9,  //图纸副本
    stWorldBoss = 10, // 世界boss
    stPetCamp = 11, // 已废
    stPetDunge = 12,  //宠物地下城
    stDefendStatue = 13,
    stMysticalCopy = 14,//双周副本
    stPetArena = 15,//宠物竞技场
    stFamousDunge = 16,//名人堂
    stLimitTimeKilled = 17,//限时击杀
    stChallenge  = 18,//试炼
    stPetEliteCopy = 19, // 幻兽精英副本，也叫幻兽试炼场，主角操作幻兽的副本
    
    stPublicScene = 30,   //大厅
    stSyncPvp = 31,    //同步pvp场景
    stTreasureFight = 32, //夺宝战
    stSyncTeamCopy = 33, //组队副本
    stPetAdventure = 35, //幻兽大冒险
    stCrossServiceWar = 36, //跨服战
    
    stScriptScene = 50, // 脚本场景
    };

extern std::string getSceneNameBySceneType(eSceneType type);
enum eTaskTarget
{
    eKill,
    eGather,
    eDialog,
    eGiveItem,
    eFingNPC,
    eMove,
    eEquip,
    eFix,
    eStudySkill,
    eBuy,
    eNpcSell,
    eFinishCopy,
    eUseItem,
    eEnterCopy,
    eTowerLevel,
    ePlayerLevel
};

enum eFinishCopyResult
{
    eFCR_SSS = 1,
    eFCR_SS  = 2,
    eFCR_S = 3,
    eFCR_A = 4,
    eFCR_B = 5,
    eFCR_C = 6,
    eFCR_D = 7
};

//线的数据（读表）
//struct  LineData{
//    int index;
//    CCPoint startPos;
//    CCPoint endPos;
//
//    LineData(int _index,CCPoint _start,CCPoint _end)
//    {
//        index = _index;
//        startPos = _start;
//        endPos = _end;
//    }
//};

//路径数据（读表）
struct PathData {
    int startPath,endPath,action;
    PathData(int _startPash,int _endPath,int _action)
    {
        startPath = _startPash;
        endPath = _endPath;
        action = _action;
    }
};

//// 资源类型
enum ResourceType
{
    eKeyFrameAnimate= 1, // 关键帧动画
    eBoneAnimate    = 2, // 骨髂动画
    eStaticImage    = 3, // 静态图片
    eBattleEffect   = 4, // 战斗效果
};

//资源的缩放类型
enum ResScaleType
{
    eNormalScale = 0,                //正常的缩放
    eScaledOriginalTexture = 1  //资源的原始图片已经被缩放了，用于光效，特效等大图片资源
};

// resource id manager

//击中类型
enum HitType
{
    eNoneHitType = 0,
    eDodge = 1,     //闪避
    eHit = 2,       //普通击中
    eCri = 3        //暴击
};

enum EffectWordColor
{
    ecRed = 0,
    ecOrange,
    ecGreen,
    ecYellow,

    effectColorNum
};

// 图层显示先级
enum LayerOrder{
    eUILayer  = 99,
    eULCloseApp = 98,
    eULRegister = 96,
    eULLogin = 95,
    eULTask = 91,
    eUISysAnnouncement = 90,
    eULMainUI = 89,
    eULDialog = 92,
    eGameLayer = 49,
    eForegroundLayer = 31,      //前景层---by 杰
    eObjLayer = 30,
    eMapLayer = 19,
    eMiddlegroundLayer = 18,
    eBackgroundLayer = 17,
};

////
//enum Components{
//    eStateMach=0,
//    eSprite,
//
//    MaxComponents,
//};
struct TaskInfor
{
    int      taskID;
    std::string tasktype;
    std::string taskrequire;
    std::string taskaward;
};
//进入战斗的对话框
enum DialogState
{
    eDialogStart,
    eDialogFinsh,
    eDialogNewbie,
    eDialogNewPet,
    eDialogRandomBoss,
};
//角色模板ID
enum RoleTpltID
{
    eMaleWarrior = 1, //男剑士
    eMaleMage = 2,     //男法师
    eMaleAssassin = 3,    //男刺客
    eFemaleWarrior = 4, //女剑士
    eFemaleMage = 5,   //女法师
    eFemaleAssassin = 6,   //女刺客
};

enum ConstellationName
{
    eConstellationNon,
    eAries,                 //白羊
    eTaurus,                //金牛
    eGemini,                //双子
    eCancer,                //巨蟹
    eLeo,                   //狮子
    eVirgo,                 //处女
    eLibra,                 //天秤
    eScorpio,               //天蝎
    eSagittarius,           //射手
    eCapricorn,             //魔蝎
    eAquarius,              //水瓶
    ePisces,                //双鱼
};
#define ROLE_BORN_SCENE 10000
//#define ROLE_BORN_SCENE 501101
#define MAX_OBJSKILL 8
enum SkillOwner{
    eOwnerMonster,      //怪物技能
    eOwnerWarrior,      //战士技能
    eOwnerMage,         //法师技能
    eOwnerAssassin,     //刺客技能
};
enum MonsterType
{
    eTypeError = 0,
    eLongRange = 1,
    eShorRange = 2,
    eBoss = 3,
};
enum SkillTarget {
    eTargetSelf = 0x0001,    //给自己有效
    eTargetRole = 0x0002,    //对其它玩家有效
    eTargetMonster = 0x0004, //对怪物有效
};
enum SkillAffect
{
    eNon,               //没有效果
    //攻击类
    eSkillRush,
    eSkillStaticEffect,
    eSkillFlyOnce,
    eSkillFlyContinue,
    eSkillBuffEffect,
    eSkillNewBody,
    eSkillMovableNewBodyAttack,
    eSkillLockTargetEffect,

    //纯表现类
    eShakeScreen,               //震屏

    //伤害类
    eHurtHitted,
    eHurtKnockBack,
    eHurtKnockDown,
    eHurtDizzy,
    eHurtFireExplosion,
    eHurtPoisoned,
    
    eSilenceEffect,         //沉默效果
    eControlEffect,         //控制效果
    eConstraintEffect,       //束缚效果

    //数值类
    eBuffIncrHP,            //持续性回血
    eBuffSuckBlood,         //吸血
    eBuffChangeAttack,      //改变攻击
    eBuffChangeDogge,       //改变闪避
    eBuffChangeCri,         //改变暴击
    eBuffChangeDef,         //改变防御
    eBuffChangeHit,         //改变命中
    eBuffChangeSpeed,

    //被动技能效果
    eIncrAtkForever,        //加攻
    eIncrDefForever,        //防
    eIncrMaxHPForever,      //血上限
    eIncrHitForever,        //命中
    eIncrDodgeForever,      //闪避
    eIncrCriForever,        //暴击
    eIncrSpeedForever,      //速度
    eIncrHpIncrForever,     //回血

    eIncrStreForever,       //加力
    eIncrInteForever,       //智
    eIncrPhysForever,       //体
    eIncrCapaForever,       //耐
};

//SkillAction的类型定义，用于标示SkillAction的子类。类似于ObjType，有了这个，不需要强转类型就知道
//--linshusen
enum SkillActionType
{
    eNoSkillAction = 0,
    eRush,                      //冲锋类技能
    eFly,                       //飞行类技能
    eContinueFly,               //持续性攻击的飞行类技能
    eStaticEffect,              //攻击固定特效类技能
    eHurtEffect,                //伤害特效类技能
    eBuffEffect,                //BUFF类技能
    eSuckBloodEffect,           //吸血技能
    eNewBodyEffect,             //变身类技能
    eMovableNewBodyAttackEffect,//可移动的变身类攻击特效
    eLockTargetEffect,          //锁定攻击目标的技能
    eKnockBackAction,           //被击退类
    eShakeScreenEffect,         //震屏效果
};

//技能效果的施放对象类型，可以进行与或的。所以枚举值必须是2的n次方
enum SkillAffectTargetType
{
    eForEnemies     = 1,    //对敌人施放
    eForSelf        = 2,    //对自己施放
    eForSelfGroup   = 4,    //对自己和队友施放
    eForPet         = 8,    //对宠物
};

//光效相对于角色出现的位置
enum ResPosType
{
    eOnFoot         =1,     //在脚底出现
    eOnBoundingBox  =2,     //在包围盒上方（大约腰部胸部）
    eOnHead         =3,     //在头顶
};

enum chatChannel
{
    eUnKnowChat,
    eWorld,      //世界
    eArmygroup,  //军团
    ePrivatechat,//私聊
    eGuild,      //公会
    eSystem,     //系统
    eSyncTeam,   //同步组队
    eMaxChannel  //频道的总数
};

enum awardTap{
    eLogin,     //登录
    eRoleAward,   // 角色奖励
    eInvitation, //激活码
    eLvlAward,    // 等级奖励
    eFriendInvitation, // 好友邀请
};
enum  presentStata
{
    eGuildball,  //公会大厅
    eGuildShop,  //公会商店
    eGuildSkill, //公会技能
};

enum SkillLearnState
{
    eNoLearn,       //没学过,不能学
    eNoLearnCanLearn,       //没学过，可以学习
    eHasLearnCanLearn,       //学过，可以学
    eHasLearnNoLearn,       //学过，不可以学
    eFinishLearn      //学完了
};


enum GameFunctionType
{
    eNonType,
    eOneEquipNoDrop,   //单个装备不可以出售
    eMultipleEquipDrop,//多个装备不可以出售
    
    eVipEffectShop,    //Vip影响神秘商店
    eVipNoEffectShop,  //Vip不影响
    
    eChangeMysticalCopy, //幻兽大本营（动态列表）
    eNoChangeMysticalCopy,//不修改幻兽大本营（静态列表）
    
    eKrLvlRewardTyp,      // 有等级奖励
    eNoKrLvlRewardTyp,    // 没有等级奖励
    
    eMaxlvlAddType,      //最高等级上限增加值
    eExtlvAddType,           //与角色等级差增加值
    
    eOldSuperActivityType,   // 幻兽大本营 和 精英副本 互换
    eNewSuperActivityType,
    
    eServerControlBBS,      //服务器觉得bbs时候开放
    eClientControlBBS,
    
    ///充值接口开关类型，对应GameFunction.ini 里面的chargeInterface。除了常规的sdk充值，还提供了备用的 调用游戏内webView打开网页充值 和  弹出浏览器打开网页充值 -------by J杰
        eNormalCharge,    //常规sdk充值
        eWebViewCharge, //webView充值
        eBroswerCharge,        //浏览器充值
    
    ePetMergeList,      //可合体幻兽列表
    eTypeForPetMerger,  //用于幻兽合体的类型
	
    eEffectLimitTimeKilled,  // 活动影响限时击杀
	eNoEffectLimitTimeKilled,  // 活动不限时击杀
    
    eFriendInvitationOpen,
    eFriendInvitationClose,
};
enum LeaveTlkCopyType
{
    eGiveUpRewardsType = 0,  //不消耗门票，放弃奖励
    eGetRewardsType = 1,    //消耗门票，领取奖励
};

#define PREV_MAX_ROLE_LVL 70
#define MAX_ROLE_LVL 100

#define MAX_PET_LIST 50




//#define TPLT_MIN_ID 1000
//#define TPLT_MAX_ID 99999
//服务器状态
enum ServerState
{
    eServerOK = 0,              //服务器正常
    eServerMaintaining = 1,     //服务器正在维护
    eServerClosed = 2           //服务器已关闭
};
//服务器定义
struct Server
{
    int mID;
    int mViewID;
    std::string mName;
    std::string mIP;
    int mPort;
    bool mIsNew;
    bool mIsDefault;
    ServerState mState;
    std::string mDescription;
    
    Server()
    {
        mID = 0;
        mViewID = 0;
        mName = "";
        mIP = "";
        mPort = 0;
        mIsNew = false;
        mIsDefault = false;
        mDescription = "";
        mState = eServerOK;
    }
};

struct PlatformNotice
{
    int mRet;
    std::string mContent;
    
    PlatformNotice()
    {
        mRet=0;
        mContent="";
    }
};
struct JumpWebsite
{
    bool isOpen;
    std::string mContent;
    JumpWebsite()
    {
        isOpen = false;
        mContent = "";
    }
};
struct ObjSave
{
   
#ifdef CLIENT
    ReadWrite_Crypt(ObjJob, mJob, Job)             //职业
    ReadWrite_Crypt(ObjSex, mSex, Sex)             //性别
    ReadWrite_Crypt(int, mRoleType, RoleType)
    ReadWrite_Crypt(int, mExp, Exp)    //当前经验值
    ReadWrite_Crypt(int, mHpForDelete, HpForDelete)//当前生命值-------这个要转移到BattleProp里面--linshusen
    ReadWrite_Crypt(int, mMp, Mp)//当前魔法值
    ReadWrite_Crypt(int, mFat, Fat)//当前精力值
    ReadWrite_Crypt(int, mGold, Gold)//金币
    ReadWrite_Crypt(int, mRmb, Rmb)//金钻
    ReadWrite_Crypt(int, mBattlePoint, BattlePoint)//战功
    ReadWrite_Crypt(int, mConstellVal, ConstellVal)  //星灵值
    ReadWrite_Crypt(int, mLastFinishCopy, LastFinishCopy)
 
    ReadWrite_Crypt(int, mVipLvl, VipLvl)
    ReadWrite_Crypt(int, mVipExp, VipExp)
    ReadWrite_Crypt(int, mVipAward, VipAward)
    ReadWrite_Crypt(int, mVipLvlAward, VipLvlAward)
    ReadWrite_Crypt(int, mChargeVal, ChargeVal)
    ReadWrite_Crypt(int, mEliteTimes, EliteTimes)
    ReadWrite_Crypt(int, mEliteBuyTimes, EliteBuyTimes)
    ReadWrite_Crypt(int, mFatFreshTime, FatFreshTime)
    ReadWrite_Crypt(int, mFatBuyTimes, FatBuyTimes)
    ReadWrite_Crypt(int, mPvpAward, PvpAward)
    ReadWrite_Crypt(int, mPvpTimes, PvpTimes)
    ReadWrite_Crypt(int, mPvpBuyTimes, PvpBuyTimes)
    ReadWrite_Crypt(int, mLoginAward, LoginAward)
    ReadWrite_Crypt(int, mMoneyTreeTimes, MoneyTreeTimes)
    ReadWrite_Crypt(int, mCrossWeek, CrossWeek)
    ReadWrite_Crypt(int, mCrossDay, CrossDay)
    ReadWrite_Crypt(int, mTeamCopyResetTimes, TeamCopyResetTimes)
    ReadWrite_Crypt(int, mBanChat, BanChat)
    ReadWrite_Crypt(int, mBattleForce, BattleForce)
    ReadWrite_Crypt(int, mIsInPvp, IsInPvp)
    ReadWrite_Crypt(int, mPetEliteTimes, PetEliteTimes)
    ReadWrite_Crypt(int, mPetEliteBuyTimes, PetEliteBuyTimes)
    //ReadWrite_Crypt(int, mDailyQuestRefreshTimes, DailyQuestRefreshTimes)
    ReadWrite_Crypt(int, mDoublePotionNum, DoublePotionNum); //双倍药水奖励 次数
    ReadWrite_Crypt(int, mEnchantDust, EnchantDust)       // 附魔尘, 普通的
    ReadWrite_Crypt(int, mEnchantGold, EnchantGold)       // 附魔钻,
    ReadWrite_Crypt(int, mEnchantId, EnchantId);          //附魔id
    ReadWrite_Crypt(int, mEnchantLvl, EnchantLvl);        //附魔等级
    ReadWrite_Crypt(int, mPetAdventureTimes, PetAdventureTimes); //幻兽大冒险次数
    ReadWrite_Crypt(int, mPetAdventureBuyTimes, PetAdventureBuyTimes); //幻兽大冒险购买次数
    
    ReadWrite_Crypt(int, mPetAdventureRobotPetTimes, PetAdventureRobotPetTimes); //幻兽大冒险机器人次数
    ReadWrite_Crypt(int, mPetAdventureRobotPetBuyTimes, PetAdventureRobotPetBuyTimes); //幻兽大冒险机器人购买次数
    
    ReadWrite_Crypt(int, mCrossServiceWarWorshipTimes, CrossServiceWarWorshipTimes);//膜拜次数
    ReadWrite_Crypt(int, mCrossServiceWarWorshipBuyTimes, CrossServiceWarWorshipBuyTimes);//膜拜购买次数
    ReadWrite_Crypt(int, mCrossServiceWarTimes, CrossServiceWarTimes);//跨服战次数
    ReadWrite_Crypt(int, mCrossServiceWarBuyTimes, CrossServiceWarBuyTimes);//跨服战购买次数
    
    
    ReadWrite_Crypt(int, mRoleAwakeLvl, RoleAwakeLvl)  // 觉醒等级
    
    ReadWrite_Crypt(int, mRoleAccumulateRechargeRecord, RoleAccumulateRechargeRecord)  //累计充值记录
    ReadWrite_Crypt(int, mRoleAccumulateRmbConsumedRecord, RoleAccumulateRmbConsumedRecord)  //累计消费记录
#else
    //READWRITE(int, mLvl, Lvl)
    READWRITE(ObjJob, mJob, Job)             //职业
    READWRITE(ObjSex, mSex, Sex)             //性别
    READWRITE(int, mRoleType, RoleType)
    READWRITE(int, mExp, Exp)    //当前经验值
    READWRITE(int, mHpForDelete, HpForDelete)//当前生命值-------这个要转移到BattleProp里面--linshusen
    READWRITE(int, mMp, Mp)//当前魔法值
    READWRITE(int, mFat, Fat)//当前精力值
    READWRITE(int, mGold, Gold)//金币
    READWRITE(int, mRmb, Rmb)//金钻
    READWRITE(int, mBattlePoint, BattlePoint)//战功
    READWRITE(int, mConstellVal, ConstellVal)  //星灵值
    READWRITE(int, mLastFinishCopy, LastFinishCopy)
    
    READWRITE(int, mDungLevel1, DungLevel1)
    READWRITE(int, mDungLevel2, DungLevel2)
    READWRITE(int, mDungLevel3, DungLevel3)
    
    READWRITE(int, mDungResetTimes1, DungResetTimes1)
    READWRITE(int, mDungResetTimes2, DungResetTimes2)
    READWRITE(int, mDungResetTimes3, DungResetTimes3)
    
    READWRITE(int, mDungTimes, DungTimes)
    
    READWRITE(int, mVipLvl, VipLvl)
    READWRITE(int, mVipExp, VipExp)
    READWRITE(int, mVipAward, VipAward)
    READWRITE(int, mVipLvlAward, VipLvlAward)
    READWRITE(int, mChargeVal, ChargeVal)
    READWRITE(int, mEliteTimes, EliteTimes)
    READWRITE(int, mEliteBuyTimes, EliteBuyTimes)
    READWRITE(int, mFatFreshTime, FatFreshTime)
    READWRITE(int, mFatBuyTimes, FatBuyTimes)
    READWRITE(int, mPvpAward, PvpAward)
    READWRITE(int, mPvpTimes, PvpTimes)
    READWRITE(int, mPvpBuyTimes, PvpBuyTimes)
    READWRITE(int, mLoginAward, LoginAward)
    READWRITE(int, mMoneyTreeTimes, MoneyTreeTimes)
    READWRITE(int, mCrossWeek, CrossWeek)
    READWRITE(int, mCrossDay, CrossDay)
    READWRITE(int, mTeamCopyResetTimes, TeamCopyResetTimes)
    READWRITE(int, mBanChat, BanChat)
    READWRITE(int, mBattleForce, BattleForce)
    READWRITE(int, mIsInPvp, IsInPvp)
    READWRITE(int, mDailyQuestRefreshTimes, DailyQuestRefreshTimes)
    READWRITE(int, mDailyQuestCount, DailyQuestCount)
    READWRITE(int, mActivePetId, ActivePetId)
//    READWRITE(int, mTowerDefenseTimes, TowerDefenseTimes)
    READWRITE(int, mMysticalCopyTimes, MysticalCopyTimes)
    READWRITE(int, mPetPvpTimes, PetPvpTimes)
    READWRITE(int, mNextWorldChatTime, NextWorldChatTime)
    READWRITE(int, mNextPvpResetTime, NextPvpResetTime)
    READWRITE(int, mWeaponQua, WeaponQua)
    READWRITE(int, mRoleAnger, RoleAnger)
    READWRITE(int, mLastLogintm, LastLogintm)
    READWRITE(int, mEnchantDust, EnchantDust)       // 附魔尘, 普通的
    READWRITE(int, mEnchantGold, EnchantGold)       // 附魔钻,
    READWRITE(int, mLoginDays, LoginDays)           // 登陆天数
    READWRITE(int, mRoleAwakeLvl, RoleAwakeLvl)     // 觉醒等级

#endif
    
#ifdef CLIENT
    ObjSave()
    {
        setJob(eWarrior);
        setSex(eBoy);
        setRoleType(0);
        setExp(0);
        setHpForDelete(0);
        setMp(0);
        setFat(0);
        setGold(0);
        setRmb(0);
        setBattlePoint(0);
        setConstellVal(0);
        setLastFinishCopy(0);
        setVipLvl(0);
        setVipExp(0);
        setVipAward(0);
        setVipLvlAward(0);
        setChargeVal(0);
        setEliteBuyTimes(0);
        setEliteTimes(0);
        setFatFreshTime(0);
        setFatBuyTimes(0);
        setPvpAward(0);
        setPvpTimes(0);
        setPvpBuyTimes(0);
        setLoginAward(0);
        setMoneyTreeTimes(0);
        setCrossWeek(0);
        setCrossDay(0);
        setTeamCopyResetTimes(0);
        setBanChat(0);
        setBattleForce(0);
        setIsInPvp(0);
        setPetEliteTimes(0);
        setPetEliteBuyTimes(0);
        setDoublePotionNum(0);
        
        setEnchantDust(0);
        setEnchantGold(0);
        setEnchantId(0);
        setEnchantLvl(0);
        
        setPetAdventureTimes(0);
        setPetAdventureBuyTimes(0);
        
        setCrossServiceWarTimes(0);
        setCrossServiceWarBuyTimes(0);
        setCrossServiceWarWorshipTimes(0);
        setCrossServiceWarWorshipBuyTimes(0);
        
        setRoleAwakeLvl(0);
        
        setRoleAccumulateRechargeRecord(0);
        setRoleAccumulateRmbConsumedRecord(0);
    }
#endif
};

struct BaseProp
{
#ifdef CLIENT
    ReadWrite_Crypt(int, mStre, Stre)//力量
    ReadWrite_Crypt(int, mInte, Inte)//智力
    ReadWrite_Crypt(int, mPhys, Phys)//体力
    ReadWrite_Crypt(int, mCapa, Capa)//耐力
#else
    READWRITE(int, mStre, Stre)//力量
    READWRITE(int, mInte, Inte)//智力
    READWRITE(int, mPhys, Phys)//体力
    READWRITE(int, mCapa, Capa)//耐力
#endif
    BaseProp()
    {
        setStre(0);
        setInte(0);
        setPhys(0);
        setCapa(0);
    }
    BaseProp& operator+=(const BaseProp& prop)
    {
        setStre(getStre() + prop.getStre());
        setInte(getInte() + prop.getInte());
        setPhys(getPhys() + prop.getPhys());
        setCapa(getCapa() + prop.getCapa());
        return *this;
    }
    BaseProp& operator*=(float rate)
    {
        setStre(getStre() * rate);
        setInte(getInte() * rate);
        setPhys(getPhys() * rate);
        setCapa(getCapa() * rate);
        return *this;
    }
};

// 基础属性乘法加成系数
struct BasePropMultiFactor
{
#ifdef CLIENT
    ReadWrite_Crypt(float, mStreFactor, StreFactor)//力量
    ReadWrite_Crypt(float, mInteFactor, InteFactor)//智力
    ReadWrite_Crypt(float, mPhysFactor, PhysFactor)//体力
    ReadWrite_Crypt(float, mCapaFactor, CapaFactor)//耐力
#else
    READWRITE(float, mStreFactor, StreFactor)//力量
    READWRITE(float, mInteFactor, InteFactor)//智力
    READWRITE(float, mPhysFactor, PhysFactor)//体力
    READWRITE(float, mCapaFactor, CapaFactor)//耐力
#endif
    BasePropMultiFactor()
    {
        setStreFactor(0.0f);
        setInteFactor(0.0f);
        setPhysFactor(0.0f);
        setCapaFactor(0.0f);
    }
    
    
    BasePropMultiFactor& operator+=(const BasePropMultiFactor& prop)
    {
        setStreFactor( getStreFactor() + prop.getStreFactor() );
        setInteFactor( getInteFactor() + prop.getInteFactor() );
        setPhysFactor( getPhysFactor() + prop.getPhysFactor() );
        setCapaFactor( getCapaFactor() + prop.getCapaFactor() );
        return *this;
    }
};

// 战斗属性乘法加成系数
struct BattlePropMultiFactor
{
    BattlePropMultiFactor()
    {
        setLvlFactor(0.0f);
        setMaxHpFactor(0.0f);
        setMaxMpFactor(0.0f);
        setAtkFactor(0.0f);
        setDefFactor(0.0f);
        setHitFactor(0.0f);
        setDodgeFactor(0.0f);
        setCriFactor(0.0f);
        setMoveSpeedFactor(0.0f);
        setHpIncrFactor(0.0f);
    }
    
    BattlePropMultiFactor& operator+=(const BattlePropMultiFactor& prop)
    {
        setLvlFactor(getLvlFactor()+prop.getLvlFactor());
        setMaxHpFactor(getMaxHpFactor()+prop.getMaxHpFactor());
        setMaxMpFactor(getMaxMpFactor() + prop.getMaxMpFactor());
        setAtkFactor(getAtkFactor() + prop.getAtkFactor());
        setDefFactor(getDefFactor() + prop.getDefFactor());
        setHitFactor(getHitFactor() + prop.getHitFactor());
        setDodgeFactor(getDodgeFactor() + prop.getDodgeFactor());
        setCriFactor(getCriFactor() + prop.getCriFactor());
        setMoveSpeedFactor(getMoveSpeedFactor() + prop.getMoveSpeedFactor());
        setHpIncrFactor(getHpIncrFactor() + prop.getHpIncrFactor());
        return *this;
    }
    
#ifdef CLIENT
    ReadWrite_Crypt(float, mLvlFactor, LvlFactor)
    ReadWrite_Crypt(float, mMaxHpFactor, MaxHpFactor) //最大生命值
    ReadWrite_Crypt(float, mMaxMpFactor, MaxMpFactor)//最大魔法值
    ReadWrite_Crypt(float, mAtkFactor, AtkFactor)//攻击
    ReadWrite_Crypt(float, mDefFactor, DefFactor)//防御
    ReadWrite_Crypt(float, mHitFactor, HitFactor)//命中
    ReadWrite_Crypt(float, mDodgeFactor, DodgeFactor)//闪避
    ReadWrite_Crypt(float, mCriFactor, CriFactor)//暴击
    ReadWrite_Crypt(float, mMoveSpeedFactor, MoveSpeedFactor)//速度
    ReadWrite_Crypt(float, mHpIncrFactor, HpIncrFactor)//回血
#else
    READWRITE(float, mLvlFactor, LvlFactor) //lvl
    READWRITE(float, mMaxHpFactor, MaxHpFactor) //最大生命值
    READWRITE(float, mMaxMpFactor, MaxMpFactor)//最大魔法值
    READWRITE(float, mAtkFactor, AtkFactor)//攻击
    READWRITE(float, mDefFactor, DefFactor)//防御
    READWRITE(float, mHitFactor, HitFactor)//命中
    READWRITE(float, mDodgeFactor, DodgeFactor)//闪避
    READWRITE(float, mCriFactor, CriFactor)//暴击
    READWRITE(float, mMoveSpeedFactor, MoveSpeedFactor)//速度
    READWRITE(float, mHpIncrFactor, HpIncrFactor)//回血
#endif
    
};


struct BattleProp
{
    BattleProp()
    {
        setAtk(0);
        setLvl(0);
        setHp(0);
        setMaxHp(0);
        setMaxMp(0);
        setDef(0);
        setHit(0);
        setDodge(0);
        setCri(0.0);
        setMoveSpeed(0);
        setHpIncr(0);
    }

    BattleProp& operator+=( BattleProp& prop)
    {
        setHp(getHp()+prop.getHp());
        setMaxHp(getMaxHp()+prop.getMaxHp());
        setMaxMp(getMaxMp() + prop.getMaxMp());
        setAtk(getAtk() + prop.getAtk());
        setDef(getDef() + prop.getDef());
        setHit(getHit() + prop.getHit());
        setDodge(getDodge() + prop.getDodge());
        setCri(getCri() + prop.getCri());
        setMoveSpeed(getMoveSpeed() + prop.getMoveSpeed());
        setHpIncr(getHpIncr() + prop.getHpIncr());
        return *this;
    }
    
    BattleProp& operator*=(float rate)
    {
        setHp(getHp() * rate);
        setMaxHp(getMaxHp() * rate);
        setMaxMp(getMaxMp() * rate);
        setAtk(getAtk() * rate);
        setDef(getDef() * rate);
        setHit(getHit() * rate);
        setDodge(getDodge() * rate);
        setCri(getCri() * rate);
        setMoveSpeed(getMoveSpeed() * rate);
        setHpIncr(getHpIncr() * rate);
        return *this;
    }
    
    BattleProp operator*(float rate)
    {
        BattleProp prop;
        prop.setHp(getHp() * rate);
        prop.setMaxHp(getMaxHp() * rate);
        prop.setMaxMp(getMaxMp() * rate);
        prop.setAtk(getAtk() * rate);
        prop.setDef(getDef() * rate);
        prop.setHit(getHit() * rate);
        prop.setDodge(getDodge() * rate);
        prop.setCri(getCri() * rate);
        prop.setMoveSpeed(getMoveSpeed() * rate);
        prop.setHpIncr(getHpIncr() * rate);
        return prop;
    }
    
#ifdef CLIENT
    ReadWrite_Crypt(int, mLvl, Lvl)
    ReadWrite_Crypt(int, mHp, Hp)
    ReadWrite_Crypt(int, mMaxHp, MaxHp) //最大生命值
    ReadWrite_Crypt(int, mMaxMp, MaxMp)//最大魔法值
    Virtual_ReadWrite_Crypt(int, mAtk, Atk)//攻击
    Virtual_ReadWrite_Crypt(int, mDef, Def)//防御
    Virtual_ReadWrite_Crypt(float, mHit, Hit)//命中
    Virtual_ReadWrite_Crypt(float, mDodge, Dodge)//闪避
    Virtual_ReadWrite_Crypt(float, mCri, Cri)//暴击
    ReadWrite_Crypt(int, mMoveSpeed, MoveSpeed)//速度
    ReadWrite_Crypt(int, mHpIncr, HpIncr)//回血
#else
    READWRITE(int, mLvl, Lvl) //lvl
    READWRITE(int, mHp, Hp)
    READWRITE(int, mMaxHp, MaxHp) //最大生命值
    READWRITE(int, mMaxMp, MaxMp)//最大魔法值
    VIRTUAL_READWRITE(int, mAtk, Atk)//攻击
    VIRTUAL_READWRITE(int, mDef, Def)//防御
    VIRTUAL_READWRITE(float, mHit, Hit)//命中
    VIRTUAL_READWRITE(float, mDodge, Dodge)//闪避
    VIRTUAL_READWRITE(float, mCri, Cri)//暴击
    READWRITE(int, mMoveSpeed, MoveSpeed)//速度
    READWRITE(int, mHpIncr, HpIncr)//回血

#endif
    
    // 计算最基础的普通攻击，没暴击没浮动
    int calBasicDamage(BattleProp *src)
    {
        int dmg = 0;
        float DefPro = this->getDef() / (float)(this->getDef() + this->getLvl()*60 + 500);
        dmg = src->getAtk()*(1.0f - DefPro);
        dmg = dmg > 0 ? dmg : 1;
        return dmg;
    }

    //计算普通攻击伤害
    HitType calDamage(BattleProp *src, int& dmg)
    {
        float DefPro = this->getDef() / (float)(this->getDef() + this->getLvl()*60 + 500);
        float rol1 = rand()%100;
        dmg = src->getAtk()*(1.0f - DefPro) * ( 0.95f + 0.1 *(rol1/100));
        dmg = dmg > 0 ? dmg : 1;

        int rol = rand()%10000; //精确到小数点后两位
        float realCri = powf(src->getCri() / (50 * src->getLvl()), 0.5);
        
        if ( (int)(realCri * 10000) > rol) {
            dmg = dmg * 1.5f;
            return eCri;
        }
        dmg = dmg >= 0 ? dmg : 0;
        return eHit;
    }
    //计算暴击伤害
    int calCriDamage(BattleProp *src)
    {
        int dmg = 0;
        float DefPro = getDef() / (float)(getDef() + getLvl()*60 + 500);
        float rol1 = rand()%100;
        dmg = src->getAtk()*(1.0f - DefPro) * ( 0.95f + 0.1 *(rol1/100));
        dmg = dmg > 0 ? dmg : 1;

        dmg = dmg * 1.5f;
        return dmg;
    }
};

int calcBattleForce(BattleProp * battleProp);
int calVerifyCode(BattleProp * battleProp);

//验证战斗属性的类型（时间点）---linshusen
enum VerifyPropType
{
    eVPT_EnterCopy = 1,     //进入副本时
    //eVPT_InCopy = 2,     //副本中过段时
    eVPT_InTown = 3,        //在主城中
    eVPT_InPVP = 4,
    eVPT_InCopy = 5,        //在副本中
};

struct DungeAward
{
    int sceneid;
    int flopIndex;
    std::string drops;
    
    int gold;
    int exp;
    int battlepoint;
    int constell;
    int petexp;
	std::string awardItems;
};

enum npcQuestSign
{
    nqsNone = 0,
    nqsAccept = 1,
    nqsSubmit = 2,
    nqsDoing = 3,
};

enum eNpcType  //npc类型
{
    ePersonNpcType = 0,    //人物npc
    eBulletinBoardNpcType, //公告栏npc
    eDecorationNpcType,    //装饰npc
};

enum GameState
{
    eLoading,
    eReady,
    ePlaying,
    eCutScene,
    ePass,
    eFail,
//    ePause,   //14.1.3 去掉暂停状态，不排除会再次开启，需要开启是把注释掉的ePause代码都开启就好了  -------J杰
};

    //战斗模式
enum FightModel
    {
        eInTown,            //在主城
        eSingelPlayerCopy,  //单人副本
        eMultiPlayerCopy,   //多人副本
        ePvp,               //pvp
        ePetPvp,            //宠物竞技场，没有角色参加
        eMultiPvp,          //多人pvp（名人堂）
        eControlPet,        //操作幻兽
        eSyncFighting,       //同步战斗
        ePetAdventure,       //幻兽大冒险模式 主角和任意幻兽出战
        
        
    };

enum msgType{
    mtRecvQuest,
    mtGiveUpQuest,
    mtFinishQuest,
    mtLoginAward,
    mtLoginVipAward,
    mtMoneyTreeState,
    mtClickMoneyTree,
    mtHandUp,
    mtBeginPvp,
    mtBeginTeamCopy,
    mtLottery,//抽奖状态
    mtInvitationCode,//激活码
    mtBeginTeamDungeon,
    mtBeginPetDungeon,
    mtBeginMakeprintCopy,
    mtRefreshPvp,
    mtPetCopy,
    mtActivity,
    mtWorldBoss,
    mtPPLogin,
    mtTBLogin,
    mtEnterDefendStatue,
    mtPetCollectAward,
    mtCheckPersonInfor,
    mtCheckPetInfor,
    mtEnterMysticalCopy,
    mtResetDailyQuest,
    mtGetPetArenaData,
    mtSearchPetArenaEnemy,
    mtBeginPetArena,
    mtTrainBoss,// 公会boss培养
    mtPresentGuildGoldAndRmb,//公会捐赠
    mtPetSkillUpdate,  //技能升级
    mtPetSkillStudy,   //技能学习
    mtPetUpGrowth,   //幻兽洗炼
    mtEnterLimitTimerCopy,  //限时活动
    mtEnterChallenge,
    mtSkillUnEquip,
    mtSkillLoadEquip,
    mtPetEliteCopy,     //幻兽竞技场
    mtCopy,             //普通副本
    mtReqPetEliteList,
    mtChargeActivity,
    mtSynPvp,           // 同步竞技
    mtFirstCharge,      // vip大礼
    mtEnterNormalDunge,  //进入普通地下城
    mtEnterEliteCopy,    //精英副本
    mtPutonFashion,        //使用时装
    mtTakeoffFashion,
    mtTotemOffer,       // 幻兽献祭
    mtMonthCard,        // 月卡系统
    mtSyncTeamCopyRoomList,
    mtSyncTeamCopyCopyList,
    mtSyncTeamCopyRoom,
    mtSyncTeamCopyInvite,
    mtDungeCopySweeping,
    mtPetAdventureCopy,  //幻兽大冒险
    mtCrossServiceWar, //跨服战
    mtBeginCS,   //开始跨服战
    mtRefreshCS, //刷新跨服战
    mtWeaponEnchant,     // 武器附魔
    mtActivateBag,      //
    mtRoleAward,        //角色奖励
    mtRoleAwake,        //角色觉醒
    mtEquipStreng,      //装备强化
    mtEquipBaptize,     //装备洗炼
    mtPvpShop,
    mtRetinueSkillUnEquip, // 侍魂技能 卸下
    mtRetinueSkillEquip,    // 侍魂技能 装备
    mtRetinueLvlUp,
    mtRetinueStarUp,
};

enum  TaskState
{
    eAcceptTask =1,
    eHandTask,
    eConversation,
    eDoingTask,
};

enum PetType
{
    ePetCloseAtk = 1,  //近攻
    ePetCloseDef, //近防
    ePetRemoteAtk, //远攻
};

//enum QualityColor
//{
//    //绿蓝紫粉金
//    eQuaGreen = 1,
//    eQuaBlue,
//    eQuaPurple,
//    eQuaPink,
//    eQuaGolden,
//    
//    eQuaWhiteGold,  //白金
//    eQuaDarkGold,   //暗金
//    eQuaRedGold,    //赤金
//    
//};
enum PetMonsterDegree
{
   //普通、精英、boss
    dOrdinary = 1,
    dElite,
    dBoss,
    dSpecies,
};
enum LoadingType
{
    ltBeginGame = 4,
    ltLoginRole = 9,
    ltEnterScene  = 5,
};
enum ProcessType
{
    ptStart = 1,
    ptProcess,
    ptEnd,
    ptError,
};

//装备枚举，对应装备栏格子
enum EquipPartType
{
    kEPTHeadGuard, //头部
    kEPTNecklace,//项链
    kEPTBracelet,//手镯
    kEPTRing,//戒指
    kEPTWeapon,//武器
    kEPTBreastGuard,//胸甲
    kEPTHandGuard,//护手
    kEPTLegGuard,//护腿
    kEPTFootGuard,//护脚
    kEPTFashion,//时装
    kEPTCount
};

enum {
   /*
    0.系统                        content=color&content1=color1&content2=color2
    1.获得紫色\粉色幻兽            rolename=abcd&petmod=310001
    2.获得(提品)了紫色\粉色装备    rolename=abcd&equipid=310001
    3.通关地下城20层             rolename=abcd&dungeonid=310000&floor=12
    4.竞技场排名提升到第一        rolename=abcd&rank=310001
    5.幻兽提升到8\9\10星级       rolename=abcd&petmod=310001&petstar=8
    6.合成8级或以上的石头         rolename=abcd&stoneid=310001
    7.vip等级提升                rolename=abcd&lvl=7
    8.幻兽大比拼结果1/4         round=1&winers=34&wowner=成功主人名&wpetmod=10012&losers=45&lowner=失败主人名&lpetmod=10035
    9.幻兽大比拼结果1/2         同上
    10.幻兽大比拼结果决赛        同上
    11.幻兽大比拼-慧眼识珠     name1=玩家1&name2=玩家2&name3=玩家3&wowner=成功主人名&wpetmod=10012
    */
    kBroadcastSystem = 0,
    kBroadcastGetPet = 1,
    kBroadcastPromoteEquip = 2,
    kBroadcastDungeon = 3,
    kBroadcastPvpRank = 4,
    kBroadcastPetStar = 5,
    kBroadcastGetStone = 6,
    kBroadcastVipLvl = 7,
    kBroadcastPetMatchResultQuarter = 8,
    kBroadcastPetMatchResultHalf = 9,
    kBroadcastPetMatchResultFinal = 10,
    kBroadcastPetMatchDiscerningEyes = 11,
};


//苹果充值返回值
enum enAppleRechargeRet
{
    enAppleRecharge_Undefine = 0,                       //未定义
    enAppleRecharge_Success = 1,                        //成功
    enAppleRecharge_Fail = 2,                           //失败
    enAppleRecharge_TransExist =3,                      //该单充过了
    enAppleRecharge_ServerBusy =4,                       //服务器忙
    enAppleRecharge_ServerError =5                       //服务器错误
};


//宠物吞噬返回值
enum enPetAbsorbRetsult
{
    enPetAbsorbRetsult_SuccessAndStarlvlUpgrade = 0 ,   //成功,且升星成功
    enPetAbsorbRetsult_SuccussButStarlvlUnchange,       //成功,但升星失败
    enPetAbsorbRetsult_PetIdError,                      //失败，宠物不存在
    enPetAbsorbRetsult_ActivePetCanotBeAbsorbed ,       //失败，出战宠不能被吃
    enPetAbsorbRetsult_CanotAbsorbedYourSelf,           //失败，不能吃自己
    enPetAbsorbRetsult_PetAbsorAmountError,             //失败，被吃宠物数量非法[1,6]
    enPetAbsorbRetsult_NeedMoreGold,                    //失败，金币不足
    enPetAbsorbRetsult_UnknownError,                    //失败，服务器SB了
    
    enPetAbsorbRetsult_SuccussAndStageUpgrade,          //成功，且升阶成功2013.9.12
    enPetAbsorbRetsult_SuccussButStageUnchange,         //成功，但升阶失败
};

    //幻兽合并返回值
enum ePetMergeRetsult
{
    ePetMergeRetsult_UnknowError = -1,          // 未知错误
    ePetMergeRetsult_Ok,                        // 合并成功
    ePetMergeRetsult_MergeNumError,             // 合并数量错误
    ePetMergeRetsult_PetIdError,                // 宠物ID错误，可能角色没有这个宠物,或者配置表没找到此宠物
    ePetMergeRetsult_CanotMergeYourself,        // 不能合并自己
    ePetMergeRetsult_ActivePetCanotBeMerged,    // 出战宠不能被合并
    ePetMergeRetsult_PetAssistCanotBeMerged,    // 助阵宠物不能被合并
    ePetMergeRetsult_PetsNotSameTypeOrNotSameQua,// 宠物不是相同类型或者不是相同品质的
    ePetMergeRetsult_MainPetIsMaxStageLvl,       // 主幻兽已经是最大升阶等级了
    ePetMergeRetsult_YouNeedMoreGold,            // 金钱不够
	ePetMergeRetsult_QuaError					// 选择宠物的品质有误
};

// 宠物升级技能返回值
enum ePetStudySkill
{
    ePetStudySkill_Success = 0,         //成功
    ePetStudySkill_Fail,                //失败
    ePetStudySkill_BookSkillNotMatch,   //普通特殊技能 ， 书品质与宠不匹配
    ePetStudySkill_NeedMoreGold,        //金币不足
};

// 宠物技能解锁返回值
enum ePetUnlockSkill
{
    ePetUnlockSkill_Success = 0,    // 成功
    ePetUnlockSkill_UnknownError,   // 未知错误
    ePetUnlockSkill_NotEnoughMoney, // 金币不够
    ePetUnlockSkill_SkillIsUnlocked,// 技能已经解锁了
    ePetUnlockSkill_NotEnoughItem,  // 物品不足
    ePetUnlockSkill_ItemNotMatched, // 物品没有匹配到
    ePetUnlockSkill_CfgError,       // 配置错误
};

// 宠物洗练成长值返回值
enum ePetRefineGrowth
{
    ePetRefineGrowth_Succ = 0,  //成功
    ePetRefineGrowth_Fail,      //失败，要扣成长值
    ePetRefineGrowth_GrowthMax, //成长值已最大
    ePetRefineGrowth_NeedMorePill,  //成长丸不够
};

//抽奖结果返回值
enum enLotteryReuslt
{
    enLotteryReuslt_Success = 0,                        //成功
    enLotteryReuslt_NeedMoreRmb ,                       //失败，金钻不够
    enLotteryReuslt_TimesEmtpy ,                        //失败，抽奖次数用光
    enLotteryReuslt_BagFull,                            //失败，背包满了
    enLotteryReuslt_UnknowError                         //失败，服务器SB了
};

// 色子抽奖结果返回值
enum eDiceLotteryReuslt
{
    eDiceLotteryReuslt_Success = 0,                     //成功
    eDiceLotteryReuslt_NeedMoreRmb ,                    //失败，金钻不够
    eDiceLotteryReuslt_BagFull                          //失败，背包满了
};

// 系统聊天框指令
enum enSystemChatResult
{
	enSystemChatResult_Success = 0,                     // 成功
	enSystemChatResult_Matching,						// 规则不对
	enSystemChatResult_No								// 不处理
};

//激活码结果返回值
enum enActivationReuslt
{
    enActivationReuslt_Success = 0,                     // 成功
    enActivationReuslt_Invalid,                         // 激活码无效
    enActivationReuslt_Use,                             // 激活码已使用
    enActivationReuslt_ReceiveType,                     // 该类型激活码已使用
    enActivationReuslt_Platform,                        // 该激活码不能用于该平台
    enActivationReuslt_BagFull,                         // 失败，背包满了
    enActivationReuslt_SelfUse,                         // 自己使用了激活码
	enActivationReuslt_Expired,                         //
	enActivationReuslt_Start,							// 未开始
	enActivationReuslt_End,								// 已过期
	enActivationReuslt_Os,								// 不能用于该手机系统
	enActivationReuslt_Server,							// 不能用于该服
    enActivationReuslt_UnknowError                      // 服务器出错
};
	
enum enStoreGoodsType
{
	enStoreGoodsType_Non,
	enStoreGoodsType_RmbCost,		// 金钻
	enStoreGoodsType_ConsumeCost,	// 金币
    enStoreGoodsType_OnsaleCost,    // 特卖
    enStoreGoodsType_PropsExchange  // 道具兑换商品
};
    
// 神秘商人列表结果返回值
enum enMysteriousListReuslt
{
    enMysteriousListReuslt_Success = 0,                 // 成功
    enMysteriousListReuslt_NoOpen,                      // 活动未开启
    enMysteriousListReuslt_UnknowError                  // 异常
};
  
// 手动刷新神秘商人列表结果返回值
enum enManuallyMysteriousReuslt
{
    enManuallyMysteriousReuslt_Success = 0,             // 成功
    enManuallyMysteriousReuslt_NoOpen,                  // 活动未开启
    enManuallyMysteriousReuslt_NoRmb,                   // 金钻不足
    enManuallyMysteriousReuslt_RefreshTimesIsUseUp,     //刷新次数用完
    enManuallyMysteriousReuslt_UnknowError              // 异常
};

// 购买神秘商人商品结果返回值
enum enBuyMysteriousReuslt
{
    enBuyMysteriousReuslt_Success = 0,             // 成功
    enBuyMysteriousReuslt_NoOpen,                  // 活动未开启
    enBuyMysteriousReuslt_Timeout,                 // 购买超时
	enBuyMysteriousReuslt_TimeErr,                 // 时间戳不一至
    enBuyMysteriousReuslt_ErrIndexId,              // 索引不在列表中
    enBuyMysteriousReuslt_NoCost,                  // 资源不足
    enBuyMysteriousReuslt_BagFull,                 // 失败，背包满了
    enBuyMysteriousReuslt_Buy,                     // 已购买了
	enBuyMysteriousReuslt_Limit,				   // 达到购买上限
	enBuyMysteriousReuslt_NoAction,                // 不能购买, 不在活动时间内
	enBuyMysteriousReuslt_NoMatch,                 // 索引与物品不匹配
    enBuyMysteriousReuslt_UnknowError              // 异常
};
	
// 兑换神秘商人商品结果返回值
enum enBuyMysteriousExchange
{
	enBuyMysteriousExchange_Success = 0,             // 成功
	enBuyMysteriousExchange_BagFull,                 // 失败，背包满了
	enBuyMysteriousExchange_BagShortage,             // 物品不足
	enBuyMysteriousExchange_NoMatch,                 // 索引与物品不匹配
	enBuyMysteriousExchange_LvlNoMatch,				 // 等级不满足
	enBuyMysteriousExchange_Limit,					 // 达到购买上限
	enBuyMysteriousExchange_NoAction,				 // 不能购买, 不在活动时间内
	enBuyMysteriousExchange_UnknowError              // 异常	
};
    
// 使用称号返回
enum enUseHonroReuslt
{
	enUseHonroReuslt_Success = 0,             // 成功
	enUseHonroReuslt_NoGet,                   // 未获得称号
    enUseHonroReuslt_Expire,                  // 过期
	enUseHonroReuslt_UnknowError              // 异常
};
    

	
// 申请订单返回状态
enum enRegisterOrderReuslt
{
	enRegisterOrderReuslt_Success = 0,                  // 成功
	enRegisterOrderReuslt_Timeout,                      // 超时
	enRegisterOrderReuslt_Wait,							// 次数频繁
	enRegisterOrderReuslt_OrderErr,						// 
	enRegisterOrderReuslt_InsertTbErr,                  // 下单失败
	enRegisterOrderReuslt_UnknowError
};
	
//幻兽大比拼-赌场台面状态
enum enPetCasinoStatus
{
    enPetCasinoStatus_Wagering = 0,                     //押注中
    enPetCasinoStatus_Close,                            //未开始
    enPetCasinoStatus_NoMatchToday,                     //没比赛了
};


//幻兽大比拼-押注结果
enum enPetCasinoWagerResult
{
    enPetCasinoWagerResult_Success = 0,                 //下注成功
    enPetCasinoWagerResult_CasinoClose,                 //失败，未开始
    enPetCasinoWagerResult_NeedMoreGold,                //失败，金币或金钻不足
    
    //enPetCasinoWagerResult_NeedMoreLvl,                 //失败，等级不足以下大注

    enPetCasinoWagerResult_AlreadyWager,                //失败，已经下注了
    enPetCasinoWagerResult_PetError,                    //失败，没有该宠
    enPetCasinoWagerResult_UnknowError                  //失败，未知错误
};
//活动类型 : 限时活动
enum enActivityType
{
    enFriendTower  = 1,         //好友塔防
    enWordBoss =2,              //世界boss
    enStone = 3,                //宝石抽奖
    enDrawingCopy = 4,          //变异精英
    enComparePet =5,            //幻兽大比拼
    enUnderWithFriendCopy = 6,  //好友地下城
    enPetCopy = 7,              //幻兽大本营 (老)
    enFriendCopy = 8,           //好友副本
    enUnderCopy = 9,            //地下城
    enMySticMerchant = 10,      //神秘商人
    enDefendStatue = 11,        // 保卫神像
    enMysticalCopy = 12,        //幻兽大本营 变更 为 精英副本
    enPetArena = 13,            //幻兽竞技场
    enDailyAnswer=14,           //每日答题
    enFamousHall = 15,          // 名人堂
    enDiceGame = 18,            // 骰子游戏
    enPetEliteCopy = 19,        //幻兽试炼场 .嘉年华
    enTreasureFightCopy = 43,
    
    enTicketsActivity = 51,     //所有的门票活动的入口
};

//2.0 活动类型: 装备活动 和 幻兽活动
enum enOrdinaryActivityTpye
{
    enOrdinaryActEliteCopy     = 1,        //精英副本
    enOrdinaryActFriendCopy    = 2,        //好友副本
    enOrdinaryActUnderCopy     = 3,        //地下城
    enORdinaryActivityTicket   = 4,        //所有的门票活动
    enOrdinaryActPetCopy       = 7,        //幻兽大本营
    enOrdinaryActPetEliteCopy  = 8,        //幻兽试炼场
    enOrdinaryActPetArena      = 9,        //幻兽竞技场
    enOrdinaryActFamousHall    = 10,       //名人堂
    enOrdinaryActSyncTeamCopy = 11,        // 组队副本
    enOrdinaryActPetAdventrue = 35         //幻兽大冒险
};

    
//世界boss
enum eWorldBossErrcode
{
    eWorldBossOK = 0,
    eActNoOpen, //未开始
    eBossDied, // 已死亡
    eAlreadyIn, // 已经在里面
    eLvlNoReach, //等级不够
    eNotGuildMember,//不是本公会成员
    eBossUnknowError
};
    
// 图鉴模板类型
enum IllustrationsType
{
    eIllType_Non = 0,
    eIllType_General = 1,   // 普通
    eIllType_Elite,         // 精英
    eIllType_Boss,          // BOSS
    eIllType_species,       //稀有
};
    
// 图鉴宠物品质
enum IllustrationsQuality
{
    eIllQua_Green = 0,   // 绿
    eIllQua_Blue,        // 蓝
    eIllQua_Purple,      // 紫
    eIllQua_Count,
};
    
// 图鉴奖励状态
enum IllustrationsState
{
    eIllState_Non = 0,      // 未开启
    eIllState_NotReceive,   // 未领取
    eIllState_Receive,      // 已领取
};
  
// 获取图鉴奖励返回结果
enum enIllustrationsRewardReuslt
{
    enIllustrationsRewardReuslt_Success = 0,             // 成功
    enIllustrationsRewardReuslt_NoFound,                 // 图鉴不存在
    enIllustrationsRewardReuslt_NotReceive,              // 不是可领取状态
    enIllustrationsRewardReuslt_AddBagErr,              // 奖励失败
    enIllustrationsRewardReuslt_UnknowError              // 异常
};
    
// 活动道具类型
enum EnumItemPropsType
{
    KItemPropsPhoneCard = 1,    // 移动手机卡
    KItemPropsPhone = 2,        // 手机
	KItemPropsTelecomPhoneCard = 3,    // 电信手机卡
	KItemPropsUnicomPhoneCard = 4,    // 联通手机卡
	KItemPropsTickets = 5	// 飞机票
};

// Boss类型
enum enumBossType
{
    eBossType_WorldBoss=0,  //  世界boss
    eBossType_GuildBoss,    //  公会boss
};

// 小助手跳转类型
enum eHelperGotoType
{
    ehGt_PetPractise = 1,           // 幻兽试炼
    ehGt_WorldBoss = 2,             // 世界boss
    ehGt_Bingo = 116,                 // 每日抽奖
    ehGt_Aberance = 4,              // 变异精英
    ehGt_petMacth = 5,              // 幻兽大比拼
    ehGt_BuddyDt = 6,               // 好友地下城
    ehGt_BuddyCpy = 8,              // 好友副本
    ehGt_petBase = 7,               // 幻兽大本营
    ehGt_dt = 9,                    // 地下城
    ehGt_MythBman = 115,             // 神秘商人
    ehGt_ProtectIdol = 11,          // 保卫神像
    ehGt_DoubleWeek = 12,           // 双周本
    ehGt_DiceGame = 18,             // 骰子游戏
    ehGt_FamousHall = 15,           // 名人堂
    ehGt_Answer = 14,               // 答题
    ehGt_Ticket = 51,               // 门票活动
    
    ehGt_NatrualTask = 100,         // 普通任务
    ehGt_DailyTask = 101,           // 日常任务
    ehGt_NatrualCopy = 102,         // 普通副本
    ehGt_EliteCopy = 103,           // 精英副本
    ehGt_AdAward = 104,             // 冒险家宝藏
    ehGt_Arena = 105,               // 竞技场
    ehGt_ArenaAward = 106,          // 竞技场奖励
    ehGt_StrongEquip = 107,         // 强化装备
    ehGt_UpEquip = 108,             // 提品装备
    ehGt_SynEquip = 109,            // 镶嵌装备
    ehGt_LightStar = 110,              // 星座
    ehGt_SkillUp = 111,             // 技能升级
    ehGt_SynStuff = 112,            // 物品合成
    ehGt_SwallowPet = 113,          // 吞噬幻兽
    ehGt_PetArena = 114,            // 幻兽竞技场
    ehGt_FriendCopy = 117,          // 好友副本
    ehGt_PetMonster = 118,          // 幻兽试炼
};
    
enum SkillType
{
    eUnknowSkill,
    ePassive,
    eInitiative,
    eAnger,  //怒气技能
    eTriggerSkill, //触发技能
};

enum ePetSkillType              // 宠物技能类型
{
    ePetSkillType_Special = 0,  // 特殊技能
    ePetSkillType_Common = 1 ,  // 普通技能
};

enum eGuildBless            // 祝福
{
    eGuildBless_Success = 0, // 成功
    eGuildBless_TargetOffline, // 目标不在线
    eGuildBless_TargetBlessFull, // 目标被祝福次数已满
    eGuildBless_NeedMoreRmb ,   // 金钻不足
    eGuildBless_AlreadyBless // 已经祝福过
};

enum eGuildInspire          // 鼓舞
{
    eGuildInspire_Success = 0, // 成功
    eGuildInspire_YourTimeUsedUp, // 你已用完你的鼓舞了
    eGuildInspire_GuildMaxTime,  // 公会已经被最大鼓舞了
    eGuildInspire_NeedMoreRmb    // 金钻不足
};
    
enum GuildImpeachInfoError
{
    eGuildImpeachInfoError_Error,
    eGuildImpeachInfoError_MasterOfflineLessThanNDay,   //会长离线时间少于N天
    eGuildImpeachInfoError_NoApplicant, //没有申请者
    eGuildImpeachInfoError_HaveApplicant,   //有申请者
};
    
enum GuildImpeachError
{
    eGuildImpeachError_Error = -1,
    eGuildImpeachError_Ok,
    eGuildImpeachError_ImpeachAppliantIsYouNow, //当前自己就是申请者
    eGuildImpeachError_LessRmb,                 //金钻不够
    eGuildImpeachError_ContribLessThanApplicant,//贡献值低于申请者
};
	
enum eGuildAppointPosition
{
	eGuildAppointPosition_Success = 0,
	eGuildAppointPosition_IsSelf,			// 不能对自己
	eGuildAppointPosition_NoPosition, 		// 没有权限
	eGuildAppointPosition_NoPosRange,		// 超出指派职位
	eGuildAppointPosition_NoGuild,			// 目标角色不存在公会
	eGuildAppointPosition_NoGuildLike,		// 目标角色公会与会长不一至
	eGuildAppointPosition_PositionIsFull,	// 职位已满
	eGuildAppointPosition_UnknowError,		// 服务器异常
};
	
enum eGuildOccupyStoreResult
{
	eGuildOccupyStoreResult_Success = 0,
	eGuildOccupyStoreResult_NoIndex,			// 不存在该物品索引
	eGuildOccupyStoreResult_BagFull,			// 背包满了
	eGuildOccupyStoreResult_NoConstrib,			// 贡献不足
	eGuildOccupyStoreResult_NoOccupyPoints,		// 占据分不足
	eGuildOccupyStoreResult_ConsumeError,		// 消耗值异常
	eGuildOccupyStoreResult_BuyNumRangeErr,		// 超过购买数值范围
	eGuildOccupyStoreResult_OccupyBuyLuaErr,	// guildStore_occupy_buy lua脚本不存在
	eGuildOccupyStoreResult_NoGuild,			// 不存在公会
	eGuildOccupyStoreResult_UnknowError,		// 服务器异常
};
    
struct TitleInfo
{
    int nTitleID;   // 称号id
    bool bNewGet;   // 是否新得到
};
    
enum PetPvpType
{
    eUnknown = 0,
    eBronze = 1,
    eSilver,
    eGolden,
    eGlobal,
};
enum GuildActivity {
    enGuildBoss = 41,//公会boss
    enGuildChallenge, // 公会试炼
    enGuildTreasureFight,
//    enGuildEasyTower = 2,//公会入侵
//    enGuildNormalTower=3,
//    enGuildHardTower=4,

};
    //获取奖励状态
enum eGetRewardsResult
{
    eGetRewardsResult_Ok,
    eGetRewardsResult_NoDay, //天数未达到
    eGetRewardsResult_Fullbag,  //背包满了
    eGetRewardsResult_RewardAlreayGet, //奖励已经领取了
    eGetRewardsResult_UnknowError,
};
    
    //获取累计几天答题奖励类型
enum eGetRewardsResultType
{
    eGetRewardsResultType_success = 0,
    eGetRewardsResultType_fallshort //次数未达到
};
    
//累计奖励类型
enum eAnswerInfoGetResult
{
    eAnswerInfoGetResult_Success = 0,
    eAnswerInfoGetResult_NoOpen,    // 活动未开启
    eAnswerInfoGetResult_UnknowError   // 服务器异常
};
    
enum eAnswerQuestionChangeResult
{
    eAnswerQuestionChangeResult_Success = 0,
    eAnswerQuestionChangeResult_NoOpen,    // 活动未开启
    eAnswerQuestionChangeResult_NoCost,     // 资源不足
    eAnswerQuestionChangeResult_NoVip,     // 不是vip
    eAnswerQuestionChangeResult_NoNum,      // 次数用完
    eAnswerQuestionChangeResult_IsOver,     // 游戏结束
    eAnswerQuestionChangeResult_UnknowError   // 服务器异常
};
    
enum econtinuousRewardType
{
    econtinuousRewardType_Question = 1, //题目累加奖励
    econtinuousRewardType_Day, //连续几天答题奖励
    econtinuousRewardType_FullScore, //连续满分
};
    
enum eAnswerNotifyResult
{
    eAnswerNotifyResult_Success = 0,
    eAnswerNotifyResult_BagBull,        // 背包已满， 下次登录再发放
    eAnswerNotifyResult_UnknowError
};

enum eAnswerSelectResult
{
    eAnswerSelectResult_Success = 0,
    eAnswerSelectResult_ActivityOver,
};
    
enum PetAiType
{
    eInitiativeMod,
    eAssistMod,
};

enum eAccuRechargeAwardResult   // 累计充值\消费领奖结果
{
    eAccuRechargeAwardResult_Succ = 0,
    eAccuRechargeAwardResult_BagFull = 1, // 背包满
    eAccuRechargeAwardResult_AlreadyAwarded =2, // 领过了
    eAccuRechargeAwardResult_RmbError =3 , // 没有这个rmb
};
    
enum eOnceRechargeAwardResult   // 一次充值、领奖结果
{
    eOnceRechargeAwardResult_Succ = 0,
    eOnceRechargeAwardResult_BagFull = 1, // 背包满
    eOnceRechargeAwardResult_AlreadyAwarded =2, // 领过了
    eOnceRechargeAwardResult_TimeError =3 , // 没有这个Time
};
enum eMonthRechargeAwardResult   // 月卡充值\领奖结果
{
    eMonthRechargeAwardResult_Succ = 0,
    eMonthRechargeAwardResult_BagFull, // 背包满
    eMonthRechargeAwardResult_AlreadyAwarded, // 领过了
    eMonthRechargeAwardResult_RmbError, // 没有这个rmb  或者 是已经领取30天，要想再领取需要充值
    eMonthRechargeAwardResult_GetAwardError,	// 在错误的天里领取东西（第七天，想领取第八天的奖励）
	
	eMonthRechargeAwardResult_NoGet,		// 未获得月卡
	eMonthRechargeAwardResult_DataErr,		// 玩家该月卡数据错误
	
	eMonthRechargeAwardResult_IndexErr,		// index错误
	eMonthRechargeAwardResult_NoActivity,	// 没有这个活动
};
    
enum eInviteResult   // 
{
    eInviteResult_Succ = 0,
    eInviteResult_BagFull = 1, // 背包满
    eInviteResult_AlreadyAwarded =2, // 领过了
    eInviteResult_GetAwardError=3, // 错误后的返回码
	eInviteResult_IsSelf=4,			// 不能输自己的譤请码
};
    
enum eRenameType
{
    eRoleRename = 1,        //角色rename
    eGuildRename        //公会rename
};

const int ALL_SERVER_ID = -1; // 全服活动,黄翀崇必须用-1表示该活动为全服活动
const int DEFAULT_SERVER_ID = 0; // 默认服务器ID

enum MultipleLogin
{
    eTaiwanLogin = 0,
    eFacebookLogin,
    eGoogleLogin,
    eKuaiyongVxinyouLogin,
    eKuaiyong7659Login,
    eEnLogin,
    eEnFacebookLogin
};

// 棒子服等级奖励领取结果
enum eKrLvAwardResult
{
    eKrLvAwardResult_Success = 0,
    eKrLvAwardResult_NoOpen,    // 没有开放
    eKrLvAwardResult_NoLvl,     // 等级不到
    eKrLvAwardResult_AlreadyGet,// 已经领过
    eKrLvAwardResult_Error      // 其它错误
};

//使用某个宠来完成幻兽试炼
enum ePetEliteUsePet
{
    ePetEliteUsePet_Success = 0,
    ePetEliteUsePet_AlreadyUsed,    // 宠用过了
    ePetEliteUsePet_CopyTimeUseUp,  // 次数用完了
    ePetEliteUsePet_PetIdError      // 没有这个宠
};

//普通日常活动: 装备活动 宠物蛋活动
enum eOrdinaryActType
{
    eOrdinaryActType_equipType = 1,
    eOrdinaryActType_petType = 2
};


//女神祝福，领取精力状态返回类型
enum eQueenBlessStatusType
{
    eQueenBlessStatusType_UnknowError = -1,
    eQueenBlessStatusType_Ok,       //目前只会返回这个值
    eQueenBlessStatusType_NotOpen,
    
};

//获取精力返回状态类型
enum eQueenBlessGetType
{
    eQueenBlessGetType_Ok = 0,
    eQueenBlessGetType_isAlreadyGet, //已经领取过了
    eQueenBlessGetType_FatIsMax, //精力达到上线
    eQueenBlessGetType_NotDuringTime, //不在活动时间
};

enum QueenBlessingReceiveStatus
{
    eNotCanReceive = 0,
    eCanReceive,
    eAlreadyReceive,
};

enum eUseResetDungeTimesItemResult
    {
        eUseResetDungeTimesItemResult_UnknowError = -1,
        eUseResetDungeTimesItemResult_Ok,
        eUseResetDungeTimesItemResult_ConfigError,
        eUseResetDungeTimesItemResult_BagCapacityFull,
        eUseResetDungeTimesItemResult_ItemExpired,
        eUseResetDungeTimesItemResult_NotGoDunge, //没有打过地下城
    };

enum SyncPvpEndReason
{
    eSyncPvpEndTimeout,
    eSyncPvpEndPlayerLeave,
    eSyncPvpEndPlayerDead,
};
    
    
enum eCapsuletoyResult
{
    eCapsuletoyResult_UnknowError = -1,      // 未知错误
    eCapsuletoyResult_Ok,                    // OK
    eCapsuletoyResult_NotEnoughRmb,          // 金钻不够
    eCapsuletoyResult_FreeTimesIsUseUp,      // 免费次数用完了
    eCapsuletoyResult_BuyTimesIsUseUp,       // 购买次数用完了
    eCapsuletoyResult_BuyTimesIsLessThanNTimes, // 购买次数小于N次
    eCapsuletoyResult_NotTimeToGetEgg,       // 免费领取时间还没到
    eCapsuletoyResult_NotHaveThisActivity,   // 没有买x送y优惠活动
    eCapsuletoyResult_ActivityIsNotOpen,    // 活动没开
    eCapsuletoyResult_BagIsFull,            //背包太满
};
    
enum eTwisteggGetResult
{
	eTwisteggGetResult_UnknowError = -1,      // 未知错误
	eTwisteggGetResult_Ok,                    // OK
	eTwisteggGetResult_NotConfig,				// 扭蛋项不存在
	eTwisteggGetResult_NotActivityId,				// 扭蛋项活动配置id不一至
	eTwisteggGetResult_IsTimeout,					// 该项扭蛋已超时
	eTwisteggGetResult_FreeNumOver,					// 免费次数用完了
	eTwisteggGetResult_FreeTime,					// 免费次数刷新时间未达到
	eTwisteggGetResult_NotBagSpace,					// 背包空余格子不足
	eTwisteggGetResult_NeedNot,						// 需要消耗的资源不足
	eTwisteggGetResult_LvlNot,						// 等级不足
	eTwisteggGetResult_NotGetGoods,					// 扭不到物品
	eTwisteggGetResult_BagFull,						// 背包满了
	eTwisteggGetResult_NotConsume,					// 配置表，没配置消耗
	eTwisteggGetResult_UseConsumeErr,				// 扣除消耗的资源失败了
	
};

enum eTotemImmolation
{
	eTotemImmolation_Success = 0,
	eTotemImmolation_TotemErr,			// 错误的图腾id
	
	eTotemImmolation_SysLvlErr,			// 图腾等级数据异常
	
	eTotemImmolation_PetEmpty,			// 没有选宠物
	eTotemImmolation_PetErr,			// 宠物不存在
	eTotemImmolation_PetIsActive,		// 是出战宠
	eTotemImmolation_PetIsAssist,		// 是助阵宠
	eTotemImmolation_PetIsFull,			// 选择宠物过多
	eTotemImmolation_PetNoStage,		// 选择宠物的阶级不满足
	eTotemImmolation_PetNoLvl,			// 选择宠物的等级不满足
	eTotemImmolation_PetNoMonster,		// 选择宠物的模板id不满足
	eTotemImmolation_PetNoMonsterCfg,	// 选择宠物的模板id没有对应配置表
	eTotemImmolation_PetNoQua,			// 选择宠物的品质不满足
};

    
//幻兽大冒险
enum PetAdventureType
{
    eUnKnowType,
    ePetAdventure_permanent_activity = 1,
    ePetAdventure_temporary_activity,
    ePetAdventure_normal,
};
   
// 消耗精力领奖励
enum eConsumeFatAwardResult   
{
    eConsumeFatAwardResult_Succ = 0,
    eConsumeFatAwardResult_BagFull = 1, // 背包满
    eConsumeFatAwardResult_AlreadyAwarded =2, // 领过了
    eConsumeFatAwardResult_FatError =3 , // 没有这个精力
};
// 角色奖励领取状态
enum RoleAwardGetState
{
    eRoleAwardNoGet = 0,  // 不可领取
    eRoleAwardCanGet = 1, // 可领取
    eRoleAwardhasGet = 2, // 已领取
};

enum MysticalCopyType
{
    eMysticalCopy_Normal = 0,
    eMysticalCopy_Cycle = 1,
};

//　称号关键key
enum eHonorKey
{
    eHonorKey_Non = 0,
    eHonorKey_Gold,			// 金币
    eHonorKey_Kill,			// 杀怪
    eHonorKey_Lvl,			// 等级
    eHonorKey_EquipQua,		// 装备品质
    eHonorKey_EquipLvl,		// 装备等级
    eHonorKey_EquipStone,	// 装备宝石
    eHonorKey_Cons,			// 星座
    eHonorKey_Power,		// 战斗力
    eHonorKey_Copy,			// 副本
    eHonorKey_Ill,			// 图鉴
    eHonorKey_Constrib,		// 公会贡献
    eHonorKey_Exploit,		// 公会功勋
    eHonorKey_Prestige,		// 声望
    eHonorKey_BossRankOne,  // 排名第一
    eHonorKey_BossLast,		// 最后一击
    eHonorKey_PvpKill,		// 竞技场杀人
    eHonorKey_PvpRankOne,	// 竞技场排名
    eHonorKey_Gambler,		// 赌圣
    eHonorKey_PetPvpRankOne,	// 幻兽竞技场排名
    eHonorKey_PetPvpKill,	// 幻兽竞技场击败
    eHonorKey_UseItem,		// 使用物品
    eHonorKey_GuildMasterTreasurecopy,	// 公会夺宝战排名(会长)
    eHonorKey_GuildTreasurecopy,		// 公会夺宝战排名
    eHonorKey_SynPvpRank				// 同步竞技场排名
};

//　称号应用key
enum eHonorApp
{
    eHonorApp_Non = 0,
    eHonorApp_WorldTalk,	// 世界聊天
    eHonorApp_OnlineTips	// 上线提示
};

enum eHonorType
{
    eHonorType_Non = 0,
    eHonorType_Lvl,			// 等级
    eHonorType_Power,       // 战斗力
    eHonorType_Battle,      // 战斗
    eHonorType_Pvp,          // 竞技
    eHonorType_Popular,     // 声望
    eHonorType_Union,       // 公会
    eHonorType_Synthe       // 综合
};
	
enum eMidastouchResult
{
	eMidastouchResult_UnknowError = -1,			// 未知错误
	eMidastouchResult_Ok,                    
	eMidastouchResult_NumOver,					// 炼金次数用完
	eMidastouchResult_NotUse,					// 上次炼金未充值使用，不能继续炼
	eMidastouchResult_CurNoConfig,				// 当前次数没有配置对应的炼金项
	
	eMidastouchResult_NotMidastouch,			// 当前没有炼金
	eMidastouchResult_Use,						// 炼金已使用
	eMidastouchResult_RmbNoMeet,				// 当前金钻不满足
	
};

enum eConstellupgResult
{
	eConstellupgResult_UnknowError = -1,			// 未知错误
	eConstellupgResult_Ok,
	eConstellupgResult_NoConfig,					// 配置表错误
	eConstellupgResult_IsFull,						// 当前星阶已点满 
	eConstellupgResult_NoCoin,						// 消耗不足
};

//跨服战
enum CrossServiceWar
{
    eCrossServiceWar_UnknowError = 0,   //未知错误
    eCrossServiceWar_WorShip = 1,       //膜拜
    eCrossServiceWar_CrossServiceWar = 2,  //跨服战
    eCrossServiceWar_RankingService = 3,   //服务器排行榜
    eCrossServiceWar_RaningSimple = 4,     //个人排行榜
};


//时装收集

#define FASHIONCOLLECT_MAX_GRID_FOR_MATERIAL (6)
#define FASHIONCOLLECT_LIMIT_SIZE_OF_MATERIALBOX (200)

enum FashionCollectType
{
    eFashionCollect_None = 0,
    eBaseFashionCollect  = 1,
    eHotWeaponFashionCollect  = 2,
    eHotBodyFashionCollect = 3,
};

extern PetAdventureType getPetAdventureType(std::string type);


extern ObjSex RoleType2Sex(int roletype);
extern ObjJob RoleType2Job(int roletype);

//Retinue
#define RETINUEBAG_MAX_SIZE 200

#endif
