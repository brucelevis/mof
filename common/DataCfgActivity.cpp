//
//  DataCfgActivity.cpp
//  GameSrv
//
//  Created by Huang Kunchao on 13-7-26.
//
//

#include "DataCfgActivity.h"
#include "Utils.h"
#include "DataCfg.h"
#include <list>
#include <algorithm>
#include <functional>
#include <stdint.h>
#include <limits.h>
#include "Utils.h"
#include "json/json.h"
using namespace std;
//////////////////////////////////////////////////
//ActivityInviteCfgMgr
//激活码配置
tagActivityInviteCfg*  ActivityInviteCfgMgr::_cfg = NULL;
std::string  ActivityInviteCfgMgr::_fullpath;

ActivityInviteCfgMgr::~ActivityInviteCfgMgr()
{
    SafeDelete(_cfg);
    _cfg = NULL;
}

void ActivityInviteCfgMgr::load(std::string fullpath)
{
    _fullpath = fullpath;
    tagActivityInviteCfg* cfg = readcfg();
    replacecfg(cfg);
}

void ActivityInviteCfgMgr::replacecfg(tagActivityInviteCfg* cfg)
{
    if(NULL == cfg){
        log_error("ActivityInviteCfgMgr::replacecfg ERROR");
        return;
    }
    SafeDelete(_cfg);
    _cfg = cfg;
}

tagActivityInviteCfg* ActivityInviteCfgMgr::readcfg()
{
    
    tagActivityInviteCfg* cfg = new tagActivityInviteCfg;
    if(cfg == NULL){
        log_error("ActivityInviteCfgMgr::readcfg new 内存不足");
        return NULL;
    }
    
    GameInifile inifile(_fullpath);
    //GameInifile MLIniFile(Cfgable::getFullFilePath("ActivityInviteCfgMultiLanguage.ini"));
    cfg->opentoday = true;
    if( "False" == inifile.getValue("Open", "Open")){
        cfg->opentoday = false;
        return cfg;
    }
    
//    string info_id = inifile.getValue("WordInfo", "Info_id");
//    cfg->mInfo = MLIniFile.getValue(info_id, "desc");//inifile.getValue("WordInfo", "Info");
   
    
    int matchnum = inifile.getValueT("Open", "InviteNum",0);
    if (matchnum >0) {
        string matchstr = Utils::makeStr("InviteNum%d",0);
        int needLvl = inifile.getValueT(matchstr, "needLvl",20);
        int needCount = inifile.getValueT(matchstr, "needCount", 5*0);
        string awardStr = inifile.getValue(matchstr,"items");
        int awardType= inifile.getValueT(matchstr,"awardType",1);
        cfg->CountToawardType.insert(map<int, int>::value_type(needLvl*10000+needCount,awardType));
        cfg->CountToLvl.insert(map<int, int>::value_type(needLvl*10000+needCount,needLvl));
        cfg->CountToAward.insert(map<int,string>::value_type(needLvl*10000+needCount,awardStr));
    }
    
    for (int matchi = 1; matchi <= matchnum; ++matchi) {
        
        string matchstr = Utils::makeStr("InviteNum%d",matchi);
        int needLvl = inifile.getValueT(matchstr, "needLvl",20);
        int needCount = inifile.getValueT(matchstr, "needCount", 5*matchi);
        string awardStr = inifile.getValue(matchstr,"items");
        if(needLvl<0 || needCount <0)
            continue;
        int awardType= inifile.getValueT(matchstr,"awardType",2);
        cfg->CountToawardType.insert(map<int, int>::value_type(needLvl*10000+needCount,awardType));
        cfg->CountToLvl.insert(map<int, int>::value_type(needLvl*10000+needCount,needLvl));
        cfg->CountToAward.insert(map<int,string>::value_type(needLvl*10000+needCount,awardStr));
    }
    return cfg;
    
}

const tagActivityInviteCfg* ActivityInviteCfgMgr::getcfg()
{
    return _cfg;
}
//////////////////////////////////////////////////
//幻兽大比拼活动配置

tagActivityPetCosinoCfg*  ActivityPetCosinoCfgMgr::_cfg = NULL;
std::string  ActivityPetCosinoCfgMgr::_fullpath;

ActivityPetCosinoCfgMgr::~ActivityPetCosinoCfgMgr()
{
    SafeDelete(_cfg);
    _cfg = NULL;
}

void ActivityPetCosinoCfgMgr::load(std::string fullpath)
{
    _fullpath = fullpath;
    tagActivityPetCosinoCfg* cfg = readcfg();
    replacecfg(cfg);
}

void ActivityPetCosinoCfgMgr::replacecfg(tagActivityPetCosinoCfg* cfg)
{
    if(NULL == cfg){
        log_error("ActivityPetCosinoCfg::replacecfg ERROR");
        return;
    }
    SafeDelete(_cfg);
    _cfg = cfg;
}

//今天使用配置
static std::string _TodayDynamicCofig(GameInifile& inifile)
{
    time_t nowtime = time(NULL);
    struct tm date;
    localtime_r(&nowtime, &date);

    /*
     使用 enWeek
     0 星期天
     1 星期一
     ...
     6 星期六
     */
    char week[8]={0};
    snprintf(week,sizeof(week), "%d",date.tm_wday);

    string  todayvalue = inifile.getValue("Week", week);
    //log_info("星期"<<week<<"\t使用"<<todayvalue);


    std::map<string,string> props;
    const char *section = "SpecialDate";
    vector<string> keys;
    inifile.getKeys(section, inserter(keys, keys.begin()));

    for (int k = 0; k < keys.size(); k++)
    {
        string value = inifile.getValue(section, keys[k]);
        props.insert(make_pair(keys[k], value));
    }


    for (std::map<string,string>::iterator iter= props.begin(); iter != props.end(); ++iter) {
        if( Utils::sameDate(iter->first.c_str()) ){
            todayvalue = iter->second;
            //log_info("特殊日子"<<iter->first<<"\t使用"<<todayvalue);
            break;
        }
    }

    return todayvalue;
}

//“8:30” 转成今天的时间
static time_t _TimeStrConvertToday(const char* str)
{

    time_t nowtime = time(NULL);
    struct tm date;
    localtime_r(&nowtime, &date);

    int hour = 0 , minute = 0;
    sscanf(str,"%d:%d",&hour,&minute);

    date.tm_hour = hour;
    date.tm_min = minute;
    date.tm_sec = 0;
    return mktime(&date);
}

struct _tagLevelRange
{
    int min;
    int max;
};

//检测等级区间是否重叠
static bool _OverlapLevel(const vector<string> & vec)
{
    vector<_tagLevelRange> ranges;

    for (int i = 0 , ni = vec.size(); i<ni; ++i) {
        int min = 0 , max = 0;
        sscanf(vec[i].c_str(), "%d-%d",&min,&max);
        _tagLevelRange r;
        r.min = min;
        r.max = max;
        ranges.push_back(r);
    }

    for (int i = 0,ni = ranges.size(); i<ni; ++i) {
        for (int j = i+1 ,nj = ranges.size(); j<nj; ++j) {
            const _tagLevelRange& a = ranges[i];
            const _tagLevelRange& b = ranges[j];

            if(b.min <= a.min && a.min <= b.max)
                return true;
            if(b.min <= a.max && a.max <= b.max)
                return true;
        }
    }
    return false;
}

static bool _OverlapLevel(const std::set<string> & lvlset)
{
    vector<string> vec;
    for (std::set<string>::const_iterator iter=lvlset.begin(); iter != lvlset.end(); ++iter) {
        vec.push_back(*iter);
    }
    return _OverlapLevel(vec);
}


ActivityPetCosinoWagerLevel* tagActivityPetCosinoCfg::getWager(int lvl)
{
    for (WagerMap::iterator iter = wager.begin(); iter != wager.end(); ++iter) {
        int min = 0,max = 0;
        sscanf(iter->first.c_str(), "%d-%d",&min,&max);
        if(min <= lvl && lvl <= max)
            return &(iter->second);
    }
    return NULL;
}

std::string tagActivityPetCosinoCfg::getInfo()
{
    return mInfo;
}



string tagActivityPetCosinoCfg::dump()
{
    stringstream ss;
    ss << "幻兽大比拼配置:";
    ss<< "今天有"<< times.size() <<"比赛,";

    for (int i = 0; i<times.size(); ++i) {

        time_t nowtime = times[i][0];
        struct tm date;
        localtime_r(&nowtime, &date);
        ss<<"第"<<i+1<<"场 " << date.tm_hour <<":"<< date.tm_min <<",";
    }

//    for (WagerMap::iterator iter= wager.begin(); iter != wager.end(); ++iter) {
//        ss<<"\t"<<iter->first<<":["<<iter->second.gold.size();
//        ss<<" "<<iter->second.rmb.size();
//        ss<<" "<<iter->second.thief.size()<<"]";
//    }

    return ss.str();
}





tagActivityPetCosinoCfg* ActivityPetCosinoCfgMgr::readcfg()
{
    tagActivityPetCosinoCfg* cfg = new tagActivityPetCosinoCfg;
    if(cfg == NULL){
        log_error("ActivityPetCosinoCfg::readcfg new 内存不足");
        return NULL;
    }

    GameInifile inifile(_fullpath);
    //--lihengjin
    GameInifile MLIniFile;
    if(!(Cfgable::getFullFilePath("ActivityPetCasinoMultiLanguage.ini") == "ActivityPetCasinoMultiLanguage.ini"))
    {
        MLIniFile = GameInifile(Cfgable::getFullFilePath("ActivityPetCasinoMultiLanguage.ini"));
    }   
    cfg->opentoday = true;
    if( "False" == inifile.getValue("Open", "Open")){
        cfg->opentoday = false;
        return cfg;
    }

    string todayval = _TodayDynamicCofig(inifile);
    if( "False" == todayval){
        cfg->opentoday = false;
        return cfg;
    }

    string info_id = inifile.getValue("WordInfo", "Info_id");
    //by lihengjin
    std::string str_info;
    if(inifile.exists("WordInfo", "Info"))
    {
        str_info = inifile.getValue("WordInfo", "Info");
    }else
    {
        str_info = MLIniFile.getValue(info_id, "desc");
    }
    cfg->mInfo = str_info;//inifile.getValue("WordInfo", "Info");
    cfg->mAwardOwnerGold = inifile.getValueT("Petowners", "rewards",0);

    int matchnum = inifile.getValueT("Open", "MatchNum",0);

    for (int matchi = 1; matchi <= matchnum; ++matchi) {

        string matchstr = Utils::makeStr("Match%d",matchi);
        string timestr = inifile.getValue(matchstr, "Time");
        if(timestr.empty())
            continue;

        vector<int> vec;

        StringTokenizer tokens(timestr,";");
        for (StringTokenizer::Iterator it = tokens.begin(); it != tokens.end(); ++it) {
            vec.push_back(_TimeStrConvertToday(it->c_str()));
        }

        for (int i = 0; i< vec.size()-1; ++i) {
            if(vec[i] > vec[i+1])
            {
                log_error(timestr);
                log_error("ActivityPetCosinoCfg::readcfg 时间配错");
                vec.clear();
                break;
            }
        }
        if( ! vec.empty())
            cfg->times.push_back(vec);
    }

    //all level string
    vector<string> keys;
    inifile.getKeys("WagerGold", inserter(keys, keys.begin()));
    if( _OverlapLevel(keys) )
    {
        log_error("ActivityPetCosinoCfg::readcfg 等级重叠");
        return cfg;
    }

    for (vector<string>::iterator iter = keys.begin(); iter != keys.end(); ++iter) {

        ActivityPetCosinoWagerLevel wagerlvl;

        string str = inifile.getValue("WagerGold", *iter);
        do{
            StringTokenizer token(str,";");
            for(int i =0;i< token.count();i++){
                wagerlvl.gold.push_back(atoi(token[i].c_str()));
            }
        }while(0);

        str = inifile.getValue("WagerRmb", *iter);
        do{
            StringTokenizer token(str,";");
            for(int i =0;i< token.count();i++){
                wagerlvl.rmb.push_back(atoi(token[i].c_str()));
            }
        }while(0);

        str = inifile.getValue("WagerThief", *iter);
        do{
            StringTokenizer token(str,";");
            for(int i =0;i< token.count();i++){
                wagerlvl.thief.push_back(atoi(token[i].c_str()));
            }
        }while(0);

        if( wagerlvl.gold.size() == wagerlvl.rmb.size() &&
           wagerlvl.gold.size() == wagerlvl.thief.size()){
            cfg->wager[*iter] = wagerlvl;
        }else{
            log_error("ActivityPetCosinoCfg::readcfg 配置出错");
        }


    }
    //PetDataBak
    int NumPetDataBak = inifile.getValueT("Open", "NumPetDataBak",0);
    string matchstr("PetDataBak");
    for (int i=1; i<=NumPetDataBak; ++i) {
        string numStr = Utils::makeStr("%d",i);
        string PetInfo = inifile.getValue(matchstr, numStr);
        if (PetInfo.empty()) {
            continue;
        }
        cfg->PetDataBak.push_back(PetInfo);
    }
     //log_info("幻兽大比拼今天有"<<cfg->times.size()<<"场");
    return cfg;
}


const tagActivityPetCosinoCfg* ActivityPetCosinoCfgMgr::getcfg()
{
    return _cfg;
}




#pragma mark -
#pragma mark 抽奖配置

LotItemVec* LotteryCfg::get(int userlvl)
{
    for (LvlMap::iterator iter = _lvlmap.begin(); iter != _lvlmap.end(); ++iter) {
        int min=0,max= 0;
        sscanf(iter->first.c_str(), "%d-%d",&min,&max);
        if(min<=userlvl && userlvl <=max)
            return &(iter->second);
    }
    return NULL;
}
string LotteryCfg::dump()
{
    stringstream ss;
    ss << "抽奖配置:";

    for (LvlMap::iterator it = _lvlmap.begin(); it != _lvlmap.end(); ++it) {
        ss << it->first <<":";
        ss << it->second.size()<<"件;";
    }
    return ss.str();
}

void LotteryCfg::insert(const LotteryItem& item)
{
    LvlMap::iterator iter = _lvlmap.find(item.lvlstr);
    if (iter == _lvlmap.end()) {
        _lvlmap[item.lvlstr] = LotItemVec();
        iter = _lvlmap.find(item.lvlstr);
    }
    iter->second.push_back(item);
}

LotteryCfg*   LotteryCfgMgr::_cfg = NULL;
std::string   LotteryCfgMgr::_fullpath;

LotteryCfgMgr::~LotteryCfgMgr()
{
    SafeDelete(_cfg);
    _cfg= NULL;
}

void LotteryCfgMgr::load(string fullpath)
{
    _fullpath = fullpath;
    replace(read());
}

LotteryCfg* LotteryCfgMgr::read()
{
    try{
        GameInifile inifile(_fullpath);
        string filename = _TodayDynamicCofig(inifile);
        string path = Cfgable::getFullFilePath(filename);
        return readTodayCfg(path.c_str());
    }catch(...){
        log_error("LotteryCfg ERROR !!!");
    }
    return NULL;
}

void LotteryCfgMgr::replace(LotteryCfg* cfg)
{
    if(NULL == cfg){
        log_error("LotteryCfgMgr::replacecfg ERROR");
        return;
    }
    SafeDelete(_cfg);
    _cfg = cfg;
}

const LotItemVec* LotteryCfgMgr::getItem(int userlvl)
{
    if(NULL == _cfg)
        return NULL;
    return _cfg->get(userlvl);
}

int lotteryItemTypeConvert(const char* name)
{
    static map<string,int> _map;
    if(_map.empty()){
        _map["constell"]= enLotteryItemType_Constell;
        _map["fat"] = enLotteryItemType_Fat;
        _map["pet"]= enLotteryItemType_PetEgg;
        _map["exp"]= enLotteryItemType_Exp;
        _map["gold"]= enLotteryItemType_Gold;
        _map["batpoint"]=enLotteryItemType_BattlePoint;
        _map["propstone"]=enLotteryItemType_Stone;
    }

    map<string,int>::iterator iter = _map.find(name);
    if(iter != _map.end())
        return iter->second;
    else
        return enLotteryItemType_XX;
}


LotteryCfg* LotteryCfgMgr::readTodayCfg(const char* todayfullpath)
{
    LotteryCfg* cfg = new LotteryCfg;
    assert(cfg);
    
    for (int freeType = 0; freeType< 2; ++freeType)
    {
        string path = todayfullpath;
        if(eLotteryItemNotFree == freeType )
            path += ".ini";
        
        if(eLotteryItemFree == freeType )
            path += "_free.ini";
        
        GameInifile inifile(path);
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));

        //log_info("抽奖表物品数量"<<sections.size());

        std::set<string> lvlset;
        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++ )
        {
            string lvlstr = inifile.getValue(*iter, "lvl");
            lvlset.insert(lvlstr);
        }

        if( _OverlapLevel(lvlset)){
            log_error("抽奖表配置表等级重叠");
            SafeDelete(cfg);
            return NULL;
        }


        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++ )
        {
            LotteryItem li;
            li.id = inifile.getValueT(*iter, "id",0);
            li.name = inifile.getValue(*iter, "name");
            
            li.type = inifile.getValueT(*iter, "type", 0);
            if (li.type > 7)
            {
                li.type = 7;
            }
            
            li.prob = inifile.getValueT(*iter, "prop",0.0f);
            li.lvlstr = inifile.getValue(*iter, "lvl");
            li.freeType = freeType;
            li.broadcast = inifile.getValueT(*iter, "broadcast",0);

            if( 0 != li.id ){
                cfg->insert(li);
            }else{
                log_error("LotteryCfg 是否有错，有id为0的道具?");
            }
        }


    }

    return cfg;
}


//////////////////////////////////////////////////
#pragma mark - 色子抽奖


DiceGrids DiceLotteryCfgMgr::mGrids;
DiceItems DiceLotteryCfgMgr::mItems;
int DiceLotteryCfgMgr::mFreeTime;
DiceGrid DiceLotteryCfgMgr::mDgDump;

void DiceLotteryCfgMgr::load(const string& gridpath, const string& itempath)
{
    loadGrid(gridpath.c_str());
    loadItem(itempath.c_str());
}

void DiceLotteryCfgMgr::loadGrid(const char* gridpath)
{
    assert(gridpath);
    mGrids.clear();
	// 用于起点
	mDgDump.gridid = -1;
	mDgDump.isStart = true;
	mDgDump.prop = 0;
    
    GameInifile inifile(gridpath);
    //by lihengjin
    GameInifile MLIniFile;
    if(!(Cfgable::getFullFilePath("dicelotteryMultiLanguage.ini") == "dicelotteryMultiLanguage.ini"))
    {
        MLIniFile = GameInifile(Cfgable::getFullFilePath("dicelotteryMultiLanguage.ini"));
    }
    mFreeTime = inifile.getValueT("global", "freetime", 0 );
	int count = inifile.getValueT("global", "indexcount", 0 );

    for (int i = 1; i <= count; ++i)
    {
		DiceGrid tmp;
        tmp.isStart = false;
        string key = xystr("%d", i);
        tmp.gridid = inifile.getValueT(key, "gridid", 0 );
        tmp.icon = inifile.getValue(key, "icon");
        tmp.prop = inifile.getValueT(key, "prop", 0);
		string lvl = inifile.getValue(key, "lvl");
		
		assert(tmp.gridid > 0);

        sscanf(lvl.c_str(), "%d-%d", &tmp.lvlmin, &tmp.lvlmax);
		assert(tmp.lvlmin > 0 && tmp.lvlmax > 0);
        
		// 区间有交集的,　保证id不重复
        for (DiceGrids::iterator it = mGrids.begin(); it != mGrids.end(); ++it)
        {
			if (it->lvlmin > tmp.lvlmax || tmp.lvlmin > it->lvlmax)
			{
				continue;
			}
			assert(it->gridid != tmp.gridid);
        }
        mGrids.push_back(tmp);
    }
	// 验证总数
    assert(mGrids.size() == count);
}

