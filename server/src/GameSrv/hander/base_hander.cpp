
#include "hander_include.h"
#include "Skill.h"
#include "DefConf.h"
#include "DBRolePropName.h"
#include "Pvp.h"
#include "Scene.h"
#include "ItemBag.h"
#include "redis_helper.h"
#include "cmd_def.h"
#include "md5.h"
#include "sdk_inc.h"
#include "json/json.h"
#include "gate.h"
#include "GlobalMsg_def.h"
#include "Global.h"
#include "mail_imp.h"
#include "accountmgr.h"
#include "GuildRole.h"
#include "Constellation.h"
#include "RandName.h"
#include "RolePropVerify.h"
#include "AntiCheating.h"
#include "ip_filter.h"
#include "basedef.h"
#include "MysticalCopy.h"
#include "CheckRequest.h"
#include "WeaponEnchant.h"


#include "mail_imp.h"
#include "offline_action.h"
#include "target.h"
#include "SceneMgr.h"
#include "Wardrobe.h"
#include "lua_helper.h"
#include "Robot.h"
#include "basedef.h"
#include "Retinue.h"
#include "centerClient.h"
#include "cs_pvp/cs_pvp_battle.h"


extern int calcPetBattleForce(BattleProp* battleprop);

class GetSceneDisplayedPets : public Traverser
{
public:
    virtual bool callback(void* traverseobj)
    {
        if (mAck->petinfos.size() >= mDisplayedNum) {
            return false;
        }
        
        Pet* pet = (Pet*)traverseobj;
        
        obj_pet_base_info info;
        info.ownerid = pet->owner;
        info.petid = pet->petid;
        info.petmod = pet->petmod;
        mAck->petinfos.push_back(info);
        
        return true;
    }
    
    ack_scene_displayed_pets* mAck;
    int mDisplayedNum;
    
    GetSceneDisplayedPets(ack_scene_displayed_pets* ack)
    {
        mAck = ack;
        mDisplayedNum = GlobalCfg::sCityPetNum;
    }
};

class GetSceneDisplayedRetinues : public Traverser
{
public:
    virtual bool callback(void* traverseobj)
    {
        if (mAck->retinueinfos.size() >= mDisplayedNum) {
            return false;
        }
        
        Retinue *retinue = (Retinue *)traverseobj;
        
        obj_Retinue_base_info info;
        info.ownerid = retinue->getOwner();
        info.retinueid = retinue->getId();
        info.retinuemod = retinue-> getModId();
        mAck->retinueinfos.push_back(info);
        
        return true;
    }
    
    ack_scene_displayed_retinues* mAck;
    int mDisplayedNum;
    
    GetSceneDisplayedRetinues(ack_scene_displayed_retinues* ack)
    {
        mAck = ack;
        mDisplayedNum = GlobalCfg::sCityRetinueNum;
    }
};



void sendSceneDisplayedPets(Role* role)
{
    Scene* scene = role->getScene();
    if (NULL == scene) {
        return;
    }
    ack_scene_displayed_pets ack;
    GetSceneDisplayedPets traverser(&ack);
    scene->traversePets(&traverser);
    role->send(&ack);
}

void sendSceneDisplayedRetinues(Role* role)
{
    Scene* scene = role -> getScene();
    if(scene == NULL){
        return ;
    }
    ack_scene_displayed_retinues ack;
    GetSceneDisplayedRetinues traverser(&ack);
    scene -> traverseRetinues(&traverser);
    role->send(&ack);
}


void broadcastSceneDisplayedPets(Scene* scene, int roleId)
{
    ack_scene_displayed_pets ack;
    GetSceneDisplayedPets traverser(&ack);
    scene->traversePets(&traverser);
    scene->broadcast(&ack, roleId);
}

void broadcastSceneDisplayRetinues(Scene* scene, int roleId)
{
    ack_scene_displayed_retinues ack;
    GetSceneDisplayedRetinues traverser(&ack);
    scene -> traverseRetinues(&traverser);
    scene -> broadcast(&ack, roleId);
}


void onFirstEnterCity(Role* role)
{
    create_cmd(enterGroup, enterGroup);
    enterGroup->groupid = WORLD_GROUP;
    enterGroup->sid = role->getSessionId();
    sendMessageToGate(0, CMD_MSG, enterGroup, 0);
    
    SSessionDataMgr.setState(role->getSessionId(), kSessionInGame);
    
    //offline item
    string offlineitem = role->loadProp("offlineitems");
    if (!offlineitem.empty())
    {
        vector<string> offlineitems = StrSpilt(offlineitem, ";");
        
        ItemArray items;
        string comefrom = Utils::makeStr("离线物品");
        role->addAwards(offlineitems, items, comefrom.c_str());
        
        role->saveProp("offlineitems", "");
    }
    
    procRoleOfflineAction(role);
}

void onRoleEnterCity(Role* role, Scene* prevScene)
{
    Pet* pet = role->mPetMgr->getActivePet();
    int displayPetNum = GlobalCfg::sCityPetNum;
    
    Retinue *retinue = role -> getRetinueMgr() -> getActiveRetinue();
    int displayRetinueNum = GlobalCfg::sCityRetinueNum;
    
    
    if (prevScene == NULL) {
        onFirstEnterCity(role);
    } else {
        if (pet) {
            int rank = prevScene->removePet(pet);
            if (rank > 0 && rank <= displayPetNum) {
                broadcastSceneDisplayedPets(prevScene, role->getInstID());
            }
        }
        if ( retinue ) {
            int rank = prevScene->removeRetinue(retinue);
            if (rank > 0 && rank <= displayRetinueNum) {
                broadcastSceneDisplayRetinues(prevScene, role -> getInstID());
            }
        }
    }

    Scene* scene = role->getScene();
    if (pet && scene) {
        int rank = scene->addPet(pet);
        if (rank > 0 && rank <= displayPetNum) {
            broadcastSceneDisplayedPets(scene, role->getInstID());
        }
    }
    if (retinue && scene) {
        int rank = scene->addRetinue(retinue);
        if (rank > 0 && rank <= displayRetinueNum) {
            broadcastSceneDisplayRetinues(scene, role->getInstID());
        }
    }
}

bool verifyVersion(int sid)
{
    string version = SSessionDataMgr.getProp(sid, "version");
    if (version != Process::env.getString("version"))
    {
        // added by jianghan
        //kickSession(sid, "客户端版本和服务器版本不一致");
        //kickSession(sid, -2);
        //return false;
        // end add
    }

    return true;
}

hander_msg(req_auto_testtool, req)
{

}}

hander_msg(req_verify_server_info, req)
{
    SSessionDataMgr.setProp(sessionid, "version", req.version);

    ack_verify_server_info ack;
    ack.serverid  = Process::env.getInt("server_id");
    ack.servername = Process::env.getString("server_name");
    ack.version = Process::env.getString("version");

    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_register,req)
{
    if (!verifyVersion(sessionid)){
        return;
    }

    if (req.account.length() < 6||req.password.length() < 6) {
        ack_login ack;
        ack.errorcode = CE_UNKNOWN;
        sendNetPacket(sessionid, &ack);
        return;
    }

    ack_login ack;
    //根据不同的平台给用户帐号加不同的前缀
    string account = "raw_" + req.account;
    if (tryRegisterAccount(req.account.c_str(), "raw", Game::tick)) {
        doRedisCmd("hmset player:%s pwd %s", account.c_str(), req.password.c_str());
        ack.errorcode = 0;
        ack.json = "";
        sendNetPacket(sessionid, &ack);

        SSessionDataMgr.setProp(sessionid, "account", account);
        SSessionDataMgr.setProp(sessionid, "platform", "raw");
        SSessionDataMgr.setState(sessionid, kSessionAuth);
        return;
    }
    else
    {
        //账号已经被注册了
        ack.errorcode = CE_ACCOUNT_UNAVAILABLE;
        sendNetPacket(sessionid, &ack);
    }
}}

