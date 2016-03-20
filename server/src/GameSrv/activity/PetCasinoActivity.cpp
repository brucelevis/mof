//
//  PetCasinoActivity.cpp
//  GameSrv
//
//  Created by Huang Kunchao on 13-12-16.
//
//

#include "PetCasinoActivity.h"
#include "BroadcastNotify.h"
#include "Paihang.h"
#include "GameLog.h"
#include "mail_imp.h"
#include "DataCfgActivity.h"
#include "DynamicConfig.h"
#include "EnumDef.h"
#include "RandName.h"
#include "daily_schedule_system.h"
#include "Honor.h"

/************************  幻兽大比拼活动  ********************************
 
 幻兽大比拼动态配置设计
 
 <最新配置>: 读线程定时读文件的发回来的配置，该配置不断刷新
 <比赛配置>: 比赛开始后，从<最新配置>拷贝出来的只读副本
 
 1、比赛前，不断读配置，替换配置以达到动态目的，称为 <最新配置>
 从配置表中读出并记今天全部比赛开始时间，然后把最近一场加进定时器列表等待开场
 5分钟后，刷新配置，如果发现今天比赛开始时间有变，取消刚刚的定时，替换新的定时等待开场
 
 2、比赛开始后，不能替换配置，要保护该场<比赛配置>
 比赛开始后，读配置线程仍然工作，读-替换配置生成<最新配置>操作一直进行。
 所以，开场后，要立刻把当前配置副本拷走，生成<比赛配置>,用于该控制该场比赛。
 然后把该场比赛中的各轮时间加进定时器控制比赛，比赛正常运转。
 一场比赛开始后，就要完整比完这场比赛，任何配置改变都不能影响此场比赛
 
 3、如果停服，立马停下该场比赛。
 清理比赛中数据，包括取消定时，返完用户押金等
 
 */

static const string CASION_ACTION_DB_KEY_PETID  = "petcasionactivity";
static const string CASION_ACTION_DB_KEY_HISTORY  = "historyasionactivity";
static const string CASION_ACTION_DB_KEY_TIME  = "timestartandend";

const int SYNC_PAIHANG_PET_AMOUNT = 64;        //取回幻兽前N个

class MatchPet;
class PetCasionWagerCompensateMgr;
typedef vector<MatchPet> Round;
typedef vector<Round> MatchHistory;

void g_setNextTime(int valStartTime,int valEndTime);
std::pair<int, int> CalMatchStartAndEndTime(int t);
PetRecorveState CalRecorveState(size_t t,size_t MatchStartTime ,size_t MatchEndTime);
bool GetObjFromDB(std::pair< vector<MatchPet>, MatchHistory> & outVectMatchPetToMatchHistory);
void matchMgrFuZhi(bool open,vector<MatchPet> & vectPets,MatchHistory & vectHistorys ,tagActivityPetCosinoCfg & valMatchcfg ,int valStartTime);

void logicFuntion(int roundNum,int HiselementNum,std::pair< vector<MatchPet>, MatchHistory> & pairVectMatchPetToMatchHistory ,tagActivityPetCosinoCfg  & PetCosinoCfg ,std::pair<size_t, size_t> MatchTime ,PetCasionWagerCompensateMgr & valPetCaMgr);

void finishLoadPaihangPetDataByRecorveOnly(int rid,int type,vector<SortPetInfo> data);

void startMatchCb(void* param);       //比赛开始
void endRoundCb(void* param);         //一轮结束
void endMatchCb(void* param);         //比赛结束

struct GamblerInfo
{
    GamblerInfo(){
        gold = 0;
        thiefrmb = 0;
        thiefed = false;
        useRmb = false;
    }
    string  rolename;       //用户名
    int     gold;           //押金币
    bool    useRmb;         //是否用金钻压的
    int     thiefrmb;       //小偷费用
    bool    thiefed;        //偷过了
};

typedef map<int,GamblerInfo> GamblerMap;
typedef std::pair<int , GamblerInfo> gmbType;

struct MatchPet
{
    int     id;             //宠物ID
    int     petmod;         //模板id
    string  name;           //宠物名字
    string  ownername;      //主人名字
    int     owneID;         //主人id
    int     battle;         //战斗力
    GamblerMap  gmb;        //支持者
    bool    win;            //是否胜利
};

class MatchPetFactor
{
public:
    static void CreatorMatchPet(int petID,MatchPet & MPet);
    static void SetMatchPetWin(MatchPet & val,bool b);
    static void AddMatchPetGablerMap(MatchPet & val,gmbType & gmbval);
};

void MatchPetFactor::CreatorMatchPet(int petID,MatchPet & tmpPet)
{
    
    RedisResult resultPetInfo(redisCmd("hget %s %d",
                                       CASION_ACTION_DB_KEY_PETID.c_str(),
                                       petID));
    Json::Reader reader;
    Json::Value value;
    std::string str=resultPetInfo.readStr();
    if (!reader.parse(str, value) || value.type() != Json::objectValue) {
        return ;
    }

    //{"battle":12609,"id":790,"mod":104053,"owenID":657271,"ownname":"弗德艾德姆","petname":"铠甲巨人"}
    
    tmpPet.id = value["id"].asInt();
    tmpPet.petmod=value["mod"].asInt();
    tmpPet.battle=value["battle"].asInt();
    tmpPet.name = value["petname"].asString();
    tmpPet.owneID=value["owenID"].asInt();
    //tmpPet.owneID= (int)CONFIGUREPETOWNEID;
    tmpPet.ownername=value["ownname"].asString();
                            

}
void MatchPetFactor::SetMatchPetWin(MatchPet & val,bool b)
{
        val.win = b;
}
void MatchPetFactor::AddMatchPetGablerMap(MatchPet & val,gmbType & gmbval)
{
        val.gmb.insert(gmbval);
}


class RoundFactor
{
public:
    static void CreatorRound(std::vector<int> v_petID,Round & v_MatchPet);
    
};

void RoundFactor::CreatorRound(std::vector<int> v_petID,Round & v_MatchPet)
{
    ;
    std::vector<int>::iterator beg = v_petID.begin();
    std::vector<int>::iterator end = v_petID.end();
    for(;beg!=end;++beg)
    {
        MatchPet  MPet;
        MatchPetFactor::CreatorMatchPet(*beg,MPet);
        v_MatchPet.push_back(MPet);
        
        
    }
}


class RoundPetIdFinder
{
public:
    RoundPetIdFinder(int id):_id(id){}
    bool operator()(const Round::value_type& v){
        return v.id == _id;
    }
protected:
    int _id;
};


class PaihangPetData
{
public:
    vector<SortPetInfo> data;
};
static PaihangPetData _PaihangPetData;

void finishLoadPaihangPetData(int rid,int type,vector<SortPetInfo> data)
{
    vector<SortPetInfo>& v =  _PaihangPetData.data;
    v.clear();
    v.insert(v.end(),data.begin(),data.end());
}

#pragma mark -
#pragma mark PetCasionWagerCompensate 押金自动补偿
#pragma mark -

class PetCasionWagerCompen
{
public:
    int         mCostType;          //金币/金钻
    int         mCostValue;
    int         mGold     ;          //金币
    string      mRoundName;         //第几轮
    string      mRoleName;          //角色名
    int         mpetID    ;           // pet ID
    int         mRoleID ;           //角色 ID
    int         mRoleLvl;           //角色 Lvl
    
    PetCasionWagerCompen():mCostType(0),mCostValue(0){}
    ~PetCasionWagerCompen(){}
    
    string str() const
    {
        Json::Value value;
        value["costtype"] = mCostType;
        value["costvalue"] = mCostValue;
        value["roundname"]= mRoundName;
        value["rolename"]= mRoleName;
        value["petid"]= mpetID;
        value["roleID"]=mRoleID;
        value["roleLvl"]=mRoleLvl;
        value["gold"]=mGold;
        return Json::FastWriter().write(value);
    }
    
    void load(const char* str)
    {
        if(NULL == str) return;
        
        Json::Reader reader;
        Json::Value retval;
        
        if ( ! reader.parse(str, retval)){
            xyerr("PetCasionWager json parse error , %s ",str);
            return;
        }
        mpetID = retval["petid"].asInt();
        mRoleID = retval["roleID"].asInt();
        mCostType = retval["costtype"].asInt();
        mCostValue = retval["costvalue"].asInt();
        mRoundName = retval["roundname"].asString();
        mRoleName = retval["rolename"].asString();
        mRoleLvl=retval["roleLvl"].asInt();
        mGold=retval["gold"].asInt();
    }
};

std::ostream& operator<<(std::ostream& out, const PetCasionWagerCompen& obj)
{
    return out << obj.str();
}

std::istream& operator>>(std::istream& in,PetCasionWagerCompen& obj)
{
    string tmp;
    in>>tmp;
    obj.load(tmp.c_str());
    return in;
}

class PetCToGamblerInfoFactor
{
public:
    static void TranceTo(PetCasionWagerCompen & val_petCasion,GamblerInfo & val_gambler);
};

void PetCToGamblerInfoFactor::TranceTo(PetCasionWagerCompen &val_petCasion, GamblerInfo & val_gambler)
{
    //Role * role = SRoleMgr.GetRole(val_petCasion.mRoleID);
    
    tagActivityPetCosinoCfg _matchcfg = *(ActivityPetCosinoCfgMgr::getcfg());
    
    ActivityPetCosinoWagerLevel*  wagercfg = _matchcfg.getWager(val_petCasion.mRoleLvl);
    
    bool wagerok = false;
    int goldindex = 0;
    if(val_petCasion.mCostType == eRmbCost){
        for (int i = 0; i< wagercfg->rmb.size(); ++i) {
            if(val_petCasion.mCostValue == wagercfg->rmb[i]){
                wagerok = true;
                goldindex = i;
                break;
            }
        }
    }else if(val_petCasion.mCostType == eGoldCost){
        for (int i = 0; i < wagercfg->gold.size(); ++i) {
            if(val_petCasion.mCostValue == wagercfg->gold[i]){
                wagerok = true;
                goldindex = i;
                break;
            }
        }
    }
    //if( ! wagerok)
        //return SendWagerAck(role->getSessionId(),ack,enPetCasinoWagerResult_NeedMoreGold);
    
    val_gambler.rolename = val_petCasion.mRoleName;
    val_gambler.gold = val_petCasion.mGold;
    val_petCasion.mCostType == eRmbCost ?val_gambler.useRmb = true:val_gambler.useRmb=false ;
    val_gambler.thiefrmb = wagercfg->thief[goldindex];
    val_gambler.thiefed = false;
    
}
#pragma mark -
#pragma mark Class PetMatchMgr
#pragma mark -




