//
//  DataCfgActivity.h
//  GameSrv
//  配置表分支-活动配置
//  Created by Huang Kunchao on 13-7-26.
//
//

#ifndef GameSrv_DataCfgActivity_h
#define GameSrv_DataCfgActivity_h

#include <map>
#include <string>
#include <set>
#include <vector>
#include "Defines.h"
#include "GuildShare.h"
#include "inifile.h"
#include "msg.h"

using namespace std;

class IniFile;

#if defined(CLIENT)
    typedef IniFile GameInifile;
#else
    #include "GameIniFile.h"
#endif

#pragma mark -
#pragma mark 轮盘抽奖
//抽奖配置
enum enLotteryItemType              //转盘物品分类
{
    enLotteryItemType_Constell=0,   //星灵+
    enLotteryItemType_Fat ,          //体力
    enLotteryItemType_PetEgg ,       //幻兽蛋
    enLotteryItemType_Exp,          //经验+
    enLotteryItemType_Gold,         //金币+
    enLotteryItemType_BattlePoint,  //战功+
    enLotteryItemType_Stone,        //宝石袋
    enLotteryItemType_XX            //其它
};

enum enumLotteryItemFree
{
    eLotteryItemNotFree = 0,    //不免费
    eLotteryItemFree        //免费
};

struct LotteryItem                  //物品
{
    LotteryItem()
    {
        id = 0;
        type =enLotteryItemType_XX;
        prob = 0.0f;
        freeType = 0;
        broadcast = false;
    }

    int     id;                     //物品id
    string  name;                   //名字
    int     type;                   //enLotteryItemType
    float   prob;                   //概率
    string  lvlstr;                 //等级
    int     freeType;               //0不免费 1免费
    bool    broadcast;              //抽中后广播
};

enum enWeek {                       //星期
    enWeek_Sunday = 0,
    enWeek_Monday = 1,
    enWeek_Tuesday = 2,
    enWeek_Wednesday = 3,
    enWeek_Thursday = 4,
    enWeek_Friday = 5,
    enWeek_Saturday = 6
};

typedef vector<LotteryItem> LotItemVec;

class LotteryCfg
{
    friend class LotteryCfgMgr;
public:
    LotteryCfg(){}
    ~LotteryCfg(){}
    LotItemVec* get(int userlvl);
    string dump();
    void insert(const LotteryItem& item);
protected:
    typedef std::map<string,LotItemVec> LvlMap;
    LvlMap _lvlmap;
};

class LotteryCfgMgr
{
public:
    LotteryCfgMgr(){}
    ~LotteryCfgMgr();
    static void load(string fullpath);
    static LotteryCfg* read();
    static void replace(LotteryCfg* cfg);
    static const LotItemVec* getItem(int userlvl);
protected:
    static LotteryCfg* readTodayCfg(const char* todayfullpath);
protected:
    static LotteryCfg*   _cfg;
    static std::string   _fullpath;
};

#pragma mark -
#pragma mark 色子抽奖配置

const int DICE_MAX_GRID = 18;   // 色子抽奖最大格子数


struct DiceGrid // 色子格子
{
	bool isStart;
    int gridid; // id
    string icon;   // 图标
    int prop;   // 格子权重
	int lvlmin; // 等级区间
	int lvlmax; // 等级区间
};

struct DiceItem // 色子物品
{
    int index;  // 唯一标记
    int item;   // 物品ID
    int amount; // 物品数量
    std::string name; // 物品名
    int gridid; // 对应格子
    int prop;   // 物品在该类格子合部物品中的权重
    int type;   // enLotteryItemType
    bool broadcast; // 抽中后广播
	int lvlmin; // 等级区间
	int lvlmax; // 等级区间
};

typedef std::vector<DiceGrid> DiceGrids;    // 全部格子，增加一个start格子
typedef std::vector<DiceItem> DiceGridItems;    // 一格子全部物品
typedef std::map<int,DiceGridItems> DiceItems;  // 全部物品

class DiceLotteryCfgMgr
{
public:
    static void load(const string& gridpath, const string& itempath);
    static int needRmg(int time ,int& costType);   // 需要金钻
    static const DiceGrid& getGrid(int position, int lvl); //位置求格子
	
    static const DiceGridItems& getGridItems(int grid); //求格子全部物品
    static DiceGrids mGrids;
    static int getFreeTimes(){ return mFreeTime; }      
    static void getGrid(int lvl, vector<DiceGrid*> &vec);
protected:
    static void loadGrid(const char* gridpath);
    static void loadItem(const char* itempath);
    static int mFreeTime;   // 免费次数
    static DiceItems mItems;
	static DiceGrid mDgDump; // 空的格子,　用于起点
};


#pragma mark -
#pragma mark 幻兽大比拼活动配置


struct ActivityPetCosinoWagerLevel       //某一等级押注
{
    vector<int>     gold;                //可押金币
    vector<int>     rmb;                 //可押金钻
    vector<int>     thief;               //小盗之手价格
};


class tagActivityPetCosinoCfg
{
public:
    typedef std::map<string,ActivityPetCosinoWagerLevel> WagerMap;
    
    tagActivityPetCosinoCfg():opentoday(true){}
    ~tagActivityPetCosinoCfg(){}
    
    string dump();
    string getInfo();
    
    ActivityPetCosinoWagerLevel* getWager(int lvl);
    
    /*
     一维 某场比赛时间，开始，第一轮 ,... , 第N轮, 结束
     二维 今天各场比赛时间， 第一场,...，第N场
     */
    typedef vector<vector<int> > TimeVec;
    
    bool                opentoday;          //今天开放吗
    TimeVec             times;              //比赛时间
    string              mInfo;              //玩法说明
    int                 mAwardOwnerGold;    //奖励主人金币
    WagerMap            wager;              //押注
    vector<string> PetDataBak ;        //当参赛宠数量不够用时，从这里抽数据参赛
};

class tagActivityInviteCfg
{
public:
    
    tagActivityInviteCfg():opentoday(true){}
    ~tagActivityInviteCfg(){}
    
    string dump();
    string getInfo();
    
    typedef map<int ,int> MapCountToLvl;
    typedef map<int ,int> MapCountToShow;
    typedef map<int,string> MapCountToAward;
    
    bool                opentoday;          //今天开放吗
    MapCountToLvl       CountToLvl;         //次数与等级
    MapCountToAward     CountToAward;       //阶段次数与阶段奖励
    MapCountToShow      CountToawardType;   // 1表示发给自己，不用显示的礼包，2表示要显示
    string              mInfo;              //玩法说明
};

class ActivityInviteCfgMgr
{
public:
    ActivityInviteCfgMgr(){}
    ~ActivityInviteCfgMgr();
    
    static void load(std::string fullpath);
    
    static tagActivityInviteCfg* readcfg();
    