hander_msg(req_login, req)
{
    if (!verifyVersion(sessionid)){
        return;
    }
    
    string ip = SSessionDataMgr.getProp(sessionid, "ip");
    bool issuperip = g_IpFilterMgr.isFiltered("super", ip.c_str());
    int testmode = Process::env.getInt("testmode");
    int gmmode = Process::env.getInt("gm_mode");
    if (!issuperip && !testmode && !gmmode)
    {
        return;
    }

    ack_login ack;
    ack.errorcode = CE_UNKNOWN;
    
    string inputAccount = req.account;
    string account;
    size_t pos = inputAccount.find_first_of('-');
    if (pos != string::npos)// && Process::env.getInt("testmode"))
    {
        string roleIdStr;
        string prefix = string(inputAccount.c_str(), pos);
        if (prefix == "id")
        {
            roleIdStr = inputAccount.substr(pos + 1);
            RedisResult result(redisCmd("hget role:%s playeraccount", roleIdStr.c_str()));
            account = result.readStr();
        }
        else if (prefix == "bat")
        {
            int rank = Utils::safe_atoi(inputAccount.substr(pos + 1).c_str());
            if (rank > 0)
            {
                rank--;
                
                RedisResult result(redisCmd("zrevrange paihang:bat %d %d", rank, rank));
                roleIdStr = result.readHash(0);
                result.setData(redisCmd("hget role:%s playeraccount", roleIdStr.c_str()));
                account = result.readStr();
            }
        }
        
        SSessionDataMgr.setProp(sessionid, "raw_id", roleIdStr);
    }
    
    if (account.empty())
    {
        account = "raw_" + req.account;
    }
    
    //根据不同的平台给用户帐号加不同的前缀
    
        ack.json = "";

        SSessionDataMgr.setProp(sessionid, "account", account);
        SSessionDataMgr.setProp(sessionid, "platform", "raw");
        SSessionDataMgr.setState(sessionid, kSessionAuth);
        ack.errorcode = 0;
    
    sendNetPacket(sessionid, &ack);
}}


hander_msg(req_logout,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);

    ack_logout ack;
    ack.errorcode = CE_UNKNOWN;
    if (role != NULL)
    {
        SRoleMgr.destroyRole(role);
        ack.errorcode = 0;
    }

    sendNetPacket(sessionid, &ack);
}}

bool roleLvlSort(obj_role h1, obj_role h2){
	return h1.lvl > h2.lvl;
}

hander_msg(req_getroles,req)
{
    std::string account = SSessionDataMgr.getProp(sessionid, "account");
    if (account.empty()) {
        return;
    }

    ack_getroles ack;

    RedisResult result(redisCmd("hget player:%s roleids",account.c_str()));
    StringTokenizer token(result.readStr(), ";", StringTokenizer::TOK_IGNORE_EMPTY);
    for (StringTokenizer::Iterator iter = token.begin(); iter != token.end(); ++iter) {

        int roleid = atoi((*iter).c_str());

        result.setData(redisCmd("hmget role:%d roletype lvl rolename scenetypeid lastlogin x y weaponfashion bodyfashion proplist homeFashion enchantid enchantlvl awakelvl", roleid));
        int roletype = result.readHash(0, 0);
        int rolelvl = result.readHash(1, 0);
        string rolename = result.readHash(2);
        int scenetypeid = result.readHash(3, 0);
        int lastlogin = result.readHash(4, 0);
        int x = result.readHash(5, 0);
        int y = result.readHash(6, 0);
        int weaponfashion = result.readHash(7, 0);
        int bodyfashion = result.readHash(8, 0);
        string prolist = result.readHash(9);
        int cityBodyFashion = result.readHash(10,0);
        int enchantId = result.readHash(11, 0);
        int enchantLvl = result.readHash(12, 0);
        int awakelvl = result.readHash(13, 0);
        if (rolename.empty()) {
            continue;
        }
        
        Json::Value value;
        Json::Reader reader;
        if (reader.parse(prolist.c_str(), value)){
            int deletetime = value["deletetime"].asInt();
            if (deletetime > 0){
                if (deletetime < Game::tick){
                    DeleteRoleRoleInfoInDB(roleid, sessionid);
                    continue;
                }
                else{
                    Json::FastWriter writer;
                    value["deletetime"] = deletetime - (int)Game::tick;
                    prolist = writer.write(value);
                }
            }
        }

        obj_role role;
        role.roleid = roleid;
        role.roletype = roletype;
        role.lvl = rolelvl;
        role.rolename = rolename;
        role.scenetypeid = scenetypeid;
        role.lastlogin = lastlogin;
        role.x = x;
        role.y = y;
        role.bodyfashion = bodyfashion;
        role.weaponfashion = weaponfashion;
        role.cityBodyFashion = cityBodyFashion;
        role.proplist = prolist;
        role.enchantId = enchantId;
        role.enchantLvl = enchantLvl;
        role.awakelvl = awakelvl;
        ack.errorcode = 0;
        ack.roles.push_back(role);
    }

    if (ack.roles.size() != token.count())
    {
        std::ostringstream sstream;
        for (int i = 0; i < ack.roles.size(); ++i)
        {
            sstream << ack.roles[i].roleid << ";";
        }

        doRedisCmd("hset player:%s roleids %s", account.c_str(), sstream.str().c_str());
    }
    
//    sort(ack.roles.begin(), ack.roles.end(), roleLvlSort);
//	/// 检查角色是否超过６个,　只下发６个角色消息
//	if (ack.roles.size() > 6) {
//		ack.roles.erase(ack.roles.begin() + 6, ack.roles.end());
//	}

    sendNetPacket(sessionid, &ack);
}}