class PetMatchMgr                   //一场比赛
{
public:
    PetMatchMgr();
    ~PetMatchMgr();
    void start(int startime);       //比赛开始
    void stop();                    //正常结束
    void forcestop();               //强制结束
    inline bool isOpen();           //正在比赛
    void roundEnd();                //完成一轮
    void save_initHistory(){_history.clear(); initHistory();}
public:
    void reqStatus(Role* role,req_activity_pet_casino_status& req);     //请求台面
    void reqWager(Role* role,req_activity_pet_casino_wager& req);       //请求押注
    void reqThief(Role* role,req_activity_pet_casino_thief& req);       //请求小偷
    void reqCurrentHistory(Role* role,req_activity_pet_casino_current_history& req);//请求当前历史
protected:
    bool initCfg(int startime);     //初始配置
    bool initPet();                 //初始参赛宠
    void initHistory2();             //初始历史
    void initHistory();             //初始历史
    
    const vector<int>& matchTime();   //各轮时间
    Round& curRound();                //当前轮
    string curRoundName();
    
    void getRoleWager(int roleid,int& petid,int& wager);
    void sendLoserMail(int rid,const char* rname,const char* pname ,const char* powner);
    void sendWinerMailAndAddGold(int rid,const char* rname,const char* pname,const char* powner,int gold);
    void sendPetOwnerMail(const char* ownerName,const char* petName,int gold, bool champion);
    
    GamblerInfo* lastRoleGmbInfo(Role* role);
    
    void setRoundTimer();               //启动定时
    void discerningEyes();              //慧眼识珠
    void awardPetOwner();               //奖励幻兽主人
    
    void writelog();
public:
    void setAllData(bool open,vector<MatchPet> & vectPets,MatchHistory & vectHistorys ,tagActivityPetCosinoCfg & valMatchcfg ,int valStartTime){
        _open       =   open        ;
        _pets       =   vectPets        ;
        _history    =   vectHistorys     ;
        _matchcfg   =   valMatchcfg    ;
        _startTime  =   valStartTime   ;
    }
    void gm_setRoundTime(){setRoundTimer();}
protected:
    bool                        _open;          //正在比赛吗
    vector<MatchPet>            _pets;          //参赛宠
    MatchHistory                _history;       //比赛历史
    tagActivityPetCosinoCfg     _matchcfg;      //比赛配置
    int                         _startTime;     //开始时间
public:
    //下面这个函数会将“大比拼活动的运行时信息进行保存到数据库中”
    //调用的地方，1 活动开始时（初始化表中的数据） 2 每轮比赛结束时（更新表中的数据）
    void savePetInfoRunningInfoToDB();
    void saveHistoryInfoRunningInfoToDB();
    //CASION_ACTION_DB_KEY_TIME
    void saveTimeInfoRunningInfoToDB();
    void cleanRunningInfoToDB();
};

#pragma mark -
#pragma mark PetMatchMgr 实现

class PetCasionWagerCompensateMgr
{
    typedef RedisDhash<int, PetCasionWagerCompen, rnPetCasinoWager> WagerMap;
public:
    PetCasionWagerCompensateMgr()
    {
        mWagerMap.active(999992);
    }
    ~PetCasionWagerCompensateMgr(){}
    
    void insert(int roleid , const PetCasionWagerCompen& wager)
    {
        mWagerMap.set(roleid, wager);
    }
    void clear()
    {
        mWagerMap.clear();
    }
    
    void CompensateOrRecorver();
    
    void Compensate();
    
    map<int , map <int,GamblerInfo> > & GetAllInfoFromDB(map<int , map <int,GamblerInfo> >  & mapPetIDtoGamblerInfos )
    {
        mWagerMap.load(999992);
        //map<int , map <int,GamblerInfo> > total_data;
        for (WagerMap::iterator it = mWagerMap.begin();it != mWagerMap.end() ; ++it)
        {
            map<int ,map <int,GamblerInfo>  >::iterator itFind = mapPetIDtoGamblerInfos.find(it->second.mpetID);
            if (itFind != mapPetIDtoGamblerInfos.end()) {
                GamblerInfo valGamblerInfo;
                PetCToGamblerInfoFactor::TranceTo(it->second, valGamblerInfo);
                itFind->second.insert(std::make_pair(it->second.mRoleID, valGamblerInfo));
            }
            else
            {
                map<int,GamblerInfo> mapGamblerInfo;
                GamblerInfo valGamblerInfo;
                PetCToGamblerInfoFactor::TranceTo(it->second, valGamblerInfo);
                mapGamblerInfo.insert(std::make_pair(it->second.mRoleID, valGamblerInfo));
                mapPetIDtoGamblerInfos.insert(std::make_pair(it->second.mpetID, mapGamblerInfo));
            }
            
        }
        return mapPetIDtoGamblerInfos;
    }

    
protected:
    void mail(int roleid , const PetCasionWagerCompen& wager)
    {
        stringstream attach;
        if(eRmbCost == wager.mCostType)
            attach<<"rmb "<<wager.mCostValue<<"*1;";
        else
            attach<<"gold "<<wager.mCostValue<<"*1;";
        
        MailFormat *f = MailCfg::getCfg("pet_battle_compensation");
        if (f)
        {
            
            //尊敬的%s，由于服务器开了个小差，导致“幻兽大比拼”活动无法继续进行，您在%s下注的%d%s已经返还，引起的不便请见谅！
            string mail_content = f->content;
            string rolename = wager.mRoleName;
            string roundname = wager.mRoundName;
            string costvalue = Utils::itoa(wager.mCostValue);
            string costname = costName(wager.mCostType);
            
            if (4 != find_and_replace(mail_content, 4, rolename.c_str(), roundname.c_str(), costvalue.c_str(), costname.c_str()))
            {
                log_warn("mail content format error. [pet_battle_compensation]");
            }
            
            bool ret = sendMail(0,
                                f->sendername.c_str(),
                                wager.mRoleName.c_str(),
                                f->title.c_str(),
                                mail_content.c_str(),
                                attach.str().c_str(),
                                Utils::itoa(roleid).c_str());
            
            if( false ==  ret )
                StoreOfflineItem(roleid,attach.str().c_str());
        }
    }
    
protected:
    WagerMap    mWagerMap;
};





void PetCasionWagerCompensateMgr::Compensate()
{

        mWagerMap.load(999992);
        
        if(mWagerMap.size() != 0)
        {
            Xylog log(eLogName_PetCasion,0);
            log<<"幻兽大比拼补偿人数"<<mWagerMap.size();
        }
        for (WagerMap::iterator it = mWagerMap.begin();it != mWagerMap.end() ; ++it)
        {
            mail(it->first,it->second);
        }
    // 去除条件
    clear();

}
void PetCasionWagerCompensateMgr::CompensateOrRecorver()
{
    //这是根据当前时间确定的 恢复状态，数据库中保存了 比赛开始及结束的时间，
    //如果数据库中没有时间，则用最近的比赛时间打开比赛
    
    //如果数据库中没有信息，也许要重新从 排行榜中取数据 ,这份数据先进行保留，也许会用到，实际上这时候数据还取不到
    
    
    //首先在数据库中取出数据
    std::pair<size_t, size_t> MatchTime(std::make_pair(0, 0));

    RedisResult TimeResult(redisCmd("hvals %s",CASION_ACTION_DB_KEY_TIME.c_str()));
    std::string StrTime=TimeResult.readHash(0,"");
    if (!StrTime.empty()) {
        StringTokenizer TokenTime(StrTime, ":");
        MatchTime.first =Utils::safe_atoi(TokenTime[0].c_str());
        MatchTime.second =Utils::safe_atoi(TokenTime[1].c_str());
    }
    
//    int TimeElementnum = TimeResult.getHashElements();
//    for (int j=0;j<TimeElementnum;++j)
//    {
//        std::string tmpStr=TimeResult.readHash(j,"");
//        
//        StringTokenizer reToken(tmpStr, ":");
//        MatchTime.first =Utils::safe_atoi(reToken[0].c_str());
//        MatchTime.second =Utils::safe_atoi(reToken[1].c_str());
//    }
    //如果不错误的时间要进行重新设置
    if(MatchTime.first == MATCHERROR ||
       MatchTime.first > time(NULL)  ){
        MatchTime.first = 0;
        MatchTime.second =0;
    }
    //如果数据库中没有找到比赛时间信息，则将最近的比赛开启
    if (MatchTime.first == 0 || MatchTime.first > time(NULL)) {
        MatchTime = CalMatchStartAndEndTime(time(NULL));
    }
    if (MatchTime.first == MATCHERROR) {
        Compensate();
        Xylog log(eLogName_PetCasion,0);
        log<<"MatchTime.first == MATCHERROR 恢复操作结束";
        return ;
    }
    std::pair< vector<MatchPet>, MatchHistory>  pairVectMatchPetToMatchHistory;
    if(!GetObjFromDB(pairVectMatchPetToMatchHistory))
    {
        //数据库中没有数据，恢复操作进行终止
        Xylog log(eLogName_PetCasion,0);
        log<<"!GetObjFromDB(pairVectMatchPetToMatchHistory) 恢复操作结束";
        Compensate();
        return;
    }
    //计算出比赛进行的状态
    PetRecorveState switchState = CalRecorveState(time(NULL), MatchTime.first, MatchTime.second);
    // 恢复  参赛宠 及 比赛历史轮记录
    RedisResult HisResult(redisCmd("hvals %s", CASION_ACTION_DB_KEY_HISTORY.c_str()));
    int Hiselementnum = HisResult.getHashElements();
    tagActivityPetCosinoCfg  PetCosinoCfg = *ActivityPetCosinoCfgMgr::getcfg();
    
    switch (switchState) {
        case NO_START:
            //重启时候，比赛没有开始（比赛时间有限制）
            // 如果之前是在在下注情况下，则进行补尝 ，如果之前没有下注，调用下面的函数也不会有效果
            // 所以进行调用
            //g_setNextTime(MatchTime.first,MatchTime.second);
            Compensate();
            break;
        case NO_ROUND_1:
            //比赛开始，但第一轮还没有开始，处于可下注状态
            g_setNextTime(MatchTime.first,MatchTime.second);
            //重启时候，对第一轮处理
            logicFuntion(1,Hiselementnum,pairVectMatchPetToMatchHistory,PetCosinoCfg,MatchTime,*this);
            //SActMgr.StartActiveByActID(enComparePet);
            break;
        case NO_ROUND_2:
            //比赛开始，但第二轮还没有开始，处于可下注状态
            g_setNextTime(MatchTime.first,MatchTime.second);
            //重启时候，对第二轮处理
            logicFuntion(2,Hiselementnum,pairVectMatchPetToMatchHistory,PetCosinoCfg,MatchTime,*this);
            //SActMgr.StartActiveByActID(enComparePet);
            break;
        case NO_ROUND_3:
            //比赛开始，但第三轮还没有开始，处于可下注状态
            g_setNextTime(MatchTime.first,MatchTime.second);
            //但重启时候，对第三轮处理
            logicFuntion(3,Hiselementnum,pairVectMatchPetToMatchHistory,PetCosinoCfg,MatchTime,*this);
            //SActMgr.StartActiveByActID(enComparePet);
            break;
        case NO_END:
            //比赛即将结束
            g_setNextTime(MatchTime.first,MatchTime.second);
            //但重启时候，对结束处理
            logicFuntion(4,Hiselementnum,pairVectMatchPetToMatchHistory,PetCosinoCfg,MatchTime,*this);
            //SActMgr.StartActiveByActID(enComparePet);
            break;
            
        case READY_END:
            //比赛已经结束
            //重启时候，这比赛已经结束（比赛时间有限制）
            Compensate();
            break;
        case RECORVESTATEERROR:
            //down机时的那场比赛结束，对下一场比赛处理,由于现在从排行榜取不出数据，则下一场比赛将不能开启，
            //下下场比赛将会正常启动
            //重启时候，这比赛已经结束，但这里可以模拟这刚结束那场比赛的结果，然后通知给玩家
        {
            //Compensate();
            
            MatchTime = CalMatchStartAndEndTime(time(NULL));
            PetRecorveState j=CalRecorveState(time(NULL),MatchTime.first,MatchTime.second);
            if(j > NO_START  && j < READY_END){
                // 已经进入下一比赛
                Compensate();
                // 为上一场比赛收尾处理 :从排行榜中得到数据，然后进行摸拟
                getSortPetData(0, 1, SYNC_PAIHANG_PET_AMOUNT, finishLoadPaihangPetDataByRecorveOnly);
                return ;
            }
            else{
                Compensate();
            }
            break;
        }
        default:
            // 计算错误
            // 未知情况处理，直接进行邮件
            Compensate();
            break;
    }
}