    static void replacecfg(tagActivityInviteCfg* cfg);
    
    static const tagActivityInviteCfg* getcfg();
    
protected:
    static tagActivityInviteCfg*  _cfg;
    
    static std::string  _fullpath;
};

class ActivityPetCosinoCfgMgr
{
public:
    ActivityPetCosinoCfgMgr(){}
    ~ActivityPetCosinoCfgMgr();
    
    static void load(std::string fullpath);
    
    static tagActivityPetCosinoCfg* readcfg();
    
    static void replacecfg(tagActivityPetCosinoCfg* cfg);
    
    static const tagActivityPetCosinoCfg* getcfg();
    
protected:
    static tagActivityPetCosinoCfg*  _cfg;
    
    static std::string  _fullpath;
};


#pragma mark - 称号系统配置

struct HonorDef
{
	// 称号ID
	int id;
	// 称号类型
	eHonorType type;
	// 称号名称
	string name;
	// 有加成属性
	bool isAtt;
	// 完成的条件数量
	int needNum;
	// 条件关键字
	eHonorKey key;
	string keyName;
	// 应用关键字
	std::set<eHonorApp> appSet;
	// 条件
	string cond;
	std::set<string> condSet;
	// 图片
	string icon;
	//　描述
	string desc;
    // 完成度是否显示
    int show;
    // 顺序id
    int order;
	
	// 有效时间
	int effectiveTime;
	// 光效资源Id
    vector<int> resourcesIds;
#ifdef CLIENT
    // 精力
    ReadWrite_Crypt(int, mAddMaxFat, AddMaxFat)
    // 攻击力
    ReadWrite_Crypt(int, mAtk, Atk);
    // 防御力
    ReadWrite_Crypt(int, mDef, Def);
    // 血亮
    ReadWrite_Crypt(int, mAddMaxHp, AddMaxHp);
    // 命中
    ReadWrite_Crypt(float, mHit, Hit);
    // 闪避
    ReadWrite_Crypt(float, mDodge, Dodge);
    // 暴击
    ReadWrite_Crypt(float, mCri, Cri);
    // 力量
    ReadWrite_Crypt(int, mStre, Stre);
    // 智力
    ReadWrite_Crypt(int, mInte, Inte);
    // 体力
    ReadWrite_Crypt(int, mPhys, Phys);
    // 耐力
    ReadWrite_Crypt(int, mCapa, Capa);
#else
    // 精力
    READWRITE(int, mAddMaxFat, AddMaxFat);
    // 攻击力
    READWRITE(int, mAtk, Atk);
    // 防御力
    READWRITE(int, mDef, Def);
    // 血亮
    READWRITE(int, mAddMaxHp, AddMaxHp);
    // 命中
    READWRITE(float, mHit, Hit);
    // 闪避
    READWRITE(float, mDodge, Dodge);
    // 暴击
    READWRITE(float, mCri, Cri);
    // 力量
    READWRITE(int, mStre, Stre);
    // 智力
    READWRITE(int, mInte, Inte);
    // 体力
    READWRITE(int, mPhys, Phys);
    // 耐力
    READWRITE(int, mCapa, Capa);
#endif
    HonorDef()
    {
		id = 0;
        setAddMaxFat(0);
        setAtk(0);
        setDef(0);
        setAddMaxHp(0);
        setHit(0);
        setDodge(0);
        setCri(0);
        setStre(0);
        setInte(0);
        setPhys(0);
        setCapa(0);
    }
};

class HonorCfg
{
private:
	typedef std::map<int, HonorDef*> HonorTd;
public:
	typedef HonorTd::const_iterator Iterator;
	HonorDef & getCfg(int id);
	Iterator begin() const {return mHonorMap.begin();}
	Iterator end() const {return mHonorMap.end();}

	// 获取达到条件的称号
	void getCfg(string type, vector<HonorDef*> &vec);
	void getCfg(eHonorType type, vector<HonorDef*> &vec);
	void getCfg(eHonorKey key, string &cond, vector<HonorDef*> &vec);
	
	void getCfg(eHonorKey key, int num, vector<HonorDef*> &vec);
	
	void getCfg(eHonorApp key, vector<HonorDef*> &vec);
	
	// 获取称号类型
	void getTypeCfg(vector<string> &vec);
    
    // 获取称号类型对应id
    int getTypeId(string sName){
        if (mTypeMap.find(sName) != mTypeMap.end()) {
            return mTypeMap[sName];
        }
        return 0;
    }
    
    // 获取称号名称
    string getTypeName(eHonorType id);
    
	void read(GameInifile &inifile);
private:
	HonorTd mHonorMap;
	HonorDef mDumb;
	
	// 存储名称对应的称号类型
	std::map<string, eHonorType> mTypeMap;
};

class HonorCfgMgr
{
public:
	typedef HonorCfg::Iterator Iterator;
	static Iterator begin() {return sTitleCfg.begin();}
	static Iterator end() {return sTitleCfg.end();}
	
	static void load();
	
	static HonorDef & getHonor(int id) {
		return sTitleCfg.getCfg(id);
	}
	
	// 获取所有称号类型
	static void getAllType(vector<string> &vec){
		return sTitleCfg.getTypeCfg(vec);
	}
    
    // 根据类型名获取类型id
    static int getTypeIdByName(string sName){
        return sTitleCfg.getTypeId(sName);
    }
    
    // 获取类姓名城
    static string getTypeName(eHonorType id){
        return sTitleCfg.getTypeName(id);
    }
    
	// 根据类型获取
	static void getHonorByType(string type, vector<HonorDef*> &vec) {
		return sTitleCfg.getCfg(type, vec);
	}
	static void getHonorByType(eHonorType type, vector<HonorDef*> &vec) {
		return sTitleCfg.getCfg(type, vec);
	}
	
	static void getHonor(eHonorKey key, string &cond, vector<HonorDef*> &vec) {
		return sTitleCfg.getCfg(key, cond, vec);
	}
	
	static void getHonorApp(eHonorApp key, vector<HonorDef*> &vec) {
		return sTitleCfg.getCfg(key, vec);
	}
	
private:
	static HonorCfg sTitleCfg;
};



#pragma mark - 公会系统配置


#pragma mark - 基础 society.ini
struct GuildCreateDef
{
	// 需要等级
	int needLvl;
	// 需要Glod
	int needGlod;
};

struct GuildLvlDef
{
	// 公会等级
	int lvl;
	// 公会人数
	int memberNum;
	// 需要财富
	int needWealth;
	// 需要建设度
	int needOverbuild;
	// 捐献金钻次数
	int donateRmbNum;
	// 捐献金币战功次数
	int donateGoldBatNum;
	// 副会长人数
	int deputyChairmanNum;
	// 精英人数
	int eliteNum;
	// 纹章等级
	int heraldry;
    //公会boss等级限制
    int maxbosslvl;
    //对应开放的试炼等级
    int trainLvl;
    // 捐献时，所需Vip等级
    int donateRmbVipLvl;
};