hander_msg(req_createrole, req)
{
    std::string account = SSessionDataMgr.getProp(sessionid, "account");
    if (account.empty()) {
        return;
    }

    ack_createrole ack;
    ack.errorcode = 1;
    
    //判断角色名是否合法
    if (!isValidRoleName(req.rolename)) {
        ack.errorcode = CE_INVALID_NAME;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    RedisResult result;
    //判断角色名是否存在
    bool isNameExist = false;
    do {
        result.setData(redisCmd("exists rolename:%s", req.rolename.c_str()));
        if (result.readInt() == 1) {
            isNameExist = true;
            break;
        }
        
        if (g_RobotMgr.getData(req.rolename.c_str()) != NULL) {
            isNameExist = true;
            break;
        }
    } while (0);
    
    int gender = (req.roletypeid - 1) / 3;
    if (isNameExist) {
        g_RandNameMgr.update(gender == eBoy, req.rolename);
        ack.errorcode = CE_ROLE_NAME_EXIST;
        sendNetPacket(sessionid, &ack);
        return;
    }

    result.setData(redisCmd("hget player:%s roleids",account.c_str()));
    std::string roles = result.readStr();
    StringTokenizer token(roles,";", StringTokenizer::TOK_IGNORE_EMPTY);
    
    int roleCountLimit = 6;
    if (Process::env.getString("lang") == "en") {
        roleCountLimit = 1;
    }

    if (token.count() < roleCountLimit) {
        result.setData(redisCmd("incr %s", Role::sGenIdKey));
        int roleid = result.readInt();

        //判断该id是否被占用
        result.setData(redisCmd("exists role:%d", roleid));
        if (result.readInt() == 1) {
            ack.errorcode = 1;
            sendNetPacket(sessionid,&ack);
            return;
        }

        doRedisCmd("hmset player:%s roleids %s%d;",account.c_str(), roles.c_str(), roleid);

        doRedisCmd("hmset role:%d roletype %d lvl %d rolename %s "
                   "scenetypeid %d lastlogin %d x %d y %d proplist %s skilltable %s playeraccount %s",
                   roleid, req.roletypeid, 1, req.rolename.c_str(), SDefConf.def_scene,
                   0, SDefConf.def_x, SDefConf.def_y, req.proplist.c_str(), "", account.c_str());

        {

            //初始化任务
            string initquest = GlobalCfg::ReadStr("initquest", "");
            if (!initquest.empty()) {
                doRedisCmd("hset questProgress:%d %s 0",roleid, initquest.c_str());
            }

            //初始化背包物品
            string awarddesc = SSessionDataMgr.getProp(sessionid, "platform") + "_awards";
            string noviceaward = GlobalCfg::ReadStr(awarddesc.c_str());
            vector<string> awards = StrSpilt(noviceaward, ";");
            ItemArray items;
            RewardStruct reward;
            rewardsCmds2ItemArray(awards, items, reward);
            BackBag backbag(80);
            for (int i = 0; i < items.size(); i++)
            {
                backbag.Store(roleid, i, items[i]);
            }

            string initlvl = GlobalCfg::ReadStr("initlvl", "1");
            string initx = GlobalCfg::ReadStr("initx", "100");
            string inity = GlobalCfg::ReadStr("inity", "100");
            string initscene = GlobalCfg::ReadStr("initscene", "501101");
            string inilastcopy = GlobalCfg::ReadStr("initlastcopy", "0");
            string iniFat = GlobalCfg::ReadStr("initfat","0");
            string iniFatFreshTime = "0";
            string inieliteTimes = GlobalCfg::ReadStr("initelitetimes", "0");
            string iniTeamCopyResetTimes = "0";
            string iniviplvl = "0";
            string inivipexp = "0";
            string iniDungLevelrecord1 = "0";
            string iniDungLevelrecord2 = "0";
            string iniDungLevelrecord3 = "0";

            string iniDungResetTimes1 = "0";
            string iniDungResetTimes2 = "0";
            string iniDungResetTimes3 = "0";
            
            string iniLoginDays = "0";

            map<int, string> initprops;
            initprops[eRolePropGold] = Utils::itoa(reward.reward_gold);
            initprops[eRolePropExp] = Utils::itoa(reward.reward_exp);
            initprops[eRolePropBattlePoint] = Utils::itoa(reward.reward_batpoint);
            initprops[eRolePropConstellVal] = Utils::itoa(reward.reward_consval);
            initprops[eRolePropEnchantDust] = Utils::itoa(reward.reward_enchantdust);
            initprops[eRolePropEnchantGold] = Utils::itoa(reward.reward_enchantgold);
            initprops[eRolePropLvl] = initlvl;
            initprops[eRolePropX] = initx;
            initprops[eRolePropY] = inity;
            initprops[eRolePropSceneType] = initscene;
            initprops[eRoleLastFinishCopy] = inilastcopy;
            initprops[eRolePropFat] = iniFat;
            initprops[eRolefatFreshTime] = iniFatFreshTime;
            initprops[eRoleEliteTimes] = inieliteTimes;
            initprops[eRoleTeamCopyResetTimes] = iniTeamCopyResetTimes;
            initprops[eRolePropVipLvl] = iniviplvl;
            initprops[eRolePropVipExp] =inivipexp;

            initprops[eRolePropDungLevel1] = iniDungLevelrecord1;
            initprops[eRolePropDungLevel2] = iniDungLevelrecord2;
            initprops[eRolePropDungLevel3] = iniDungLevelrecord3;

            initprops[eRolePropDungResetTimes1] = iniDungResetTimes1;
            initprops[eRolePropDungResetTimes2] = iniDungResetTimes2;
            initprops[eRolePropDungResetTimes3] = iniDungResetTimes3;

            initprops[eRoleLoginDays] = iniLoginDays;
            
            char roleidstr[32];
            sprintf(roleidstr, "role:%d", roleid);
            RedisArgs args;
            RedisHelper::beginHmset(args, roleidstr);
            for (map<int, string>::iterator iter = initprops.begin(); iter != initprops.end(); iter++)
            {
                RedisHelper::appendHmset(args, GetRolePropName(iter->first), iter->second.c_str());
            }
            RedisHelper::commitHmset(get_DbContext(), args);
            doRedisCmd("set rolename:%s %d", req.rolename.c_str(), roleid);
        }

        ack.errorcode = 0;
        ack.roleid = roleid;

        g_RandNameMgr.update(gender == eBoy, req.rolename);

        LogMod::addLogCreateRole(roleid, req.rolename.c_str(), sessionid);
    } else {
        ack.errorcode = CE_ROLE_FULL;
    }
    sendNetPacket(sessionid,&ack);
}}




bool tryRenameRoleNameWithoutChangeRolenameKey(Role* role, string newname)
{
    //判断角色名是否存在
    RedisResult result;
    
    result.setData(redisCmd("exists rolename:%s", newname.c_str()));
    
    if (result.readInt() == 1) {
        return false;
    }
    
    string oldName = role->getRolename();
    mailChangeRoleName(role->getInstID(), oldName.c_str(), newname.c_str());
    role->setRolename(newname);
    
    SRoleMgr.delRoleNameIdx(oldName);
    SRoleMgr.addRoleNameIdx(newname, role);
    
    doRedisCmd("set rolename:%s %d", newname.c_str(), role->getInstID());
    doRedisCmd("hmset role:%d %s %s", role->getInstID(), GetRolePropName(eRolePropRoleName), newname.c_str());
    
    RoleGuildProperty& prop = SRoleGuild(role->getInstID());
    if ( !prop.isNull()) {
        prop.setRoleName(newname.c_str());
    }
    
    return true;
}