PetCasionWagerCompensateMgr _PetCasionWagerCompensateMgr;



void g_setNextTime(int val_startTime,int val_end_time);

void PetMatchMgr::cleanRunningInfoToDB()
{
    Xylog log(eLogName_PetCasion_TraceBack,0);
    log<<"PetMatchMgr::cleanRunningInfoToDB 清除数据";
    doRedisCmd("del %s * ", CASION_ACTION_DB_KEY_PETID.c_str());
    doRedisCmd("del %s * ", CASION_ACTION_DB_KEY_HISTORY.c_str());
    doRedisCmd("del %s * ", CASION_ACTION_DB_KEY_TIME.c_str());
    
}
void PetMatchMgr::savePetInfoRunningInfoToDB()
{
    // 保存参赛宠信息
    vector<MatchPet>::iterator beg = _pets.begin();
    vector<MatchPet>::iterator end = _pets.end();
    
    for (;beg != end;++beg)
    {
        std::string petName = beg->name;
        std::string ownName = beg->ownername;
        int battle = beg->battle;
        int petID = beg->id;
        int petmod = beg->petmod;
        int owneID = beg->owneID;
        
        //{"battle":12609,"id":790,"mod":104053,
        // "owenID":657271,"ownname":"弗德艾德姆","petname":"铠甲巨人"}
        Json::Value value;
        std::string str;
        value["id"] = petID;
        value["mod"] = petmod;
        value["battle"] = battle;
        value["ownname"] = ownName;
        value["petname"] = petName;
        value["owenID"]=owneID;
        str = Json::FastWriter().write(value);
        doRedisCmd("hset %s %d %s", CASION_ACTION_DB_KEY_PETID.c_str(),petID,str.c_str());
        
    }
    
}



void PetMatchMgr::saveTimeInfoRunningInfoToDB()
{
    std::pair<int, int> timePair = CalMatchStartAndEndTime(time(NULL));
    if(timePair.first == MATCHERROR){
        //如果计算出的是错误的时间，则时间数据不能入库，此处进行过滤
        return ;
    }
    doRedisCmd("hset %s %s %d:%d", CASION_ACTION_DB_KEY_TIME.c_str(),CASION_ACTION_DB_KEY_TIME.c_str(),timePair.first,timePair.second );
}
void PetMatchMgr::saveHistoryInfoRunningInfoToDB()
{
    
    // save the "CurRound" in the history throw push-back
    int index = _history.size();
    
    vector<Round>::reverse_iterator rbeg = _history.rbegin();
    if (rbeg == _history.rend()) {
        return;
    }
    vector<MatchPet>::iterator MPbeg = rbeg->begin();
    vector<MatchPet>::iterator MPeng = rbeg->end();
    std::stringstream ss;
    std::string strTMP;
    // ";" is stoken for Pets , ":" is stoken for pet's informathion which are 'id' and 'win'
    for (;MPbeg!=MPeng;++MPbeg)
    {
        ss<<MPbeg->id<<":"<<MPbeg->win<<";";
    }
    ss>>strTMP;
    ss.clear();
    
    doRedisCmd("hset %s %d %s", CASION_ACTION_DB_KEY_HISTORY.c_str(), index,strTMP.c_str());
}

PetMatchMgr::PetMatchMgr()
{
    _open = false;
    _startTime = 0;
}
PetMatchMgr::~PetMatchMgr()
{
}

void PetMatchMgr::start(int startime)
{
    if(_open){
        Xylog log(eLogName_PetCasion_TraceBack,0);
        log<<"PetMatchMgr::start==>  活动已经开启，不能再开";
        return;
    }
        
    
    if( ! initCfg(startime) ){
        Xylog log(eLogName_PetCasion_TraceBack,0);
        log<<"PetMatchMgr::start==>  配制表出错，不能启动";
        return;
    }
    if( ! initPet() ){
        Xylog log(eLogName_PetCasion_TraceBack,0);
        log<<"PetMatchMgr::start==>  宠物数据出错，不能启动";
        return;
    }
    
    initHistory();
    
    _open = true;
    
    setRoundTimer();
    
    Xylog log(eLogName_PetCasion,0);
    log<<"幻兽比拼开波";
}

void PetMatchMgr::stop()
{
    if(!_open)
    {
        Xylog log(eLogName_PetCasion_TraceBack,0);
        log<<"PetMatchMgr::stop ===> 活动没有开启，不能stop";
        return;
    }
    _open = false;
    
    writelog();
    
    Xylog log(eLogName_PetCasion,0);
    log<<"幻兽比拼结束";
}

void PetMatchMgr::forcestop()
{
    if(!_open)
        return;
    
    _open = false;
    
    //返还金币
}

bool PetMatchMgr::isOpen()
{
    return _open;
}


void PetMatchMgr::sendLoserMail(int rid,const char* rname,const char* pname ,const char* powner)
{
    MailFormat *f = MailCfg::getCfg("pet_battle_lose");
    if (f)
    {
        string mail_title = f->title;
        //%d轮比赛
        find_and_replace(mail_title, 1, curRoundName().c_str());
        

        string mail_content = f->content;
        //很可惜！你在《幻兽大比拼》中下注%s的%s在%s比赛失利，你未获得任何奖励。
        if (3 != find_and_replace(mail_content, 3, powner, pname, curRoundName().c_str()))
        {
            log_warn("mail content format error. [pet_battle_lose]");
        }
        
        sendMail(0,
                 f->sendername.c_str(),
                 rname,
                 mail_title.c_str(),
                 mail_content.c_str(),
                 "",
                 Utils::itoa(rid).c_str());
    }
    
}

void PetMatchMgr::sendWinerMailAndAddGold(int rid,const char* rname,const char* pname,const char* powner,int gold)
{
    stringstream attach;
    attach<<"gold "<<gold<<"*1;";
    
    MailFormat *f = MailCfg::getCfg("pet_battle_win");
    if (f)
    {
        //%d轮比赛
        string mail_title = f->title;
        find_and_replace(mail_title, 1, curRoundName().c_str());
        
        //恭喜你在《幻兽大比拼》中下注%s的%s在%s比赛胜利，你得到总计%d金币的奖励。
        string mail_content = f->content;
        string gold_str = Utils::itoa(gold);
        
        if (4 != find_and_replace(mail_content, 4, powner, pname, curRoundName().c_str(), gold_str.c_str()))
        {
            log_warn("mail content format error.[pet_battle_win]");
        }
        
        bool ret = sendMail(0,
                            f->sendername.c_str() ,
                            rname,
                            mail_title.c_str(),
                            mail_content.c_str(),
                            attach.str().c_str(),
                            Utils::itoa(rid).c_str());
        
        if( false ==  ret ){
            StoreOfflineItem(rid,attach.str().c_str());
        }
    }
}