struct rmbTrainGuildBossDef
{
    vector<int> trainNeedRmb;
    vector<int> addBossExp;
};

struct GuildBossLvlDef
{
    // 等级
    int level;
    // 怪物ID
    int monsterId;
    // 经验值
    int exp;
    // 场景ID
    int sceneId;
    // 最低等级进入限制
    int minGuildLvl;

    // 奖励公会财富
    int fortuneAward;
    // 奖励公会建设度
    int constructionAward;
    // 个人贡献奖励
    float constributeAwardFactor;
    // 个人功勋奖励
    float exploitAwardFactor;
    
    // 每日道具培养公会BOSS次数
    int itemTrainBossTimePerDay;
    
    // 每日金钻培养次数上限
    int rmbTrainBossTimePerDay;
    // 需要金钻
    int rmbTrainNeedRmb(int rmbTrainTime);
    // 培养增加经验
    int rmbTrainAddBossExp(int rmbTrainTime);
    
    // 金钻培养
    rmbTrainGuildBossDef rmbTrainDef;
    
};


struct GuildBossKillDef             //公会boss活动奖励配置
{
    int getRankBonus(int rank);     // 返回排名对应奖励
    vector<int> rankBonus;          // 排名奖励
    vector<int> rankRange;          // 排名
    
    int lastHit;                    // 最后一击
    int clientRankNum;              // 伤害排名人数
    int rebornSeconds;              // 复活时间
    int preparetime;
    
    void clear()
    {
        rankBonus.clear();
        rankRange.clear();
        lastHit = 0;
        clientRankNum = 0;
        rebornSeconds = 0;
    }
};



struct GuildPresentDef
{
	// 可捐献的金币
	int donateGold;
	// 可捐献的战功
	int donateBat;
    // 可捐献的个人金钻
    int donateRmbSelf;
	// 增加的公会财富
	int addwealth;
	// 增加的贡献
	int addcontribute;
	// 增加的功勋
	int addfeats;
	// 增加的建设度
	int addoverbuild;
    // 金币增加的贡献
    int goldaddcontribute;
    // 金币增加的功勋
    int goldaddfeats;
};

struct GuildPresentRmbDef
{
	// 可捐献的金钻
	int donateRmb;
	// 金钻捐献增加公会财富
	int rmbaddwealth;
	// 捐献增加的贡献
	int rmbaddcontribute;
	// 金钻捐献增加的功勋
	int rmbaddfeats;
	// 金钻增加的建设度
	int rmbaddoverbuild;
};

class GuildPresentCfg
{
public:
	GuildPresentDef * getCfg(int num);
    GuildPresentDef * getPresentCfg(int num);
	GuildPresentRmbDef & getRmbCfg() {return mGuildPresentRmb;}
	int getSize() {return mGuildPresentMap.size();}
	void read(GameInifile &inifile);
private:
	typedef std::map<int, GuildPresentDef*> GuildPresentTd;
	GuildPresentTd mGuildPresentMap;
	GuildPresentRmbDef mGuildPresentRmb;
};

class GuildBasicCfg
{
public:
    GuildLvlDef * getCfg(float wealth, int overbuild);
	GuildLvlDef * getCfg(int guildLvl);
	int getSize() {return mGuildLvlMap.size();}
	GuildCreateDef * getCreateCfg() {return &mCreateDef;}
	void read(GameInifile &inifile);
    
    int getImpeachTime() const;
    int getImpeachCost() const;
    int getMaxApplicant() const;
    int getWaitApplySecond() const;
    int getDonateRmbVipLvl() const;
    
private:
	GuildCreateDef mCreateDef;
	typedef std::map<int, GuildLvlDef*> GuildLvlTd;
	GuildLvlTd mGuildLvlMap;
    int mImpeachTime;   // 弹核时间
    int mImpeachCost;   // 弹核价格
    int mMaxApplicant;  // 公会申请者上限
    int mWaitApplySecond;   // 离开公会，再次申请或创建需要等待的时间
    int mDonateRmbVipLvl;// vip 限制
};

#pragma mark - 技能 guildskill.ini

struct GuildSkillDef
{
	// 技能id
	int id;
	// 技能等级
	int lvl;
	// 最高等级
	int maxLvl;
	// 需求公会等级
	int needGuildLvl;
	// 需求贡献
	int needContribute;
	// 需求战功
	int needBattlePoint;
};

class GuildSkillCfg
{
public:
	void read(GameInifile &inifile);
	GuildSkillDef * getCfg(int skillId, int lvl);
	vector<int> const & getSkillIdVec();
    bool getAllSkillByGuildLvl(int guildLVl,std::map<int, int> & outData);
private:
	typedef std::map<int, GuildSkillDef*> GuildSkillTd;
	GuildSkillTd mGuildSkillMap;
	// 公会技能id
	std::vector<int> mSkillIdVec;
};

#pragma mark - 商店 guildgoods.ini guildmerchant.ini

// 商店物品基类
struct RandGoodsDef
{
	// 物品ID
    int goodsId;
    // 限制等级
    int lvl;
    // 权重
    int pro;
	// 消耗值区间
    int consumeMin;
    int consumeMax;
	// 金钻区间
    int brmbMin;
    int brmdMax;
	// 消耗值出现概率, 与金钻与斥
	float consumePro;
    //添加物品描述 id
    std::string desc_id;
};

class RandGoodsCfgBase
{
public:	
    int getFreeNum() {return mFreeNum;}
    int getInterval() {return mInterval;}
    int getUpdateNeedRmb() {return mUpdateRmb;}
    
    int getVipRefreshTimes(const int &viplvl);
    int getGuildVipRefreshTimes(const int &viplvl); //公会商店VIP刷新次数限制
	
	// 根据权重随机物品
    void randGoods(int level, vector<RandGoodsDef*> &vec);
 	virtual bool checkIsIndex(int index, int goodsId){return true;}
    RandGoodsDef * getGoods(int index);
    bool isUseGuildViplvlRefreshLimit();
protected:
	void readMerchant(GameInifile &inifile);
	void readGoods(GameInifile &inifile);

protected:
	// 免费刷新次数
	int mFreeNum;
    // 刷新时间
    int mInterval;
    // 手动消耗金钻
    int mUpdateRmb;
	
	typedef std::vector<RandGoodsDef*> RandGoodsTd;
	RandGoodsTd mGoodsVec;
    static std::map<int, RandGoodsDef*> cfgGoods;
	std::map<int, int> mLvlRangeMap;
    //vip level
    std::map<int, int> mViplvlRangeMap;
    //vip影响公会商店刷新次数
    map<int, int> mGuildViplvlRangeMap;
    