void DiceLotteryCfgMgr::loadItem(const char* itempath)
{
#if defined(CLIENT)
    return;
#endif
    assert(itempath);
    
    mItems.clear();
    for (DiceGrids::iterator it = mGrids.begin(); it != mGrids.end(); ++it)
    {
        mItems.insert(make_pair(it->gridid, DiceGridItems() ));
    }
    
    GameInifile inifile(itempath);
    //by lihengjin
    GameInifile MLIniFile;
    if(!(Cfgable::getFullFilePath("diceitemMultiLanguage.ini") == "diceitemMultiLanguage.ini"))
    {
        MLIniFile = GameInifile(Cfgable::getFullFilePath("diceitemMultiLanguage.ini"));
    }
    std::list<std::string> sections;
    inifile.getSections(std::back_inserter(sections));
    
    DiceItem tmp;
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++ )
    {
        tmp.index = inifile.getValueT(*iter, "index", 0 );
        tmp.item = inifile.getValueT(*iter, "id", 0 );
        tmp.amount = inifile.getValueT(*iter, "num", 0 );
        tmp.gridid = inifile.getValueT(*iter, "gridid", 0 );
        tmp.prop = inifile.getValueT(*iter, "prop", 0);
        
        // MultiLanguage adapt
        std::string nameID = inifile.getValue(*iter, "name_id");
        //by lihengjin
        std::string nameStr;
        if(inifile.exists(*iter, "name"))
        {
            nameStr = inifile.getValue(*iter, "name");
        }else
        {
            nameStr = MLIniFile.getValue(nameID, "desc");
        }
        //std::string nameStr = MLIniFile.getValue(nameID, "desc");
        tmp.name = nameStr;//inifile.getValue(*iter, "name");
        tmp.broadcast = inifile.getValueT(*iter, "broadcast",0);
        tmp.type = lotteryItemTypeConvert(inifile.getValue(*iter,"type").c_str());
        
        assert( tmp.item > 0 && tmp.amount > 0 );
		
		string lvl = inifile.getValue(*iter, "lvl");
        sscanf(lvl.c_str(), "%d-%d", &tmp.lvlmin, &tmp.lvlmax);
		assert(tmp.lvlmin > 0 && tmp.lvlmax > 0);
        
        DiceItems::iterator itGrid = mItems.find( tmp.gridid );
        if( itGrid == mItems.end() )
        {
            log_error("色子物品出错，忽略"<< tmp.item <<"\t"<<tmp.gridid );
            continue;
        }
        itGrid->second.push_back(tmp);
    }
    
    
}

int DiceLotteryCfgMgr::needRmg(int lotterytime , int& costType)
{
    //if(lotterytime < mFreeTime )
        //return 0;
    
    int buytime = lotterytime - mFreeTime;
    
    return BuyCfg::getCost(eDiceLotteryBuy,buytime, costType);
}

const DiceGrid& DiceLotteryCfgMgr::getGrid(int position, int lvl)
{
	vector<DiceGrid*> vec;
	getGrid(lvl, vec);
	
    assert(vec.size() > 0);
    return *vec[position % vec.size()];
}

void DiceLotteryCfgMgr::getGrid(int lvl, vector<DiceGrid*> &vec)
{
	vec.push_back(&mDgDump);
	for (DiceGrids::iterator it = mGrids.begin(); it != mGrids.end(); ++it)
    {
		if (lvl < it->lvlmin || lvl > it->lvlmax) {
			continue;
		}
		vec.push_back(&(*it));
    }
}

const DiceGridItems& DiceLotteryCfgMgr::getGridItems(int grid)
{
    DiceItems::iterator it = mItems.find(grid);
    assert( it != mItems.end() );
    return it->second;
}


#pragma mark - 称号系统配置



void HonorCfg::read(GameInifile &inifile)
{
	mHonorMap.clear();

	std::map<string, eHonorKey> keymap;
	keymap["eHonorKey_Gold"] = eHonorKey_Gold;
	keymap["eHonorKey_Kill"] = eHonorKey_Kill;
	keymap["eHonorKey_Lvl"] = eHonorKey_Lvl;
	keymap["eHonorKey_EquipQua"] = eHonorKey_EquipQua;
	keymap["eHonorKey_EquipLvl"] = eHonorKey_EquipLvl;
	keymap["eHonorKey_EquipStone"] = eHonorKey_EquipStone;
	keymap["eHonorKey_Cons"] = eHonorKey_Cons;
	keymap["eHonorKey_Power"] = eHonorKey_Power;
	keymap["eHonorKey_Copy"] = eHonorKey_Copy;
	keymap["eHonorKey_Ill"] = eHonorKey_Ill;
	keymap["eHonorKey_Constrib"] = eHonorKey_Constrib;
	keymap["eHonorKey_Exploit"] = eHonorKey_Exploit;
	keymap["eHonorKey_Prestige"] = eHonorKey_Prestige;
	keymap["eHonorKey_BossRankOne"] = eHonorKey_BossRankOne;
	keymap["eHonorKey_BossLast"] = eHonorKey_BossLast;
	keymap["eHonorKey_PvpKill"] = eHonorKey_PvpKill;
	keymap["eHonorKey_PvpRankOne"] = eHonorKey_PvpRankOne;
	keymap["eHonorKey_Gambler"] = eHonorKey_Gambler;
	keymap["eHonorKey_PetPvpRankOne"] = eHonorKey_PetPvpRankOne;
	keymap["eHonorKey_PetPvpKill"] = eHonorKey_PetPvpKill;
	keymap["eHonorKey_UseItem"] = eHonorKey_UseItem;
	keymap["eHonorKey_GuildMasterTreasurecopy"] = eHonorKey_GuildMasterTreasurecopy;
	keymap["eHonorKey_GuildTreasurecopy"] = eHonorKey_GuildTreasurecopy;
	keymap["eHonorKey_SynPvpRank"] = eHonorKey_SynPvpRank;
	
	
	std::map<string, eHonorApp> appmap;
	appmap["eHonorApp_WorldTalk"] = eHonorApp_WorldTalk;
	appmap["eHonorApp_OnlineTips"] = eHonorApp_OnlineTips;
	
	try
    {
        //by lihengjin
        GameInifile MLIniFile;
        if(!(Cfgable::getFullFilePath("honorMultiLanguage.ini") == "honorMultiLanguage.ini"))
        {
            MLIniFile  = GameInifile(Cfgable::getFullFilePath("honorMultiLanguage.ini"));
        }
		std::list<string> sections;
        inifile.getSections(std::back_inserter(sections));
        //由于多语言表删除，所以这两个数值得放到honor.ini里。--lihengjin
        string typeId = "";
        string typeName = "";
        if(inifile.exists("50000", "typeid") && inifile.exists("50000", "typename"))
        {
            typeId = inifile.getValue("50000", "typeid");
            typeName = inifile.getValue("50000", "typename");//
        }else
        {
            typeId = MLIniFile.getValue("50000", "typeid");
            typeName = MLIniFile.getValue("50000", "typename");//
        }
        StringTokenizer tokens1(typeName, ";");
        StringTokenizer tokens2(typeId, ";");
        for(int i = 0; i < tokens1.count(); i++)
        {
            mTypeMap.insert(std::make_pair(tokens1[i], (eHonorType)atoi(tokens2[i].c_str())));
        }
        
        for (std::list<std::string>::iterator it = sections.begin(); it != sections.end(); it++)
        {
            if (*it == "50000") {
                continue;
            }
			HonorDef *def = new HonorDef();
			def->id = inifile.getValueT(*it, "id", 0);
//            string typeId = inifile.getValue(*it, "type_id");  --未用到--lihengjin
//            string typeStr = MLIniFile.getValue(typeId, "desc");
//            if (mTypeMap.find(typeStr) != mTypeMap.end()) {
//                def->type = mTypeMap[typeStr];//mTypeMap[inifile.getValue(*it, "type")];
//
//            }
            def->type = (eHonorType)Utils::safe_atoi(inifile.getValue(*it, "type").c_str());
			// assert(def->type != eHonorType_Non);

            string nameId = inifile.getValue(*it, "name_id");
            //by lihengjin
            std::string str_name;
            if(inifile.exists(*it, "name"))
            {
                str_name = inifile.getValue(*it, "name");
            }else
            {
                str_name = MLIniFile.getValue(nameId, "desc");
            }
			def->name = str_name;//inifile.getValue(*it, "name");
			def->needNum = inifile.getValueT(*it, "needNum", 0);
			def->keyName = inifile.getValue(*it, "key");
			def->key = keymap[def->keyName];
			
			// 应用功能加载
			string appStr = inifile.getValue(*it, "app");
			StringTokenizer appToken(appStr, ";");
			for (int i = 0; i < appToken.count(); i++) {
				def->appSet.insert(appmap[appToken[i]]);
			}
            def->order = inifile.getValueT( *it, "order", 0 );
			
			// 条件加载
			def->cond = inifile.getValue(*it, "cond");
			StringTokenizer token(def->cond, ";");
			for (int i = 0; i < token.count(); i++) {
				def->condSet.insert(token[i]);
			}

			def->icon = inifile.getValue(*it, "icon");
            vector<int> mResourcesIds;
            strToNumVec(inifile.getValue(*it, "resourcesId").c_str(),";",mResourcesIds);
            def->resourcesIds = mResourcesIds;
			def->needNum = inifile.getValueT(*it, "needNum", 0);
			
			def->effectiveTime = inifile.getValueT(*it, "effectiveTime", 0);

			def->setAddMaxFat(inifile.getValueT(*it, "addMaxFat", 0));
			def->setAtk(inifile.getValueT(*it, "atk", 0));
            def->setDef(inifile.getValueT(*it, "def", 0));
            def->setAddMaxHp(inifile.getValueT(*it, "addMaxHp", 0));
            def->setHit(inifile.getValueT(*it, "hit", 0.0));
            def->setDodge(inifile.getValueT(*it, "dodge", 0.0));
            def->setCri(inifile.getValueT(*it, "cri", 0.0));
            def->setStre(inifile.getValueT(*it, "stre", 0));
			def->setInte(inifile.getValueT(*it, "inte", 0));
			def->setPhys(inifile.getValueT(*it, "phys", 0));
			def->setCapa(inifile.getValueT(*it, "capa", 0));

            string descId = inifile.getValue(*it, "desc_id");
            //by lihengjin
            std::string str_descId;
            if(inifile.exists(*it, "desc"))
            {
                str_descId = inifile.getValue(*it, "desc");
            }else
            {
                str_descId = MLIniFile.getValue(descId, "desc");
            }
			def->desc = str_descId;//inifile.getValue(*it, "desc");
            def->show = inifile.getValueT(*it, "show", 0);
			if (def->getAddMaxFat() != 0
				|| def->getAtk() != 0
				|| def->getDef() != 0
				|| def->getAddMaxHp() != 0
				|| def->getHit() != 0
				|| def->getDodge() != 0
				|| def->getCri() != 0
				|| def->getStre() != 0
				|| def->getInte() != 0
				|| def->getPhys() != 0
				|| def->getCapa() != 0){
				def->isAtt = true;
			} else {
				def->isAtt = false;
			}

			mHonorMap.insert(make_pair(def->id, def));
		}
	}
    catch(...)
    {
        log_error("HonorCfg ERROR !!!");
    }
}

HonorDef & HonorCfg::getCfg(int id)
{
	HonorTd::iterator it = mHonorMap.find(id);
	if (it == mHonorMap.end()) {
		return mDumb;
	}
	return *it->second;
}

void HonorCfg::getCfg(string type, vector<HonorDef*> &vec)
{
	eHonorType typeId = mTypeMap[type];
	getCfg(typeId, vec);
}

void HonorCfg::getCfg(eHonorType type, vector<HonorDef*> &vec)
{
	HonorTd::iterator it;
	for (it = mHonorMap.begin(); it != mHonorMap.end(); it++) {
		if (it->second->type == type) {
			vec.push_back(it->second);
		}
	}
}

void HonorCfg::getCfg(eHonorKey key, string &cond, vector<HonorDef*> &vec)
{
	HonorTd::iterator it;
	for (it = mHonorMap.begin(); it != mHonorMap.end(); it++) {
		HonorDef *def = it->second;
		if (def->key != key) {
			continue;
		}

		if (!cond.empty() && def->condSet.find(cond) == def->condSet.end()) {
			continue;
		}

		vec.push_back(def);
	}
}

void HonorCfg::getCfg(eHonorKey key, int num, vector<HonorDef*> &vec)
{
	HonorTd::iterator it;
	for (it = mHonorMap.begin(); it != mHonorMap.end(); it++) {
		if (it->second->key == key && it->second->needNum <= num) {
			vec.push_back(it->second);
		}
	}
}

void HonorCfg::getCfg(eHonorApp key, vector<HonorDef*> &vec)
{
	HonorTd::iterator it;
	for (it = mHonorMap.begin(); it != mHonorMap.end(); it++) {
		if (it->second->appSet.find(key) != it->second->appSet.end()) {
			vec.push_back(it->second);
		}
	}
}

void HonorCfg::getTypeCfg(vector<string> &vec)
{
	std::map<string, eHonorType>::iterator it;
    std::map<eHonorType, string> mIdTypeMap;
    mIdTypeMap.clear();
	for (it = mTypeMap.begin(); it != mTypeMap.end(); it++) {
		//vec.push_back(it->first);
        mIdTypeMap.insert(make_pair(it->second, it->first));
	}
    for (std::map<eHonorType, string>::iterator itrFind = mIdTypeMap.begin(); itrFind != mIdTypeMap.end(); ++itrFind) {
        vec.push_back(itrFind->second);
    }
}

string HonorCfg::getTypeName(eHonorType id)
{
    std::map<string, eHonorType>::iterator itrFind = mTypeMap.begin();
    for (; itrFind != mTypeMap.end(); ++itrFind) {
        if (itrFind->second == id) {
            return itrFind->first;
        }
    }
    return "";
}

HonorCfg HonorCfgMgr::sTitleCfg;
void HonorCfgMgr::load()
{
	try
    {
		{
			GameInifile inifile(Cfgable::getFullFilePath("honor.ini"));
			sTitleCfg.read(inifile);
		}
    }
    catch(...)
    {
        log_error("HonorMgr ERROR !!!");
    }
}

#pragma mark - 公会系统配置

void GuildBasicCfg::read(GameInifile &inifile)
{
	try
    {
		mCreateDef.needLvl = inifile.getValueT("create_society", "need_lvl", 0);
		mCreateDef.needGlod = inifile.getValueT("create_society", "need_gold", 0);
        
        mImpeachTime = inifile.getValueT("golbal", "society_impeach_time", 14);
        mImpeachCost = inifile.getValueT("golbal", "society_impeach_cost", 50);
        mWaitApplySecond = inifile.getValueT("golbal", "wait_for_apply_second", 14);

        mMaxApplicant = inifile.getValueT("golbal", "max_applied_number", 500);
        mDonateRmbVipLvl = inifile.getValueT("golbal", "donatermb_vip", 0);

        int lvlMax = inifile.getValueT("golbal", "societymaxlvl", 0);
		assert(lvlMax > 0);

		for (int lvl=1; lvl <= lvlMax; lvl++) {
			char tmp[64] = {0};
			sprintf(tmp, "%d", lvl);

			GuildLvlDef *def = new GuildLvlDef();
			def->lvl = inifile.getValueT(tmp, "society_lvl", 0);
			def->memberNum = inifile.getValueT(tmp, "maxmember", 0);
			def->needWealth = inifile.getValueT(tmp, "need_wealth", 0);
			def->needOverbuild = inifile.getValueT(tmp, "need_overbuild", 0);
			def->donateGoldBatNum = inifile.getValueT(tmp, "DonateGoldBatNum", 0);
			def->donateRmbNum = inifile.getValueT(tmp, "DonateRmbNum", 0);

			//string str = inifile.getValue(tmp, "kGuildViceMaster");
			//StringTokenizer token(str, ";");
			//assert(token.count() >= 2);

			def->deputyChairmanNum = inifile.getValueT(tmp, "kGuildViceMaster", 0);
			def->eliteNum = inifile.getValueT(tmp, "kGuildElite", 0);
			def->heraldry = inifile.getValueT(tmp, "heraldry", 0);
            def->maxbosslvl = inifile.getValueT(tmp, "maxbosslvl", 0);
            def->trainLvl = inifile.getValueT(tmp, "max_train_lvl", 1);
			mGuildLvlMap.insert(make_pair(def->lvl, def));
		}
		assert(mGuildLvlMap.size() > 0);
    }
    catch(...)
    {
        log_error("GuildBasicCfg ERROR !!!");
    }
}

int GuildBasicCfg::getImpeachTime() const
{
    return mImpeachTime;
}

int GuildBasicCfg::getImpeachCost() const
{
    return mImpeachCost;
}

int GuildBasicCfg::getMaxApplicant() const
{
    return mMaxApplicant;
}
int GuildBasicCfg::getWaitApplySecond() const
{
    return mWaitApplySecond;
}
int GuildBasicCfg::getDonateRmbVipLvl() const
{
    return mDonateRmbVipLvl;
}
GuildLvlDef * GuildBasicCfg::getCfg(float wealth, int overbuild)
{
	GuildLvlTd::reverse_iterator rit;
	for (rit = mGuildLvlMap.rbegin(); rit != mGuildLvlMap.rend(); ++rit)
	{
		if (ceil(wealth) >= rit->second->needWealth && overbuild >= rit->second->needOverbuild) {
			return rit->second;
		}
	}
	//xyerr("[公会][系统配置] 计算公会等级数据有误 wealth:%d overbuild:%d", wealth, overbuild);
	return mGuildLvlMap.begin()->second;
}

GuildLvlDef * GuildBasicCfg::getCfg(int guildLvl)
{
	GuildLvlTd::iterator it = mGuildLvlMap.find(guildLvl);
	if (it != mGuildLvlMap.end()) {
		return it->second;
	}
	return NULL;
}