void PetMatchMgr::sendPetOwnerMail(const char* ownerName,const char* petName,int gold, bool champion)
{
    stringstream attach;
    attach<<"gold "<<gold<<"*1;";
    
    bool ret = false;
    MailFormat *f  = NULL;
    string mail_content;
    
    if (!champion)
    {
        f = MailCfg::getCfg("pet_battle_join");
    }
    else
    {
        f = MailCfg::getCfg("pet_battle_winer");
    }
    if (f)
    {
        string mail_content = f->content;
        string gold_str = Utils::itoa(gold);
        //您的幻兽[%s]参加了《幻兽大比拼》活动，您得到总计%d金币的奖励。
        //您的幻兽[%s]参加了《幻兽大比拼》活动，并获得总冠军，您得到总计%d金币的奖励。
        if (2 != find_and_replace(mail_content, 2, petName, gold_str.c_str()))
        {
            log_warn("mail content format error.[pet_battle_winner] or [pet_battle_join]");
        }
        
        ret = sendMail(0,
                       f->sendername.c_str(),
                       ownerName,
                       f->title.c_str(),
                       mail_content.c_str(),
                       attach.str().c_str(),
                       "");
        
        if( false ==  ret )
        {
            xyerr("[邮件失败]name=%s gold=%d",ownerName,gold);
        }
    }
}




void PetMatchMgr::writelog()
{
    Xylog logbegin(eLogName_PetCasion,0);
    logbegin<<"现场直播--------------------------------";
    logbegin.save();
    
    
    Round& hst = curRound();
    for (int i = 0; i < hst.size(); ++i) {
        MatchPet& mp = hst[i];
        stringstream ss;
        ss<<mp.name<<"["<<mp.id<<"],bat:"<<mp.battle<<",win:"<<mp.win;
        
        typedef map<int ,vector<int> > GoldMap;
        GoldMap goldmap;
        for (GamblerMap::iterator iter = mp.gmb.begin(); iter != mp.gmb.end(); ++iter) {
            int gold = iter->second.gold;
            int roleid = iter->first;
            GoldMap::iterator jt = goldmap.find(gold);
            if(jt != goldmap.end())
                jt->second.push_back(roleid);
            else{
                vector<int> temp; temp.push_back(roleid);
                goldmap[gold]= temp;
            }
        }
        
        for (GoldMap::iterator iter = goldmap.begin(); iter != goldmap.end(); ++iter) {
            vector<int>& vec = iter->second;
            ss<<"\t gold["<<iter->first<<"]:";
            for (int i = 0; i < vec.size(); ++i) {
                ss<<vec[i]<<",";
            }
        }
        
        Xylog log(eLogName_PetCasion,0);
        log<<ss.str();
        log.save();
    }
    
    Xylog logend(eLogName_PetCasion,0);
    logend<<"-------------------------------------";
}


string PetMatchMgr::curRoundName()
{
    Round& hst = curRound();
    int size = hst.size();
    stringstream ss;
    
    //决赛
    string str1 = GameTextCfg::getString("2001");
    //半决赛
    string str2 = GameTextCfg::getString("2002");
    
    if(size <= 1)
        return "";
    if(size == 2)
        return str1;//"决赛";
    if(size == 4)
        return str2;//"半决赛";
    if(size % 2 == 0)
    {
        ss<<"1/"<<size/2<< str1; //"决赛";
        return ss.str();
    }
    
    return "";
}

void PetMatchMgr::setRoundTimer()
{
    int timeindex = _history.size();
    int roundtime = matchTime()[timeindex];//star , round1 , round2 ,...,roundN, end
    int t  = roundtime - time(0);
    if (t < 0) {
        // t 小于零的情况正常情况下不会发生
        // 只有在恢复补尝时会发生，恢复补尝时将其设置为1立即执行
        t  =  1;
    }
    addTimer(endRoundCb,NULL, t,1);
}

void PetMatchMgr::roundEnd()
{
    if(!_open)
        return;
    
    Round& hst = curRound();
    if(hst.size()<=1)
        return;
    
    Round newhst ;
    
    for (int i = 0; i< hst.size(); i = i+2)
    {
        
        MatchPet *win = NULL, *lose = NULL;
        vector<int> bat;
        bat.push_back(hst[i].battle);
        bat.push_back(hst[i+1].battle);
        int index = getRand(bat.data(), 2);
        if(index == 0){
            win = &(hst[i]);
            lose = &(hst[i+1]);
        }else{
            win = &(hst[i+1]);
            lose = &(hst[i]);
        }
        
        win->win = true;
        lose->win = false;
        
        float winer_gold = 0.0f,loser_gold = 0.0f;
        GamblerMap::iterator iter;
        for (iter = win->gmb.begin(); iter != win->gmb.end(); ++iter)
        {
            winer_gold += iter->second.gold;
        }
        for (iter = lose->gmb.begin(); iter != lose->gmb.end(); ++iter)
        {
            loser_gold += iter->second.gold;
            sendLoserMail(iter->first, iter->second.rolename.c_str(),
                          lose->name.c_str(), lose->ownername.c_str());
        }
        
        if( 0 < winer_gold )
        {
            for (iter = win->gmb.begin(); iter != win->gmb.end(); ++iter)
            {
                int rolewin = iter->second.gold * loser_gold / winer_gold + iter->second.gold;
                sendWinerMailAndAddGold(iter->first, iter->second.rolename.c_str(),
                                        win->name.c_str(), win->ownername.c_str(),rolewin);
            }
        }
        
        MatchPet npet;
        npet = *win;
        npet.gmb.clear();
        newhst.push_back(npet);
        
        //发送走马灯
        bcMatchRetParam bc;
        bc.round = _history.size();
        bc.winers = win->gmb.size();
        bc.wowner = win->ownername;
        bc.wpetmod = win->petmod;
        bc.losers = lose->gmb.size();
        bc.lowner = lose->ownername;
        bc.lpetmod= lose->petmod;
        
        switch (_history.size())
        {
            case 1:
                broadcastPetMatchResult(kBroadcastPetMatchResultQuarter,bc);
                break;
            case 2:
                broadcastPetMatchResult(kBroadcastPetMatchResultHalf,bc);
                break;
            case 3:
                broadcastPetMatchResult(kBroadcastPetMatchResultFinal,bc);
                break;
        }
        
    }
    
    //清除补偿金
    _PetCasionWagerCompensateMgr.clear();
    
    writelog();
    
    _history.push_back(newhst);
    if(newhst.size() == 1)//冠军出来了
    {
        discerningEyes();
        if (newhst[0].owneID!=CONFIGUREPETOWNEID) {
            awardPetOwner();
        }
        return;
    }
    
    Xylog log(eLogName_PetCasion,0);
    log<<"进入下一轮";
    setRoundTimer();//启动下轮
}

//慧眼识珠
void PetMatchMgr::discerningEyes()
{
    //0、验证是否最后一轮了
    int size  = _history.size();
    if(size <= 1)
        return;
    
    if(_history[size-1].size() != 1)
        return;
    
    int petid = curRound()[0].id;
    vector<int> roleids;
    
    //1、求出最后一轮用金钻买中的人
    for (int i = 0; i < _history[size-2].size(); ++i)
    {
        MatchPet& mp = _history[size-2][i];
        if(mp.id != petid)
            continue;
        
        for (GamblerMap::iterator iter = mp.gmb.begin(); iter != mp.gmb.end(); ++iter)
        {
            if( iter->second.useRmb )
                roleids.push_back(iter->first);
        }
    }
    
    //2、验证他们前面有没有买中
    for (int i = 0; i < size - 2; ++i )
    {
        Round& round = _history[i];
        for (int j = 0; j < round.size(); ++j)
        {
            MatchPet& mp = round[j];
            if(mp.id != petid)
                continue;
            
            for (int k = roleids.size() -1 ; k >= 0; --k)
            {
                int roleid = roleids[k];
                
                //此轮没买 或者 没有用金钻买
                if(mp.gmb.count(roleid) == 0 || ! mp.gmb[roleid].useRmb)
                {
                    roleids.erase(roleids.begin()+k);
                }
            }
        }
    }
    
    //3、求出前三个人的名字
    vector<string> names;
    for (int i = 0; i < roleids.size(); ++i)
    {
        Role* role = SRoleMgr.GetRole(roleids[i]);
        if(role == NULL)
            continue;
        names.push_back(role->getRolename());
        
        if(names.size() == 3)
            break;
    }
    
    //4、发送走马灯
    bcMatchPetDiscernEyeParam bc;
    for (int i = 0; i< names.size(); ++i)
    {
        switch (i) {
            case 0:
                bc.name1 = names[i];
                break;
            case 1:
                bc.name2 = names[i];
                break;
            case 2:
                bc.name3 = names[i];
                break;
        }
    }
    
    bc.wowner = curRound()[0].ownername;
    bc.wpetmod = curRound()[0].petmod;
    
    if(! names.empty())
        broadcastPetMatchDiscerningEyes(bc);
    
	//5、称号检查
	for (int i = 0; i < roleids.size(); ++i)
    {
		SHonorMou.procHonor(eHonorKey_Gambler, roleids[i]);
	}
}

//奖励幻兽主人
void PetMatchMgr::awardPetOwner()
{
    int size  = _history.size();
    if(size <= 1)
        return;
    
    if(_history[size-1].size() != 1)
        return;
    
    int winpetid = curRound()[0].id;
    const int baseAwardGold = _matchcfg.mAwardOwnerGold;
    
    for (int i =0 ; i < _history[0].size(); ++i)
    {
        bool champion = false;
        int award = baseAwardGold;
        const MatchPet& pet = _history[0][i];
        if(pet.id == winpetid)
        {
            champion = true;
            award *= 2;
        }
        sendPetOwnerMail(pet.ownername.c_str(),pet.name.c_str(),award,champion);
    }
}

bool PetMatchMgr::initCfg(int startime)
{
    const tagActivityPetCosinoCfg* cfg = ActivityPetCosinoCfgMgr::getcfg();
    if(NULL == cfg){
        Xylog log(eLogName_PetCasion_TraceBack,0);
        log<<"PetMatchMgr::initCfg==>  条件NULL == cfg 成立，配制表出错";
        return false;
    }

    _matchcfg = *cfg;
    _startTime = startime;
    
    
    if( matchTime().empty()){
        Xylog log(eLogName_PetCasion_TraceBack,0);
        log<<"PetMatchMgr::initCfg==>  条件matchTime().empty() 成立，配制表出错，时间为空";
        return false;
    }
    return true;
}