    //
    bool mIsUseViplvlRefreshLimit;
    bool mIsUseGuildViplvlRefreshLimit;
private:
	typedef RandGoodsDef* RandGoodsDefPointer;
	static bool GoodsVecCmp(const RandGoodsDefPointer& left, const RandGoodsDefPointer& right);
};

class GuildGoodsCfg : public RandGoodsCfgBase
{
public:
    std::vector<RandGoodsDef*> mRemainGoodsVec;
	virtual bool checkIsIndex(int index, int goodsId){return true;}
	void read();
    void RemainGoods(vector<int> &vec);
    int  getRemainGoodsSize();
    vector<RandGoodsDef*> getRemainGoods(){return mRemainGoodsVec;}
    
};
#pragma mark - 占城特权 guildchampagoods.ini

struct ChampaGoodsDef
{
	// 物品index
	int index;
	// 物品ID
    int goodsId;
	// 物品数量
	int goodsNum;
	// 消耗值
    int consume;
	// 需求占领分
	int occupied;
	// 图片
	string pic;
	
	string goodsName;
	string goodsDesc;
};

class GuildChampaGoodsCfg
{
public:
	static void load();
	static void readChampaGoods(GameInifile &inifile);
	static void readOccupiedConfig(GameInifile &inifile);
	
	
	static int getOccupied(int rank);
	static ChampaGoodsDef* getChampaGoods(int index);
	
	// 占领分比例
	static std::map<int, int> mOccupied;
	
	// 物品容器
	static std::map<int, ChampaGoodsDef> mChampaGoods;
};

#pragma mark - 纹章 guildgad.ini

struct GuildGadDef
{
    //加密公会纹章属性
#ifdef CLIENT
    ReadWrite_Crypt(int, mId, id);
    READWRITE(string, mName, Name);
    ReadWrite_Crypt(int, mlvl, Lvl);
	ReadWrite_Crypt(int, mFeatsRankBegin, FeatsRankBegin);
	ReadWrite_Crypt(int, mFeatsRankEnd, FeatsRankEnd);
    READWRITE(GuildMemberPosition, mPost, Post);
    READWRITE(string, mIcon, Icon);
    ReadWrite_Crypt(int, mAddMaxFat, AddMaxFat);
    ReadWrite_Crypt(int, mAtk, Atk);
    ReadWrite_Crypt(int, mDef, Def);
    ReadWrite_Crypt(int, mAddMaxHp, AddMaxHp);
    ReadWrite_Crypt(float, mHit, Hit);
    ReadWrite_Crypt(float, mDodge, Dodge);
    READWRITE(string, mDesc, Desc);
#else
    READWRITE(int, mId, id);
    READWRITE(string, mName, Name);
    READWRITE(int, mlvl, Lvl);
	READWRITE(int, mFeatsRankBegin, FeatsRankBegin);
	READWRITE(int, mFeatsRankEnd, FeatsRankEnd);
    READWRITE(GuildMemberPosition, mPost, Post);
    READWRITE(string, mIcon, Icon);
    READWRITE(int, mAddMaxFat, AddMaxFat);
    READWRITE(int, mAtk, Atk);
    READWRITE(int, mDef, Def);
    READWRITE(int, mAddMaxHp, AddMaxHp);
    READWRITE(float, mHit, Hit);
    READWRITE(float, mDodge, Dodge);
    READWRITE(string, mDesc, Desc);
#endif
    GuildGadDef()
    {
        setid(0);
        setLvl(0);
        setFeatsRankBegin(0);
        setFeatsRankEnd(0);
        setAddMaxFat(0);
        setAtk(0);
        setDef(0);
        setAddMaxHp(0);
        setHit(0);
        setDodge(0);
    }
};

class GuildGadCfg
{
public:
	void read(GameInifile &inifile);
	GuildGadDef * getCfg(int guildLvl, GuildMemberPosition post);
	GuildGadDef * getCfg(int gadId);
	GuildGadDef * getCfg(int guildLvl, int exploitRank);
private:
	typedef std::map<int, GuildGadDef*> GuildGadTd;
	GuildGadTd mGuildGadMap;
};

#pragma mark - 配置管理

class GuildCfgMgr
{
public:
	static void load();
	
	// 公会技能
	static GuildSkillDef * getSkillDef(int skillId, int lvl);
	static vector<int> const & getSkillIdVec();
    
    static bool getAllSkillByGuildLvl(int guildLVl,std::map<int, int> & outData)
    {
        sSkillCfg.getAllSkillByGuildLvl(guildLVl,outData);
        return true;
    }
	
	// 公会等级系统数据(财富, 建设度)
	static GuildLvlDef * getGuildLvlDef(float wealth, int overbuild);
	static GuildLvlDef * getGuildLvlDef(int guildLvl);
	static int getGuildMaxLvl() {return sBasicCfg.getSize();}
	
	// 个人捐献数据(捐献次数, 0次开始)
	static GuildPresentDef * getGuildPresentDef(int num);
    static GuildPresentDef * getGuildSelfPresentDef(int num);
	// 个人金钻捐献数据
	static GuildPresentRmbDef & getGuildPresentRmbDef();
	
	static GuildCreateDef * getCreateDef() {return sBasicCfg.getCreateCfg();}
	
	// 公会商店数据
	static GuildGoodsCfg * getGoodsCfg() {return &sGoodsCfg;}
	static void RemainGoods(vector<int> &vec);
    static vector<RandGoodsDef*> getRemainGoods(){return sGoodsCfg.getRemainGoods();}
    static int getRemainGoodsSize();
	// 纹章系统数据(公会等级,　职位)
	static GuildGadDef * getGadDef(int guildLvl, GuildMemberPosition post);
	static GuildGadDef * getGadDef(int gadid);
	static GuildGadDef * getGadDef(int guildLvl, int exploitRank);
    
    // 公会boss的属性
    static GuildBossLvlDef* getGuildBossLvlDef(int bossExp);
    static GuildBossLvlDef* getGuildBossUpLvlMaxExp(int bossExp);
    static GuildBossLvlDef* getGuildBossLvlExp(int lvl);//根据lvl得到每一级最多经验
    static GuildBossKillDef* getGuildBossKillCfg();
    static int getBossPrepareTime();
    
    //获取会长弹劾时间
    static int getImpeachTime();
    static int getImpeachCost();
    
    //获取申请者上限
    static int getMaxApplicant();
    
    // 再次申请或创建需要等待的时间
    static int getWaitApplySecond();
    
    static int getDonateRmbVipLvl();
protected:
    static void loadGuildBossCfg();
    static void loadGuildBossKillCfg(GameInifile& inifile);
    
    
private:
	static GuildBasicCfg sBasicCfg;
	static GuildPresentCfg sPresentCfg;
	static GuildSkillCfg sSkillCfg;
	static GuildGoodsCfg sGoodsCfg;
	static GuildGadCfg sGadCfg;
    