void GuildPresentCfg::read(GameInifile &inifile)
{
	try
    {
		mGuildPresentRmb.rmbaddwealth = inifile.getValueT("present", "Rmbaddwealth", 0);
		mGuildPresentRmb.rmbaddcontribute = inifile.getValueT("present", "Rmbaddcontribute", 0);
		mGuildPresentRmb.rmbaddfeats = inifile.getValueT("present", "Rmbaddfeats", 0);
		mGuildPresentRmb.rmbaddoverbuild = inifile.getValueT("present", "Rmbaddoverbuild", 0);
		mGuildPresentRmb.donateRmb = inifile.getValueT("present", "DonateRmb", 0);

		StringTokenizer DonateGoldToken(inifile.getValue("present", "DonateGold"), ";");
		//StringTokenizer DonateBatToken(inifile.getValue("present", "DonateBat"), ";");
        StringTokenizer DonateRmbSelfToken(inifile.getValue("present", "DonateRmbSelf"), ";");
        
		StringTokenizer addwealthToken(inifile.getValue("present", "addwealth"), ";");
		StringTokenizer addcontributeToken(inifile.getValue("present", "addcontribute"), ";");
		StringTokenizer addfeatsToken(inifile.getValue("present", "addfeats"), ";");
		StringTokenizer addoverbuildToken(inifile.getValue("present", "addoverbuild"), ";");
        StringTokenizer GoldaddcontributeToken(inifile.getValue("present", "Goldaddcontribute"), ";");
        StringTokenizer GoldaddfeatsToken(inifile.getValue("present", "Goldaddfeats"), ";");

		int size = DonateGoldToken.count();
		//assert(DonateBatToken.count() == size);
        assert(DonateRmbSelfToken.count() == size);
        
		assert(addwealthToken.count() == size);
		assert(addcontributeToken.count() == size);
		assert(addfeatsToken.count() == size);
		assert(addoverbuildToken.count() == size);

		for (int i = 0; i < size; i++) {
			GuildPresentDef *def = new GuildPresentDef();
			def->donateGold = Utils::safe_atoi(DonateGoldToken[i].c_str());
			//def->donateBat = Utils::safe_atoi(DonateBatToken[i].c_str());
            def->donateRmbSelf = Utils::safe_atoi(DonateRmbSelfToken[i].c_str());

			def->addwealth = Utils::safe_atoi(addwealthToken[i].c_str());
			def->addcontribute = Utils::safe_atoi(addcontributeToken[i].c_str());
			def->addfeats = Utils::safe_atoi(addfeatsToken[i].c_str());
			def->addoverbuild = Utils::safe_atoi(addoverbuildToken[i].c_str());
            def->goldaddcontribute = Utils::safe_atoi(GoldaddcontributeToken[i].c_str());
            def->goldaddfeats = Utils::safe_atoi(GoldaddfeatsToken[i].c_str());

			mGuildPresentMap.insert(make_pair(i, def));
		}
		assert(mGuildPresentMap.size() > 0);
    }
    catch(...)
    {
        log_error("GuildPresentCfg ERROR !!!");
    }
}

GuildPresentDef * GuildPresentCfg::getCfg(int num)
{
	// 下标从0开始
	GuildPresentTd::iterator it = mGuildPresentMap.find(num);
	if (mGuildPresentMap.end() != it) {
		return it->second;
	}
	return NULL;
}
GuildPresentDef * GuildPresentCfg::getPresentCfg(int num)
{
	// 下标从0开始
	GuildPresentTd::iterator it = mGuildPresentMap.find(num);
	if (mGuildPresentMap.end() != it) {
		return it->second;
	}
	return mGuildPresentMap.rbegin()->second;
}


void GuildSkillCfg::read(GameInifile &inifile)
{
	mGuildSkillMap.clear();
	mSkillIdVec.clear();
	try
    {
		std::list<string> sections;
		std::list<std::string>::iterator it;
        inifile.getSections(std::back_inserter(sections));
        for (it = sections.begin(); it != sections.end(); it++)
        {
			int id = inifile.getValueT(*it, "id", 0);
			int maxLvl = inifile.getValueT(*it, "max_lvl", 0);

			assert(maxLvl > 0 && id > 0);

			StringTokenizer prevIdToken(inifile.getValue(*it, "prev_id"), ";");
			StringTokenizer guildLvlToken(inifile.getValue(*it, "guild_lvl"), ";");
			StringTokenizer needGongxianToken(inifile.getValue(*it, "need_gongxian"), ";");
			StringTokenizer needBattlePointToken(inifile.getValue(*it, "need_battle_point"), ";");

			assert(prevIdToken.count() == maxLvl);
			assert(guildLvlToken.count() == maxLvl);
			assert(needGongxianToken.count() == maxLvl);
			assert(needBattlePointToken.count() == 0 || needBattlePointToken.count() == maxLvl);

			for (int i = 0; i < maxLvl; i++) {
				GuildSkillDef *def = new GuildSkillDef();
				def->id = id;
				def->lvl =  Utils::safe_atoi(prevIdToken[i].c_str());
				def->maxLvl = maxLvl;
				def->needGuildLvl = Utils::safe_atoi(guildLvlToken[i].c_str());
				def->needContribute = Utils::safe_atoi(needGongxianToken[i].c_str());
				def->needBattlePoint = needBattlePointToken.count() == 0 ? 0 : Utils::safe_atoi(needBattlePointToken[i].c_str());

				std::pair<GuildSkillTd::iterator, bool> ret;
				ret = mGuildSkillMap.insert(make_pair(def->id + def->lvl, def));
				assert(ret.second == true);
			}
			mSkillIdVec.push_back(id);
		}
	}
	catch(...)
	{
		log_error("GuildSkillCfg ERROR !!!");
	}
}

GuildSkillDef * GuildSkillCfg::getCfg(int skillId, int lvl)
{
	GuildSkillTd::iterator it = mGuildSkillMap.find(skillId + lvl);
	if (it != mGuildSkillMap.end()) {
		return it->second;
	}
	return NULL;
}

bool GuildSkillCfg::getAllSkillByGuildLvl(int guildLVl,std::map<int, int> & outData)
{
    outData.clear();
    //思路是从配制文件中找出同时与guildLVl 和 skillID 配的项
    vector<int>::iterator beg = mSkillIdVec.begin();
    vector<int>::iterator end = mSkillIdVec.end();
    
    for (;beg!=end ;++beg ) {
        map<int,GuildSkillDef*>::reverse_iterator  rbeg_GS= mGuildSkillMap.rbegin();
        map<int,GuildSkillDef*>::reverse_iterator  rend_GS=mGuildSkillMap.rend();
        for (; rbeg_GS!=rend_GS; ++rbeg_GS) {
            if (rbeg_GS->first /100 == *beg / 100   &&
                rbeg_GS->second->needGuildLvl == guildLVl){
                outData.insert(std::make_pair(*beg, rbeg_GS->first - *beg));
                break;
            }
        }
    }
    return true;
}

vector<int> const & GuildSkillCfg::getSkillIdVec()
{
	return mSkillIdVec;
}


void RandGoodsCfgBase::readMerchant(GameInifile &inifile)
{
    mIsUseViplvlRefreshLimit = false;
    mIsUseGuildViplvlRefreshLimit = false;
	mUpdateRmb = inifile.getValueT("rmb", "rmb", 0);
	mFreeNum = inifile.getValueT("free", "free", 0);
	mInterval = inifile.getValueT("time", "time", 0);
	
	assert(mInterval > 0);
	
	vector<string> keys;
	inifile.getKeys("lvlgoods", inserter(keys, keys.begin()));
	assert(keys.size() > 0);
	
	for (vector<string>::iterator it = keys.begin(); it != keys.end(); it++)
	{
		StringTokenizer token(*it, "-");
		assert(token.count() == 2);
		
		int key = Utils::safe_atoi(token[1].c_str());
		int value = inifile.getValueT("lvlgoods", *it, 0);
		mLvlRangeMap.insert(make_pair(key, value));
	}
    
    string isUse = inifile.getValue("use_viplvl_refresh_limit", "use_guild_viplvl_refresh_limit", "false");
    mIsUseGuildViplvlRefreshLimit = isUse == "true" ? true : false;
    if (mIsUseGuildViplvlRefreshLimit)
    {
        vector<string> keys;
        inifile.getKeys("guild_viplvl_refresh_limit", inserter(keys, keys.begin()));
        
        assert(keys.size() > 0);
        
        for (vector<string>::iterator it = keys.begin(); it != keys.end(); it++)
        {
            StringTokenizer token(*it, "-");
            assert(token.count() == 2);
            
            int key = Utils::safe_atoi(token[1].c_str());
            int value = inifile.getValueT("guild_viplvl_refresh_limit", *it, 0);
            mGuildViplvlRangeMap.insert(make_pair(key, value));
        }
    }
}

map<int, RandGoodsDef*> RandGoodsCfgBase::cfgGoods;

bool RandGoodsCfgBase::GoodsVecCmp(const RandGoodsDefPointer& left, const RandGoodsDefPointer& right)
{
    return left->lvl < right->lvl;
}

void RandGoodsCfgBase::readGoods(GameInifile &inifile)
{
	try
    {
        // by lihengjin
        GameInifile MLIniFile;
        if(!(Cfgable::getFullFilePath("guildgoodsMultiLanguage.ini") == "guildgoodsMultiLanguage.ini"))
        {
            MLIniFile = GameInifile(Cfgable::getFullFilePath("guildgoodsMultiLanguage.ini"));
        }
        mGoodsVec.clear();
        cfgGoods.clear();
        
        std::list<string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator it = sections.begin(); it != sections.end(); it++)
        {
            RandGoodsDef *def = new RandGoodsDef();
            def->goodsId = Utils::safe_atoi((*it).c_str(), 0);
            def->lvl = inifile.getValueT(*it, "lvl", 0);
            def->pro = inifile.getValueT(*it, "pro", 0);

            StringTokenizer bgoldToken(inifile.getValue(*it, "consume", ""), ";");
            assert(bgoldToken.count() == 2);

            def->consumeMin = Utils::safe_atoi(bgoldToken[0].c_str(), 0);
            def->consumeMax = Utils::safe_atoi(bgoldToken[1].c_str(), 0);

            StringTokenizer brmdToken(inifile.getValue(*it, "brmb", ""), ";");
            assert(brmdToken.count() == 2);

            def->brmbMin = Utils::safe_atoi(brmdToken[0].c_str(), 0);
            def->brmdMax = Utils::safe_atoi(brmdToken[1].c_str(), 0);

			def->consumePro = inifile.getValueT(*it, "consume_pro", 0.5);

            //公会商店物品 描述
            string descId =inifile.getValue(*it, "desc_id");
            //by lihengjin
            string descStr;
            if(inifile.exists(*it, "desc"))
            {
                descStr = inifile.getValue(*it, "desc");
            }else
            {
                descStr = MLIniFile.getValue(descId, "desc");
            }
            //string descStr = MLIniFile.getValue(descId, "desc");
            def->desc_id = descStr;
            
            mGoodsVec.push_back(def);
            cfgGoods.insert(make_pair(def->goodsId, def));
        }
        std::sort(mGoodsVec.begin(), mGoodsVec.end(), GoodsVecCmp);
    }
    catch(...)
    {
        log_error("RandGoodsCfgBase ERROR !!!");
    }
}

RandGoodsDef* RandGoodsCfgBase::getGoods(int index)
{
    map<int, RandGoodsDef*>::iterator iter = cfgGoods.find(index);
    if (cfgGoods.end() != iter) {
        return (iter->second);
    }
    return NULL;
}

bool RandGoodsCfgBase::isUseGuildViplvlRefreshLimit()
{
    return mIsUseGuildViplvlRefreshLimit;
}

int RandGoodsCfgBase::getVipRefreshTimes(const int &viplvl)
{
    for (std::map<int, int>::iterator iter = mViplvlRangeMap.begin();
         iter != mViplvlRangeMap.end(); ++iter)
    {
        if (viplvl >=0 && viplvl <= iter->first && viplvl <=VipPrivilegeCfg::getMaxLvl())
        {
            return iter->second;
        }
    }
    log_error("role vip level ERROR !!!");
    return 0;
}

int RandGoodsCfgBase::getGuildVipRefreshTimes(const int &viplvl)
{
    for (std::map<int, int>::iterator iter = mGuildViplvlRangeMap.begin();
         iter != mGuildViplvlRangeMap.end(); ++iter)
    {
        if (viplvl >=0 && viplvl <= iter->first && viplvl <= VipPrivilegeCfg::getMaxLvl())
        {
            return iter->second;
        }
    }
    log_error("role vip level ERROR !!!");
    return 0;
}

void RandGoodsCfgBase::randGoods(int level, vector<RandGoodsDef*> &vec)
{
	vector<int> bat;
    vector<RandGoodsDef*>::iterator it;
    for (it = mGoodsVec.begin(); it != mGoodsVec.end(); it++) {
        if (level < (*it)->lvl) {
            continue;
        }
        bat.push_back((*it)->pro);
    }

    if (bat.empty()) {
        return;
    }

    // 取等级数量
    int randNum = 0;
    for (std::map<int, int>::iterator it = mLvlRangeMap.begin(); it != mLvlRangeMap.end(); it++) {
        if (level <= it->first) {
            randNum = it->second;
            break;
        }
    }
    if (randNum == 0) {
        randNum = mLvlRangeMap.rbegin()->second;
    }

    int batSize = bat.size();
	randNum = min(randNum , batSize);

    // 随机物品
    vector<int> index(randNum, 0);
    getRands(bat.data(), batSize, randNum, index.data(), false);
    for (vector<int>::iterator it = index.begin() ; it != index.end(); it++) {
        vec.push_back(mGoodsVec[*it]);
    }
}
void GuildGoodsCfg::RemainGoods(vector<int> &vec){
    mRemainGoodsVec.clear();
    vector<int>::iterator findTmp;
    vector<RandGoodsDef*>::iterator it;
    for (it=mGoodsVec.begin(); it!=mGoodsVec.end(); it++) {
        findTmp=std::find(vec.begin(), vec.end(), (*it)->goodsId);
        if (findTmp!=vec.end()) {
            continue;//找到,继续循环,找不到,存起来
        }
        mRemainGoodsVec.push_back(*it);
    }
}
int GuildGoodsCfg::getRemainGoodsSize(){
    return mRemainGoodsVec.size();
}


void GuildGoodsCfg::read()
{
	try
    {
		{
			GameInifile inifile(Cfgable::getFullFilePath("guildmerchant.ini"));
			readMerchant(inifile);
		}
		{
			GameInifile inifile(Cfgable::getFullFilePath("guildgoods.ini"));
			readGoods(inifile);
		}
    }
    catch(...)
    {
        log_error("GuildGoodsCfg ERROR !!!");
    }
}

std::map<int, int> GuildChampaGoodsCfg::mOccupied;
std::map<int, ChampaGoodsDef> GuildChampaGoodsCfg::mChampaGoods;
void GuildChampaGoodsCfg::load()
{
	{
		GameInifile inifile(Cfgable::getFullFilePath("guildchampagoods.ini"));
		readChampaGoods(inifile);
	}
	{
		GameInifile inifile(Cfgable::getFullFilePath("guildoccupiedconfig.ini"));
		readOccupiedConfig(inifile);
	}
}

void GuildChampaGoodsCfg::readChampaGoods(GameInifile &inifile)
{
	std::list<string> sections;
	std::list<std::string>::iterator it;
	inifile.getSections(std::back_inserter(sections));
	for (it = sections.begin(); it != sections.end(); it++)
	{
		ChampaGoodsDef def;
		def.index = inifile.getValueT(*it, "index", 0);
		def.goodsId = inifile.getValueT(*it, "goods_id", 0);
		def.goodsNum = inifile.getValueT(*it, "goods_num", 0);
		def.consume = inifile.getValueT(*it, "consume", 0);
		def.occupied = inifile.getValueT(*it, "occupied", 0);
		def.goodsName = inifile.getValue(*it, "goods_name");
		def.goodsDesc = inifile.getValue(*it, "goods_desc");
		def.pic = inifile.getValue(*it, "pic");
		
		assert(def.index > 0 && def.goodsId > 0 && def.consume > 0);
		
		mChampaGoods.insert(make_pair(def.index, def));
	}
}

void GuildChampaGoodsCfg::readOccupiedConfig(GameInifile &inifile)
{
	std::list<string> sections;
	std::list<std::string>::iterator it;
	inifile.getSections(std::back_inserter(sections));
	for (it = sections.begin(); it != sections.end(); it++)
	{
		int rank = inifile.getValueT(*it, "rank", 0);
		int num = inifile.getValueT(*it, "num", 0);
		
		assert(rank > 0 && num > 0);
		
		mOccupied.insert(make_pair(rank, num));
	}
}

int GuildChampaGoodsCfg::getOccupied(int rank)
{
	return mOccupied[rank];
}

ChampaGoodsDef* GuildChampaGoodsCfg::getChampaGoods(int index)
{
	map<int, ChampaGoodsDef>::iterator it = mChampaGoods.find(index);
	if (it == mChampaGoods.end()) {
		return NULL;
	}
	return &it->second;
}


void GuildGadCfg::read(GameInifile &inifile)
{
	std::map<string, GuildMemberPosition> postmap;
	postmap["kGuildPositionNotset"] = kGuildPositionNotset;
	postmap["kGuildMaster"] = kGuildMaster;
	postmap["kGuildViceMaster"] = kGuildViceMaster;
	postmap["kGuildElite"] = kGuildElite;
	postmap["kGuildMember"] = kGuildMember;

	try
    {
        //lihengjin
        GameInifile MLIniFile;
        if(!(Cfgable::getFullFilePath("guildgadMultiLanguage.ini") == "guildgadMultiLanguage.ini"))
        {
            MLIniFile = GameInifile(Cfgable::getFullFilePath("guildgadMultiLanguage.ini"));
        }
		std::list<string> sections;
		std::list<std::string>::iterator it;
        inifile.getSections(std::back_inserter(sections));
        for (it = sections.begin(); it != sections.end(); it++)
        {
			GuildGadDef *def = new GuildGadDef();

            def->setid(inifile.getValueT(*it, "id", 0));
            string name_id = inifile.getValue(*it, "name_id");
            //by lihengjin
            std::string str_name;
            if(inifile.exists(*it, "name"))
            {
                str_name = inifile.getValue(*it, "name");
            }else
            {
                str_name = MLIniFile.getValue(name_id, "desc");
            }
            def->setName(str_name);
            def->setLvl(inifile.getValueT(*it, "lvl", 0));
			
			int begin = 0;
			int end = 0;
			sscanf(inifile.getValue(*it, "FeatsRank").c_str(), "%d-%d", &begin, &end);
			assert(begin > 0 && end > 0);
			
			def->setFeatsRankBegin(begin);
			def->setFeatsRankEnd(end);
            def->setPost(postmap[inifile.getValue(*it, "GuildPost")]);
            def->setIcon(inifile.getValue(*it, "icon"));
            def->setAddMaxFat(inifile.getValueT(*it, "maxfat", 0));
            def->setAtk(inifile.getValueT(*it, "atk", 0));
            def->setDef(inifile.getValueT(*it , "def", 0));
            def->setAddMaxHp(inifile.getValueT(*it, "maxhp", 0));
            def->setHit(inifile.getValueT(*it, "hit", 0.0));
            def->setDodge(inifile.getValueT(*it, "dodge", 0.0));
            string desc_id = inifile.getValue(*it, "desc_id");
            //by lihengjin
            std::string str_desc;
            if(inifile.exists(*it, "desc"))
            {
                str_desc = inifile.getValue(*it, "desc");
            }else
            {
                str_desc = MLIniFile.getValue(desc_id, "desc");
            }
            def->setDesc(str_desc);
            mGuildGadMap.insert(make_pair(def->getid(), def));

		}
	}
	catch(...)
	{
		log_error("GuildGadCfg ERROR !!!");
	}
}

GuildGadDef * GuildGadCfg::getCfg(int guildLvl, GuildMemberPosition post)
{
	GuildGadTd::iterator it;
	for (it = mGuildGadMap.begin(); it != mGuildGadMap.end(); it++) {
        if (it->second->getLvl() == guildLvl
			&& it->second->getPost() == post) {
			return it->second;
		}
	}
	return NULL;
}

GuildGadDef * GuildGadCfg::getCfg(int gadId)
{
	GuildGadTd::iterator it = mGuildGadMap.find(gadId);
	if (it != mGuildGadMap.end()) {
		return it->second;
	}
	return NULL;
}