bool PetMatchMgr::initPet()
{
    _pets.clear();
    
    int round = matchTime().size() - 2;   //扣去开始结束
    if(round < 1 ){
        Xylog log(eLogName_PetCasion_TraceBack,0);
        log<<"PetMatchMgr::initPet==>  条件round < 1成立 ";
        return false;
    }
    int petAmount = pow(2,round);
    
    for (int i = 0; i < _PaihangPetData.data.size(); ++i) {
        if(_pets.size() >= petAmount)
            break;
        
        const SortPetInfo& sp = _PaihangPetData.data[i];
        MatchPet mp;
        mp.owneID=sp.masterid;
        mp.id = sp.petid;
        mp.petmod = sp.petmod;
        
        PetCfgDef* petcfg = PetCfg::getCfg(sp.petmod);
        if(NULL == petcfg)
            continue;
        
        mp.name = petcfg->name;
        mp.ownername = sp.mastername;
        mp.battle = sp.battle;
        mp.win  = true;
        
        _pets.push_back(mp);
    }
    
    if( _pets.size() < petAmount){      //宠物从配制表中取
        //_pets.clear();
        //return false;
        const tagActivityPetCosinoCfg* cfg = ActivityPetCosinoCfgMgr::getcfg();
        // 这里进行数据补尝处理，从策划的配制数据中取出进行补尝
        vector<string>::const_iterator begPet = cfg->PetDataBak.begin();
        vector<string>::const_iterator endPet = cfg->PetDataBak.end();
        for (;begPet!= endPet && _pets.size() <petAmount;++begPet ) {
            StringTokenizer tokens(*begPet,":");
            SortPetInfo tmpPetInfo;
            tmpPetInfo.masterid = CONFIGUREPETOWNEID;
            tmpPetInfo.petmod = Utils::safe_atoi(tokens[0].c_str());
            tmpPetInfo.petid= _pets.size()+VIRTUALPETMATCHID;
            tmpPetInfo.battle= Utils::safe_atoi(tokens[1].c_str());
            tmpPetInfo.mastername = g_RandNameMgr.getRandName(true);
            tmpPetInfo.growth = CONFIGUREGROUTH;
            tmpPetInfo.rank = CONFIGURERANK;
            
            
            const SortPetInfo& sp = tmpPetInfo;
            MatchPet mp;
            mp.owneID = sp.masterid;
            mp.id = sp.petid;
            mp.petmod = sp.petmod;
            
            PetCfgDef* petcfg = PetCfg::getCfg(sp.petmod);
            if(NULL == petcfg)
                continue;
            
            mp.name = petcfg->name;
            mp.ownername = sp.mastername;
            mp.battle = sp.battle;
            mp.win  = true;
            
            _pets.push_back(mp);
        }

    }
    
    return true;
}


void PetMatchMgr::initHistory()
{
    _history.clear();
    Round hst;
    
    if(_pets.size() <= 1)
        return;
    
    vector<int> tmp,indexs;
    for (int i = 0; i< _pets.size(); ++i) {
        tmp.push_back(i);
    }
    
    while (! tmp.empty()) {
        if(tmp.size() == 1)
        {
            indexs.push_back(tmp[0]);
            break;
        }
        int randnum = rand() % tmp.size();
        indexs.push_back(tmp[randnum]);
        tmp.erase(tmp.begin()+randnum);
    }
    
    for (int i = 0; i < indexs.size(); ++i) {
        int idx = indexs[i];
        hst.push_back(_pets[idx]);
    }
    
    stringstream ss;
    for (int i = 0; i < hst.size(); ++i) {
        ss<<hst[i].id<<"["<<hst[i].battle<<"]\t";
    }
    
    
    _history.push_back(hst);
}

void PetMatchMgr::initHistory2()
{
    _history.clear();
    
    
    
    /*
     A:  1   8
     B:  4   5
     -----------------
     C:  2   7
     D:  3   6
     */
    
    Round hst;
    
    int size = _pets.size();
    hst.resize(size);
    
    if(size <= 1)
        return;
    
    if( 2 == size){
        for (int i = 0; i< size; ++i) {
            hst[i] = _pets[i];
        }
        _history.push_back(hst);
        return;
    }
    
    //先放种子选手
    for (int i = 0 ; i < size * 0.5; i = i + 2) {
        MatchPet& pet1= _pets[i];       //pet1
        MatchPet& pet2= _pets[i+1];     //pet2
        
        int pos1 = i;              //0
        int pos2 = size - i - 2 ;  //6
        
        hst[pos1] = pet1;
        hst[pos2] = pet2;
    }
    
    //再放普通选手
    for (int i = size -1; i >= size * 0.5; i = i-2) {
        MatchPet& pet8= _pets[i];       //pet8
        MatchPet& pet7= _pets[i-1];     //pet7
        
        int pos8 = size - i;        //1
        int pos7 = i;               //7
        
        hst[pos7] = pet7;
        hst[pos8] = pet8;
    }
    
    
    stringstream ss;
    for (int i = 0; i < hst.size(); ++i) {
        ss<<hst[i].id<<"["<<hst[i].battle<<"]\t";
    }
    //xylog(PetCasionLog,"初始化战斗大比拼，%s",ss.str().c_str());
    _history.push_back(hst);
}

const vector<int>& PetMatchMgr::matchTime()
{
    for (int i = 0, ni = _matchcfg.times.size(); i<ni; ++i) {
        if(_matchcfg.times[i][0] == _startTime){
            return (_matchcfg.times[i]);
        }
    }
    static vector<int> temp;
    return temp;
}

Round& PetMatchMgr::curRound()
{
    static Round temp;
    int size = _history.size();
    if( 0 == size)
        return temp;
    else{
        return _history[size-1];
    }
}

void PetMatchMgr::getRoleWager(int roleid,int& petid,int& wager)
{
    Round& hst = curRound();
    for (int i = 0; i < hst.size(); ++i) {
        MatchPet& mp = hst[i];
        GamblerMap::iterator iter = mp.gmb.find(roleid);
        if(iter != mp.gmb.end() ){
            petid = mp.id;
            wager = iter->second.gold;
            return;
        }
    }
}

void PetMatchMgr::reqStatus(Role* role,req_activity_pet_casino_status& req)
{
    if(role == NULL)
        return;
    
    int roleid = role->getInstID();
    
    ack_activity_pet_casino_status ack;
    
    if( ! _open ){
        ack.status = enPetCasinoStatus_Close;
        return sendNetPacket(role->getSessionId(), &ack);
    }
    
    ack.matchtime.insert(ack.matchtime.end(),matchTime().begin(),matchTime().end());
    int curtime = time(0);
    for (int i = 0; i<ack.matchtime.size(); ++i) {
        ack.matchtime[i] -= curtime;
    }
    
    ack.status = enPetCasinoStatus_Wagering;
    for (int i = 0; i < _pets.size(); ++i) {
        obj_casino_pet obj;
        obj.petid = _pets[i].id;
        obj.petmod = _pets[i].petmod;
        obj.name = _pets[i].name;
        obj.ownername = _pets[i].ownername;
        obj.battle  = _pets[i].battle;
        ack.allpets.push_back(obj);
    }
    
    for (int i = 0; i < _history.size(); ++i) {
        Round& rhst = _history[i];
        stringstream ss;
        for (int j = 0; j< rhst.size(); ++j) {
            ss<< rhst[j].id <<":"<<rhst[j].gmb.size()<<";";
        }
        ack.history.push_back(ss.str());
    }
    
    getRoleWager(roleid, ack.petid, ack.wager);
    
    
    GamblerInfo* pinfo = lastRoleGmbInfo(role);
    if(NULL == pinfo || pinfo->thiefed )
        ack.thief = 0;
    else
        ack.thief = 1;
    
    sendNetPacket(role->getSessionId(), &ack);
}

static void SendWagerAck(int sessionid,ack_activity_pet_casino_wager& ack,int code)
{
    ack.errorcode = code;
    sendNetPacket(sessionid, &ack);
}

void PetMatchMgr::reqWager(Role* role,req_activity_pet_casino_wager& req)       //请求押注
{
    if(role == NULL)
        return;
    int roleid = role->getInstID();
    ack_activity_pet_casino_wager ack;
    
    //未开
    if( ! _open )
        return SendWagerAck(role->getSessionId(),ack,enPetCasinoWagerResult_CasinoClose);
    
    //已下注
    int petid = 0,rolewager=0;
    getRoleWager(roleid, petid, rolewager);
    if( petid != 0)
        return SendWagerAck(role->getSessionId(),ack,enPetCasinoWagerResult_AlreadyWager);
    
    //比赛结束
    Round& hst = curRound();
    if(hst.size() <= 1)
        return SendWagerAck(role->getSessionId(),ack,enPetCasinoWagerResult_PetError);
    
    //宠物ID有误
    Round::iterator iterpet = std::find_if(hst.begin(), hst.end(), RoundPetIdFinder(req.petid));
    if( hst.end() == iterpet)
        return SendWagerAck(role->getSessionId(),ack,enPetCasinoWagerResult_PetError);
    
    
    //配置出错
    ActivityPetCosinoWagerLevel*  wagercfg = _matchcfg.getWager(role->getLvl());
    if(wagercfg == NULL){
        xyerr("ERROR PetMatchMgr::reqWager(Role* role,");
        return SendWagerAck(role->getSessionId(),ack,enPetCasinoWagerResult_UnknowError);
    }
    
    //金币有误
    bool wagerok = false;
    int goldindex = 0;
    if(req.wager_type == eRmbCost){
        for (int i = 0; i< wagercfg->rmb.size(); ++i) {
            if(req.wager == wagercfg->rmb[i]){
                wagerok = true;
                goldindex = i;
                break;
            }
        }
    }else if(req.wager_type == eGoldCost){
        for (int i = 0; i < wagercfg->gold.size(); ++i) {
            if(req.wager == wagercfg->gold[i]){
                wagerok = true;
                goldindex = i;
                break;
            }
        }
    }
    if( ! wagerok)
        return SendWagerAck(role->getSessionId(),ack,enPetCasinoWagerResult_NeedMoreGold);
    
    //金币金钻不够
    if ( role->CheckMoneyEnough(req.wager, req.wager_type,"幻兽大比拼押注") != CE_OK)
        return SendWagerAck(role->getSessionId(),ack,enPetCasinoWagerResult_NeedMoreGold);
    
    GamblerInfo info;
    info.gold = wagercfg->gold[goldindex];
    info.thiefrmb = wagercfg->thief[goldindex];
    info.thiefed = false;
    info.useRmb = (req.wager_type == eRmbCost);
    info.rolename = role->getRolename();
    iterpet->gmb[roleid] = info;
    
    
    Xylog log(eLogName_PetCasionWager, roleid);
    log<<req.petid<<req.wager_type<<info.gold;
    
    if (role->getDailyScheduleSystem()) {
        role->getDailyScheduleSystem()->onEvent(kDailySchedulePetMatch, 0, 1);
    }
    
    //记录补偿金
    PetCasionWagerCompen compensation;
    compensation.mCostType = req.wager_type;
    compensation.mCostValue = req.wager;
    compensation.mGold      = info.gold;
    compensation.mRoundName = curRoundName();
    compensation.mRoleName = role->getRolename();
    compensation.mRoleID = role->getId();
    compensation.mpetID = req.petid;
    compensation.mRoleLvl = role->getLvl();
    _PetCasionWagerCompensateMgr.insert(roleid, compensation);
    
    ack.petid = iterpet->id;
    ack.wager = info.gold;
    return SendWagerAck(role->getSessionId(),ack,enPetCasinoWagerResult_Success);
}