    static vector<GuildBossLvlDef> sGuildBossCfg;
    static GuildBossKillDef sGuildBossKillCfg;
};

#pragma mark - 物品转换

struct ItemChangeDef
{
	// 原物品ID
	int proto_id;
	// 目标物品ID
	int goal_id;
	// 转换系数
	float change;
	// 转换金钻系数
	int compensation_rmb;
};

class ItemChangeCfg
{
public:
	typedef std::map<int, ItemChangeDef> ItemChangeTd;
	typedef ItemChangeTd::const_iterator Iterator;
	
	void read(GameInifile &inifile);
	ItemChangeDef & getCfg(int id);
	Iterator begin() const {return mItemChangeMap.begin();}
	Iterator end() const {return mItemChangeMap.end();}
	
private:
	
	ItemChangeTd mItemChangeMap;
	ItemChangeDef mItemChangeDumb;
};

class ItemChangeCfgMgr
{
public:
	typedef ItemChangeCfg::Iterator Iterator;
	static Iterator begin() {return sCfg.begin();}
	static Iterator end() {return sCfg.end();}
	
	static void load();
	
	static ItemChangeDef & getItemChange(int id) {
		return sCfg.getCfg(id);
	}
private:
	static ItemChangeCfg sCfg;
};

#pragma mark - 充值活动数据
struct ChargeActItemDef
{
#ifdef CLIENT
    ReadWrite_Crypt(int, mRmb, Rmb);
#else
    READWRITE(int, mRmb, Rmb);
#endif
    string items;
    ChargeActItemDef()
    {
        setRmb(0);
        items = "";
    }
};

//消耗精力获取奖励
struct FatActItemDef
{
#ifdef CLIENT
    ReadWrite_Crypt(int, mFat, Fat);
#else
    READWRITE(int, mFat, Fat);
#endif
    string items;
    FatActItemDef()
    {
        setFat(0);
        items = "";
    }
};

class ChargeActivityDef
{
    
#ifdef CLIENT
    ReadWrite_Crypt(int, mActivityId, ActivityId);
    ReadWrite_Crypt(int, mStartTime, StartTime);
    ReadWrite_Crypt(int, mOverTime, OverTime);
    ReadWrite_Crypt(int, mAwardType, AwardType);
#else
    READWRITE(int, mActivityId, ActivityId);
    READWRITE(int, mStartTime, StartTime);
    READWRITE(int, mOverTime, OverTime);
    READWRITE(int, mAwardType, AwardType);
#endif
    string platform;
    string activityType;
    string activityDesc;
    vector<int> ServerIds;
    vector<ChargeActItemDef*> awards;
    READWRITE(int, mAwardNum, AwardNum);
public:
    ChargeActivityDef();
    virtual ~ChargeActivityDef();
    vector<ChargeActItemDef*> getAwards();
    string getAward(int rmb);
    bool includeCurSr( int srId );
    
    static int server_time; // 同步服务器时间
};
#pragma mark - 累计充值数据
class AddChargeActDef : public ChargeActivityDef
{
public:
    AddChargeActDef(){}
    virtual ~AddChargeActDef(){}
};
#pragma mark - 累计消费数据
class AddConsumeActDef : public ChargeActivityDef
{
public:
    AddConsumeActDef(){}
    virtual ~AddConsumeActDef(){}
};
#pragma mark - 累加单笔充值数据
class AddOnceChargeActDef : public ChargeActivityDef
{
public:
    AddOnceChargeActDef(){}
    virtual ~AddOnceChargeActDef(){}
};
#pragma mark - 单笔充值数据
class OnceChargeActDef : public ChargeActivityDef
{
public:
    OnceChargeActDef(){}
    virtual ~OnceChargeActDef(){}
};
#pragma mark - 首冲奖励（只保存了文本）
class FirstChargeDef : public ChargeActivityDef
{
public:
    FirstChargeDef(){}
    virtual ~FirstChargeDef(){}
};
#pragma mark - 消耗精力换奖励数据
class ConsumeFatAwardActDef : public ChargeActivityDef
{
public:
    ConsumeFatAwardActDef(){}
    virtual ~ConsumeFatAwardActDef(){}
};

#pragma mark - 充值活动配置
class ChargeActivityCfg
{
protected:
    vector<ChargeActivityDef*> datas;
public:
    typedef vector<ChargeActivityDef*>::iterator Iterator;
    Iterator begin(){ return datas.begin(); }
    Iterator end(){ return datas.end(); }
    vector<ChargeActivityDef*> getCfg(int serverId);
    vector<ChargeActivityDef*> getCfgPlatform(int serverId, string platform);
    virtual void load(string fullpath){}
    virtual ~ChargeActivityCfg(){}
};
#pragma mark - 累计充值活动配置
class AddChargeActCfg : public ChargeActivityCfg
{
public:
    void load(string fullpath);
    virtual ~AddChargeActCfg(){}
    //lihengjin
    void addData(vector<obj_accumulate_recharge>  accumulate);
};
#pragma mark - 累计消费活动配置
class AddConsumeActCfg : public ChargeActivityCfg
{
public:
    void load(string fullpath);
    virtual ~AddConsumeActCfg(){}
    //lihengjin
    void addData(vector<obj_accumulate_consume> accumulateConsume);
};
#pragma mark - 可叠加单笔充值配置
class AddOnceChargeActCfg : public ChargeActivityCfg
{
public:
    void load(string fullpath);
    virtual ~AddOnceChargeActCfg(){}
    //lihengjin
    void addData(vector<obj_operate_recharge> operateRecharge);
};
#pragma mark - 不可叠加单笔充值配置
class OnceChargeActCfg : public ChargeActivityCfg
{
public:
    void load(string fullpath);
    virtual ~OnceChargeActCfg(){}
    //lihengjin
    void addData(vector<obj_operate_once_recharge> operateOnceRecharge);
};
#pragma mark - 首冲奖励(只读取文本)
class FirstChargeCfg : public ChargeActivityCfg
{
public:
    vector<ChargeActivityDef*> getcfgSpecial(){ return datas; }
    void load(string fullpath);
    virtual ~FirstChargeCfg(){}
    //lihengjin
    void addData(vector<obj_gift_activity> firstCharge);
    
};
#pragma mark - 消耗精力换奖励数据
class ConsumeFatAwardActCfg : public ChargeActivityCfg
{
public:
    void load(string fullpath);
    virtual ~ConsumeFatAwardActCfg(){}
    void addData(vector<obj_fat_award_activity> FatAward);
};