GuildGadDef * GuildGadCfg::getCfg(int guildLvl, int exploitRank)
{
	GuildGadTd::iterator it;
	for (it = mGuildGadMap.begin(); it != mGuildGadMap.end(); it++) {
		if (it->second->getLvl() == guildLvl
			&& exploitRank >= it->second->getFeatsRankBegin()
			&& exploitRank <= it->second->getFeatsRankEnd()) {
			return it->second;
		}
	}
	return NULL;
}




GuildBasicCfg GuildCfgMgr::sBasicCfg;
GuildPresentCfg GuildCfgMgr::sPresentCfg;
GuildSkillCfg GuildCfgMgr::sSkillCfg;
GuildGoodsCfg GuildCfgMgr::sGoodsCfg;
GuildGadCfg GuildCfgMgr::sGadCfg;
vector<GuildBossLvlDef> GuildCfgMgr::sGuildBossCfg;
GuildBossKillDef GuildCfgMgr::sGuildBossKillCfg;

void GuildCfgMgr::load()
{
	try
    {
		{
			GameInifile inifile(Cfgable::getFullFilePath("guild.ini"));
			sBasicCfg.read(inifile);
			sPresentCfg.read(inifile);
		}
		{
			GameInifile inifile(Cfgable::getFullFilePath("guildskill.ini"));
			sSkillCfg.read(inifile);
		}
		sGoodsCfg.read();
		{
			GameInifile inifile(Cfgable::getFullFilePath("guildgad.ini"));
			sGadCfg.read(inifile);
		}
        loadGuildBossCfg();
    }
    catch(...)
    {
        log_error("GuildMgr ERROR !!!");
    }
}

GuildLvlDef * GuildCfgMgr::getGuildLvlDef(float wealth, int overbuild)
{
	return sBasicCfg.getCfg(wealth, overbuild);
}



GuildLvlDef * GuildCfgMgr::getGuildLvlDef(int guildLvl)
{
	return sBasicCfg.getCfg(guildLvl);
}

GuildPresentDef * GuildCfgMgr::getGuildPresentDef(int num)
{
	return sPresentCfg.getCfg(num);
}

GuildPresentDef * GuildCfgMgr::getGuildSelfPresentDef(int num)
{
    return  sPresentCfg.getPresentCfg(num);
}

GuildPresentRmbDef & GuildCfgMgr::getGuildPresentRmbDef()
{
	return sPresentCfg.getRmbCfg();
}


GuildSkillDef * GuildCfgMgr::getSkillDef(int skillId, int lvl)
{
	return sSkillCfg.getCfg(skillId, lvl);
}

vector<int> const & GuildCfgMgr::getSkillIdVec()
{
	return sSkillCfg.getSkillIdVec();
}

void GuildCfgMgr::RemainGoods(vector<int> &vec){
    return sGoodsCfg.RemainGoods(vec);
}
int GuildCfgMgr::getRemainGoodsSize(){
    return sGoodsCfg.getRemainGoodsSize();
}
GuildGadDef * GuildCfgMgr::getGadDef(int guildLvl, GuildMemberPosition post)
{
	return sGadCfg.getCfg(guildLvl, post);
}

GuildGadDef * GuildCfgMgr::getGadDef(int gadid)
{
	return sGadCfg.getCfg(gadid);
}

GuildGadDef * GuildCfgMgr::getGadDef(int guildLvl, int exploitRank)
{
	return sGadCfg.getCfg(guildLvl, exploitRank);
}


GuildBossLvlDef* GuildCfgMgr::getGuildBossLvlDef(int bossExp)
{
    for (int i = sGuildBossCfg.size() - 1 ; i>=0; --i) {
        if( sGuildBossCfg[i].exp <= bossExp )
            return &(sGuildBossCfg[i]);
    }
    return NULL;
}

GuildBossLvlDef* GuildCfgMgr::getGuildBossUpLvlMaxExp(int bossExp){
    for (int i=0;i<=sGuildBossCfg.size()-1;i++) {
        if (sGuildBossCfg[i].exp>bossExp) {
            return &(sGuildBossCfg[i]);
        }
        if (bossExp>=sGuildBossCfg[sGuildBossCfg.size()-1].exp) {
            return &(sGuildBossCfg[sGuildBossCfg.size()-1]);
        }
    }
    return NULL;
}

GuildBossLvlDef* GuildCfgMgr::getGuildBossLvlExp(int lvl){
    
    lvl = min( (int)sGuildBossCfg.size(), lvl);
    
    for (vector<GuildBossLvlDef>::iterator it= sGuildBossCfg.begin(); it!= sGuildBossCfg.end(); ++it)
    {
        if(it->level == lvl)
        {
            return &(*it);
        }
    }

    return NULL;
}

GuildBossKillDef* GuildCfgMgr::getGuildBossKillCfg()
{
    return &sGuildBossKillCfg;
}

int GuildCfgMgr::getImpeachTime()
{
    return sBasicCfg.getImpeachTime();
}

int GuildCfgMgr::getImpeachCost()
{
    return sBasicCfg.getImpeachCost();
}

int GuildCfgMgr::getMaxApplicant()
{
    return sBasicCfg.getMaxApplicant();
}
int GuildCfgMgr::getWaitApplySecond()
{
    return sBasicCfg.getWaitApplySecond();
}
int GuildCfgMgr::getDonateRmbVipLvl()
{
    return sBasicCfg.getDonateRmbVipLvl();
}
int GuildBossLvlDef::rmbTrainNeedRmb(int rmbTrainTime)
{
    if(rmbTrainTime >= rmbTrainBossTimePerDay)
        return INT_MAX;

    if(rmbTrainTime < 0 )
    {
        log_info("rmbTrainNeedRmb bound error !");
        return INT_MAX;
    }
    
    int index = std::min ( (int)rmbTrainTime , (int)rmbTrainDef.trainNeedRmb.size()-1 );
    return rmbTrainDef.trainNeedRmb[index];
}

int GuildBossLvlDef::rmbTrainAddBossExp(int rmbTrainTime)
{
    if(rmbTrainTime >= rmbTrainBossTimePerDay)
        return 0;

    if(rmbTrainTime < 0)
    {
        log_info("rmbTrainAddBossExp bound error !");
        return 0;
    }
    
    int index = std::min ( (int)rmbTrainTime , (int)rmbTrainDef.addBossExp.size()-1 );
    return rmbTrainDef.addBossExp[index];
}

int GuildBossKillDef::getRankBonus(int rank)
{
    if(rank < 1 || rankBonus.empty() || rankBonus.size() != rankRange.size() )
    {
        log_info("getRankBonus error !"<<rank <<"\t" <<rankBonus.size()<<"\t" << rankRange.size());
        return 0;
    }

    for (int index = 0; index < rankRange.size(); ++index)
    {
        if(rank <= rankRange[index] )
            return rankBonus[index];
    }
    return *(rankBonus.rbegin());
}

#define readGuildBossLevelToken(first,second,item) \
{\
    vector<double> result = inifile.getTableT(first,second,0.0);\
    for(int i = 0 ,ni = result.size() ; i < ni ; ++i)\
        sGuildBossCfg[i].item = result[i];\
}

void GuildCfgMgr::loadGuildBossCfg()
{
    GameInifile inifile(Cfgable::getFullFilePath("guildboss.ini"));

    sGuildBossCfg.clear();

    vector<string> idstr = inifile.getTable("root", "boss_id");
    int trainNum = inifile.getValueT("boss_exp", "RmbTrainBossNum", 0);
    
    sGuildBossCfg.resize(idstr.size());

    readGuildBossLevelToken("root","scene_id",sceneId);
    readGuildBossLevelToken("root","boss_id",monsterId);
    readGuildBossLevelToken("root","fortune_award",fortuneAward);
    readGuildBossLevelToken("root","construction_award",constructionAward);
    readGuildBossLevelToken("root","constribute_award_factor",constributeAwardFactor);
    readGuildBossLevelToken("root","exploit_award_factor",exploitAwardFactor);
    readGuildBossLevelToken("root","guildlvl",minGuildLvl);

    readGuildBossLevelToken("boss_exp","boss_exp",exp);
    readGuildBossLevelToken("boss_exp","ItemTrainBossNum",itemTrainBossTimePerDay);
//    readGuildBossLevelToken("boss_exp","RmbTrainBossNum",rmbTrainBossTimePerDay);

    rmbTrainGuildBossDef rmbTrainDef;
    rmbTrainDef.trainNeedRmb = inifile.getTableT("train_boss", "train_needrmb", 0);
    rmbTrainDef.addBossExp = inifile.getTableT("train_boss", "add_bossexp", 0);

    for (int i = 0; i< sGuildBossCfg.size(); ++i)
    {
        sGuildBossCfg[i].level = i + 1;
        sGuildBossCfg[i].rmbTrainDef = rmbTrainDef;
        sGuildBossCfg[i].rmbTrainBossTimePerDay = trainNum;
    }

    loadGuildBossKillCfg(inifile);
}

void GuildCfgMgr::loadGuildBossKillCfg(GameInifile& inifile)
{
    sGuildBossKillCfg.clear();
    sGuildBossKillCfg.rankBonus = inifile.getTableT("root", "rank_bonus", 0);
    sGuildBossKillCfg.rankRange = inifile.getTableT("root", "rank_range", 0);
    sGuildBossKillCfg.lastHit = inifile.getValueT("root", "last_hit", 0);
    sGuildBossKillCfg.clientRankNum = inifile.getValueT("root", "client_rank_num", 0);
    sGuildBossKillCfg.rebornSeconds = inifile.getValueT("root", "reborn_seconds", 0);
    sGuildBossKillCfg.preparetime = inifile.getValueT("root", "prepare_time", 300);
}


int GuildCfgMgr::getBossPrepareTime()
{
    return sGuildBossKillCfg.preparetime;
}


#pragma mark -



#pragma mark - 物品转换
void ItemChangeCfg::read(GameInifile &inifile)
{
	mItemChangeMap.clear();
	memset(&mItemChangeDumb, 0, sizeof(mItemChangeDumb));
	
	std::list<string> sections;
	std::list<std::string>::iterator it;
	inifile.getSections(std::back_inserter(sections));
	for (it = sections.begin(); it != sections.end(); it++)
	{
		ItemChangeDef def;
		def.proto_id = inifile.getValueT(*it, "proto_id", 0);
		def.goal_id = inifile.getValueT(*it, "goal_id", 0);
		def.change = inifile.getValueT(*it, "change", 0.0);
		def.compensation_rmb = inifile.getValueT(*it, "compensation_rmb", 0);
		mItemChangeMap.insert(make_pair(def.proto_id, def));
	}
}

ItemChangeDef & ItemChangeCfg::getCfg(int id)
{
	ItemChangeTd::iterator it = mItemChangeMap.find(id);
	if (it == mItemChangeMap.end()) {
		return mItemChangeDumb;
	}
	return it->second;
}


ItemChangeCfg ItemChangeCfgMgr::sCfg;
void ItemChangeCfgMgr::load()
{
	try
    {
		{
			GameInifile inifile(Cfgable::getFullFilePath("item_change.ini"));
			sCfg.read(inifile);
		}
    }
    catch(...)
    {
        log_error("ItemChangeMgr ERROR !!!");
    }
}

#pragma mark - 充值活动数据
int ChargeActivityDef::server_time = 0;
ChargeActivityDef::ChargeActivityDef()
{
    setActivityId(0);
    setStartTime(0);
    setOverTime(0);
    setAwardType(0);
    ServerIds.clear();
    platform = "";
    activityType = "";
    activityDesc = "";
    awards.clear();
    setAwardNum(0);
}
ChargeActivityDef::~ChargeActivityDef()
{
    
}
vector<ChargeActItemDef*> ChargeActivityDef::getAwards()
{
    return awards;
}
string ChargeActivityDef::getAward(int rmb)
{
    vector<ChargeActItemDef*>::iterator itrBegin = awards.begin();
    for (; itrBegin != awards.end(); ++itrBegin)
    {
        ChargeActItemDef* def = (ChargeActItemDef*)*itrBegin;
        if(def->getRmb() == rmb)
            return def->items;
    }
    return "";
}
bool ChargeActivityDef::includeCurSr( int srId )
{
    if( find( ServerIds.begin(), ServerIds.end(), -1) != ServerIds.end() )
        return true;
    
    if( find( ServerIds.begin(), ServerIds.end(), srId ) != ServerIds.end() )
        return true;
    else
        return false;
}
#pragma mark - 充值活动配置
vector<ChargeActivityDef*> ChargeActivityCfg::getCfg(int serverId)
{
    vector<ChargeActivityDef*> vec;
    vec.clear();
    Iterator itr = ChargeActivityCfg::begin();
    do
    {
        if (itr == ChargeActivityCfg::end())
        {
            break;
        }
        ChargeActivityDef* def = (ChargeActivityDef*)(*itr);
#if defined(CLIENT)
        if( def->includeCurSr( serverId ) && ChargeActivityDef::server_time > def->getStartTime() && ChargeActivityDef::server_time < def->getOverTime())
            vec.push_back( def );
        ++itr;
#else
        if( def->includeCurSr( serverId ))
            vec.push_back( def );
        ++itr;
#endif
    } while (1);
    return vec;
}
vector<ChargeActivityDef*> ChargeActivityCfg::getCfgPlatform(int serverId, string platform)
{
    vector<ChargeActivityDef*> vec;
    vec.clear();
    Iterator itr = ChargeActivityCfg::begin();
    do
    {
        if (itr == ChargeActivityCfg::end()) {
            break;
        }
        ChargeActivityDef* def = (ChargeActivityDef*)(*itr);
#if defined(CLIENT)
        if (def->includeCurSr( serverId ) && Utils::safe_atoi(def->platform.c_str()) == -1 && ChargeActivityDef::server_time > def->getStartTime() && ChargeActivityDef::server_time < def->getOverTime())
        {
            vec.push_back(def);
        }
        if (def->includeCurSr( serverId ) && def->platform == platform && ChargeActivityDef::server_time > def->getStartTime() && ChargeActivityDef::server_time < def->getOverTime())
        {
            vec.push_back(def);
        }
        ++itr;
        
#else
        if (def->includeCurSr( serverId ) && Utils::safe_atoi(def->platform.c_str()) == -1)
        {
            vec.push_back(def);
        }
        if (def->includeCurSr( serverId ) && def->platform == platform)
        {
            vec.push_back(def);
        }
        ++itr;
#endif
    } while (1);
    
    return vec;
}
#pragma mark - 累计充值活动配置
void AddChargeActCfg::load(string fullpath)
{
   	try
    {
        GameInifile inifile(fullpath);
        //by lihengjin
        GameInifile Minifile;
        if(!(Cfgable::getFullFilePath("AccumulateRechargeAwardActMultiLanguage.ini") == "AccumulateRechargeAwardActMultiLanguage.ini"))
        {
            Minifile = GameInifile(Cfgable::getFullFilePath("AccumulateRechargeAwardActMultiLanguage.ini"));
        }
        vector<string> sections;
        inifile.getSections(std::back_inserter(sections));
        std::vector<string>::iterator itr = sections.begin();
        std::vector<string>::iterator itrEnd = sections.end();
        
        for (; itr != itrEnd; ++itr)
        {
            AddChargeActDef* actDef = new AddChargeActDef();
            int index = inifile.getValueT(*itr, "index", 0);
            std::string servers = inifile.getValue(*itr, "server_id");
            StringTokenizer srTokens( servers, ";" );
            for ( int i = 0; i < srTokens.count(); ++i )
            {
                actDef->ServerIds.push_back( Utils::safe_atoi( srTokens[i].c_str() ) );
            }
            std::string platform = inifile.getValue(*itr, "Channel");
            int endTime = Utils::parseDate( inifile.getValue(*itr, "overdate").c_str() );
            int startTime = Utils::parseDate(inifile.getValue(*itr, "startdate").c_str());
            if (time(0) < startTime)
                continue;
            if(time(0) > endTime)
                continue;
            std::string titleId = inifile.getValue(*itr, "title_id");
            //int actId = Minifile.getValueT(titleId, "id", 0);
            //by lihengjin
            std::string actType;
            if(inifile.exists(*itr, "title"))
            {
                actType = inifile.getValue(*itr, "title");
            }else
            {
                actType = Minifile.getValue(titleId, "desc");
            }
           // std::string actType = Minifile.getValue(titleId, "desc");
            std::string desId = inifile.getValue(*itr, "content_id");
            //lihengjin
            std::string actDes;
            if(inifile.exists(*itr, "content"))
            {
                actDes = inifile.getValue(*itr, "content");
            }else
            {
                actDes = Minifile.getValue(desId,  "desc");
            }
            //std::string actDes = Minifile->getValue(desId, "desc");
            int awardType = inifile.getValueT(*itr, "award_type", 0);
            
            actDef->setAwardType(awardType);
            actDef->setActivityId(index);
            actDef->activityType = actType;
            actDef->setStartTime(startTime);
            actDef->setOverTime(endTime);
            actDef->activityDesc = actDes;
            actDef->platform = platform;
            
            const int awardnum = inifile.getValueT(*itr, "awardnum", 0);
            //int size = sizeof(ChargeActivityDef);
            for (int i = 0; i < awardnum; ++i)
            {
                ChargeActItemDef* item = new ChargeActItemDef();
                std::string need = Utils::makeStr("needrecharge%d", i + 1);
                std::string award = Utils::makeStr("itemawards%d", i + 1);
                item->setRmb(inifile.getValueT(*itr, need, 0));
                item->items = inifile.getValue(*itr, award);
                actDef->awards.push_back(item);
            }
            
            datas.push_back(actDef);
        }
    }
    catch(...)
    {
        log_error("ChargeActivityCfg ERROR !!!");
    }
}
//lihengjin

void AddChargeActCfg::addData(vector<obj_accumulate_recharge> accumulate)
{
    datas.clear();
    //获取累计充值活动配置数据
    for (int i = 0 ; i < accumulate.size(); i++) {
        AddChargeActDef* actDef = new AddChargeActDef();
        int index = accumulate[i].index;
        std::string servers = accumulate[i].server_id;
        StringTokenizer srTokens( servers, ";" );
        for ( int j = 0; j < srTokens.count(); ++j )
        {
            actDef->ServerIds.push_back( Utils::safe_atoi( srTokens[j].c_str() ) );
        }
        std::string platform = accumulate[i].Channel;
        int endTime = accumulate[i].overdate;
        int startTime = accumulate[i].startdate;
        std::string actType = accumulate[i].title;
        std::string actDes = accumulate[i].content;
        int awardType = accumulate[i].award_type;
        
        actDef->setAwardType(awardType);
        actDef->setActivityId(index);
        actDef->activityType = actType;
        actDef->setStartTime(startTime);
        actDef->setOverTime(endTime);
        actDef->activityDesc = actDes;
        actDef->platform = platform;
        
        /*
        const int awardnum = accumulate[i].awardnum;
        //int size = sizeof(ChargeActivityDef);
        actDef->setAwardNum(awardnum);
        ChargeActItemDef* item1 = new ChargeActItemDef();
        item1->setRmb(accumulate[i].needrecharge1);
        item1->items = accumulate[i].itemawards1;
        actDef->awards.push_back(item1);*/
        //解析result，取得服务器信息。
        Json::Value jsonValue;
        Json::Reader reader;
        if (!reader.parse(accumulate[i].configJson, jsonValue))
        {
           //err
        }
        
        for (int jsonI = 0; jsonI < jsonValue.size(); jsonI++) {
            int need = xyJsonAsInt(jsonValue[jsonI]["need"]);
            string award = xyJsonAsString(jsonValue[jsonI]["award"]);
            
            ChargeActItemDef* item = new ChargeActItemDef();
            item->setRmb(need);
            if(award == "")
            {
                printf("fdssdfsdfsd");
            }
            item->items = award;
            actDef->awards.push_back(item);
            
        }

        datas.push_back(actDef);
        
    }
	 
}