//请求当前历史
void PetMatchMgr::reqCurrentHistory(Role* role,req_activity_pet_casino_current_history& req)
{
    if(NULL == role)
        return;
    
    int roleid = role->getInstID();
    ack_activity_pet_casino_current_history ack;
    
    //未开
    if( ! _open ){
        return sendNetPacket(role->getSessionId(), &ack);
    }
    
    //结束
    Round& hst = curRound();
    if(hst.size() <= 1)
        return sendNetPacket(role->getSessionId(),&ack);
    
    stringstream ss;
    for (int j = 0; j< hst.size(); ++j) {
        ss<< hst[j].id <<":"<<hst[j].gmb.size()<<";";
    }
    ack.history = ss.str();
    
    sendNetPacket(role->getSessionId(),&ack);
}

void SendAckThief(int sessionid,int code)
{
    ack_activity_pet_casino_thief ack;
    ack.errorcode = code;
    sendNetPacket(sessionid, &ack);
}

GamblerInfo* PetMatchMgr::lastRoleGmbInfo(Role* role)
{
    if(role == NULL)
        return NULL;
    
    if( !_open)
        return NULL;
    
    int size = _history.size();
    if(size <= 1)
        return NULL;
    
    Round& hst = _history[size - 2];
    
    GamblerInfo* pinfo = NULL;
    for (int i = 0;i< hst.size();++i) {
        MatchPet& mp = hst[i];
        if(mp.win)
            continue;
        GamblerMap::iterator iter = mp.gmb.find(role->getInstID());
        if(iter != mp.gmb.end()){
            pinfo = &(iter->second);
            break;
        }
    }
    
    return pinfo;
}

void PetMatchMgr::reqThief(Role* role,req_activity_pet_casino_thief& req)       //请求小偷
{
    int roleid = role->getInstID();
    
    GamblerInfo* pinfo = lastRoleGmbInfo(role);
    
    if( NULL == pinfo || pinfo->thiefed)
        return SendAckThief(role->getSessionId(), 2);
    
    if ( role->CheckMoneyEnough(pinfo->thiefrmb, eRmbCost,"幻兽大比拼小偷之手") != CE_OK)
        return SendAckThief(role->getSessionId(), 1);
    
    role->addGold(pinfo->gold,"幻兽大比拼小偷之手");
    pinfo->thiefed = true;
    SendAckThief(role->getSessionId(), 0);
    
    Xylog log(eLogName_PetCasionThief, roleid);
    log<<pinfo->thiefrmb<<pinfo->gold;
}

///////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Class PetCasino
#pragma mark -

struct NextMatchTime
{
    int start;
    int end;
    
    NextMatchTime(){
        reset();
    }
    
    void reset(){
        start = end = 0;
    }
    
    bool noMatch(){
        return 0 == start;
    }
};

class PetCasino
{
public:
    PetCasino();
    ~PetCasino();
    void update();
    void startMatchEvent();
    void endRoundEvent();
    void endMatchEvent();
    PetMatchMgr& getPetMatchMgr();
    void reqStatus(Role* role,req_activity_pet_casino_status& req);
    
    void setNextTime(int valStartTime,int valEndTime){_ntime.start = valStartTime;_ntime.end = valEndTime;}
protected:
    bool cfgchange( const tagActivityPetCosinoCfg* cfg);
    void calNextMatchTime();                        //求下场比赛时间
    void addMatchTimer();
    void remMatchTimer();
protected:
    NextMatchTime            _ntime;                //下场比赛开场时间
    vector<int>              _AllMatchTime;         //比赛时间
    PetMatchMgr              _PetMatchMgr;          //比赛管理
};

#pragma mark -
#pragma mark PetCasino 实现

PetCasino::PetCasino()
{
}
PetCasino::~PetCasino()
{
}
void PetCasino::addMatchTimer()
{
    
    Xylog log(eLogName_PetCasion,0);
    log<<"注册定时,倒计时，开工，结束"<< (int )(_ntime.start-time(0))<< (int)(_ntime.end-time(0));
    
    addTimer(startMatchCb,NULL, _ntime.start-time(0),1);
    addTimer(endMatchCb,NULL, _ntime.end-time(0),1);
    
    getSortPetData(0, 1, SYNC_PAIHANG_PET_AMOUNT, finishLoadPaihangPetData);
}
void PetCasino::remMatchTimer()
{
    Xylog log(eLogName_PetCasion,0);
    log<<"取消定时";
    
    remTimer(startMatchCb);
    remTimer(endMatchCb);
}

void PetCasino::update()
{
    const tagActivityPetCosinoCfg* cfg = ActivityPetCosinoCfgMgr::getcfg();
    if(cfg == NULL){
        xyerr("ERROR ! tagActivityPetCosinoCfg * cfg = NULL");
        return;
    }
    if(  ! cfg->opentoday )       //今天没比赛
    {
        Xylog log(eLogName_PetCasion_TraceBack,0);
        log<<"今天没比赛";
        return;
    }
    
    if(_PetMatchMgr.isOpen())     //比赛开始后忽略任何配置
    {
        Xylog log(eLogName_PetCasion_TraceBack,0);
        log<<"比赛开始后忽略任何配置";
        return;
    }
    //还没设定时
    if( _ntime.noMatch() ){
        
        Xylog log(eLogName_PetCasion_TraceBack,0);
        log<<"还没设定时";
        log<<"calNextMatchTime()之前 开始时间："<<_ntime.start<<"\t"<<"结束时间："<<_ntime.end;
        calNextMatchTime();
        
        if( _ntime.noMatch() )
            return;
        
        cfgchange(cfg);
        
        addMatchTimer();
        
        log<<"calNextMatchTime()之后 比赛开始时间："<<_ntime.start<<"\t"<<"结束时间："<<_ntime.end;
        
        return;
    }
    
    //已设定时
    
    if(  cfgchange(cfg) ){          //配置改变,更新配置，重新定时
        Xylog log(eLogName_PetCasion_TraceBack,0);
        log<<"已设定时";
        
        remMatchTimer();
        log<<"calNextMatchTime()之前 开始时间："<<_ntime.start<<"\t"<<"结束时间："<<_ntime.end;
        
        calNextMatchTime();
        
        if( _ntime.noMatch() )
            return;
        
        addMatchTimer();
        log<<"calNextMatchTime()之后 比赛开始时间："<<_ntime.start<<"\t"<<"结束时间："<<_ntime.end;
    }
}

void PetCasino::startMatchEvent()
{
    _PetMatchMgr.start(_ntime.start);
    
    //在这个地方将参赛宠信息保存起来
    // _pet  _History 信息
    getPetMatchMgr().cleanRunningInfoToDB();
    Xylog log(eLogName_PetCasion_TraceBack,0);
    log<<"PetCasino::startMatchEvent 将运行时数据保存到数据库";
    getPetMatchMgr().saveHistoryInfoRunningInfoToDB();
    getPetMatchMgr().savePetInfoRunningInfoToDB();
    getPetMatchMgr().saveTimeInfoRunningInfoToDB();
}

void PetCasino::endRoundEvent()
{
    _PetMatchMgr.roundEnd();
    //在这个地方将参赛宠信息保存起来
    //_History 信息
    //保存比赛运行时信息到数据库
    getPetMatchMgr().saveHistoryInfoRunningInfoToDB();
    
    //每一轮比赛结束时候才能进行清除赌注信息
    _PetCasionWagerCompensateMgr.clear();
}

void PetCasino::endMatchEvent()
{
    _PetMatchMgr.stop();
    _ntime.reset();
    
    //在这个地主将保存起来参赛宠信息删除
    // _pet  _History 信息
    getPetMatchMgr().cleanRunningInfoToDB();
    //比赛结束时也要清除数据，可能是多此一
    _PetCasionWagerCompensateMgr.clear();
}

PetMatchMgr& PetCasino::getPetMatchMgr()
{
    return _PetMatchMgr;
}