hander_msg(req_select_role,req)
{
    //账号安全判断
    std::string account = SSessionDataMgr.getProp(sessionid, "account");
    if (account.empty()){
        return;
    }
    
    int roleId = req.roleid;
    if (roleId == 0)
    {
        string rawId = SSessionDataMgr.getProp(sessionid, "raw_id");
        if (!rawId.empty()) {
            roleId = Utils::safe_atoi(rawId.c_str());
        } else {
            return;
        }
    }

    RedisResult result;
    result.setData(redisCmd("hget player:%s roleids",account.c_str()));
    string roleids = result.readStr();
    if (roleids.find(Utils::itoa(roleId)) == string::npos){
        return;
    }

    ack_select_role ack;
    ack.errorcode = 1;
    ack.roleid = roleId;
    do
    {
        result.setData(redisCmd("hget role:%d banrole", roleId));
        if (result.readInt() == 1){
            break;
        }

        ack.errorcode = 0;
    }
    while (0);

    sendNetPacket(sessionid, &ack);
    
    //检查重复登陆
    if (ack.errorcode == 0)
    {
        Role* role = SRoleMgr.GetRole(roleId);
        if (role != NULL)
        {
            notify_disconnect_tip tip;
            string dupLoginStr = StrMapCfg::getString("dup_login", "你的号在别处登陆");
            tip.tip = dupLoginStr;
            sendNetPacket(role->getSessionId(), &tip);
            SRoleMgr.disconnect(role);
        }

        role = SRoleMgr.createRole(roleId);
        if (role == NULL)
        {
            return;
        }

        SRoleMgr.mapRoleSession(roleId, sessionid);
        ack.errorcode = 0;

        //init
        role->saveProp(GetRolePropName(eLastLogin), Utils::itoa(Game::tick).c_str());
        //role->RoleInit();
        role->mLoginMoment = Game::tick;

        //open pvp
        if (role->getLvl() >= PVP_OPEN_LVL) {
            role->addToPvp();
        }
        LogMod::addLoginLog(role->getInstID(), role->getRolename().c_str(), role->getSessionId(), SRoleMgr.getRoleNum());

        mailAddRole(role->getRolename().c_str());

        //这里增加上线代码
        GlobalRoleOnline(role);
        
        // 检测角色名重复
        // 10级以下，vip等级为0的全部删除角色
        // 40级以上的保留角色名
        // 40级以下的修改角色名
        // 如果存在两个40级以上的角色，等级较高的角色保留名字
        // 名字被修改的角色给予一定补偿（改名卡,...）
        int delLevel = 10;
        int keepNameLevel = 40;
        int retryTimes = 50;
        
        string mailSender = "系统";
        string mailTitle = "角色改名补偿";
        string mailContent = "因角色同名导致邮件领取异常，目前已修复此问题，由此带来的不便深表歉意，为此特准备：改名卡*1，战功*10W，星灵*200，紫色经验哥布林*5，祝您游戏愉快！";
        string mailItems = "item 440002*1;item 383104*5;consval 200*1;batpoint 100000*1;";
        
        result.setData(redisCmd("get rolename:%s", role->getRolename().c_str()));
        int checkRoleId = result.readInt();
        if (checkRoleId != role->getInstID()) {
            Xylog problemLog(eLogName_DupNameProblem, role->getInstID());
            {
                //基本信息
                problemLog << role->getRolename();
                problemLog << SSessionDataMgr.getProp(role->getSessionId(), "account");
                problemLog << SSessionDataMgr.getProp(role->getSessionId(), "platform");
            }
            
            do {
                bool shouldRename = true;
                if (role->getLvl() >= keepNameLevel) {
                    RedisResult tmp(redisCmd("hget role:%d lvl", checkRoleId));
                    if (tmp.readInt() < role->getLvl()) {
                        Role* prevRole = SRoleMgr.GetRole(checkRoleId);
                        if (prevRole) {
                            kickAwayRoleWithTip(prevRole);
                        }
                        
                        shouldRename = false;
                        doRedisCmd("set rolename:%s %d", role->getRolename().c_str(), role->getInstID());

                        problemLog << "保留名字";
                        break;
                    }
                }
                
                if (role->getLvl() < delLevel && role->getVipLvl() < 1) {
                    problemLog << "删除角色";
                    DeleteRoleRoleInfoInDB(role->getInstID(), role->getSessionId());
                    kickAwayRoleWithTip(role);
                    break;
                }
                
                string newName;
                bool isSuccess = false;
                for (int i = 1; i <= retryTimes; i++) {
                    newName = strFormat("%s[%d]", role->getRolename().c_str(), i);
                    RedisResult tmp(redisCmd("exists rolename:%s", newName.c_str()));
                    if (tmp.readInt() == 1) {
                        continue;
                    }
                    
                    if (tryRenameRoleNameWithoutChangeRolenameKey(role, newName)) {
                        string roleName = role->getRolename();
                        sendMail(role->getInstID(), mailSender.c_str(), roleName.c_str(),
                                 mailTitle.c_str(), mailContent.c_str(), mailItems.c_str(), "");
                        
                        isSuccess = true;
                        break;
                    }
                }
                
                if (isSuccess) {
                    problemLog << "改名成功" << newName.c_str();
                } else {
                    problemLog << "改名失败";
                }
                kickAwayRoleWithTip(role);
            }
            while (0);
        }
        // 检测角色名重复结束
        
    }
}}

handler_msg(req_restore_role, req)
{
    std::string account = SSessionDataMgr.getProp(sessionid, "account");
    if (account.empty()){
        return;
    }

    ack_delete_role ack;
    ack.errorcode = 1;

    RedisResult ret(redisCmd("hget player:%s roleids",account.c_str()));

    StringTokenizer token(ret.readStr(), ";");
    for (int i = 0; i != token.count(); ++i)
    {
        if ( req.roleid == atoi(token[i].c_str()))
        {
            RedisResult ret(redisCmd("hget role:%d proplist", req.roleid));
            Json::Reader reader;
            Json::Value value;
            Json::FastWriter writer;

            string proplist = ret.readStr();
            if (!proplist.empty() && !reader.parse(proplist.c_str(), value))
            {
                break;
            }

            value.removeMember("deletetime");
            proplist = writer.write(value);
            doRedisCmd("hset role:%d proplist %s", req.roleid, proplist.c_str());

            ack.errorcode = 0;
        }
    }

    sendNetPacket(sessionid,&ack);
}}


hander_msg(req_delete_role, req)
{
    std::string account = SSessionDataMgr.getProp(sessionid, "account");
    if (account.empty()){
        return;
    }

    ack_delete_role ack;
    ack.errorcode = 1;

    RedisResult ret(redisCmd("hget player:%s roleids",account.c_str()));

    StringTokenizer token(ret.readStr(), ";");
    for (int i = 0; i != token.count(); ++i)
    {
        if ( req.roleid == atoi(token[i].c_str()))
        {
            RedisResult ret(redisCmd("hget role:%d proplist", req.roleid));
            Json::Reader reader;
            Json::Value value;
            Json::FastWriter writer;

            string proplist = ret.readStr();
            //玩家数据之前被异常修改，此处做一下兼容
            if (proplist.length() < 2) {
                proplist = "";
            }
            
            if (!proplist.empty() && !reader.parse(proplist.c_str(), value)) {
                break;
            }
            
            if (!value.isNull() && !value.isObject()) {
                break;
            }

            value["deletetime"] = int(Game::tick + 7 * 24 * 3600);
            proplist = writer.write(value);
            doRedisCmd("hset role:%d proplist %s", req.roleid, proplist.c_str());

            ack.errorcode = 0;
        }
    }

    sendNetPacket(sessionid,&ack);
}}

hander_msg(req_randname,req)
{
    ack_randname ack;
    ack.name = g_RandNameMgr.getRandName(req.male);
    //ack.name = SRandName.Rand(req.male);
    sendNetPacket(sessionid,&ack);
}}