#pragma mark - 累计消费活动配置
void AddConsumeActCfg::load(string fullpath)
{
   	try
    {
        GameInifile inifile(fullpath);
        //--lihengjin
        GameInifile Minifile;
        if(!(Cfgable::getFullFilePath("AccumulateConsumeAwardActMultiLanguage.ini") == "AccumulateConsumeAwardActMultiLanguage.ini"))
        {
            Minifile = GameInifile(Cfgable::getFullFilePath("AccumulateConsumeAwardActMultiLanguage.ini"));
        }
        vector<string> sections;
        inifile.getSections(std::back_inserter(sections));
        std::vector<string>::iterator itr = sections.begin();
        std::vector<string>::iterator itrEnd = sections.end();
        
        for (; itr != itrEnd; ++itr)
        {
            AddConsumeActDef* actDef = new AddConsumeActDef();
            int index = inifile.getValueT(*itr, "index", 0);
            std::string servers = inifile.getValue(*itr, "server_id");
            StringTokenizer srTokens( servers, ";" );
            for ( int i = 0; i < srTokens.count(); ++i )
            {
                actDef->ServerIds.push_back( Utils::safe_atoi( srTokens[i].c_str() ) );
            }
            std::string platform = inifile.getValue(*itr, "Channel");
            int endTime = Utils::parseDate( inifile.getValue(*itr, "overdate").c_str() );
            int startTime = Utils::parseDate(inifile.getValue(*itr, "startdate").c_str());
            if (time(0) < startTime)
                continue;
            if(time(0) > endTime)
                continue;
            std::string titleId = inifile.getValue(*itr, "title_id");
            //int actId = Minifile.getValueT(titleId, "id", 0);
            //by lihengjin
            std::string actType;
            if(inifile.exists(*itr, "title"))
            {
                actType = inifile.getValue(*itr, "title");
            }else
            {
                actType = Minifile.getValue(titleId, "desc");
            }
            //std::string actType = Minifile.getValue(titleId, "desc");
            std::string desId = inifile.getValue(*itr, "content_id");
            //by lihengjin
            std::string actDes;
            if(!inifile.exists(*itr, "content_id"))
            {
                actDes = inifile.getValue(*itr, "content");
            }else
            {
                actDes = Minifile.getValue(desId, "desc");
            }
           // std::string actDes = Minifile.getValue(desId, "desc");
            int awardType = inifile.getValueT(*itr, "award_type", 0);
            
            actDef->setAwardType(awardType);
            actDef->setActivityId(index);
            actDef->activityType = actType;
            actDef->setStartTime(startTime);
            actDef->setOverTime(endTime);
            actDef->activityDesc = actDes;
            actDef->platform = platform;
            
            const int awardnum = inifile.getValueT(*itr, "awardnum", 0);
            //int size = sizeof(ChargeActivityDef);
            for (int i = 0; i < awardnum; ++i)
            {
                ChargeActItemDef* item = new ChargeActItemDef();
                std::string need = Utils::makeStr("needrecharge%d", i + 1);
                std::string award = Utils::makeStr("itemawards%d", i + 1);
                item->setRmb(inifile.getValueT(*itr, need, 0));
                item->items = inifile.getValue(*itr, award);
                actDef->awards.push_back(item);
            }
            
            datas.push_back(actDef);
        }
    }
    catch(...)
    {
        log_error("AddConsumeActCfg ERROR !!!");
    }
}

void AddConsumeActCfg::addData(vector<obj_accumulate_consume> accumulateConsume)
{
	
    datas.clear();
    for (int i = 0 ; i < accumulateConsume.size(); i++)
    {
        AddConsumeActDef* actDef = new AddConsumeActDef();
        int index = accumulateConsume[i].index;
        std::string servers = accumulateConsume[i].server_id;
        StringTokenizer srTokens( servers, ";" );
        for ( int j = 0; j < srTokens.count(); ++j )
        {
            actDef->ServerIds.push_back( Utils::safe_atoi( srTokens[j].c_str() ) );
        }
        std::string platform = accumulateConsume[i].Channel;
        int endTime = accumulateConsume[i].overdate;
        int startTime = accumulateConsume[i].startdate;
        std::string actType = accumulateConsume[i].title;
        std::string actDes = accumulateConsume[i].content;
        int awardType = accumulateConsume[i].award_type;
        actDef->setAwardType(awardType);
        actDef->setActivityId(index);
        actDef->activityType = actType;
        actDef->setStartTime(startTime);
        actDef->setOverTime(endTime);
        actDef->activityDesc = actDes;
        actDef->platform = platform;
        
        /*
        const int awardnum = accumulateConsume[i].awardnum;
        //int size = sizeof(ChargeActivityDef);
        actDef->setAwardNum(awardnum);
        ChargeActItemDef* item1 = new ChargeActItemDef();
        item1->setRmb(accumulateConsume[i].needrecharge1);
        item1->items = accumulateConsume[i].itemawards1;
        actDef->awards.push_back(item1);
        */
        //解析result，取得服务器信息。
        Json::Value jsonValue;
        Json::Reader reader;
        if (!reader.parse(accumulateConsume[i].configJson, jsonValue))
        {
            //err
        }
        
        for (int jsonI = 0; jsonI < jsonValue.size(); jsonI++) {
            int need = xyJsonAsInt(jsonValue[jsonI]["need"]);
            string award = xyJsonAsString(jsonValue[jsonI]["award"]);
            
            ChargeActItemDef* item = new ChargeActItemDef();
            item->setRmb(need);
            item->items = award;
            actDef->awards.push_back(item);
            
        }

        
        datas.push_back(actDef);

    }
}


#pragma mark - 可叠加单笔充值配置
void AddOnceChargeActCfg::load(string fullpath)
{
   	try
    {
        GameInifile inifile(fullpath);
        //lihengjin
        GameInifile Minifile;
        if(!(Cfgable::getFullFilePath("OperateActOnceRechargeAwardMultiLanguage.ini") == "OperateActOnceRechargeAwardMultiLanguage.ini"))
        {
            Minifile =  GameInifile(Cfgable::getFullFilePath("OperateActOnceRechargeAwardMultiLanguage.ini"));
        }
        vector<string> sections;
        inifile.getSections(std::back_inserter(sections));
        std::vector<string>::iterator itr = sections.begin();
        std::vector<string>::iterator itrEnd = sections.end();
        
        for (; itr != itrEnd; ++itr)
        {
            AddOnceChargeActDef* actDef = new AddOnceChargeActDef();
            int index = inifile.getValueT(*itr, "index", 0);
            std::string servers = inifile.getValue(*itr, "server_id");
            StringTokenizer srTokens( servers, ";" );
            for ( int i = 0; i < srTokens.count(); ++i )
            {
                actDef->ServerIds.push_back( Utils::safe_atoi( srTokens[i].c_str() ) );
            }
            std::string platform = inifile.getValue(*itr, "Channel");
            int endTime = Utils::parseDate( inifile.getValue(*itr, "overdate").c_str() );
            int startTime = Utils::parseDate(inifile.getValue(*itr, "startdate").c_str());
            if (time(0) < startTime)
                continue;
            if(time(0) > endTime)
                continue;
            std::string titleId = inifile.getValue(*itr, "title_id");
            //int actId = Minifile.getValueT(titleId, "id", 0);
            //by lihengjin
            std::string actType;
            if(inifile.exists(*itr, "title"))
            {
                actType = inifile.getValue(*itr, "title");
            }else
            {
                actType = Minifile.getValue(titleId, "desc");
            }
            //std::string actType = Minifile.getValue(titleId, "desc");
            std::string desId = inifile.getValue(*itr, "content_id");
            //by lihengjin
            std::string actDes;
            if(!inifile.exists(*itr, "content_id"))
            {
                actDes = inifile.getValue(*itr, "content");
            }else
            {
                actDes = Minifile.getValue(desId, "desc");
            }
            //std::string actDes = Minifile.getValue(desId, "desc");
            int awardType = inifile.getValueT(*itr, "award_type", 0);
            
            actDef->setAwardType(awardType);
            actDef->setActivityId(index);
            actDef->activityType = actType;
            actDef->setStartTime(startTime);
            actDef->setOverTime(endTime);
            actDef->activityDesc = actDes;
            actDef->platform = platform;
            
            const int awardnum = inifile.getValueT(*itr, "awardnum", 0);
            //int size = sizeof(ChargeActivityDef);
            for (int i = 0; i < awardnum; ++i)
            {
                ChargeActItemDef* item = new ChargeActItemDef();
                std::string need = Utils::makeStr("needrecharge%d", i + 1);
                std::string award = Utils::makeStr("itemawards%d", i + 1);
                item->setRmb(inifile.getValueT(*itr, need, 0));
                item->items = inifile.getValue(*itr, award);
                actDef->awards.push_back(item);
            }
            
            datas.push_back(actDef);
        }
    }
    catch(...)
    {
        log_error("AddOnceChargeActCfg ERROR !!!");
    }
}

void AddOnceChargeActCfg::addData(vector<obj_operate_recharge> operateRecharge)
{
    datas.clear();
    for (int i = 0 ; i < operateRecharge.size(); i++) {
        AddOnceChargeActDef* actDef = new AddOnceChargeActDef();
        int index = operateRecharge[i].index;
        std::string servers = operateRecharge[i].server_id;
        StringTokenizer srTokens( servers, ";" );
        for ( int j = 0; j < srTokens.count(); ++j )
        {
            actDef->ServerIds.push_back( Utils::safe_atoi( srTokens[j].c_str() ) );
        }
        std::string platform = operateRecharge[i].Channel;
        int endTime = operateRecharge[i].overdate;
        int startTime = operateRecharge[i].startdate;
        std::string actType = operateRecharge[i].title;
        std::string actDes = operateRecharge[i].content;
        int awardType = operateRecharge[i].award_type;
        
        const int awardnum = operateRecharge[i].awardnum;
        actDef->setAwardNum(awardnum);
        
        actDef->setAwardType(awardType);
        actDef->setActivityId(index);
        actDef->activityType = actType;
        actDef->setStartTime(startTime);
        actDef->setOverTime(endTime);
        actDef->activityDesc = actDes;
        actDef->platform = platform;
        /*
        ChargeActItemDef* item1 = new ChargeActItemDef();
        item1->setRmb(operateRecharge[i].needrecharge1);
        item1->items = operateRecharge[i].itemawards1;
        actDef->awards.push_back(item1); 
         */
    
        //解析result，取得服务器信息。
        Json::Value jsonValue;
        Json::Reader reader;
        if (!reader.parse(operateRecharge[i].configJson, jsonValue))
        {
            //err
        }
        
        for (int jsonI = 0; jsonI < jsonValue.size(); jsonI++) {
            int need = xyJsonAsInt(jsonValue[jsonI]["need"]);
            string award = xyJsonAsString(jsonValue[jsonI]["award"]);
            
            ChargeActItemDef* item = new ChargeActItemDef();
            item->setRmb(need);
            item->items = award;
            actDef->awards.push_back(item);
            
        }
        //const int awardnum = operateRecharge[i].award_type;
        datas.push_back(actDef);

    }
	 
}

#pragma mark - 不可叠加单笔充值配置
void OnceChargeActCfg::load(string fullpath)
{
   	try
    {
        GameInifile inifile(fullpath);
        //lihengjin
        GameInifile Minifile;
        if(!(Cfgable::getFullFilePath("OperateActRechargeAwardMultiLanguage.ini") == "OperateActRechargeAwardMultiLanguage.ini"))
        {
            Minifile = GameInifile(Cfgable::getFullFilePath("OperateActRechargeAwardMultiLanguage.ini"));
        }
        vector<string> sections;
        inifile.getSections(std::back_inserter(sections));
        std::vector<string>::iterator itr = sections.begin();
        std::vector<string>::iterator itrEnd = sections.end();
        
        for (; itr != itrEnd; ++itr)
        {
            OnceChargeActDef* actDef = new OnceChargeActDef();
            int index = inifile.getValueT(*itr, "index", 0);
            std::string servers = inifile.getValue(*itr, "server_id");
            StringTokenizer srTokens( servers, ";" );
            for ( int i = 0; i < srTokens.count(); ++i )
            {
                actDef->ServerIds.push_back( Utils::safe_atoi( srTokens[i].c_str() ) );
            }
            std::string platform = inifile.getValue(*itr, "Channel");
            int endTime = Utils::parseDate( inifile.getValue(*itr, "overdate").c_str() );
            int startTime = Utils::parseDate(inifile.getValue(*itr, "startdate").c_str());
            if (time(0) < startTime)
                continue;
            if(time(0) > endTime)
                continue;
            std::string titleId = inifile.getValue(*itr, "title_id");
            //int actId = Minifile.getValueT(titleId, "id", 0);
            //by lihengjin
            std::string actType;
            if(inifile.exists(*itr, "title"))
            {
                actType = inifile.getValue(*itr, "title");
            }else
            {
                actType = Minifile.getValue(titleId, "desc");
            }
           // std::string actType = Minifile.getValue(titleId, "desc");
            std::string desId = inifile.getValue(*itr, "content_id");
            //by lihengjin
            std::string actDes;
            if(!inifile.exists(*itr, "content_id"))
            {
                actDes = inifile.getValue(*itr, "content");
            }else
            {
                actDes = Minifile.getValue(desId, "desc");
            }
            //std::string actDes = Minifile.getValue(desId, "desc");
            int awardType = inifile.getValueT(*itr, "award_type", 0);
            
            actDef->setAwardType(awardType);
            actDef->setActivityId(index);
            actDef->activityType = actType;
            actDef->setStartTime(startTime);
            actDef->setOverTime(endTime);
            actDef->activityDesc = actDes;
            actDef->platform = platform;
            
            const int awardnum = inifile.getValueT(*itr, "awardnum", 0);
            //int size = sizeof(ChargeActivityDef);
            for (int i = 0; i < awardnum; ++i)
            {
                ChargeActItemDef* item = new ChargeActItemDef();
                std::string need = Utils::makeStr("needrecharge%d", i + 1);
                std::string award = Utils::makeStr("itemawards%d", i + 1);
                item->setRmb(inifile.getValueT(*itr, need, 0));
                item->items = inifile.getValue(*itr, award);
                actDef->awards.push_back(item);
            }
            
            datas.push_back(actDef);
        }
    }
    catch(...)
    {
        log_error("OnceChargeActCfg ERROR !!!");
    }
}

void OnceChargeActCfg::addData(vector<obj_operate_once_recharge> operateOnceRecharge)
{
	
    datas.clear();
    for (int i = 0; i < operateOnceRecharge.size(); i++) {
        OnceChargeActDef* actDef = new OnceChargeActDef();
        int index = operateOnceRecharge[i].index;
        std::string servers = operateOnceRecharge[i].server_id;
        StringTokenizer srTokens( servers, ";" );
        for ( int j = 0; j < srTokens.count(); ++j )
        {
            actDef->ServerIds.push_back( Utils::safe_atoi( srTokens[j].c_str() ) );
        }
        std::string platform = operateOnceRecharge[i].Channel;
        int endTime = operateOnceRecharge[i].overdate;
        int startTime = operateOnceRecharge[i].startdate;
        std::string actType = operateOnceRecharge[i].title;
        std::string actDes = operateOnceRecharge[i].content;
        int awardType = operateOnceRecharge[i].award_type;
        
        const int awardnum = operateOnceRecharge[i].awardnum;
        actDef->setAwardNum(awardnum);
        
        actDef->setAwardType(awardType);
        actDef->setActivityId(index);
        actDef->activityType = actType;
        actDef->setStartTime(startTime);
        actDef->setOverTime(endTime);
        actDef->activityDesc = actDes;
        actDef->platform = platform;
    
        //解析result，取得服务器信息。
        Json::Value jsonValue;
        Json::Reader reader;
        if (!reader.parse(operateOnceRecharge[i].configJson, jsonValue))
        {
            //err
        }
        
        for (int jsonI = 0; jsonI < jsonValue.size(); jsonI++) {
            int need = xyJsonAsInt(jsonValue[jsonI]["need"]);
            string award = xyJsonAsString(jsonValue[jsonI]["award"]);
            
            ChargeActItemDef* item = new ChargeActItemDef();
            item->setRmb(need);
            item->items = award;
            actDef->awards.push_back(item);
            
        }
        
        
        datas.push_back(actDef);
    
    }
	 
}

#pragma mark - 首冲奖励
void FirstChargeCfg::load(string fullpath)
{
    try
    {
        GameInifile inifile(fullpath);
        //by lihengjin
        GameInifile Minifile;
        if(!(Cfgable::getFullFilePath("gift_activityMultiLanguage.ini") == "gift_activityMultiLanguage.ini"))
        {
            Minifile = GameInifile(Cfgable::getFullFilePath("gift_activityMultiLanguage.ini"));
        }
        vector<string> sections;
        inifile.getSections(std::back_inserter(sections));
        std::vector<string>::iterator itr = sections.begin();
        std::vector<string>::iterator itrEnd = sections.end();
        
        for (; itr != itrEnd; ++itr)
        {
            FirstChargeDef* actDef = new FirstChargeDef();
            int index = inifile.getValueT(*itr, "index", 0);
            std::string titleId = inifile.getValue(*itr, "title_id");
            //int actId = Minifile.getValueT(titleId, "id", 0);
            //by lihengjin
            std::string actType;
            if(inifile.exists(*itr, "title"))
            {
                actType = inifile.getValue(*itr, "title");
            }else
            {
                actType = Minifile.getValue(titleId, "desc");
            }
            //std::string actType = Minifile.getValue(titleId, "desc");
            std::string desId = inifile.getValue(*itr, "content_id");
            //by lihengjin
            std::string actDes;
            if(!inifile.exists(*itr, "content_id"))
            {
                actDes = inifile.getValue(*itr, "content");
            }else
            {
                actDes = Minifile.getValue(desId, "desc");
            }
            //std::string actDes = Minifile.getValue(desId, "desc");
            int awardType = inifile.getValueT(*itr, "award_type", 0);
            int endTime = Utils::parseDate( inifile.getValue(*itr, "endtime").c_str() );
            int startTime = Utils::parseDate(inifile.getValue(*itr, "starttime").c_str());
            if (time(0) < startTime)
                continue;
            if(time(0) > endTime)
                continue;
            actDef->setAwardType(awardType);
            actDef->setActivityId(index);
            actDef->activityType = actType;
            actDef->setStartTime(startTime);
            actDef->setOverTime(endTime);
            actDef->activityDesc = actDes;
            
            datas.push_back(actDef);
        }
    }
    catch(...)
    {
        log_error("FirstChargeCfg ERROR !!!");
    }
}

