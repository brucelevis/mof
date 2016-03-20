//
//  GuildShare.h
//  GameSrv
//  公会
//  Created by Huang Kunchao on 13-8-10.
//
//

#ifndef __GameSrv__GuildShare__
#define __GameSrv__GuildShare__
#include <string>
using namespace std;


enum GuildMemberPosition            //公会成员职位
{
    kGuildPositionNotset = 0,       //没有公会
    kGuildMaster,                   //会长
    kGuildViceMaster,               //副会长
    kGuildElite,                    //精英
    kGuildMember                    //会员
};


enum GuildManagerRight              //管理权力
{
    kGuidlViceMasterRightBegin = 0,
    kGuildKickMember ,              //踢人
    kGuildInviteMember,             //邀请
    kGuildAcceptApplicant,          //处理申请
    kGuidModifyNotice,              //修改公告
    kGuidUpgradeBuilding,           //升级建筑
    kGuildViceMasterRightEnd,

    kGuildAppointPos,               //任命副会长
    kGuildDismissPos,               //撤销副会长
    kGuildTransferMaster,           //转让会长
    kGuildDismissGuild,             //解散公会
	kGuildAppointPosition,			//指派职位
};

//权力判断
bool isGuildPosHasManagerRight(int pos, int right);

enum eGuildMgrAcceptApply                   //管理员接受公会申请
{
    eGuildMgrAcceptApply_Success= 0,        //成功
    eGuildMgrAcceptApply_NotInList =1,      //失败,不在列表上
    eGuildMgrAcceptApply_RoleDelete=2,      //失败,删号了
    eGuildMgrAcceptApply_RoleHasGuild=3,    //失败,已有公会
    eGuildMgrAcceptApply_MemNumLimit = 4,   //失败,人满了
};

enum eApplyGuild                            // 申请公会
{
    eApplyGuild_Success = 0,                // 成功申请
    eApplyGuild_GuildApplicantFull = 1,     // 此公会申请者已满
    eApplyGuild_NeedToWait = 2,             // 离开公会后一定时间（配置）内无法再申请或创建公会
    eApplyGuild_AlreadyApplied = 3,         // 已经申请过了
    eApplyGuild_RoleApplyFull = 4,         // 申请达到５个公会
    eApplyGuild_UnknownErr 
};

enum eGuildMgrRejectApply                   //管理员拒绝公会申请
{
    eGuildMgrRejectApply_Success= 0,        //成功
    eGuildMgrRejectApply_NotInList =1,      //失败,不在列表上
};


enum CreateGuildResult                  //创建公会结果
{
    kCreateGuildResult_Success = 0,     //成功
    kCreateGuildResult_NameExist = 1,   //失败，重名
    kCreateGuildResult_LevelLow = 2,    //失败，没到20级
    kCreateGuildResult_NeedMoreGold = 3,//失败，金币不够
    kCreateGuildResult_NeedToWait = 4,  //失败，离开公会后一定时间（配置）内无法再申请或创建公会
    kCreateGuildResult_Fail             //失败
};

enum eDismissGuildResult
{
	eDismissGuildResult_Success = 0,
	eDismissGuildResult_NoPermission,			// 权限不足
	eDismissGuildResult_TransferMaster,			//  正在转让会长
	eDismissGuildResult_DismissGuild,			// 正在解散公会
};


enum eTransferMasterResult
{
	eTransferMasterResult_Success = 0,
	eTransferMasterResult_NoPermission,			// 权限不足
	eTransferMasterResult_NoMember,				// 没有这个成员
	eTransferMasterResult_TransferMaster,		// 正在转让会长
	eTransferMasterResult_DismissGuild			// 正在解散公会
};

enum eGuildKickMember
{
    eGuildKickMember_Success = 0,           //成功
    eGuildKickMember_NotInGuild =1,         //失败，不在公会
    eGuildKickMember_DenyKickYourself=2,    //失败，不能踢自己
    eGuildKickMember_DenyKickMaster=3,      //失败，不能踢会长
};