hander_msg(req_getrolesave, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)

    ack_getrolesave ack;
    ack.errorcode       = 0;
    ack.exp             = role->getExp();
    ack.gold            = role->getGold();
    ack.rmb             = role->getRmb();
    ack.fat             = role->getFat();
    ack.battlePoint     = role->getBattlePoint();
    ack.constellval     = role->getConstellVal();
    ack.lastFinishCopy  = role->getLastFinishCopy();
    ack.dungTimes       = role->getDungTimes();
    ack.eliteTimes      = role->getEliteTimes() + role->getIncrEliteCopyTimes();
    ack.fatbuytime      = role->getFatBuyTimes();
    ack.elitebuytimes   = role->getEliteBuyTimes();
    ack.pvptimes        = role->getPvpTimes();
    ack.pvpbuytimes     = role->getPvpBuyTimes();
    ack.teamresettimes  = role->getTeamCopyResetTimes();
    ack.viplvl          = role->getVipLvl();
    ack.vipexp          = role->getVipExp();
    
    ack.petadventuretimes                           = role->calPetAdventureTimes();                                 //幻兽大冒险副本次数
    ack.petadventurebuytimes                        = role->getPetAdventureBuyTimesRecordPerDay();                  //幻兽大冒险副本次数购买次数
    ack.petadventurerobotpetrenttimesremain         = role->calPetAdventureRobotPetRentTimesRemain();               //幻兽大冒险机器幻兽租用次数
    ack.petadventurerobotpetbuyingrenttimesperday   = role->getPetAdventureRobotPetBuyingRentTimesRecordPerDay();   //幻兽大冒险机器幻兽当天购买次数
    
    ack.viplvlaward             = role->getVipLvlAward();
    ack.dailyQuestRefreshTimes  = role->getDailyQuestRefreshTimes();
	ack.prestige                = role->getPrestige();
    ack.petEliteTimes           = role->getPetEliteCopyLeftTime();
    ack.petEliteBuytimes        = role->getPetEliteBuyTimes();
    ack.roleAnger               = role->getRoleAnger();
    ack.doublePotionNum         = role->getDoublePotionNum();
    ack.enchantDust             = role->getEnchantDust();
    ack.enchantGold             = role->getEnchantGold();
    ack.roleAwakeLvl            = role->getRoleAwakeLvl();
    
    ack.crossservicewartimes = role->getCsPvpState()->getChallengeTime();
    ack.crossservicewarbuytimes = role->getCsPvpStateData().getRmbBuyTimes();
    
    std::set<int>::iterator iter;
    for (iter = role->mFinishEliteCopy.begin(); iter != role->mFinishEliteCopy.end(); iter++) {
        ack.finishElitecopy.push_back(*iter);
    }
    role->sendRolePropInfoAndVerifyCode();
    sendNetPacket(sessionid, &ack);
    
    notify_syn_accumulateRechargeRecord notifyRecharge;
    notifyRecharge.record = role->getRechargeRecord().getRecord();
    sendNetPacket(sessionid, &notifyRecharge);
    
    notify_syn_accumulateRmbConsumedRecord notifyRmbConsume;
    notifyRmbConsume.record = role->getRmbConsumedRecord().getRecord();
    sendNetPacket(sessionid, &notifyRmbConsume);
}}

handler_msg(req_enter_city, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)

    ack_enter_city ack;
    ack.errorcode = 1;
    ack.cityid = req.cityid;
    ack.threadid = req.threadid;

    Scene* newScene = NULL;
    Scene* prevScene = NULL;
    do
    {
        prevScene = role->getScene();
        if (prevScene && req.cityid == prevScene->getID() && req.threadid == prevScene->getThreadId())
        {
            break;
        }
        
        if (req.threadid == 0)
        {
            newScene = SceneMgr::getScene(req.cityid);
        }
        else
        {
            newScene = SceneMgr::getScene(req.cityid, req.threadid);
        }
        
        if (newScene == prevScene || newScene == NULL || !newScene->preAddRole(role))
        {
            break;
        }
        
        if (prevScene != NULL)
        {
            if (newScene->getID() != prevScene->getID())
            {
                role->saveProp(GetRolePropName(eRolePropSceneType),
                           Utils::itoa(newScene->getID()).c_str());
            }
            prevScene->removeRole(role);
        }
        newScene->addRole(role);

        ack.errorcode = 0;
        ack.threadid = newScene->getThreadId();

        role->setPreEnterSceneid(req.cityid);
    }
    while (0);

    sendNetPacket(sessionid, &ack);
    
    
    if (ack.errorcode == CE_OK) {
        //进入主城
        onRoleEnterCity(role, prevScene);
        
        //BUG:客户端切线的时候未发送req_enter_scene的消息
        if (req.threadid > 0){
            role->setPreEnterSceneid(0);
            notify_backToCity notify;
            notify.cityid = req.cityid;
            sendNetPacket(sessionid, &notify);
        }
    }
}}

handler_msg(req_get_scene_threads, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    ack_get_scene_threads ack;
    SceneList* sceneList = SceneMgr::getSceneList(req.cityid);
    if (sceneList != NULL)
    {
        for (SceneListIter iter = sceneList->begin(); iter != sceneList->end(); iter++)
        {
            Scene* scene = *iter;
            ack.threadroles.push_back(scene->getRoleNum());
        }
    }
    
    ack.cityid = req.cityid;
    sendNetPacket(sessionid, &ack);
}}

int verifyEnemyInfo(const obj_role_verify_info& enemyInfo, VerifyInfo& enemyOriginInfo)
{
    //验证好友或者敌人的属性
    if (enemyInfo.roleid == 0) {
        return CE_OK;
    }

    if ((enemyInfo.maxHp + 1) < enemyOriginInfo.maxHp) {
        return CHEAT_ENEMY_MAXHP_ERROR;
    }

    if ((enemyInfo.atk + 1) < enemyOriginInfo.atk) {
        return CHEAT_ENEMY_ATK_ERROR;
    }
	
    if ( (enemyInfo.def + 1)< enemyOriginInfo.def) {
        return CHEAT_ENEMY_DEF_ERROR;
    }

    if ( (enemyInfo.cri + 1)  < enemyOriginInfo.cri) {
        return CHEAT_ENEMY_CRI_ERROR;
    }

    if ( (enemyInfo.dodge + 1) < enemyOriginInfo.dodge) {
        return CHEAT_ENEMY_DODGE_ERROR;
    }

    if ( (enemyInfo.hit + 1 )< enemyOriginInfo.hit) {
        return CHEAT_ENEMY_HIT_ERROR;
    }

    if ( enemyInfo.petBattle > 0 && enemyInfo.petBattle < enemyOriginInfo.petBattle) {
        return CHEAT_ENEMY_PET_BATTLE_ERROR;
    }

    return CE_OK;
}