#pragma mark - 充值活动配置管理
class ChargeActivityCfgMgr
{
public:
    static bool load();
    static AddChargeActCfg mAddChargeActCfg;
    static AddConsumeActCfg mAddConsumeActCfg;
    static AddOnceChargeActCfg mAddOnceChargeCfg;
    static OnceChargeActCfg mOnceChargeCfg;
    static FirstChargeCfg mFirstChargeCfg;
    static ConsumeFatAwardActCfg mConsumeFatAwardActCfg;
};
#pragma mark - 同步竞技场荣誉对应军衔
class MilitaryCfg
{
public:
    static void load(string fullpath);
    typedef std::map<unsigned int, std::string>::iterator Iterator;
    static string GetLastMilitary(const unsigned int& glory);
    static void setPreMilitaryIter(Iterator iter){ mPreIter = iter; }
    static void setPreMilitaryIter(int value);
    static Iterator& getPrePreMilitaryIter(){ return mPreIter; }
    static int getNextMilitaryValue();
    
    static std::map<unsigned int, std::string> & GetMilitaryCgfs();
private:
    static std::map<unsigned int, std::string> militaryCfgs;
    static Iterator mPreIter;
};

#pragma mark - 同步竞技场兑换商城
struct SynPvpStoreGoodsDef
{
	// 序号ID
	int index;
	// 物品ID
    int goodsId;
    // 消耗勋章数
	int needMedal;
    // 描述id
    int descId;
    //描述id(无多语言版本)
    std::string str_desc;
    
    //需要的荣誉值
    int needHonor;
    SynPvpStoreGoodsDef()
    :index(0)
    ,goodsId(0)
    ,needMedal(0)
    ,descId(0)
    {
        
    }
};
class SynPvpStoreCfg
{
public:
    static void load(string fullpath);
    static void getSynPvpGoods(vector<SynPvpStoreGoodsDef*>& vec);
    static SynPvpStoreGoodsDef* getGoodsDefByIdx(int index);
    static vector<SynPvpStoreGoodsDef*>& getCfg(){return mSynPvpGoods;}
private:
    static vector<SynPvpStoreGoodsDef*> mSynPvpGoods;
};
class SynPvpStoreCfgML
{
public:
    static void load(string fullpath);
    static string getDescByIdx( int index );
private:
    static map<int, string> mSynPvpGoodsDescs;
};

struct SynPvpFunctionCfgDef
{
    int roleHpMultiple;
    int petHpMultiple;
    int maxFightTime;
    int maxMatchingSec;
    int fightingCountDown;
    
//    int winMedal;
//    int loseMedal;
    
//    int winHonor;
//    int losHonor;
//    
//    int matchFailMedal;
//    int matchFailHonor;
    int canGetAwardTimes;                       //每天可以获得奖励的次数
    int minFightingTimes;                       //最少战斗场次，大于这个场次才判断胜率
    
    int duelLvl;                                //决斗的最小等级
    int matchingLvl;                            //匹配的最小等级
    
    vector<int> matchSecGap;
    vector<int> matchBattleGapPercent;
    vector<int> matchWinrateGapPercent;
    vector<int> matchHonorGap;
    
};

struct SynPvpAward
{
    SynPvpAward():medal(0),
                  honor(0)
    {
        
    }
    int medal;
    int honor;
};

class SynPvpFuntionCfg
{
public:
    static void load(string fullpath);
    static int getMatchingSec()
    {
        return mCfgDef.maxMatchingSec;
    }
    
    static int getfightingCountDown()
    {
        return mCfgDef.fightingCountDown;
    }
    
    static void getBattleAndWinrateGapPercent(int matchSec, int &battleGapPercent, int &winrateGapPercent, int &honorGapPercent);
    
    static int getAwardTimes(int useTimes)
    {
        int validTimes = mCfgDef.canGetAwardTimes - useTimes;
        
        validTimes = validTimes < 0 ? 0 : validTimes;
        
        return validTimes;
    }
    
    static bool checkDuelLvl(int rolelvl)
    {
        return rolelvl >= mCfgDef.duelLvl;
    }
    
    static bool checkMatchingLvl(int rolelvl)
    {
        return rolelvl >= mCfgDef.matchingLvl;
    }
    
    static SynPvpAward getSynPvpWinAward(int matchingTimes);
    
    static SynPvpAward getSynPvpLoserAward(int matchingTimes);
    
    static SynPvpAward getSynPvpMatchingFailAward(int matchingTimes);
    
public:
    static SynPvpFunctionCfgDef mCfgDef;
    static vector<SynPvpAward> mWinnerAwards;
    static vector<SynPvpAward> mLoserAwards;
    static vector<SynPvpAward> mMatchingFailAwards;
};

/********
 
 公会宝藏战的副本配置
 
 *******/

enum TreasureCopyType
{
    eTreasureNormal = 1,                    //普通的没特殊效果的
    eTreasureRefreshMonster,                //定时刷怪
    eTreasureKillPlayer,                    //杀人双倍积分
    eTreasureKillPlayerAndFreshMonster      //杀人多倍及刷怪
};

struct TreasureCopyListCfgDef
{
    int sceneNum;
    int sceneMod;               //场景模板
    int treasureType;           //宝藏副本类型
    int freshMonsterInterval;   //刷怪的时间间隔
    int maxPlayerCount;         //最大玩家数
    int maxGuildMemCount;       //每个公会的最大成员数
    
    int monsterMod;
    int monsterScore;
    int monsterNum;
    float playerRate;    //杀人积分倍数
    float monsterAssistRate;
    float playerAssistRate;
    
    string sceneName;           //场景名称
};

class TreasureListCfg
{
public:
    static void load(string fullPath);
    static TreasureCopyListCfgDef* getTreasureCopyCfg(int copyNum);
private:
    static map<int, TreasureCopyListCfgDef*> mCfg;
};

struct GuildTreasureFightCfgDef
{
    int minCopyCount;           //最少的副本数
    int copyCountDeno;          //计算副本数公式的分母,副本数 = 参与玩家数/4
    int activityDuration;       //活动持续时间
    int leaveCopyRecoverSec;    //死亡的苏醒时间s
    int deathRecoverSec;         //主动离开的冷却时间
    int manorCount;             //占领的公会数目
    
    float roleHpRate;
    int protectTime;
};

class GuildTreasureFightCfg
{
public:
    static void load(string fullPath);
    static int getRecoverSec(bool isDeath);
    static GuildTreasureFightCfgDef mCfg;
};


#pragma mark -
#pragma mark 图腾


class SysTotemLvl
{
public:
	
	int totemId;
	// 等级
	int lvl;
	// 需要升级的数量
	int needUpNum;
	
	// 忽略宠物献祭对应的monster
	bool ignoreMonster;
	