enum GuildDonateType                        //公会捐献类型
{
    GuildDonateType_Begin = 0,
    GuildDonateType_Rmb ,                   //公会金钻
    
    GuildDonateType_Gold,                   //个人金币
    GuildDonateType_BattlePoint,            //个人战功
    GuildDonateType_SelfRmb,                //个人金钻
    
    GuildDonateType_End
};

enum eGuildDonateResult                     //捐献结果
{
    eGuildDonateResult_Success = 0,         //成功
    eGuildDonateResult_RmbNumLimit,         //公会已达日最大捐金钻次数
    eGuildDonateResult_NeedMoreRmb,         //金钻不足
    eGuildDonateResult_DnoateGoldBatNumLimit,//已达日最大捐献金币、战功次数
    eGuildDonateResult_NeedMoreGold,        //金币不足
    eGuildDonateResult_NeedMoreBattlePoint, //战功不足
    eGuildDonateResult_UnknowError,         //未知错误
    eGuildDonateResult_NeedHigherVipLvl,    // Vip等级不足
};

// 商店刷新列表结果返回值
enum StoreListReuslt
{
	StoreListReuslt_Success = 0,                 // 成功
	StoreListReuslt_NoOpen,                      // 未开启
	StoreListReuslt_UnknowError                  // 异常
};

// 商店刷新列表结果返回值
enum StoreManuallyListReuslt
{
    StoreManuallyListReuslt_Success = 0,             // 成功
	StoreManuallyListReuslt_NoOpen,                  // 未开启
    StoreManuallyListReuslt_NoRmb,                   // 金钻不足
    StoreManuallyListReuslt_RefreshTimesIsUseUp,     // 刷新次数已经用完
    StoreManuallyListReuslt_UnknowError              // 异常
};

// 商店购买结果返回值
enum StoreBuyReuslt
{
	StoreBuyReusltReuslt_Success = 0,             // 成功
    StoreBuyReusltReuslt_NoOpen,                  // 未开启
    StoreBuyReusltReuslt_Timeout,                 // 购买超时
    StoreBuyReusltReuslt_ErrIndexId,              // 索引不在列表中
    StoreBuyReusltReuslt_NoCost,                  // 资源不足
    StoreBuyReusltReuslt_BagFull,                 // 失败，背包满了
    StoreBuyReusltReuslt_Buy,                     // 已购买了
    StoreBuyReusltReuslt_UnknowError              // 异常
};

// 培养公会boss 类型
enum eTrainGuildBossType
{
    eTrainGuildBossType_ItemTrain,                  // 道具培养
    eTrainGuildBossType_RmbTrain                    // 金钻培养
};

// 培养公会boss 返回值
enum eTrainGuildBoss
{
    eTrainGuildBoss_Success = 0,                    // 成功
    eTrainGuildBoss_ActivityOpen ,                  // 失败，活动正在进行中
    eTrainGuildBoss_ItemTrainTimeLimit,             // 道具培养次数己满
    eTrainGuildBoss_RmbTrainTimeLimit,              // 金钻培养次数己满
    eTrainGuildBoss_ItemIdError,                    // 道具出错，不存在
    eTrainGuildBoss_NeedMoreRmb,                    // 金钻不足
    eTrainGuildBoss_BossExpFull,                    // BOSS的经验己满
    eTrainGuildBoss_BossMaxLvlErr,                  // BOSS等级配置错误
};

enum eGuildLogLvl       //日志级别
{
    eGuildLogLvl_A,     //最高
    eGuildLogLvl_B,
    eGuildLogLvl_C,
    eGuildLogLvl_D,     //默认
};

//计算公会等级
int calGuildLevel(float fortune, float construction);

//计算公会最大成员
int calGuildMaxMember(int level);

//计算公会boss等级
int calGuildBossLevel(int exp);



#endif /* defined(__GameSrv__GuildShare__) */