int checkVerifyRoleProp(const req_verify_role_prop& req, Role* role, string& desc)
{
    float atkScale = g_RolePropVerify.getAtkScale() + role->mAtkBuf.second;
    int atkFloat = g_RolePropVerify.getAtkValue() + role->mAtkBuf.first;
    float defScale = g_RolePropVerify.getDefScale() + role->mDefBuf.second;
    int defFloat = g_RolePropVerify.getDefValue() + role->mDefBuf.first;
    float hpScale = g_RolePropVerify.getHpScale() + role->mHpBuf.second;
    int hpFloat = g_RolePropVerify.getHpValue() + role->mHpBuf.first;

    float hitScale = g_RolePropVerify.getHitScale() + role->mHitBuf.second;
    int hitFloat = g_RolePropVerify.getHitValue() + role->mHitBuf.first;
    float dodgeScale = g_RolePropVerify.getDodgeScale() + role->mDodgeBuf.second;
    int dodgeFloat = g_RolePropVerify.getDodgeValue() + role->mDodgeBuf.first;
    float criScale = g_RolePropVerify.getCriScale() + role->mCriBuf.second;
    int criFloat = g_RolePropVerify.getCriValue() + role->mCriBuf.first;

    int petbatValue = g_RolePropVerify.getPetBatValue();
    float petbatScale = g_RolePropVerify.getPetBatScale();

    int cheatCode = 0;
    SceneCfgDef* scenecfg = SceneCfg::getCfg(role->getCurrSceneId());
    
    int sceneType = scenecfg->sceneType;
    if (sceneType == stTown || sceneType == stTreasureFight ||
        sceneType == stSyncPvp ||  role->getWorldState() != eWorldStateNone) {
        return CHEAT_NOT;
    }

    if (sceneType == stPvp)
    {
        //hpScale = 20;
        //defFloat += 200;
        cheatCode = verifyEnemyInfo(req.other, role->getEnemyVerifyInfo());
    }

    do
    {
        if (cheatCode) {
            break;
        }

        //验证主角的各种属性
        if (req.mainrole.atk < 0 || req.mainrole.atk > (role->mRealBattleProp.getAtk() * atkScale + atkFloat))
        {
            desc = Utils::makeStr("req:%d, srv:%d", req.mainrole.atk, role->mRealBattleProp.getAtk());
            cheatCode = CHEAT_ATK_ERROR;
            break;
        }

        if (req.mainrole.def < 0 || req.mainrole.def > (role->mRealBattleProp.getDef() * defScale + defFloat))
        {
            desc = Utils::makeStr("req:%d, srv:%d",req.mainrole.def, role->mRealBattleProp.getDef());
            cheatCode = CHEAT_DEF_ERROR;
            break;
        }

        if (req.mainrole.verifyHP < 0 || req.mainrole.verifyHP > (role->mRealBattleProp.getMaxHp() * hpScale + hpFloat))
        {
            desc = Utils::makeStr("req:%d, srv:%d", req.mainrole.verifyHP, role->mRealBattleProp.getMaxHp());
            cheatCode = CHEAT_HP_ERROR;
            break;
        }

        if (req.mainrole.hit < 0 || req.mainrole.hit > (role->mRealBattleProp.getHit() * hitScale + hitFloat))
        {
            desc = Utils::makeStr("req:%f, srv:%f", req.mainrole.hit, role->mRealBattleProp.getHit());
            cheatCode = CHEAT_HIT_ERROR;
            break;
        }

//        (void)dodgeFloat;
//        (void)dodgeScale;
        if (req.mainrole.dodge < 0 || req.mainrole.dodge > (role->mRealBattleProp.getDodge() * dodgeScale + dodgeFloat))
        {
            role->mBigDodgeTimes++;

            if (role->mBigDodgeTimes >= 2) {
                desc = Utils::makeStr("req:%d, srv:%d", req.mainrole.dodge, role->mRealBattleProp.getDodge());
                cheatCode = CHEAT_DODGE_ERROR;
                break;
            }
        }
        else
        {
            role->mBigDodgeTimes = 0;
        }


        if (req.mainrole.cri < 0 || req.mainrole.cri > (role->mRealBattleProp.getCri() * criScale + criFloat))
        {
            desc = Utils::makeStr("req:%d, srv:%d", req.mainrole.cri, role->mRealBattleProp.getCri());
            cheatCode = CHEAT_CRI_ERROR;
            break;
        }

//        Pet* activePet = role->mPetMgr->getActivePet();
//        if (activePet) {
//            int battle = calcPetBattleForce(&activePet->mRealBattleProp);
//            if ( req.mainrole.petBattle < 0 || req.mainrole.petBattle > battle * petbatScale + petbatValue) {
//                cheatCode = CHEAT_PETBATTLE_ERROR;
//                break;
//            }
//        }

        /******************* 验证好友属性 ********************/
        VerifyInfo* friendInfo = role->getFightedFriendInfo();
        if (friendInfo == NULL || req.other.roleid == 0) {
            break;
        }

        if (req.other.atk < 0 || req.other.atk > (friendInfo->atk * atkScale + atkFloat))
        {
            desc = Utils::makeStr("req:%d, srv:%d", req.other.atk, friendInfo->atk);
            cheatCode = CHEAT_FRIEND_ATK_ERROR;
            break;
        }

        if (req.other.def < 0 || req.other.def > (friendInfo->def * defScale + defFloat))
        {
            desc = Utils::makeStr("req:%d, srv:%d", req.other.def, req.other.def);
            cheatCode = CHEAT_FRIEND_DEF_ERROR;
            break;
        }

        if (req.other.maxHp < 0 || req.other.maxHp > (friendInfo->maxHp * hpScale + hpFloat))
        {
            desc = Utils::makeStr("req:%d, srv:%d", req.other.maxHp, friendInfo->maxHp);
            cheatCode = CHEAT_FRIEND_HP_ERROR;
            break;
        }

        if (req.other.cri < 0 || req.other.cri > (friendInfo->cri * criScale + criFloat))
        {
            desc = Utils::makeStr("req:%f, srv:%f", req.other.cri, friendInfo->cri);
            cheatCode = CHEAT_FRIEND_CRI_ERROR;
            break;
        }

        if (req.other.hit < 0 || req.other.hit > (friendInfo->hit * hitScale + hitScale))
        {
            desc = Utils::makeStr("req:%f, srv:%f", req.other.hit, friendInfo->hit);
            cheatCode = CHEAT_FRIEND_HIT_ERROR;
            break;
        }

        if (req.other.dodge < 0 || req.other.dodge > (friendInfo->dodge * dodgeScale + dodgeFloat))
        {
            desc = Utils::makeStr("req:%f, srv:%f", req.other.dodge, friendInfo->dodge);
            cheatCode = CHEAT_FRIEND_DODGE_ERROR;
            break;
        }

        if (req.other.petBattle < 0 || req.other.petBattle > (friendInfo->petBattle * petbatScale + petbatValue))
        {
            desc = Utils::makeStr("req:%d, srv:%d", req.other.petBattle, friendInfo->petBattle);
            cheatCode = CHEAT_FRIEND_PETBATTLE_ERROR;
            break;
        }
    }
    while (0);
    return cheatCode;
}