	// 献祭的monsterId
	int monsterId;
	// 献祭的品质
	int immolationQua;
	// 献祭的阶级
	int immolationStage;
	// 献祭的等级
	int immolationLvl;
	
#ifdef CLIENT
    // 攻击力
    ReadWrite_Crypt(int, mAtk, Atk);
    // 防御力
    ReadWrite_Crypt(int, mDef, Def);
    // 血亮
    ReadWrite_Crypt(int, mAddMaxHp, AddMaxHp);
    // 命中
    ReadWrite_Crypt(float, mHit, Hit);
    // 闪避
    ReadWrite_Crypt(float, mDodge, Dodge);
    // 暴击
    ReadWrite_Crypt(float, mCri, Cri);
#else
    // 攻击力
    READWRITE(int, mAtk, Atk);
    // 防御力
    READWRITE(int, mDef, Def);
    // 血亮
    READWRITE(int, mAddMaxHp, AddMaxHp);
    // 命中
    READWRITE(float, mHit, Hit);
    // 闪避
    READWRITE(float, mDodge, Dodge);
    // 暴击
    READWRITE(float, mCri, Cri);
#endif
	SysTotemLvl();
	
};

class SysTotemInfo
{
public:
	SysTotemInfo()
	{
		totemId = 0;
	}
	// 图腾id
	int totemId;

	string name;
	string icon;
	string desc;

	SysTotemLvl * getLvl(int lvl);
	
	int getLvlCount() {return mLvl.size();}
	void insertInfoLvl(SysTotemLvl &stl);
    
    map<int, SysTotemLvl>::iterator lvBegin(){ return mLvl.begin(); }
    map<int, SysTotemLvl>::iterator lvEnd(){ return mLvl.end(); }
private:
	// (图腾等级)存放每个等级数据
	map<int, SysTotemLvl> mLvl;
};


class SysTotemGroupTotem
{
public:
	int totemId;
	int totemLvl;
};

// 图腾阵列
class SysTotemGroup
{
public:
	
	int groupId;
	string name;
    
    // 额外属性需要的等级
    int getPropAddLevel();
	// 图腾阵列关系
	vector<SysTotemGroupTotem> totemVec;
	
	// 满足阵列
	bool isMeetGroup(map<int, int>& totem);
#ifdef CLIENT
    // 攻击力
    ReadWrite_Crypt(int, mAtk, Atk);
    // 防御力
    ReadWrite_Crypt(int, mDef, Def);
    // 血亮
    ReadWrite_Crypt(int, mAddMaxHp, AddMaxHp);
    // 命中
    ReadWrite_Crypt(float, mHit, Hit);
    // 闪避
    ReadWrite_Crypt(float, mDodge, Dodge);
    // 暴击
    ReadWrite_Crypt(float, mCri, Cri);
#else
    // 攻击力
    READWRITE(int, mAtk, Atk);
    // 防御力
    READWRITE(int, mDef, Def);
    // 血亮
    READWRITE(int, mAddMaxHp, AddMaxHp);
    // 命中
    READWRITE(float, mHit, Hit);
    // 闪避
    READWRITE(float, mDodge, Dodge);
    // 暴击
    READWRITE(float, mCri, Cri);
#endif
    SysTotemGroup();
};

typedef map<int, SysTotemInfo>::iterator InfoIter;
typedef map<int, SysTotemGroup>::iterator GroupIter;

class SysTotemCfg
{
public:
	SysTotemCfg(){}
	
	void readInfo(GameInifile &infoFile, GameInifile &lvlFile);
	void readGroup(GameInifile &inifile);
	void configCheck();
    
	// 获取对应等级的图腾
	SysTotemLvl * getInfoLvl(int totemId, int lvl);
	SysTotemInfo * getInfo(int totemId);
	
    // 获取图腾阵列数量
    int getGroupNum(){ return mGroup.size(); }
	
    InfoIter infoBegin(){ return mInfo.begin(); }
    InfoIter infoEnd(){ return mInfo.end(); }
    GroupIter groupBegin(){ return mGroup.begin(); }
    GroupIter groupEnd(){ return mGroup.end(); }
    GroupIter findGroupById(int groupId);
	
	// 计算加成
	void calculate(BattleProp& batprop, map<int, int>& info);
private:
	// 图腾id
	map<int, SysTotemInfo> mInfo;
	// 阵列id
	map<int, SysTotemGroup> mGroup;
};

class SysTotemCfgMgr
{
public:
	static void load();
	static SysTotemCfg sCfg;
};

class TotemMultiLanguage
{
public:
    static void load();
    static string getGroupString(int idx);
    static string getInfoString(int idx);
private:
    static map<int, string> totemGroupString;
    static map<int, string> totemInfoString;
};


#pragma mark -
#pragma mark 副本额外奖励

class CopyExtraAward
{
public:
	int index;
	int copyId;
	int beginTime;
	int endTime;
    int afterOpenServerDays; //单位：天
	int multiple;
	string extraItems;
};

class CopyExtraAwardCfg
{
public:
	typedef map<int, vector<CopyExtraAward*> > CopyExtraAwardTd;
	typedef CopyExtraAwardTd::iterator Iterator;
	
public:
	static void load();
	
	static CopyExtraAward * getExtraAward(int copyId);


private:
	static void read(GameInifile &inifile);
	static CopyExtraAwardTd mCfg;
};


#pragma mark -
#pragma mark 新扭蛋

class TwistEggGoodsDef
{
public:
	int index;
	string belongs;			// 所属库
	int goodsId;
	int goodsNum;
	int prop;				// 权值
	string specieView;		// 特效显示
	string desc;			// 说明
	
	bool generalSw;
	bool specieSw;
	bool firstSw;
	
	bool broadcast;			// 发送通知
};

class TwistEggBaseDef
{
public:
	string desc;
	
	int needItemId;			// 普通扭需要的物品id
	int needItemNum;
	
	int needEvenItemId;		// 连扭需要的物品id
	int needEvenItemNum;	// 连扭需要的物品数量
	
	int rmb;
	int gold;
	
	int beginTm;
	int endTm;
	
	string image;			// xxxxx.png
};

class TwistEggActivityDef : public TwistEggBaseDef
{
public:
	int id;
	int belongs;			// 所属
	
	string goodsStore;		// 物品库
	string viewGoods;		// 客户端显示串
};

class TwistEggCommonDef : public TwistEggBaseDef
{
public:
	TwistEggCommonDef() {activityId = 0;}
	
	int index;
	int activityId;			// 活动时间配置id, 默认为0
	bool isTime;			// 是否有活动时间配置

	string name;			// 项名称
	
	bool isTimesLimit;		// 是否有次數限制
	int freeNum;			// 免费扭次数
	int freeTime;			// 免费间隔时间
	
	bool isEvenTwist;		// 是否可以连扭
	int evenTwistNum;		// 连扭次数
	string evenTwistDesc;	// 连扭描述 
	float evenTwistRebate;	// 连扭折扣
	int evenTwistSpecieNum;	// 连扭特殊物品库数量
	