bool PetCasino::cfgchange( const tagActivityPetCosinoCfg* cfg)
{
    assert(cfg);
    vector<int> temp;
    
    for (int i = 0 ,ni = cfg->times.size(); i < ni ; ++i) {
        temp.insert(temp.end(), cfg->times[i].begin(),cfg->times[i].end());
    }
    
    bool change = false;
    if( ! Utils::sameIntVector(temp, _AllMatchTime)){
        Xylog log(eLogName_PetCasion_TraceBack,0);
        log<<"配置改变,更新配置，重新定时";
        std::stringstream ss;
        ss<<"********************************************\n";
        ss<<"temp:";
        vector<int>::iterator begIter = temp.begin();
        vector<int>::iterator endIter = temp.end();
        for (; begIter!=endIter;++begIter ) {
            ss<<*begIter<<";";
        }
        
        ss<<"\n############################################\n";
        ss<<"_AllMatchTime:";
        for (begIter=_AllMatchTime.begin() , endIter=_AllMatchTime.end(); begIter!=endIter;++begIter ) {
            ss<<*begIter<<";";
        }
        ss<<"\n********************************************\n";
        
        change = true;
        _AllMatchTime.clear();
        _AllMatchTime.insert(_AllMatchTime.end(), temp.begin(),temp.end());
    }
    return change;
}

void PetCasino::calNextMatchTime()
{ 
    _ntime.reset();
    
    const tagActivityPetCosinoCfg* cfg = ActivityPetCosinoCfgMgr::getcfg();
    if(NULL == cfg)
        return ;
    
    int t = time(NULL);
    
    for (int i = 0;i < cfg->times.size(); ++i) {
        if(t < cfg->times[i][0]){
            int lastindex = cfg->times[i].size() -1;
            _ntime.start =  cfg->times[i][0];
            _ntime.end    = cfg->times[i][lastindex];
            return;
        }
    }
}

void PetCasino::reqStatus(Role* role,req_activity_pet_casino_status& req)
{
    if(role == NULL)
        return;
    int roleid = role->getInstID();
    
    const tagActivityPetCosinoCfg* cfg = ActivityPetCosinoCfgMgr::getcfg();
    if(cfg == NULL){
        xyerr("ERROR ! tagActivityPetCosinoCfg * cfg = NULL");
        return;
    }
    
    ack_activity_pet_casino_status ack;
    
    if(  ! cfg->opentoday  || _ntime.noMatch() )
    {
        ack.status = enPetCasinoStatus_NoMatchToday;
        return sendNetPacket(role->getSessionId(), &ack);
    }
    
    //如果比赛开始了，状态就得从比赛中取，否则在外部取
    if(_PetMatchMgr.isOpen())
        return getPetMatchMgr().reqStatus(role, req);
    else{
        for (int i = 0; i< cfg->times.size(); ++i)
        {
            const vector<int> & arr = cfg->times[i];
            if(_ntime.start != arr[0])
                continue;
            
            ack.matchtime.insert(ack.matchtime.end(), arr.begin(),arr.end());
            
            int curtime = time(0);
            for (int i = 0; i<ack.matchtime.size(); ++i) {
                ack.matchtime[i] -= curtime;
            }
            
            return sendNetPacket(role->getSessionId(), &ack);
        }
    }
    
}


static PetCasino _apc;

#pragma mark -
#pragma mark 幻兽大比拼接口


//心跳
void updatePetCasino(void* param)
{
    _apc.update();
}


void g_setNextTime(int valStartTime,int valEndTime){
    _apc.setNextTime(valStartTime,valEndTime);
    //int t=val_end_time-time(0);
    //addTimer(endMatchCb,NULL,t ,1);
}

void pet_casino_wager_compensate()
{
    _PetCasionWagerCompensateMgr.CompensateOrRecorver();
    addTimerDef(updatePetCasino,NULL, 5 , FOREVER_EXECUTE);
}

//以下为贺康补的功能代码

//Hiselementnum,pairVectMatchPetToMatchHistory,PetCosinoCfg,MatchTime
void logicFuntion(int roundNum,int HiselementNum,std::pair< vector<MatchPet>, MatchHistory> & pairVectMatchPetToMatchHistory ,tagActivityPetCosinoCfg  & PetCosinoCfg ,std::pair<size_t, size_t> MatchTime ,PetCasionWagerCompensateMgr & valPetCaMgr)
{
    //摆放参赛宠
    //_apc.getPetMatchMgr().save_initHistory();
    //如果是跨轮处理，则数据库中的历史记录会小于轮数
    if (HiselementNum<roundNum) {
        //补回缺失的轮记录
        //int j = roundNum - HiselementNum;
        
        matchMgrFuZhi(true,pairVectMatchPetToMatchHistory.first,pairVectMatchPetToMatchHistory.second,PetCosinoCfg,MatchTime.first);
        //先进行邮件补尝，后来发现这里不用邮件，因为会 进行摸似重新走之前的比赛
        /*
         //如果发生，进行邮件
         val_PetCaMgr.Compensate();
         */
        //再进行轮历史信息记录补缺
        //addTimer(endRoundCb,NULL, 1,j);
        //while (--j >=0) {
            _apc.endRoundEvent();
        //}
        
        //设置比赛结束时间
        int t =MatchTime.second-time(0);
        addTimer(endMatchCb,NULL,t ,1);
        
    }
    //如果还是down时的那一轮，进行恢复
    else if(HiselementNum==roundNum){
        matchMgrFuZhi(true,pairVectMatchPetToMatchHistory.first,pairVectMatchPetToMatchHistory.second,PetCosinoCfg,MatchTime.first);
        _apc.getPetMatchMgr().gm_setRoundTime();
        //设置比赛结束时间
        int t =MatchTime.second-time(0);
        addTimer(endMatchCb,NULL,t ,1);
    }
    //数据错误，可以是因为跨了好几场比赛
    else{
        //如果发生，进行邮件
        valPetCaMgr.Compensate();
        
        //        remTimer(startMatchCb);
        //        remTimer(endMatchCb);
        
        //addTimer(startMatchCb,NULL,1 ,1);
        //addTimer(endRoundCb,NULL, 1,i_round-1);
        _apc.startMatchEvent();
        //while (--i_round >0) {
            _apc.endRoundEvent();
        //}
        int t =MatchTime.second-time(0);
        addTimer(endMatchCb,NULL,t ,1);
    }
}


void finishLoadPaihangPetDataByRecorveOnly(int rid,int type,vector<SortPetInfo> data)
{
    vector<SortPetInfo>& v =  _PaihangPetData.data;
    v.clear();
    v.insert(v.end(),data.begin(),data.end());

    //将数据保存下来，如果大比拼活动down机后，数据库中没有运行时数据，就从这里拿
     std::pair<int, int> tmp_time= CalMatchStartAndEndTime(time(NULL));
     PetRecorveState j=CalRecorveState(time(NULL),tmp_time.first,tmp_time.second);
     if(j == NO_START  or j == READY_END){
         return ;
     }
    
    
    const tagActivityPetCosinoCfg* cfg = ActivityPetCosinoCfgMgr::getcfg();
    int round = cfg->times[0].size() - 2;   //扣去开始结束
    if(round < 1 )
        return ;
    int petAmount = pow(2,round);
    if (v.size() <petAmount) {
        
        // 这里进行数据补尝处理，从策划的配制数据中取出进行补尝
        vector<string>::const_iterator begPet = cfg->PetDataBak.begin();
        vector<string>::const_iterator endPet = cfg->PetDataBak.end();
        for (;begPet!= endPet && v.size() <petAmount;++begPet ) {
            StringTokenizer tokens(*begPet,":");
            SortPetInfo tmpPetInfo;
            tmpPetInfo.masterid = CONFIGUREPETOWNEID;
            tmpPetInfo.petmod = Utils::safe_atoi(tokens[0].c_str());
            tmpPetInfo.petid= v.size()+VIRTUALPETMATCHID;
            tmpPetInfo.battle= Utils::safe_atoi(tokens[1].c_str());
            tmpPetInfo.mastername = g_RandNameMgr.getRandName(true);
            tmpPetInfo.growth = CONFIGUREGROUTH;
            tmpPetInfo.rank = CONFIGURERANK;
            v.push_back(tmpPetInfo);
        }
        
        
    }
     // 这里的宠数据从哪里得到，排行榜中
     g_setNextTime(tmp_time.first, tmp_time.second);

    //remTimer(startMatchCb);
    //remTimer(endRoundCb);
    //remTimer(endMatchCb);


     //addTimer(startMatchCb,NULL,1 ,1);
    _apc.startMatchEvent();

     check_min(j, NO_ROUND_1);
     check_max(j, NO_END);
     int i_num_count = ((unsigned int)j) - 1;
    while  (--i_num_count > 0) {
         //addTimer(endRoundCb,NULL, 1,i_num_count);
         _apc.endRoundEvent();
     }
     int t =tmp_time.second-time(0);
     addTimer(endMatchCb,NULL,t ,1);
    //_apc.endMatchEvent();
    
}
std::pair<int, int> CalMatchStartAndEndTime(int t)
{
    tagActivityPetCosinoCfg _matchcfg = *(ActivityPetCosinoCfgMgr::getcfg());
    int startTime =0;
    int endTime =0;
    if (_matchcfg.times.size() <=0) {
        return std::make_pair<int,int>(MATCHERROR, MATCHERROR);
    }
    int lastindex = _matchcfg.times[0].size() -1;
    //在某一场比赛时间段里
    for (int i=_matchcfg.times.size()-1, endLoop = 0; i>=endLoop; --i) {
        lastindex = _matchcfg.times[i].size() -1;
        
        if( t >= _matchcfg.times[i][0] && t <= _matchcfg.times[i][lastindex])
        {
            startTime= _matchcfg.times[i][0];
            endTime = _matchcfg.times[i][lastindex];
            return std::make_pair<int,int>(startTime, endTime);
        }
    }
    //将返回错误码改成返回下一轮比赛时间
    //下面是处理两场比赛之间，设置为下场比赛时间（有两个特殊点，第一场比赛及最后一场比赛）
    for (int i=_matchcfg.times.size()-1, endLoop = 0; i>endLoop; --i) {
        lastindex = _matchcfg.times[i].size() -1;
        if( _matchcfg.times[i-1][lastindex] <= t && t <= _matchcfg.times[i][0] )
        {
            startTime= _matchcfg.times[i][0];
            endTime = _matchcfg.times[i][lastindex];
            return std::make_pair<int,int>(startTime, endTime);
        }
    }
    //如果还没有到当天第一场比赛，则设置为第一场比赛（第一个特殊点进行处理）
    if(t <= _matchcfg.times[0][0])
    {
        lastindex = _matchcfg.times[0].size() -1;
        return std::make_pair<int,int>(_matchcfg.times[0][0], _matchcfg.times[0][lastindex]);
    }
    return std::make_pair<int,int>(MATCHERROR, MATCHERROR);
}
PetRecorveState CalRecorveState(size_t t,size_t MatchStartTime ,size_t MatchEndTime)
{
    tagActivityPetCosinoCfg _matchcfg = *(ActivityPetCosinoCfgMgr::getcfg());
    int flag = 0;
    
    if (t > MatchEndTime) {
        return RECORVESTATEERROR;
    }
    
    int i , endLoop = 0;
    for (i=_matchcfg.times.size()-1; i>=endLoop; --i) {
        
        if( MatchStartTime == _matchcfg.times[i][0] )
        {
            break;
        }
    }
    if (i < 0) {
        return RECORVESTATEERROR;
    }
    
    int lastindex = _matchcfg.times[i].size() -1;
    for(int j=0;j!=lastindex;++j)
    {
        if (t <= _matchcfg.times[i][j]) {
            break;
        }
        else{
            ++flag;
        }
        
    }
    /*
     NO_START = 0,
     NO_ROUND_1,
     NO_ROUND_2,
     NO_ROUND_3,
     NO_END,
     READY_END,
     RECORVESTATEERROR = 999999999,
     */
    switch (flag) {
        case 0:
            return NO_START;
        case 1:
            return NO_ROUND_1;

        case 2:
            return NO_ROUND_2;

        case 3:
            return NO_ROUND_3;

        case 4:
            return NO_END;

        case 5: 
            return READY_END;
  
        default:
            return RECORVESTATEERROR;

    } 
    
}