void FirstChargeCfg::addData(vector<obj_gift_activity> firstCharge)
{
    datas.clear();
    for (int i = 0 ; i < firstCharge.size(); i++) {
        FirstChargeDef* actDef = new FirstChargeDef();
        std::string actType = firstCharge[i].title;
        std::string actDes = firstCharge[i].content;
        int endTime = firstCharge[i].overdate;
        int startTime = firstCharge[i].startdate;
//        if (time(0) < startTime)
//            continue;
//        if(time(0) > endTime)
//            continue;
        if(ChargeActivityDef::server_time < startTime)
            continue;
        if(ChargeActivityDef::server_time > endTime)
            continue;
        
        actDef->activityType = actType;
        actDef->setStartTime(startTime);
        actDef->setOverTime(endTime);
        actDef->activityDesc = actDes;
        actDef->setActivityId(firstCharge[i].index);
        datas.push_back(actDef);
    }
}
void ConsumeFatAwardActCfg::load(string fullpath)
{
    try
    {
        datas.clear();
        GameInifile inifile(fullpath);
        //lihengjin
        GameInifile Minifile;
        vector<string> sections;
        inifile.getSections(std::back_inserter(sections));
        std::vector<string>::iterator itr = sections.begin();
        std::vector<string>::iterator itrEnd = sections.end();
        
        for (; itr != itrEnd; ++itr)
        {
            ConsumeFatAwardActDef* actDef = new ConsumeFatAwardActDef();
            int index = inifile.getValueT(*itr, "index", 0);
            std::string servers = inifile.getValue(*itr, "server_id");
            StringTokenizer srTokens( servers, ";" );
            for ( int i = 0; i < srTokens.count(); ++i )
            {
                actDef->ServerIds.push_back( Utils::safe_atoi( srTokens[i].c_str() ) );
            }
            std::string platform = inifile.getValue(*itr, "Channel");
            int endTime = Utils::parseDate( inifile.getValue(*itr, "overdate").c_str() );
            int startTime = Utils::parseDate(inifile.getValue(*itr, "startdate").c_str());
            if (time(0) < startTime)
                continue;
            if(time(0) > endTime)
                continue;
            std::string titleId = inifile.getValue(*itr, "title_id");
            //int actId = Minifile.getValueT(titleId, "id", 0);
            //by lihengjin
            std::string actType;
            if(inifile.exists(*itr, "title"))
            {
                actType = inifile.getValue(*itr, "title");
            }else
            {
                actType = Minifile.getValue(titleId, "desc");
            }
            // std::string actType = Minifile.getValue(titleId, "desc");
            std::string desId = inifile.getValue(*itr, "content_id");
            //by lihengjin
            std::string actDes;
            if(!inifile.exists(*itr, "content_id"))
            {
                actDes = inifile.getValue(*itr, "content");
            }else
            {
                actDes = Minifile.getValue(desId, "desc");
            }
            //std::string actDes = Minifile.getValue(desId, "desc");
            int awardType = inifile.getValueT(*itr, "award_type", 0);
            
            actDef->setAwardType(awardType);
            actDef->setActivityId(index);
            actDef->activityType = actType;
            actDef->setStartTime(startTime);
            actDef->setOverTime(endTime);
            actDef->activityDesc = actDes;
            actDef->platform = platform;
            
            const int awardnum = inifile.getValueT(*itr, "awardnum", 0);
            //int size = sizeof(ChargeActivityDef);
            for (int i = 0; i < awardnum; ++i)
            {
                ChargeActItemDef* item = new ChargeActItemDef();
                std::string need = Utils::makeStr("needrecharge%d", i + 1);
                std::string award = Utils::makeStr("itemawards%d", i + 1);
                item->setRmb(inifile.getValueT(*itr, need, 0));
                item->items = inifile.getValue(*itr, award);
                actDef->awards.push_back(item);
            }
            
            datas.push_back(actDef);
        }
    }
    catch(...)
    {
        log_error("OnceChargeActCfg ERROR !!!");
    }
}


//消耗精力换奖励
void ConsumeFatAwardActCfg::addData(vector<obj_fat_award_activity> FatAward)
{
	
    datas.clear();
    for (int i = 0; i < FatAward.size(); i++) {
        ConsumeFatAwardActDef* actDef = new ConsumeFatAwardActDef();
        int index = FatAward[i].index;
        std::string servers = FatAward[i].server_id;
        StringTokenizer srTokens( servers, ";" );
        for ( int j = 0; j < srTokens.count(); ++j )
        {
            actDef->ServerIds.push_back( Utils::safe_atoi( srTokens[j].c_str() ) );
        }
        std::string platform = FatAward[i].Channel;
        int endTime = FatAward[i].overdate;
        int startTime = FatAward[i].startdate;
        std::string actType = FatAward[i].title;
        std::string actDes = FatAward[i].content;
        int awardType = FatAward[i].award_type;
        
        const int awardnum = FatAward[i].awardnum;
        actDef->setAwardNum(awardnum);
        
        actDef->setAwardType(awardType);
        actDef->setActivityId(index);
        actDef->activityType = actType;
        actDef->setStartTime(startTime);
        actDef->setOverTime(endTime);
        actDef->activityDesc = actDes;
        actDef->platform = platform;
        
        //解析result，取得服务器信息。
        Json::Value jsonValue;
        Json::Reader reader;
        if (!reader.parse(FatAward[i].configJson, jsonValue))
        {
            //err
        }
        
        for (int jsonI = 0; jsonI < jsonValue.size(); jsonI++) {
            int need = xyJsonAsInt(jsonValue[jsonI]["need"]);
            string award = xyJsonAsString(jsonValue[jsonI]["award"]);
            
            ChargeActItemDef* item = new ChargeActItemDef();
            item->setRmb(need);
            item->items = award;
            actDef->awards.push_back(item);
            
        }

        datas.push_back(actDef);
    }
	 
}

AddChargeActCfg ChargeActivityCfgMgr::mAddChargeActCfg;
AddConsumeActCfg ChargeActivityCfgMgr::mAddConsumeActCfg;
AddOnceChargeActCfg ChargeActivityCfgMgr::mAddOnceChargeCfg;
OnceChargeActCfg ChargeActivityCfgMgr::mOnceChargeCfg;
FirstChargeCfg ChargeActivityCfgMgr::mFirstChargeCfg;
ConsumeFatAwardActCfg ChargeActivityCfgMgr::mConsumeFatAwardActCfg;
bool ChargeActivityCfgMgr::load()
{
    //lihengjin
//    mAddChargeActCfg.load( Cfgable::getFullFilePath( "AccumulateRechargeAwardAct.ini" ) );
//    mAddConsumeActCfg.load( Cfgable::getFullFilePath( "AccumulateConsumeAwardAct.ini" ) );
//    mAddOnceChargeCfg.load( Cfgable::getFullFilePath("OperateActOnceRechargeAward.ini") );
//    mOnceChargeCfg.load( Cfgable::getFullFilePath("OperateActRechargeAward.ini") );
//    mFirstChargeCfg.load( Cfgable::getFullFilePath("gift_activity.ini") );
   // mConsumeFatAwardActCfg.load(Cfgable::getFullFilePath("ConsumeFatAwardAct.ini"));
    return true;
}
#pragma mark - 同步竞技场荣誉对应军衔
std::map<unsigned int, std::string> MilitaryCfg::militaryCfgs;
std::map<unsigned int, std::string>::iterator MilitaryCfg::mPreIter;
void MilitaryCfg::load(string fullpath)
{
    try
    {
        GameInifile inifile(fullpath);
        //by lihengjin
        GameInifile MLIniFile;
        if(!(Cfgable::getFullFilePath("synarena_militaryMultiLanguage.ini") == "synarena_militaryMultiLanguage.ini"))
        {
            MLIniFile = GameInifile(Cfgable::getFullFilePath("synarena_militaryMultiLanguage.ini"));
        }
        vector<string> sections;
        inifile.getSections(std::back_inserter(sections));
        std::vector<string>::iterator itr = sections.begin();
        std::vector<string>::iterator itrEnd = sections.end();
        int glory = 0;
        string miliIndex = "";
        string military = "";
        for (; itr != itrEnd; ++itr)
        {
            glory = inifile.getValueT(*itr, "glory", 0);
            miliIndex = inifile.getValue(*itr, "military_id");
            //by lihengjin
            if(inifile.exists(*itr, "military"))
            {
                military = inifile.getValue(*itr, "military");
            }else
            {
                military = MLIniFile.getValue(miliIndex, "desc");
            }
            //military = MLIniFile.getValue(miliIndex, "desc");
            militaryCfgs.insert(std::make_pair(glory, military));
        }
        mPreIter = militaryCfgs.begin();
    }
    catch(...)
    {
        log_error("MilitaryCfg ERROR !!!");
    }
}

std::map<unsigned int, std::string> & MilitaryCfg::GetMilitaryCgfs()
{
    return militaryCfgs;
}
string MilitaryCfg::GetLastMilitary(const unsigned int& glory)
{
    Iterator itrBegin = mPreIter;
    Iterator itrEnd = militaryCfgs.end();
    Iterator itrTemp = mPreIter;
    do
    {
        if (glory >= itrBegin->first)
        {
            itrTemp = itrBegin;
        }
        if (glory < itrBegin->first)
        {
            return itrTemp->second;
        }
        ++itrBegin;
    } while (itrBegin != itrEnd);
    if(itrBegin == itrEnd)
        return itrTemp->second;
    return " ";
}
int MilitaryCfg::getNextMilitaryValue()
{
    Iterator itrPre = mPreIter;
    if (itrPre == militaryCfgs.end()) {
        return itrPre->first;
    }
    else
    {
        return (++itrPre)->first;
    }
}
void MilitaryCfg::setPreMilitaryIter(int value)
{
    if (value < mPreIter->first) {
        mPreIter = militaryCfgs.begin();
    }
    Iterator itrBegin = mPreIter;
    Iterator itrEnd = militaryCfgs.end();
    Iterator itrTemp = mPreIter;
    do
    {
        if (value >= itrBegin->first)
        {
            itrTemp = itrBegin;
        }
        if (value < itrBegin->first)
        {
            mPreIter = itrTemp;
            break;
        }
        ++itrBegin;
    } while (itrBegin != itrEnd);
}
#pragma mark - 同步竞技场兑换商城
vector<SynPvpStoreGoodsDef*> SynPvpStoreCfg::mSynPvpGoods;
void SynPvpStoreCfg::load(string fullpath)
{
    try
    {
        GameInifile inifile(fullpath);
        vector<string> sections;
        inifile.getSections(std::back_inserter(sections));
        std::vector<string>::iterator itr = sections.begin();
        std::vector<string>::iterator itrEnd = sections.end();
        for (; itr != itrEnd; ++itr)
        {
            SynPvpStoreGoodsDef* def = new SynPvpStoreGoodsDef;
            if (def)
            {
                def->index = inifile.getValueT(*itr, "index", 0);
                def->goodsId = inifile.getValueT(*itr, "id", 0);
                def->needMedal = inifile.getValueT(*itr, "consume", 0);
                def->needHonor = inifile.getValueT(*itr, "need_honor", 0);
                //by lihengjin
                if(inifile.exists(*itr, "desc"))
                {
                    def->str_desc = inifile.getValue(*itr, "desc", "");
                }else
                {
                    def->descId = inifile.getValueT(*itr, "desc_id", 0);
                }

                mSynPvpGoods.push_back(def);
            }
        }
    }
    catch(...)
    {
        log_error("SynPvpStoreCfg ERROR !!!");
    }
}
void SynPvpStoreCfg::getSynPvpGoods(vector<SynPvpStoreGoodsDef*>& vec)
{
    if (vec.size())
    {
        vec.clear();
    }
    vec = mSynPvpGoods;
}
SynPvpStoreGoodsDef* SynPvpStoreCfg::getGoodsDefByIdx(int index)
{
    vector<SynPvpStoreGoodsDef*>::iterator itrBegin = mSynPvpGoods.begin();
    vector<SynPvpStoreGoodsDef*>::iterator itrEnd = mSynPvpGoods.end();
    for (; itrBegin != itrEnd; ++itrBegin)
    {
        SynPvpStoreGoodsDef* def = (SynPvpStoreGoodsDef*)*itrBegin;
        if (def->index == index) {
            return def;
        }
    }
    
    return NULL;
}
map<int, string> SynPvpStoreCfgML::mSynPvpGoodsDescs;
void SynPvpStoreCfgML::load(string fullpath)
{
    try
    {
        GameInifile inifile(fullpath);
        vector<string> sections;
        inifile.getSections(std::back_inserter(sections));
        std::vector<string>::iterator itr = sections.begin();
        std::vector<string>::iterator itrEnd = sections.end();
        int index = 0;
        string desc = "";
        for (; itr != itrEnd; ++itr)
        {
            index = inifile.getValueT(*itr, "id", 0);
            desc = inifile.getValue(*itr, "desc");
            mSynPvpGoodsDescs.insert(std::make_pair(index, desc));
        }
    }
    catch(...)
    {
        log_error("SynPvpStoreCfgML ERROR !!!");
    }
}
string SynPvpStoreCfgML::getDescByIdx( int index )
{
    map<int, string>::iterator itrFind = mSynPvpGoodsDescs.find(index);
    if (itrFind != mSynPvpGoodsDescs.end())
    {
        return itrFind->second;
    }
    
    return "";
}

SynPvpFunctionCfgDef SynPvpFuntionCfg::mCfgDef;
vector<SynPvpAward> SynPvpFuntionCfg::mWinnerAwards;
vector<SynPvpAward> SynPvpFuntionCfg::mLoserAwards;
vector<SynPvpAward> SynPvpFuntionCfg::mMatchingFailAwards;

void SynPvpFuntionCfg::load(string fullpath)
{
    try
    {
        GameInifile inifile(fullpath);
        
        mCfgDef.roleHpMultiple = inifile.getValueT("common", "role_hp_multiple", 3.0f);
        mCfgDef.petHpMultiple = inifile.getValueT("common", "pet_hp_multiple", 3.0f);
        
        mCfgDef.maxFightTime = inifile.getValueT("common", "fight_count_down", 120);
        
        //匹配时间，在这个秒数内没匹配到对手就发送匹配失败的奖励
        mCfgDef.maxMatchingSec = inifile.getValueT("common", "max_match_sec", 120);
        
        //从匹配成功的结果返回客户端到正式进入战斗场景的倒数秒数
        mCfgDef.fightingCountDown = inifile.getValueT("common", "fight_countdown", 3);
        
        //每天可以获得奖励的次数
        mCfgDef.canGetAwardTimes = inifile.getValueT("common", "award_times", 0);
        
        //考虑胜率因素的起始战斗场次
        mCfgDef.minFightingTimes = inifile.getValueT("common", "winrate_startpoint", 0);
        
        //决斗的最小等级
        mCfgDef.duelLvl = inifile.getValueT("common", "duel_lvl", 0);
        
        //匹配的最小等级
        mCfgDef.matchingLvl = inifile.getValueT("common", "matching_lvl", 0);
        
//        //胜利奖励的勋章
//        mCfgDef.winMedal = inifile.getValueT("award", "win_medal", 0);
//        
//        //失败奖励的勋章，可以不奖
//        mCfgDef.loseMedal = inifile.getValueT("award", "lose_medal", 0);
//        
//        //胜利获得的荣誉值
//        mCfgDef.winHonor = inifile.getValueT("award", "win_honor", 0);
//        
//        //失败获得的荣誉值
//        mCfgDef.losHonor = inifile.getValueT("award", "lose_honor", 0);
        
        //胜利的奖励
        string winMedalStr = inifile.getValue("award", "win_medal", "");
        string winHonorStr = inifile.getValue("award", "win_honor", "");
        
        StringTokenizer winMedalToken(winMedalStr,";");
        StringTokenizer winHonorToken(winMedalStr,";");
        
        for (int i = 0; i < winMedalToken.count() && i < winHonorToken.count(); i++) {
            SynPvpAward award;
            award.medal = Utils::safe_atoi(winMedalToken[i].c_str());
            award.honor = Utils::safe_atoi(winHonorToken[i].c_str());
            mWinnerAwards.push_back(award);
        }
        
        //失败的奖励
        string losMedalStr = inifile.getValue("award", "lose_medal", "");
        string losHonorStr = inifile.getValue("award", "lose_honor", "");
        
        StringTokenizer losMedalToken(losMedalStr,";");
        StringTokenizer losHonorToken(losHonorStr,";");
        
        for (int i = 0; i < losMedalToken.count() && i < losHonorToken.count(); i++) {
            SynPvpAward award;
            award.medal = Utils::safe_atoi(losMedalToken[i].c_str());
            award.honor = Utils::safe_atoi(losHonorToken[i].c_str());
            mLoserAwards.push_back(award);
        }
        
        //匹配失败的奖励
        string matchingFailMedalStr = inifile.getValue("award", "m_fail_medal", "");
        string matchingFailHonorStr = inifile.getValue("award", "m_fail_honor", "");
        
        StringTokenizer matchFailMedalToken(matchingFailMedalStr, ";");
        StringTokenizer matchFailHonorToken(matchingFailHonorStr, ";");
        
        for (int i = 0; i < matchFailMedalToken.count() && i < matchFailHonorToken.count(); i++) {
            SynPvpAward award;
            award.medal = Utils::safe_atoi(matchFailMedalToken[i].c_str());
            award.honor = Utils::safe_atoi(matchFailHonorToken[i].c_str());
            mMatchingFailAwards.push_back(award);
        }
        
        string secGapStr = inifile.getValue("common", "sec_section", "");
        string batGapStr = inifile.getValue("common", "battle_section", "");
        string winGapStr = inifile.getValue("common", "winrate_section", "");
        string honorGapStr = inifile.getValue("common", "honor_section", "");
        
        StringTokenizer secToken(secGapStr, ";");
        StringTokenizer batToken(batGapStr, ";");
        StringTokenizer winToken(winGapStr, ";");
        StringTokenizer honorToken(honorGapStr, ";");
        
        for (int i = 0; i < secToken.count(); i++) {
            int sec = Utils::safe_atoi(secToken[i].c_str());
            mCfgDef.matchSecGap.push_back(sec);
        }
        
        for (int i = 0; i < batToken.count(); i++) {
            int bat = Utils::safe_atoi(batToken[i].c_str());
            mCfgDef.matchBattleGapPercent.push_back(bat);
        }
        
        for (int i = 0; i < winToken.count(); i++) {
            int win = Utils::safe_atoi(winToken[i].c_str());
            mCfgDef.matchWinrateGapPercent.push_back(win);
        }
        
        for (int i = 0; i < honorToken.count(); i++) {
            int honor = Utils::safe_atoi(honorToken[i].c_str());
            mCfgDef.matchHonorGap.push_back(honor);
        }
        
        int secVsize = mCfgDef.matchSecGap.size();
        
        if (mCfgDef.matchBattleGapPercent.size() < secVsize ||
            mCfgDef.matchWinrateGapPercent.size() < secVsize ||
            mCfgDef.matchHonorGap.size() < secVsize) {
            
            log_error("SynPvpFuntionCfg error");
            assert(0);
        }
        
    }
    catch(...)
    {
        log_error("SynPvpFuntionCfg error !!!");
    }
}

void SynPvpFuntionCfg::getBattleAndWinrateGapPercent(int matchSec, int &battleGapPercent, int &winrateGapPercent, int &honorGapPercent)
{
    int index = 0;
    
    int secVsize = mCfgDef.matchSecGap.size();
    
    int battleVsize = mCfgDef.matchBattleGapPercent.size();
    
    int winrateVsize = mCfgDef.matchWinrateGapPercent.size();
    
    int honorrateSize = mCfgDef.matchHonorGap.size();
    
    for (int i = 0; i < secVsize; i++) {
        
        index = i;
        
        if (matchSec <= mCfgDef.matchSecGap[i]) {
            break;
        }
    }
    
    if (index < battleVsize && index < winrateVsize && index < honorrateSize) {
        battleGapPercent = mCfgDef.matchBattleGapPercent[index];
        winrateGapPercent = mCfgDef.matchWinrateGapPercent[index];
        honorGapPercent = mCfgDef.matchHonorGap[index];
    }
}