	int rewardItem;			// 扭蛋必奖物品
	int rewardItemNum;
	
	
	
	
	int viplvl;
	int lvl;
	
	vector<TwistEggGoodsDef*> generalGoods;		// 普通物品库
	vector<TwistEggGoodsDef*> specieGoods;		// 特殊物品库
	vector<TwistEggGoodsDef*> firstGoods;		// 第一次物品库
	
	vector<int> viewGoods;		// 客户端拿来做界面显示用, 存放的是物品id
	
	
	// 检查配置是否过期
	bool checkTimeout(int time) const
	{
		if (!isTime) {
			return false;
		}
		if (time < beginTm || time > endTm) {
			return true;
		}
		return false;
	}
	
	// 获取该项的活动剩余时间
	int getFunctionTm(int time) const
	{
		if (!isTime) {
			return 0;
		}
		int temp = endTm - time;
		return temp > 0 ? temp : 0;
	}
	
	// 获取免费剩余次数（已用次数）
	int getFreeNum(int num) const
	{
		if (freeNum <= 0 || num >= freeNum) {
			return 0;
		}
		return freeNum - num;
	}
	
	// 获取免费领取倒计数(据上一次领取时长)
	int getFreeTime(int duration) const
	{
		int temp = freeTime - duration;
		return temp > 0 ? temp : 0;
	}
	
	// 获取物品消耗
	pair<unsigned int, unsigned int> getItemConsume(bool isEven) const
	{
		if (isEven) {
			return make_pair(needEvenItemId, needEvenItemNum);
		} else {
			return make_pair(needItemId, needItemNum);
		}
	}

	// 获取金钻消耗
	unsigned int getRmbConsume(bool isEven) const
	{
		if (isEven) {
			return ceil(rmb * evenTwistNum * evenTwistRebate);
		} else {
			return rmb;
		}
	}
	
	// 获取金币消耗
	unsigned int getGoldConsume(bool isEven) const
	{
		if (isEven) {
			return ceil(gold * evenTwistNum * evenTwistRebate);
		} else {
			return gold;
		}
	}
	
	unsigned int getRewardItemNum(bool isEven) const
	{
		return isEven ? rewardItemNum * evenTwistNum : 1;
	}
	
	bool randGoods(vector<const TwistEggGoodsDef*> &goods, bool isFirst, bool isEven) const;
private:
	bool randGoodsGet(vector<const TwistEggGoodsDef*>& goods,
					  const vector<TwistEggGoodsDef*>& store,
					  unsigned int num) const;
	
	bool randGoodsEven(vector<const TwistEggGoodsDef*> &goods) const;
};

class TwistEggCommonCfg
{
public:
	typedef map<int, TwistEggCommonDef>::const_iterator Iterator;
	
	void read(GameInifile &inifile);
	void readActivity(GameInifile &inifile);
	void readGoods(GameInifile &inifile);
	
	
	void getTwistEgg(Iterator& begin, Iterator& end, int time);
	const TwistEggCommonDef* getTwistEgg(int index, int time);
	
	// 获取未开启的活动配置
	TwistEggActivityDef* getNotOpenActivity(int time);
	void getNotOpenActivity(vector<const TwistEggActivityDef*> vec, int time);
	
	const TwistEggGoodsDef* getGoods(int index);
private:
//	typedef map<int, TwistEggCommonDef>::const_iterator Iterator;
//	Iterator begin() {return mCommons.begin();}
//	Iterator end() {return mCommons.end();}
	
	void updateDef(TwistEggCommonDef& base, int time);
	void setDef(TwistEggCommonDef& base, TwistEggActivityDef* def);
	void setGoodsStore(TwistEggCommonDef& base, const char* goodsStrore);
	
	TwistEggActivityDef * getDef(int belongs, int time);
	
	map<int, TwistEggCommonDef> mCommons;
	map<int, TwistEggActivityDef> mActivitys;
	
	map<string, vector<TwistEggGoodsDef*> > mGoods;
	map<int, TwistEggGoodsDef*> mGoodIndexs;
};

class TwistEggCfgMgr
{
public:
	static void load();

	// 获取扭蛋项配置, 注意传进服务器时间用于活动配置更新
	static void getTwistEgg(TwistEggCommonCfg::Iterator& begin,
							TwistEggCommonCfg::Iterator& end,
							int time);
	
	static const TwistEggCommonDef* getTwistEgg(int index, int time);
	
	
	static const TwistEggGoodsDef* getGoods(int index) {
		return sCommonCfg.getGoods(index);
	}
	
	static TwistEggActivityDef* getNotOpenActivity(int time) {
		return sCommonCfg.getNotOpenActivity(time);
	}
	
	static void getNotOpenActivity(vector<const TwistEggActivityDef*> vec, int time) {
		return sCommonCfg.getNotOpenActivity(vec, time);
	}
private:
	static TwistEggCommonCfg sCommonCfg;
};

#pragma mark - 竞技场兑换商城
struct PvpStoreGoodsDef
{
    // 序号ID
    int index;
    // 物品ID
    int goodsId;
    // 消耗勋章数
    int needPoints;
    // 描述id
    int descId;
    //描述id(无多语言版本)
    std::string str_desc;
    
//    //需要的荣誉值
//    int needHonor;
    PvpStoreGoodsDef()
    :index(0)
    ,goodsId(0)
    ,needPoints(0)
    ,descId(0)
    {
        
    }
};

#pragma mark -
#pragma mark 点石成金

class MidastouchDef
{
public:
	int index;			// 项
	float chargeRatio;	// 充值系数
	int needNum;		// 需要次数
	int needCoin;		// 需要满足的金钻
	int prop;			// 权值
};

class MidastouchCfg
{
public:
	typedef map<int, MidastouchDef>::iterator Iterator;
	
	void read(GameInifile &inifile);
	
	MidastouchDef * randItem(int num);
	MidastouchDef * getItem(int index);
	int getMaxNum() {return mMaxNum;}
    vector<float> getchargeRatioListByTimes(int times);
private:
	map<int, MidastouchDef> mItem;
	int mMaxNum;
};

class MidastouchCfgMgr
{
public:
	static void load();
	static MidastouchCfg sMidastouchCfg;
    
};


class PvpStoreCfg
{
public:
    static void load(string fullpath);
    static void getPvpGoods(vector<PvpStoreGoodsDef*>& vec);
    static PvpStoreGoodsDef* getGoodsDefByIdx(int index);
    static vector<PvpStoreGoodsDef*>& getCfg(){return mPvpGoods;}
private:
    static vector<PvpStoreGoodsDef*> mPvpGoods;
};

class PvpStoreCfgML
{
public:
    static void load(string fullpath);
    static string getDescByIdx( int index );
private:
    static map<int, string> mPvpGoodsDescs;
};




#endif /* defined(GameSrv_DataCfgActivity_h) */