handler_msg(req_old_verify_role_prop, req)
{
    return;
    hand_Sid2Role_check(sessionid, roleid, role);

    float atkScale = g_RolePropVerify.getAtkScale();
    int atkFloat = g_RolePropVerify.getAtkValue();
    float defScale = g_RolePropVerify.getDefScale();
    int defFloat = g_RolePropVerify.getDefValue();
    float hpScale = g_RolePropVerify.getHpScale();
    int hpFloat = g_RolePropVerify.getHpValue();

    float hitScale = g_RolePropVerify.getHitScale();
    int hitFloat = g_RolePropVerify.getHitValue();
    float dodgeScale = g_RolePropVerify.getDodgeScale();
    int dodgeFloat = g_RolePropVerify.getDodgeValue();
    float criScale = g_RolePropVerify.getCriScale();
    int criFloat = g_RolePropVerify.getCriValue();

    int petbatValue = g_RolePropVerify.getPetBatValue();
    float petbatScale = g_RolePropVerify.getPetBatScale();

    SceneCfgDef* scenecfg = SceneCfg::getCfg(role->getCurrSceneId());
    if (scenecfg->sceneType == stPvp)
    {
        hpScale = 20;
        defFloat += 200;
    }
    int sceneType = scenecfg->sceneType;
    if (sceneType == stTown || sceneType == stTreasureFight ||
        sceneType == stSyncPvp || sceneType == stSyncTeamCopy || role->getWorldState() != eWorldStateNone) {
        return;
    }

    int cheatCode = 0;
    do
    {
        if (req.atk > role->mRealBattleProp.getAtk() * atkScale + atkFloat)
        {
            cheatCode = CHEAT_ATK_ERROR;
            break;
        }

        if (req.def > role->mRealBattleProp.getDef() * defScale + defFloat)
        {
            cheatCode = CHEAT_DEF_ERROR;
            break;
        }

        if (req.verifyHP > role->mRealBattleProp.getMaxHp() * hpScale + hpFloat)
        {
            cheatCode = CHEAT_HP_ERROR;
            break;
        }

        if (req.hit > role->mRealBattleProp.getHit() * hitScale + hitFloat)
        {
            cheatCode = CHEAT_HIT_ERROR;
            break;
        }

        (void)dodgeFloat;
        (void)dodgeScale;
        /*if (req.dodge > role->getDodge() * dodgeScale + dodgeFloat)
         {
         cheatCode = CHEAT_DODGE_ERROR;
         break;
         }*/

        if (req.cri > role->mRealBattleProp.getCri() * criScale + criFloat)
        {
            cheatCode = CHEAT_CRI_ERROR;
            break;
        }

//        Pet* activePet = role->mPetMgr->getActivePet();
//        if (activePet) {
//            int battle = calcPetBattleForce(activePet);
//            if (req.petBattle > battle * petbatScale + petbatValue) {
//            //if (req.petBattle > activePet->mBattle * petbatScale + petbatValue) {
//                cheatCode = CHEAT_PETBATTLE_ERROR;
//                break;
//            }
//        }
    }
    while (0);

    if (cheatCode)
    {

        kickAwayRoleWithTips(role, Utils::itoa(cheatCode).c_str());
    }
}}

handler_msg(req_verify_role_prop, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);

    int cheatCode = 0;

    SceneCfgDef* scene = SceneCfg::getCfg(role->getCurrSceneId());
    int sceneType = scene->sceneType;
    
    if (sceneType == stTown || sceneType == stTreasureFight ||
        sceneType == stSyncPvp || sceneType == stSyncTeamCopy ||  role->getWorldState() != eWorldStateNone) {
        return;
    }
    
    string errorDesc = "";
    
    cheatCode = checkVerifyRoleProp(req, role, errorDesc);

    if (cheatCode)
    {
        string desc = Utils::makeStr("%d\t%s",cheatCode, errorDesc.c_str());
        kickAwayRoleWithTips(role, desc.c_str());
    }
}}

// 防作弊心跳
handler_msg(req_client_heart_beat, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    g_HeatBeatAnitCheating.heartBeat(role, req);
}}

handler_msg(req_verify_role_atk_monsters, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);

    bool isCheat = false;
    
    SceneCfgDef* scenecfg = SceneCfg::getCfg(role->getCurrSceneId());
    if (!scenecfg || scenecfg->sceneType == stTown) {
        return;
    }

    if (scenecfg->sceneType == stNewbie ||
        scenecfg->sceneType == stDefendStatue ||
        scenecfg->sceneType == stChallenge ||
        scenecfg->sceneType == stLimitTimeKilled ||
        scenecfg->sceneType == stPetArena) {
        return;
    }

    do
    {
        
        if (canInorgeBattleCheck()) {
            break;
        }
        
        for (int i = 0; i < req.monsterHurts.size(); i++) {

            if (!checkMonsterExist(req.monsterHurts[i].monsterTpltID, role, scenecfg)) {
                
                string desc = strFormat("修改怪物id。场景%d出现怪物%d", role->getCurrSceneId(), req.monsterHurts[i].monsterTpltID);
                
                kickAwayRoleWithTips(role, desc.c_str());

                return;
            }

        }

    }while(false);

    if (isCheat) {

        kickAwayRoleWithTips(role, "修改攻击");
    }
}}

#include "GuildMgr.h"

void ackGetRoleData(int sessionId, Role* role)
{
    ack_get_role_data ack;
    ack.errorcode = 0;
    ack.roleid = role->getInstID();
    ack.isrobot = 0;
    ack.roletype = role->getRoleType();
    ack.rolename = role->getRolename();
    
    int lastlogin = Game::tick - Utils::safe_atoi(role->loadProp(GetRolePropName(eLastLogin)).c_str(), 0);
    ack.lastlogin = lastlogin;
    ack.lvl = role->getLvl();
    ack.inte = role->getInte();
    ack.phys = role->getPhys();
    ack.stre = role->getStre();
    ack.capa = role->getCapa();
    
    ack.hp = role->getMaxHp();
    ack.atk = role->getAtk();
    ack.def = role->getDef();
    ack.hit = role->getHit();
    ack.dodge = role->getDodge();
    ack.cri = role->getCri();
    
    ack.enchantId = role->getEnchantId();
    ack.enchantLvl = role->getEnchantLvl();
    ack.awakeLvl = role->getRoleAwakeLvl();
    
    const ItemArray& bagitems = role->getPlayerEquip()->GetItems();
    for (int i = 0; i < role->getPlayerEquip()->GetCapacity(); i++)
    {
        if (bagitems[i].item <= 0)
        {
            continue;
        }
        
        obj_playerItem equip;
        equip.index = i;
        equip.itemid = bagitems[i].item;
        equip.bind = bagitems[i].bind;
        equip.json = bagitems[i].json;
        ack.equips.push_back(equip);
    }
    
    ack.activepet = role->getActivePetId();
    
    ConstellData constell = role->mConstellationMgr->getConstellData();
    ack.constellid = constell.consid;
    ack.constellstep = constell.step;
    
    Guild guild = SGuildMgr.getRoleGuild(ack.roleid );
    ack.guildname = guild.getGuildName();
    ack.guildLvl = guild.getLevel();
    ack.guildposition = SRoleGuild( ack.roleid ).getPosition();
	ack.guildRank = guild.getRank(ack.roleid);
    // 声望
	ack.prestige = role->getPrestige();
    ack.weaponfashion = role->getWeaponFashion();
    ack.bodyfashion = role->getBodyFashion();
    ack.cityBodyFashion = role->getHomeFashion();
    
    Wardrobe* wardrobe = role->getWardrobe();
    ack.playerWardrobeLvl = wardrobe->getLvl();
    WardrobeFashionList* fashionLists[] = {&wardrobe->getBodyFashion(), &wardrobe->getWeaponFashion(),&wardrobe->getHomeFashion()};
    for (int i = 0; i < sizeof(fashionLists) / sizeof(WardrobeFashionList*); i++) {
        WardrobeFashionList& fashionList = *fashionLists[i];
        for (WardrobeFashionList::Iterator iter = fashionList.begin();
             iter != fashionList.end(); iter++) {
            WardrobeFashion* fashion = *iter;
            if (!fashion->mExpired) {
                ack.fashionList.push_back(fashion->mId);
            }
        }
    }
    
    sendNetPacket(sessionId, &ack);
}