SynPvpAward SynPvpFuntionCfg::getSynPvpWinAward(int matchingTimes)
{
    if (mWinnerAwards.size() < 1) {
        SynPvpAward award;
        return award;
    }
    
    if (matchingTimes < mWinnerAwards.size()) {
        return mWinnerAwards[matchingTimes];
    }
    else{
        return mWinnerAwards[mWinnerAwards.size() - 1];
    }
}

SynPvpAward SynPvpFuntionCfg::getSynPvpLoserAward(int matchingTimes)
{
    if (mLoserAwards.size() < 1) {
        SynPvpAward award;
        return award;
    }
    
    if (matchingTimes < mLoserAwards.size()) {
        return mLoserAwards[matchingTimes];
    }
    else{
        return mLoserAwards[mLoserAwards.size() - 1];
    }
}

SynPvpAward SynPvpFuntionCfg::getSynPvpMatchingFailAward(int matchingTimes)
{
    if (mMatchingFailAwards.size() < 1) {
        SynPvpAward award;
        return award;
    }
    
    if (matchingTimes < mMatchingFailAwards.size()) {
        return mMatchingFailAwards[matchingTimes];
    }
    else{
        return mMatchingFailAwards[mMatchingFailAwards.size() - 1];
    }
}

/********
 
 公会宝藏战的副本配置
 
 *******/
map<int, TreasureCopyListCfgDef*> TreasureListCfg::mCfg;

void TreasureListCfg::load(string fullPath)
{
    GameInifile inifile(fullPath);
    //by lihengjin
    GameInifile MLIniFile;
    if(!(Cfgable::getFullFilePath("treasureCopyListMultiLanguage.ini") == "treasureCopyListMultiLanguage.ini"))
    {
        MLIniFile = GameInifile(Cfgable::getFullFilePath("treasureCopyListMultiLanguage.ini"));
    }
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        TreasureCopyListCfgDef* def = new TreasureCopyListCfgDef;
        
        def->sceneNum = Utils::safe_atoi((*iter).c_str());
        def->sceneMod = inifile.getValueT(*iter, "scenemod", 0);
        def->treasureType = (TreasureCopyType)(inifile.getValueT(*iter, "copy_type", 1));
        def->freshMonsterInterval = inifile.getValueT(*iter, "fresh_monster_interval", 0);
        def->maxPlayerCount = inifile.getValueT(*iter, "max_player", 0);
        def->maxGuildMemCount = inifile.getValueT(*iter, "max_guild_mem", 0);
        string nameId = inifile.getValue(*iter, "scene_name_id", "");
        //by lihengjin
        std::string str_name;
        if(inifile.exists(*iter, "scene_name"))
        {
            str_name = inifile.getValue(*iter, "scene_name");
        }else
        {
            str_name = MLIniFile.getValue(nameId, "desc", "");
        }
        def->sceneName = str_name;
        
        def->monsterNum = inifile.getValueT(*iter, "monster_num", 0);
        def->monsterMod = inifile.getValueT(*iter, "monster_mod", 0);
        def->monsterScore = inifile.getValueT(*iter, "monster_score", 0);
        def->playerRate = inifile.getValueT(*iter, "player_rate", 1.0f);
        def->monsterAssistRate = inifile.getValueT(*iter, "player_assist_rate", 0.2f);
        def->playerAssistRate = inifile.getValueT(*iter, "monster_assist_rate", 0.2f);
        mCfg.insert(make_pair(def->sceneNum, def));
    }
    
}

TreasureCopyListCfgDef* TreasureListCfg::getTreasureCopyCfg(int copyNum)
{
    map<int, TreasureCopyListCfgDef*>::iterator iter = mCfg.find(copyNum);
    
    if (iter == mCfg.end()) {
        return NULL;
    }
    
    return iter->second;
}

struct GuildTreasureFightCfgDef GuildTreasureFightCfg::mCfg;

void GuildTreasureFightCfg::load(string fullPath)
{
    GameInifile inifile(fullPath);
    mCfg.minCopyCount = inifile.getValueT("common", "min_copy_count", 5);
    mCfg.copyCountDeno = inifile.getValueT("common", "copy_count_deno", 1);
    mCfg.activityDuration = inifile.getValueT("common", "act_duration", 100);
    mCfg.leaveCopyRecoverSec = inifile.getValueT("common", "leave_recover_sec", 0);
    mCfg.deathRecoverSec = inifile.getValueT("common", "death_recover_sec", 0);
    mCfg.manorCount = inifile.getValueT("common", "manor_count", 0);
    
    mCfg.protectTime = inifile.getValueT("common", "protect_time", 0);
    mCfg.roleHpRate = inifile.getValueT("common", "hp_rate", 1.0f);
}

int GuildTreasureFightCfg::getRecoverSec(bool isDeath)
{
    if (isDeath) {
        return mCfg.deathRecoverSec;
    }
    else{
        return mCfg.leaveCopyRecoverSec;
    }
}

#pragma mark -
#pragma mark 图腾
SysTotemGroup::SysTotemGroup()
{
    setAtk(0);
    setAddMaxHp(0);
    setDef(0);
    setDodge(0);
    setCri(0);
    setHit(0);
}
int SysTotemGroup::getPropAddLevel()
{
    for (int i = 0; i < (int)totemVec.size(); ++i)
    {
        return totemVec[i].totemLvl;
    }
    return 0;
}
void SysTotemInfo::insertInfoLvl(SysTotemLvl &stl)
{
	mLvl.insert(make_pair(stl.lvl, stl));
}
SysTotemLvl::SysTotemLvl()
{
    setAtk(0);
    setAddMaxHp(0);
    setDef(0);
    setDodge(0);
    setCri(0);
    setHit(0);
}
SysTotemLvl * SysTotemInfo::getLvl(int lvl)
{
	map<int, SysTotemLvl>::iterator it = mLvl.find(lvl);
	if (it == mLvl.end()) {
		return NULL;
	}
	return &it->second;
}


bool SysTotemGroup::isMeetGroup(map<int, int>& totem)
{
	for (vector<SysTotemGroupTotem>::iterator it = totemVec.begin();
		 it != totemVec.end(); it++) {
		map<int, int>::iterator temp = totem.find(it->totemId);
		if (temp == totem.end()) {
			return false;
		}
		if (temp->second < it->totemLvl) {
			return false;
		}
	}
	return true;
}

void SysTotemCfg::readGroup(GameInifile &inifile)
{
	mGroup.clear();
	try
    {
		std::list<string> sections;
		std::list<std::string>::iterator it;
        inifile.getSections(std::back_inserter(sections));
        for (it = sections.begin(); it != sections.end(); it++)
        {
			SysTotemGroup totem;
            //by lihengjin 由于多语言表会删掉，所以groupnme_id改成index，配置表index和groupname_id保持一致
			totem.groupId = inifile.getValueT(*it, "index", 0);
			totem.name = inifile.getValue(*it, "groupname_id");
			StringTokenizer stTotem(inifile.getValue(*it, "totem_id"), ";");

			for (int i = 0; i < stTotem.count(); i++) {
				SysTotemGroupTotem t;
				t.totemId = Utils::safe_atoi(stTotem[i].c_str());
				t.totemLvl = inifile.getValueT(*it, "totem_lvl", 0);
				
				assert(t.totemId > 0 && t.totemLvl > 0);
				
				totem.totemVec.push_back(t);
			}
			
			totem.setAtk(inifile.getValueT(*it, "atk", 0));
			totem.setDef(inifile.getValueT(*it, "def", 0));
			totem.setAddMaxHp(inifile.getValueT(*it, "hp", 0));
			totem.setHit(inifile.getValueT(*it, "hit", 0.0));
			totem.setDodge(inifile.getValueT(*it, "dodge", 0.0));
			totem.setCri(inifile.getValueT(*it, "cri", 0.0));
			
			//assert(totem.totemVec.size() > 0);
			
			mGroup.insert(make_pair(totem.groupId, totem));
		}
	}
	catch(...)
	{
		log_error("SysTotemCfg readGroup ERROR !!!");
	}
}

void SysTotemCfg::readInfo(GameInifile &infoFile, GameInifile &lvlFile)
{
	mInfo.clear();
	try
    {
		// TotemInfo.ini 文件解释
		{	
			std::list<string> sections;
			std::list<std::string>::iterator it;
			infoFile.getSections(std::back_inserter(sections));
			for (it = sections.begin(); it != sections.end(); it++)
			{
				SysTotemInfo info;
				info.totemId = infoFile.getValueT(*it, "totemID", 0);
				info.name = infoFile.getValue(*it, "totem_name_id");
				info.icon = infoFile.getValue(*it, "totem_icon");
				info.desc = infoFile.getValue(*it, "totem_desc_id");
				
				assert(info.totemId > 0
					   && !info.name.empty()
					   && !info.icon.empty()
					   && !info.desc.empty());
				
				mInfo.insert(make_pair(info.totemId, info));
			}
			// 检查长度
			assert(mInfo.size() == sections.size());
		}
		
		//　TotemLvl.ini 文件解释
		{
			std::list<string> sections;
			std::list<std::string>::iterator it;
			lvlFile.getSections(std::back_inserter(sections));
			for (it = sections.begin(); it != sections.end(); it++)
			{
				vector<int> totemIds;
				strToNumVec(lvlFile.getValue(*it, "totemID").c_str(), ";", totemIds);
				int maxLvl = lvlFile.getValueT(*it, "max_totem_lvl", 0);
				
				assert(!totemIds.empty() && maxLvl > 0);
				
				vector<int> needNums;
				strToNumVec(lvlFile.getValue(*it, "need_num").c_str(), ";", needNums);
				vector<int> stages;
				strToNumVec(lvlFile.getValue(*it, "stage").c_str(), ";", stages);
				vector<int> ignoreMonsters;
				strToNumVec(lvlFile.getValue(*it, "ignoreMonster").c_str(), ";", ignoreMonsters);
				vector<int> lvls;
				strToNumVec(lvlFile.getValue(*it, "lvl").c_str(), ";", lvls);
				vector<int> quas;
				strToNumVec(lvlFile.getValue(*it, "qua").c_str(), ";", quas);
				vector<int> atks;
				strToNumVec(lvlFile.getValue(*it, "atk").c_str(), ";", atks);
				vector<int> defs;
				strToNumVec(lvlFile.getValue(*it, "def").c_str(), ";", defs);
				vector<int> hps;
				strToNumVec(lvlFile.getValue(*it, "hp").c_str(), ";", hps);
				vector<double> hits;
				strToFloatVec(lvlFile.getValue(*it, "hit").c_str(), ";", hits);
				vector<double> dodges;
				strToFloatVec(lvlFile.getValue(*it, "dodge").c_str(), ";", dodges);
				vector<double> cris;
				strToFloatVec(lvlFile.getValue(*it, "cri").c_str(), ";", cris);
				
				//　一至性检查
				assert(needNums.size() == maxLvl
					   && ignoreMonsters.size() == maxLvl
					   && stages.size() == maxLvl
					   && lvls.size() == maxLvl
					   && quas.size() == maxLvl
					   && atks.size() == maxLvl
					   && defs.size() == maxLvl
					   && hps.size() == maxLvl
					   && hits.size() == maxLvl
					   && dodges.size() == maxLvl
					   && cris.size() == maxLvl);
				
				
				for (int i = 0; i < maxLvl; i++) {
					SysTotemLvl stl;
					
					stl.lvl = i + 1;
					stl.needUpNum = needNums[i];
					stl.ignoreMonster = ignoreMonsters[i];
					stl.immolationQua = quas[i];
					stl.immolationStage = stages[i];
					stl.immolationLvl = lvls[i];
					stl.setAtk(atks[i]);
					stl.setDef(defs[i]);
					stl.setAddMaxHp(hps[i]);
					stl.setHit(hits[i]);
					stl.setDodge(dodges[i]);
					stl.setCri(cris[i]);
					
					for (vector<int>::iterator it = totemIds.begin(); it != totemIds.end(); it++) {
						stl.totemId = *it;
						stl.monsterId = *it;
						
						SysTotemInfo & info = mInfo[stl.totemId];
						//　验证
						assert(info.totemId > 0
							   && info.getLvlCount() < maxLvl);
						
						info.insertInfoLvl(stl);
					}
					
				}	
			}
	
		}
	}
	catch(...)
	{
		log_error("SysTotemCfg readInfo ERROR !!!");
	}
}

void SysTotemCfg::configCheck()
{
	for (map<int, SysTotemInfo>::iterator it = mInfo.begin(); it != mInfo.end(); it++) {
		assert(it->second.getLvlCount() > 0);
	}
	for (map<int, SysTotemGroup>::iterator it = mGroup.begin(); it != mGroup.end(); it++) {
		for (vector<SysTotemGroupTotem>::iterator sit = it->second.totemVec.begin();
			 sit != it->second.totemVec.end();
			 sit++) {
			
			assert(mInfo.find(sit->totemId) != mInfo.end());
		}
	}
}


SysTotemCfg SysTotemCfgMgr::sCfg;
void SysTotemCfgMgr::load()
{
	try
    {
		{
			GameInifile inifile(Cfgable::getFullFilePath("TotemGroup.ini"));
			sCfg.readGroup(inifile);
		}
		{
			GameInifile infoFile(Cfgable::getFullFilePath("TotemInfo.ini"));
			GameInifile lvlFile(Cfgable::getFullFilePath("TotemLvl.ini"));
			sCfg.readInfo(infoFile, lvlFile);
		}
		sCfg.configCheck();
    }
    catch(...)
    {
        log_error("SysTotemCfgMgr ERROR !!!");
    }
	
}



SysTotemLvl * SysTotemCfg::getInfoLvl(int totemId, int lvl)
{
	SysTotemInfo *info = this->getInfo(totemId);
	if (NULL == info) {
		return NULL;
	}
    SysTotemLvl *infoLvl = info->getLvl(lvl);
	if (NULL == infoLvl) {
		return NULL;
	}
	return infoLvl;
}

SysTotemInfo * SysTotemCfg::getInfo(int totemId)
{
	map<int, SysTotemInfo>::iterator it = mInfo.find(totemId);
	if (it == mInfo.end()) {
		return NULL;
	}
	return &it->second;
}

GroupIter SysTotemCfg::findGroupById(int groupId)
{
    return mGroup.find(groupId);
}

void SysTotemCfg::calculate(BattleProp& batprop, map<int, int>& info)
{
	BattleProp bat;
	// 图腾加成
	for (map<int, int>::iterator it = info.begin(); it != info.end(); it++) {
		SysTotemLvl *lvl = this->getInfoLvl(it->first, it->second);
		if (NULL == lvl) {
			continue;
		}
		bat.setAtk(bat.getAtk() + lvl->getAtk());
		bat.setDef(bat.getDef() + lvl->getDef());
		bat.setMaxHp(bat.getMaxHp() + lvl->getAddMaxHp());
		bat.setHit(bat.getHit() + lvl->getHit());
		bat.setDodge(bat.getDodge() + lvl->getDodge());
		bat.setCri(bat.getCri() + lvl->getCri());
	}
	// 阵列加成
	for (GroupIter it = mGroup.begin(); it != mGroup.end(); it++) {
		if (!it->second.isMeetGroup(info)) {
			continue;
		}
		bat.setAtk(bat.getAtk() + it->second.getAtk());
		bat.setDef(bat.getDef() + it->second.getDef());
		bat.setMaxHp(bat.getMaxHp() + it->second.getAddMaxHp());
		bat.setHit(bat.getHit() + it->second.getHit());
		bat.setDodge(bat.getDodge() + it->second.getDodge());
		bat.setCri(bat.getCri() + it->second.getCri());
	}
	batprop += bat;
}

map<int, string> TotemMultiLanguage::totemGroupString;
map<int, string> TotemMultiLanguage::totemInfoString;
void TotemMultiLanguage::load()
{
    //lihengjin
    GameInifile gpFile;
    GameInifile ifFile;
    if(!(Cfgable::getFullFilePath("TotemGroupMultiLanguage.ini") == "TotemGroupMultiLanguage.ini"))
    {
        gpFile = GameInifile(Cfgable::getFullFilePath("TotemGroupMultiLanguage.ini"));
    }
    //GameInifile gpFile(Cfgable::getFullFilePath("TotemGroupMultiLanguage.ini"));
    if(!(Cfgable::getFullFilePath("TotemInfoMultiLanguage.ini") == "TotemInfoMultiLanguage.ini"))
    {
        ifFile = GameInifile(Cfgable::getFullFilePath("TotemInfoMultiLanguage.ini"));
    }
    //GameInifile ifFile(Cfgable::getFullFilePath("TotemInfoMultiLanguage.ini"));
    totemGroupString.clear();
    totemInfoString.clear();
    try
    {
		std::list<string> sections;
		std::list<std::string>::iterator it;
        gpFile.getSections(std::back_inserter(sections));
        for (it = sections.begin(); it != sections.end(); it++)
        {
            int id = gpFile.getValueT(*it, "id", 0);
			string desc = gpFile.getValue(*it, "desc");
			totemGroupString.insert(make_pair(id, desc));
		}
        
        sections.clear();
        ifFile.getSections(std::back_inserter(sections));
        for (it = sections.begin(); it != sections.end(); it++)
        {
            int id = ifFile.getValueT(*it, "id", 0);
			string desc = ifFile.getValue(*it, "desc");
			totemInfoString.insert(make_pair(id, desc));
		}
	}
	catch(...)
	{
		log_error("TotemMultiLanguage ERROR !!!");
	}

}
string TotemMultiLanguage::getGroupString(int idx)
{
    if(totemGroupString.find(idx) != totemGroupString.end())
        return totemGroupString[idx];
    else
    {
        if( SysTotemCfgMgr::sCfg.findGroupById( idx ) != SysTotemCfgMgr::sCfg.groupEnd() )
        {
            return SysTotemCfgMgr::sCfg.findGroupById( idx )->second.name;
        }
    }
    return "";
}
string TotemMultiLanguage::getInfoString(int idx)
{
    if(totemInfoString.find(idx) != totemInfoString.end())
        return totemInfoString[idx];
    else
    {
        SysTotemInfo* info = SysTotemCfgMgr::sCfg.getInfo( idx );
        if( info )
            return info->name;
    }
    return "";
}
#pragma mark -

#pragma mark 副本额外奖励
CopyExtraAwardCfg::CopyExtraAwardTd CopyExtraAwardCfg::mCfg;
void CopyExtraAwardCfg::load()
{
	{
		GameInifile inifile(Cfgable::getFullFilePath("copyExtraAwardConfig.ini"));
		read(inifile);
	}
}

void CopyExtraAwardCfg::read(GameInifile &inifile)
{
	std::list<string> sections;
	std::list<std::string>::iterator it;
	inifile.getSections(std::back_inserter(sections));
	for (it = sections.begin(); it != sections.end(); it++)
	{
		CopyExtraAward *def = new CopyExtraAward();
		def->index = Utils::safe_atoi((*it).c_str());
		def->copyId = inifile.getValueT(*it, "copy_id", 0);
		def->beginTime = Utils::parseDate(inifile.getValue(*it, "begin_time").c_str());
		def->endTime = Utils::parseDate(inifile.getValue(*it, "end_time").c_str());
        def->afterOpenServerDays = inifile.getValueT(*it, "after_openserver_days", 0);
		def->multiple = inifile.getValueT(*it, "multiple", 0);
		def->extraItems = inifile.getValue(*it, "extra_items");
		assert(def->index > 0 && def->copyId > 0 && def->beginTime > 0 && def->endTime > 0);
			
		mCfg[def->copyId].push_back(def);
	}
}