class OrderBySize{
public:
    bool operator ()(vector<MatchPet> lval,vector<MatchPet> rval){
        if (lval.size() > rval.size()) {
            return true;
        }
        else{
            return false;
        }
    }
};

bool GetObjFromDB(std::pair< vector<MatchPet>, MatchHistory> & outVectMatchPetToMatchHistory){
     // 1 当数据库中没有数据时，开启最近的比赛
     // 2 当数据库中有数据时，进行数据恢复，数据包括下面三项：
     //     得到参赛宠，得到历史轮记录，将赌注信息添加到参赛宠及历史轮信息
     
     RedisResult resultKeys;
     resultKeys.setData(redisCmd("exists %s", CASION_ACTION_DB_KEY_PETID.c_str()));
     if (resultKeys.readInt() == 0){
         std::pair<int, int> MatchTime = CalMatchStartAndEndTime(time(NULL));
         if (MatchTime.first != MATCHERROR) {
             g_setNextTime(MatchTime.first,MatchTime.second);
             getSortPetData(0, 1, SYNC_PAIHANG_PET_AMOUNT, finishLoadPaihangPetDataByRecorveOnly);
         }
         return false;
     }
    resultKeys.setData(redisCmd("hkeys %s", CASION_ACTION_DB_KEY_PETID.c_str()));
    int elementnumKeys = resultKeys.getHashElements();
    vector<MatchPet>  vectPet;
    for (int i = 0; i < elementnumKeys; i++)
    {
        std::string petIDStr = resultKeys.readHash(i, "");
        
        int petID =Utils::safe_atoi(petIDStr.c_str());
        
        MatchPet valPet;
        MatchPetFactor::CreatorMatchPet(petID,valPet);
        vectPet.push_back(valPet);
    }
    // 恢复历史轮记录
    RedisResult HisResult(redisCmd("hvals %s", CASION_ACTION_DB_KEY_HISTORY.c_str()));
    int Hiselementnum = HisResult.getHashElements();
    MatchHistory valMatchHistory;
    for (int j=0;j<Hiselementnum;++j)
    {
        std::string roundStr=HisResult.readHash(j, "");
        StringTokenizer roundToken(roundStr,";");
        StringTokenizer::Iterator beg = roundToken.begin();
        StringTokenizer::Iterator end = roundToken.end();
        //Round 和 std::vector<MatchPet> 等值
        std::vector<MatchPet> vectMatchpet;
        for (;beg!=end;++beg){
            StringTokenizer petsToken(*beg,":");
            if (petsToken.count() <=1) {
                continue;
            }
            int petID = Utils::safe_atoi(petsToken[0].c_str());
            //bool b_win = (bool)Utils::safe_atoi(tmp2[1].c_str());
            MatchPet  valMatchPet;
            MatchPetFactor::CreatorMatchPet(petID,valMatchPet);
            vectMatchpet.push_back(valMatchPet);
        }
        if (vectMatchpet.size() > 0) {
            valMatchHistory.push_back(vectMatchpet);
        }
        
    }
     sort(valMatchHistory.begin(), valMatchHistory.end(),OrderBySize());
    // 将对于的参赛宠身上的赌注信息添加上去
    map<int , map <int,GamblerInfo> >   mapPetIDtoGamblerInfos;
    _PetCasionWagerCompensateMgr.GetAllInfoFromDB(mapPetIDtoGamblerInfos);
//    //给活动数据进行赋值
//    // rmb  to gold 
     map<int , map <int,GamblerInfo>  >::iterator mapPetIDtoGamblerInfosBegin=  mapPetIDtoGamblerInfos.begin();
     map<int , map <int,GamblerInfo>  >::iterator mapPetIDtoGamblerInfosEnd=  mapPetIDtoGamblerInfos.end();
     
    for (;mapPetIDtoGamblerInfosBegin!=mapPetIDtoGamblerInfosEnd;++mapPetIDtoGamblerInfosBegin)
    {
        vector<MatchPet>::iterator petBegin =  vectPet.begin();
        vector<MatchPet>::iterator petEnd =  vectPet.end();

        for (; petBegin!=petEnd; ++petBegin) {
            if (petBegin->id == mapPetIDtoGamblerInfosBegin->first) {
                petBegin->gmb =  mapPetIDtoGamblerInfosBegin->second;
            }
        }
        
        vector<MatchPet>::iterator MatchHistoryBegin =  valMatchHistory.rbegin()->begin();
        vector<MatchPet>::iterator MatchHistoryEnd =  valMatchHistory.rbegin()->end();
        for (; MatchHistoryBegin!=MatchHistoryEnd; ++MatchHistoryBegin) {
            if (MatchHistoryBegin->id == mapPetIDtoGamblerInfosBegin->first) {
                MatchHistoryBegin->gmb = mapPetIDtoGamblerInfosBegin->second;
            }
        }
        
    }
    
     outVectMatchPetToMatchHistory.first = vectPet;
     outVectMatchPetToMatchHistory.second = valMatchHistory;
     return true;
}

void matchMgrFuZhi(bool open,vector<MatchPet> & vectPets,MatchHistory & vectHistorys ,tagActivityPetCosinoCfg & valMatchcfg ,int valStartTime){
    _apc.getPetMatchMgr().setAllData(open,vectPets,vectHistorys,valMatchcfg,valStartTime);
}

//定时回调
void startMatchCb(void* param)
{
    Xylog log(eLogName_PetCasion_TraceBack,0);
    log<<"定时回调startMatchCb";
    _apc.startMatchEvent();
}

void endRoundCb(void* param)
{
    _apc.endRoundEvent();
}

void endMatchCb(void* param)
{
    Xylog log(eLogName_PetCasion_TraceBack,0);
    log<<"定时回调endMatchCb";
    _apc.endMatchEvent();
}

//处理网络请求
void on_req_activity_pet_casino_status(Role* role, req_activity_pet_casino_status& req)
{
    _apc.reqStatus(role, req);
}
void on_req_activity_pet_casino_wager(Role* role,req_activity_pet_casino_wager& req)
{
    _apc.getPetMatchMgr().reqWager(role, req);
}
void on_req_activity_pet_casino_thief(Role* role,req_activity_pet_casino_thief& req)
{
    _apc.getPetMatchMgr().reqThief(role, req);
}

void on_req_activity_pet_casino_current_history(Role* role,req_activity_pet_casino_current_history& req)
{
    _apc.getPetMatchMgr().reqCurrentHistory(role, req);
}


//#define TEST_BROADCAST_PET_MATCH_RESULT 1 //测试走马灯
#ifdef TEST_BROADCAST_PET_MATCH_RESULT

void testbcMatchPet(void* param)
{
    static int i = 1;
    ++i;
    
    //发送走马灯
    bcMatchRetParam bc;
    bc.round = i % 4 + 1;
    bc.winers = 865;
    bc.wowner = "主人";
    bc.wpetmod = 101111;
    bc.losers = 9999;
    bc.lowner = "主人2";
    bc.lpetmod = 101111;
    
    switch (bc.round) {
        case 1:
            broadcastPetMatchResult(kBroadcastPetMatchResultQuarter,bc);
            break;
        case 2:
            broadcastPetMatchResult(kBroadcastPetMatchResultHalf,bc);
            break;
        case 3:
            broadcastPetMatchResult(kBroadcastPetMatchResultFinal,bc);
            break;
    }
    
}

addTimerDef(testbcMatchPet, NULL, 10, FOREVER_EXECUTE);


void testbcMatchPetEyes(void* param)
{
    bcMatchPetDiscernEyeParam bc;
    int a = rand() % 4 +1;
    bc.name1 = "支持者1";
    if(a % 2 == 0)
        bc.name1.clear();
    
    bc.name2 = "支持者2";
    if(a % 3 == 0)
        bc.name2.clear();
    
    bc.name3 = "支持者3";
    if(a % 5 == 0)
        bc.name3.clear();
    
    bc.wowner = "主人名";
    bc.wpetmod = 101111;
    
    
    if(! bc.name1.empty() && ! bc.name2.empty() && ! bc.name3.empty())
        broadcastPetMatchDiscerningEyes(bc);
}

addTimerDef(testbcMatchPetEyes, NULL, 15, FOREVER_EXECUTE);

#endif //TEST_BROADCAST_PET_MATCH_RESULT