void ackGetRobotData(int sessionId, Robot* robot)
{
    ack_get_role_data ack;
    ack.errorcode = 0;
    ack.roleid = robot->mId;
    ack.isrobot = 1;
    ack.roletype = robot->mRoleType;
    ack.rolename = robot->mRoleName;
    
    ack.lvl = robot->mBattleProp.getLvl();
    ack.inte = robot->mBaseProp.getInte();
    ack.phys = robot->mBaseProp.getPhys();
    ack.stre = robot->mBaseProp.getStre();
    ack.capa = robot->mBaseProp.getCapa();
    
    ack.hp = robot->mBattleProp.getMaxHp();
    ack.atk = robot->mBattleProp.getAtk();
    ack.def = robot->mBattleProp.getDef();
    ack.hit = robot->mBattleProp.getHit();
    ack.dodge = robot->mBattleProp.getDodge();
    ack.cri = robot->mBattleProp.getCri();
    
    ack.lastlogin = 3600 * 24;
    
    const ItemArray& equips = robot->mEquips;
    for (int i = 0; i < equips.size(); i++) {
        if (equips[i].item <= 0) {
            continue;
        }
        
        obj_playerItem equip;
        equip.index = i;
        equip.itemid = equips[i].item;
        equip.bind = equips[i].bind;
        equip.json = equips[i].json;
        ack.equips.push_back(equip);
    }
    
    sendNetPacket(sessionId, &ack);
}

handler_msg(req_get_role_data, req)
{
    if (req.serverid != Process::env.getInt("server_id") && req.serverid > 0) {
        
        req_get_csRoleData_through_cs csReq;
        csReq.doReqServerid = Process::env.getInt("server_id");
        
        csReq.roleid            = req.roleid;
        csReq.onReqServerid     = req.serverid;
        csReq.clientSession     = sessionid;
        
        sendNetPacket(CenterClient::instance()->getSession(), &csReq);
    }
    else {
        bool isError = true;
        do
        {
            //bool objectType = getObjectType(req.roleid);
            //if (objectType == kObjectRobot) {
            if (req.isrobot || req.roleid == RobotCfg::sFriendId) {
                Robot* robot = g_RobotMgr.getData(req.roleid);
                if (robot) {
                    ackGetRobotData(sessionid, robot);
                }
            //} else if (objectType == kObjectRole) {
            } else {
                bool shouldDelete = false;
                Role* roledata = SRoleMgr.GetRole(req.roleid);
                if (roledata == NULL) {
                    shouldDelete = true;
                    roledata = Role::create(req.roleid, true);
                    if (roledata && !roledata->getRolename().empty()) {
                        roledata->onCalPlayerProp();
                    } else {
                        shouldDelete = false;
                        delete roledata;
                        roledata = NULL;
                    }
                }
                
                if (roledata) {
                    ackGetRoleData(sessionid, roledata);
                }
                
                if (shouldDelete) {
                    delete roledata;
                }
            }
            
            isError = false;
        } while (0);
        
        if (isError) {
            ack_get_role_data ack;
            ack.errorcode = 1;
            ack.roleid = req.roleid;
            ack.isrobot = req.isrobot;
            sendNetPacket(sessionid, &ack);
        }
    }

}}


extern bool isValidRoleName(const string& rolename);

hander_msg(req_pre_redirect_name, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_pre_redirect_name ack;
    ack.errorcode = CE_OK;
    ack.type = req.type;
    
    
    if (req.newname.empty()) {
        ack.errorcode = CE_INVALID_ROLE_NAME;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    if (!isValidRoleName(req.newname))
    {
        ack.errorcode = CE_RENAME_ILLEGAL_CHAR_ERROR;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    if (req.type == eGuildRename) {
        Guild& guild = SGuildMgr.getGuild(req.newname.c_str());
        if ( !guild.isNull() ) {
            ack.errorcode = CE_GUILD_NAME_EXIST;
        }
    }
    else
    {
        RedisResult result(redisCmd("exists rolename:%s", req.newname.c_str()));
        if (result.readInt() == 1) {
            ack.errorcode = CE_ROLE_NAME_EXIST;
        }
    }
    
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_redirect_name, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_redirect_name ack;
    ack.errorcode = CE_OK;
    ack.type = req.type;
    
    ItemGroup item = role->getBackBag()->GetItem(req.itemIndex);
    
    do
    {
        if (req.newname.empty() || !isValidRoleName(req.newname)) {
            ack.errorcode = CE_INVALID_NAME;
            break;
        }
        
        if (item.item <= 0 || item.count <= 0)
        {
            ack.errorcode = CE_ITEM_NOT_ENOUGH;
            break;
        }
        
        ItemCfgDef* cfg = ItemCfg::getCfg(item.item);
        if (cfg == NULL)
        {
            ack.errorcode = CE_ITEM_NOT_ENOUGH;
            break;
        }
        
        int itemtype = cfg->ReadInt("type");
        
        if (itemtype != KItemRenameCard) {
            ack.errorcode = CE_RENAME_CARD_ERROR;
            break;
        }
        
        int renameType = cfg->ReadInt("change_type");
        
        if (renameType != req.type) {
            ack.errorcode = CE_RENAME_CARD_ERROR;
            break;
        }
        
        switch (req.type) {
            case eRoleRename:
                ack.errorcode = role->renameRolename(req.newname);
                break;
            case eGuildRename:
                ack.errorcode = SGuildMgr.guildRename(roleid, req.newname);
                break;
            default:
                break;
        }
        
    }while(false);
    
    if (ack.errorcode == CE_OK)
    {
        item.count--;
        if (item.count == 0)
        {
            item = ItemGroup();
        }
//        BackBag::UpdateBackBag(roleid, req.itemIndex, item);
        role->updateBackBagItems(req.itemIndex, item);
    }
    
    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_client_memory_cheat, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    
    ack_client_cheat ack;
    ack.errorcode = CHEAT_MODIFY_MEMORY;
    sendNetPacket(role->getSessionId(), &ack);
    
    LogMod::addLogClientCheat(role->getInstID(), role->getRolename().c_str(),
                              role->getSessionId(), "nil",req.tostring().c_str());
    if (role->getWorldState() == eWorldStateNone) {
        SRoleMgr.disconnect(role, eClientCheat);
    }
}}

handler_msg(req_client_config_cheat, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_client_cheat ack;
    ack.errorcode = CHEAT_MODIFY_CONFIG;
    sendNetPacket(role->getSessionId(), &ack);
    
    LogMod::addLogClientCheat(role->getInstID(),
							  role->getRolename().c_str(),
                              role->getSessionId(),
                              "nil",
							  req.tostring().c_str());
	
    if (role->getWorldState() == eWorldStateNone) {
        SRoleMgr.disconnect(role, eClientCheat);
    }
}}

handler_msg(req_linkStateCheck, req)
{
    ack_linkStateCheck ack;
    sendNetPacket(sessionid, &ack);
}}