CopyExtraAward * CopyExtraAwardCfg::getExtraAward(int copyId)
{
	Iterator resultIt = mCfg.find(copyId);
	if (resultIt == mCfg.end()) {
		return NULL;
	}
	int now = time(NULL);
	vector<CopyExtraAward*>::iterator it;
	for (it = resultIt->second.begin(); it != resultIt->second.end(); it++) {
		if ((*it)->beginTime < now && now < (*it)->endTime) {
			return *it;
		}
	}
	return NULL;
}


bool TwistEggCommonDef::randGoodsGet(
									 vector<const TwistEggGoodsDef*>& goods,
									 const vector<TwistEggGoodsDef*>& store,
									 unsigned int num) const
{
	if (num <= 0) {
		return false;
	}
	
	vector<int> prop;
	vector<TwistEggGoodsDef*>::const_iterator it;
	for (it = store.begin(); it != store.end(); it++) {
		prop.push_back((*it)->prop);
	}
	
	if (prop.empty()) {
		return false;
	}

	vector<int> index(num, 0);
	getRands(prop.data(), prop.size(), num, index.data(), true);
	for (vector<int>::iterator it = index.begin() ; it != index.end(); it++) {
		goods.push_back(store[*it]);
	}
	return true;
}


bool TwistEggCommonDef::randGoodsEven(vector<const TwistEggGoodsDef*> &goods) const
{
	int nomalNum = evenTwistNum - evenTwistSpecieNum;
	this->randGoodsGet(goods, specieGoods, evenTwistSpecieNum);
	this->randGoodsGet(goods, generalGoods, nomalNum);

	return (evenTwistNum > 0 && goods.size() == evenTwistNum) ? true : false;
}


bool TwistEggCommonDef::randGoods(vector<const TwistEggGoodsDef*> &goods, bool isFirst, bool isEven) const
{
	if (isEven) {
		return this->randGoodsEven(goods);
	}
	else if(isFirst)
	{
		return this->randGoodsGet(goods, firstGoods, 1);
	}

	return this->randGoodsGet(goods, generalGoods, 1);
}

void TwistEggCommonCfg::readGoods(GameInifile &inifile)
{
	std::list<string> sections;
	std::list<std::string>::iterator it;
	inifile.getSections(std::back_inserter(sections));
	for (it = sections.begin(); it != sections.end(); it++)
	{
		TwistEggGoodsDef *def = new TwistEggGoodsDef();
		def->index = inifile.getValueT(*it, "id", 0);
		def->belongs = inifile.getValue(*it, "store");
		def->goodsId = inifile.getValueT(*it, "goodsId", 0);
		def->goodsNum = inifile.getValueT(*it, "goodsNum", 0);
		def->prop = inifile.getValueT(*it, "prop", 0);
		def->specieView = inifile.getValue(*it, "specieView");
		def->desc = inifile.getValue(*it, "desc");
		
		def->generalSw = inifile.getValueT(*it, "generalSw", false);
		def->specieSw = inifile.getValueT(*it, "specieSw", false);
		def->firstSw = inifile.getValueT(*it, "firstSw", false);
		def->broadcast = inifile.getValueT(*it, "broadcast", false);
		
		assert(def->index > 0
			   && !def->belongs.empty()
			   && def->goodsId > 0
			   && def->goodsNum > 0);
		
		assert(mGoodIndexs.find(def->index) == mGoodIndexs.end());
		
		mGoods[def->belongs].push_back(def);
		mGoodIndexs[def->index] = def;
	}
}


TwistEggActivityDef * TwistEggCommonCfg::getDef(int belongs, int time)
{
	TwistEggActivityDef* temp = NULL;
	map<int, TwistEggActivityDef>::iterator it;
	for (it = mActivitys.begin(); it != mActivitys.end(); it++) {
		if (it->second.belongs != belongs) {
			continue;
		}
		if (time < it->second.beginTm || time > it->second.endTm) {
			continue;
		}
		// 替换到最接近开启的配置
		if (NULL == temp || temp->beginTm > it->second.beginTm) {
			temp = &it->second;
		}
	}
	
	return temp;
}


void TwistEggCommonCfg::getTwistEgg(Iterator& begin, Iterator& end, int time)
{
	for (map<int, TwistEggCommonDef>::iterator it = mCommons.begin(); it != mCommons.end(); it++) {
		this->updateDef(it->second, time);
	}
	begin = mCommons.begin();
	end = mCommons.end();
}

void TwistEggCommonCfg::updateDef(TwistEggCommonDef& base, int time)
{
	// 未过期
	if (!base.checkTimeout(time)) {
		return;
	}
	// 过期了, 但当前时间没有配置了
	TwistEggActivityDef* def = this->getDef(base.index, time);
	if (NULL == def) {
		return;
	}
	// 用满足的活动配置覆盖当前的配置
	this->setDef(base, def);
	return;
}


const TwistEggCommonDef* TwistEggCommonCfg::getTwistEgg(int index, int time)
{
	map<int, TwistEggCommonDef>::iterator it = mCommons.find(index);
	if (it == mCommons.end()) {
		return NULL;
	}
	this->updateDef(it->second, time);
	return &it->second;
}

TwistEggActivityDef* TwistEggCommonCfg::getNotOpenActivity(int time)
{
	TwistEggActivityDef* temp = NULL;
	map<int, TwistEggActivityDef>::iterator it;
	for (it = mActivitys.begin(); it != mActivitys.end(); it++) {
		if (time > it->second.beginTm) {
			continue;
		}
		
		// 替换到最接近开启的配置
		if (NULL == temp || temp->beginTm > it->second.beginTm) {
			temp = &it->second;
		}
	}
	return temp;
}

void TwistEggCommonCfg::getNotOpenActivity(vector<const TwistEggActivityDef*> vec, int time)
{
	map<int, TwistEggActivityDef>::iterator it;
	for (it = mActivitys.begin(); it != mActivitys.end(); it++) {
		if (time > it->second.beginTm) {
			continue;
		}
		vec.push_back(&it->second);
	}
}

const TwistEggGoodsDef* TwistEggCommonCfg::getGoods(int index)
{
	map<int, TwistEggGoodsDef*>::iterator it = mGoodIndexs.find(index);
	if (it != mGoodIndexs.end()) {
		return it->second;
	}
	return NULL;
}

void TwistEggCommonCfg::setGoodsStore(TwistEggCommonDef& base, const char* goodsStrore)
{
	base.firstGoods.clear();
	base.generalGoods.clear();
	base.specieGoods.clear();
	
	map<string, vector<TwistEggGoodsDef*> >::iterator it = mGoods.find(goodsStrore);
	if (it == mGoods.end()) {
		return;
	}
	
	vector<TwistEggGoodsDef*>::iterator goodsIt;
	for (goodsIt = it->second.begin(); goodsIt != it->second.end(); goodsIt++) {
		TwistEggGoodsDef* temp = *goodsIt;
		if (temp->firstSw) {
			base.firstGoods.push_back(temp);
		}
		if (temp->generalSw) {
			base.generalGoods.push_back(temp);
		}
		if (temp->specieSw) {
			base.specieGoods.push_back(temp);
		}
	}
}

void TwistEggCommonCfg::setDef(TwistEggCommonDef& base, TwistEggActivityDef* def)
{
	base.activityId = def->id;
	base.desc = def->desc;
	base.needItemId = def->needItemId;
	base.needItemNum = def->needItemNum;
	base.rmb = def->rmb;
	base.gold = def->gold;
	base.beginTm = def->beginTm;
	base.endTm = def->endTm;
	base.image = def->image;
	
	this->setGoodsStore(base, def->goodsStore.c_str());
	
	strToNumVec(def->viewGoods.c_str(), ";", base.viewGoods);
}



void TwistEggCommonCfg::read(GameInifile &inifile)
{
	std::list<string> sections;
	std::list<std::string>::iterator it;
	inifile.getSections(std::back_inserter(sections));
	for (it = sections.begin(); it != sections.end(); it++)
	{
		TwistEggCommonDef def;
		def.index = inifile.getValueT(*it, "ID", 0);
		def.name = inifile.getValue(*it, "name");
		def.isTime = inifile.getValueT(*it, "isTime", false);
		def.isTimesLimit = inifile.getValueT(*it, "isTimesLimit", true);
		def.freeNum = inifile.getValueT(*it, "freeNum", 0);
		def.freeTime = inifile.getValueT(*it, "freeTime", 0);
		def.isEvenTwist = inifile.getValueT(*it, "isEvenTwist", false);
		def.evenTwistNum = inifile.getValueT(*it, "evenTwistNum", 0);
		def.evenTwistDesc = inifile.getValue(*it, "evenTwistDesc");
		def.evenTwistRebate = inifile.getValueT(*it, "evenTwistRebate", 0.0);
		def.evenTwistSpecieNum = inifile.getValueT(*it, "evenTwistSpecieNum", 0);
		
		def.needItemId = inifile.getValueT(*it, "needItem", 0);
		def.needItemNum = inifile.getValueT(*it, "needItemNum", 0);
		
		def.needEvenItemId = inifile.getValueT(*it, "needEvenItem", 0);
		def.needEvenItemNum = inifile.getValueT(*it, "needEvenItemNum", 0);
		
		def.rewardItem = inifile.getValueT(*it, "rewardItem", 0);
		def.rewardItemNum = inifile.getValueT(*it, "rewardItemNum", 0);
		
		def.desc = inifile.getValue(*it, "desc");
		def.beginTm = 0;
		def.endTm = 0;
		def.image = inifile.getValue(*it, "image");
		
		def.gold = inifile.getValueT(*it, "gold", 0);
		def.rmb = inifile.getValueT(*it, "rmb", 0);
		def.lvl = inifile.getValueT(*it, "lvl", 0);
		def.viplvl = inifile.getValueT(*it, "viplvl", 0);
		
		strToNumVec(inifile.getValue(*it, "showPet").c_str(), ";", def.viewGoods);
		
		// 加载物品库
		string goodsStore = inifile.getValue(*it, "goodsStore");
		this->setGoodsStore(def, goodsStore.c_str());
		
		if (def.needItemId > 0) {
			assert(def.needItemNum > 0);
		}
		
		if (!def.isTimesLimit) {
			assert(def.freeTime > 0);
		}
		
		if (def.isEvenTwist) {
			assert(def.evenTwistNum > 0 && def.evenTwistNum >= def.evenTwistSpecieNum);
			assert(!def.evenTwistDesc.empty());
		}
		
		if (def.rewardItem > 0) {
			assert(def.rewardItemNum > 0);
		}
		
		assert(def.index > 0
			   && !def.name.empty()
			   );
		
		mCommons[def.index] = def;
	}
}

void TwistEggCommonCfg::readActivity(GameInifile &inifile)
{
	std::list<string> sections;
	std::list<std::string>::iterator it;
	inifile.getSections(std::back_inserter(sections));
	for (it = sections.begin(); it != sections.end(); it++)
	{
		TwistEggActivityDef def;
		
		def.id = inifile.getValueT(*it, "id", 0);
		def.belongs = inifile.getValueT(*it, "belongs", 0);
		def.gold = inifile.getValueT(*it, "gold", 0);
		def.rmb = inifile.getValueT(*it, "rmb", 0);
		def.beginTm = Utils::parseDate(inifile.getValue(*it, "begin").c_str());
		def.endTm = Utils::parseDate(inifile.getValue(*it, "end").c_str());
		def.needItemId = inifile.getValueT(*it, "needItem", 0);
		def.needItemNum = inifile.getValueT(*it, "needItemNum", 0);
		def.viewGoods = inifile.getValue(*it, "showPet");
		def.desc = inifile.getValue(*it, "desc");
		def.goodsStore = inifile.getValue(*it, "goodsStore");
		def.image = inifile.getValue(*it, "image");
		
		assert(def.id > 0
			   && def.belongs > 0
			   && def.beginTm > 0
			   && def.endTm > 0
			   && !def.goodsStore.empty()
			   );
		
		map<int, TwistEggActivityDef>::iterator it;
		for (it = mActivitys.begin(); it != mActivitys.end(); it++) {
			// 活动配置有时间交叉
			assert(!(def.beginTm > it->second.beginTm && def.beginTm < it->second.endTm)
				   && !(def.endTm > it->second.beginTm && def.endTm < it->second.endTm)
				   );
		}
		
		mActivitys[def.id] = def;
	}
}


TwistEggCommonCfg TwistEggCfgMgr::sCommonCfg;
void TwistEggCfgMgr::load()
{
	{
		GameInifile inifile(Cfgable::getFullFilePath("TwistEggMachinesAward.ini"));
		sCommonCfg.readGoods(inifile);
	}
	{
		GameInifile inifile(Cfgable::getFullFilePath("TwistEggMachinesActivity.ini"));
		sCommonCfg.readActivity(inifile);
	}
	{
		GameInifile inifile(Cfgable::getFullFilePath("TwistEggMachines.ini"));
		sCommonCfg.read(inifile);
	}
}

void TwistEggCfgMgr::getTwistEgg(TwistEggCommonCfg::Iterator& begin,
								 TwistEggCommonCfg::Iterator& end,
								 int time)
{
	sCommonCfg.getTwistEgg(begin, end, time);
}

const TwistEggCommonDef* TwistEggCfgMgr::getTwistEgg(int index, int time)
{
	return sCommonCfg.getTwistEgg(index, time);
}

void MidastouchCfg::read(GameInifile &inifile)
{
	int max = 0;
	std::list<string> sections;
	std::list<std::string>::iterator it;
	inifile.getSections(std::back_inserter(sections));
	for (it = sections.begin(); it != sections.end(); it++)
	{
		MidastouchDef def;
		def.index = inifile.getValueT(*it, "index", 0);
		def.chargeRatio = inifile.getValueT(*it, "chargeRatio", 0.0);
		def.needNum = inifile.getValueT(*it, "needNum", 0);
		def.needCoin = inifile.getValueT(*it, "needCoin", 0);
		def.prop = inifile.getValueT(*it, "prop", 0);
		
		assert(def.index > 0
			   && def.chargeRatio > 0
			   && def.needCoin);
		
		if (mItem.find(def.index) != mItem.end()) {
			log_error("点石成金 index 有重复");
			assert(false);
		}
		
		mItem.insert(make_pair(def.index, def));
		
		// 取最大数
		if (max < def.needNum) max = def.needNum;
	}
	
	// 需要的次数，必须是顺序，不能跳空
	for (int i = 1; i < max; i++) {
		Iterator it = mItem.begin();
		for (; it != mItem.end(); it++) {
			if (i == it->second.needNum) {
				break;
			}
		}
		if (it == mItem.end()) {
			log_error("点石成金 needNum 中间跳空");
			assert(false);
		}
	}
	
	mMaxNum = max;
}

MidastouchDef * MidastouchCfg::randItem(int num)
{
	vector<int> props;
	vector<MidastouchDef*> temp;
	for (Iterator it = mItem.begin(); it != mItem.end(); it++) {
		if (it->second.needNum != num) {
			continue;
		}
		temp.push_back(&it->second);
		props.push_back(it->second.prop);
	}
	
	if (temp.empty()) {
		return NULL;
	}
	
	int index = getRand(props.data(), props.size());
	return temp[index];
}

MidastouchDef * MidastouchCfg::getItem(int index)
{
    Iterator it = mItem.find(index);
    if (it == mItem.end()) {
        return NULL;
    }
    return &it->second;
}

vector<float> MidastouchCfg::getchargeRatioListByTimes(int times)
{
    if (times>getMaxNum())
    {
        times = getMaxNum();
    }
    vector<float> chargeRatioList;
    for (Iterator it = mItem.begin(); it != mItem.end(); it++)
    {
        if (it->second.needNum == times)
        {
            bool isHave = false;
            for(vector<float>::iterator floatIt = chargeRatioList.begin();floatIt != chargeRatioList.end();floatIt++)
            {
                if(*floatIt == it->second.chargeRatio)
                {
                    isHave = true;
                }
            }
            if (!isHave)
            {
                chargeRatioList.push_back(it->second.chargeRatio);
            }
        }
    }
    return chargeRatioList;
}

MidastouchCfg MidastouchCfgMgr::sMidastouchCfg;
void MidastouchCfgMgr::load()
{
    {
        GameInifile inifile(Cfgable::getFullFilePath("MidasTouch.ini"));
        sMidastouchCfg.read(inifile);
    }
}

#pragma mark - 竞技场兑换商城
vector<PvpStoreGoodsDef*> PvpStoreCfg::mPvpGoods;
void PvpStoreCfg::load(string fullpath)
{
    try
    {
        GameInifile inifile(fullpath);
        vector<string> sections;
        inifile.getSections(std::back_inserter(sections));
        std::vector<string>::iterator itr = sections.begin();
        std::vector<string>::iterator itrEnd = sections.end();
        for (; itr != itrEnd; ++itr)
        {
            PvpStoreGoodsDef* def = new PvpStoreGoodsDef;
            if (def)
            {
                def->index = inifile.getValueT(*itr, "index", 0);
                def->goodsId = inifile.getValueT(*itr, "id", 0);
                def->needPoints = inifile.getValueT(*itr, "consume", 0);
                
                //by lihengjin
                if(inifile.exists(*itr, "desc"))
                {
                    def->str_desc = inifile.getValue(*itr, "desc", "");
                }else
                {
                    def->descId = inifile.getValueT(*itr, "desc_id", 0);
                }
                
                mPvpGoods.push_back(def);
            }
        }
    }
    catch(...)
    {
        log_error("PvpStoreCfg ERROR !!!");
    }
}

void PvpStoreCfg::getPvpGoods(vector<PvpStoreGoodsDef*>& vec)
{
    if (vec.size())
    {
        vec.clear();
    }
    vec = mPvpGoods;
}

PvpStoreGoodsDef* PvpStoreCfg::getGoodsDefByIdx(int index)
{
    vector<PvpStoreGoodsDef*>::iterator itrBegin = mPvpGoods.begin();
    vector<PvpStoreGoodsDef*>::iterator itrEnd = mPvpGoods.end();
    for (; itrBegin != itrEnd; ++itrBegin)
    {
        PvpStoreGoodsDef* def = (PvpStoreGoodsDef*)*itrBegin;
        if (def->index == index) {
            return def;
        }
    }
    
    return NULL;
}

map<int, string> PvpStoreCfgML::mPvpGoodsDescs;
void PvpStoreCfgML::load(string fullpath)
{
    try
    {
        GameInifile inifile(fullpath);
        vector<string> sections;
        inifile.getSections(std::back_inserter(sections));
        std::vector<string>::iterator itr = sections.begin();
        std::vector<string>::iterator itrEnd = sections.end();
        int index = 0;
        string desc = "";
        for (; itr != itrEnd; ++itr)
        {
            index = inifile.getValueT(*itr, "id", 0);
            desc = inifile.getValue(*itr, "desc");
            mPvpGoodsDescs.insert(std::make_pair(index, desc));
        }
    }
    catch(...)
    {
        log_error("SynPvpStoreCfgML ERROR !!!");
    }
}
string PvpStoreCfgML::getDescByIdx( int index )
{
    map<int, string>::iterator itrFind = mPvpGoodsDescs.find(index);
    if (itrFind != mPvpGoodsDescs.end())
    {
        return itrFind->second;
    }
    
    return "";
}

#pragma mark -



// class CsPvpGroupCfgDef
// {
// public:
//     vector<int> mServerIds;
//     int mCloseWeekBegin;
//     int mCloseWeekEnd;
// };



