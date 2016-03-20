#include "hander_include.h"
#include "worship/worship_system.h"
#include "Paihang.h"
#include "mail_imp.h"
#include "Role.h"
#include "DBRolePropName.h"
#include "Constellation.h"
#include "GuildMgr.h"
#include "GuildRole.h"
#include "Wardrobe.h"
#include "TreasureFight.h"

handler_msg(req_cs_loadPaihangData, ack)
{
    reqPaihangDbData(sessionid, ack.type);
//    log_info("current time notice");
//    printf("type: %d\n", ack.type);
}}


handler_msg(req_cs_sendRankListAwards, req)
{
    string rolename = req.rolename;
    int    roleid   = req.roleid;
    int    rank     = req.rank;
    int    rankListType = req.rankListType;
    string awards   = req.awards;
    
    string mail_type = "";
    switch (rankListType) {
        case eSortBat:
            mail_type = "CrossServiceBattleRankListAwards";
            break;
        case eSortPet:
            mail_type = "CrossServicePetRankListAwards";
            break;
        case eSortConsume:
            mail_type = "CrossServiceConsumeRankListAwards";
            break;
        case eSortRecharge:
            mail_type = "CrossServiceRechargeRankListAwards";
            break;
        default:
            break;
    }
    
    if (awards.empty() || rank <= 0 || roleid <= 0 || rolename.empty() || mail_type.empty()) {
        return;
	}
    
    string awardExplanation = awardToMailContentExplanation(awards);
    
    MailFormat* formate = NULL;
    string sendername   = "";
    string title        = "无";
    string content      = "无";
	
    formate = MailCfg::getCfg(mail_type.c_str());
    
    if (formate) {
        sendername = formate->sendername;
        title      = formate->title;
        content    = formate->content;
        
        //獎勵：{:str:}
        find_and_replace(content, 2, Utils::itoa(rank).c_str(), awardExplanation.c_str());
    }
    
    bool ret = sendMail(roleid, sendername.c_str(), rolename.c_str(), title.c_str(), content.c_str(), awards.c_str(), "");
    
    if( false ==  ret ){
        StoreOfflineItem(roleid, awards.c_str());
    }
    
    switch (rankListType) {
        case eSortBat:
        {
            Xylog log(eLogName_CrossServiceBattleRankListAwards, roleid);
            log << rolename << rankListType << rank << awards;
            break;
        }
        case eSortPet:
        {
            Xylog log(eLogName_CrossServicePetRankListAwards, roleid);
            log << rolename << rankListType << rank << awards;
            break;
        }
        case eSortConsume:
        {
            Xylog log(eLogName_CrossServiceConsumeRankListAwards, roleid);
            log << rolename << rankListType << rank << awards;
            break;
        }
        case eSortRecharge:
        {
            Xylog log(eLogName_CrossServiceRechargeRankListAwards, roleid);
            log << rolename << rankListType << rank << awards;
            break;
        }
        default:
            break;
    }
    
}}

void crossServicGetRoleData(int sessionId, Role* role, int doReqServerid, int clientSessionid)
{
    ack_cs_get_role_Data ack;
    ack.doReqServerid    = doReqServerid;
    ack.onReqServerid    = Process::env.getInt("server_id");
    ack.clientSession    = clientSessionid;
    
    ack.errorcode   = 0;
    ack.roleid      = role->getInstID();
    ack.isrobot     = 0;
    ack.roletype    = role->getRoleType();
    ack.rolename    = role->getRolename();
    
    int lastlogin   = Game::tick - Utils::safe_atoi(role->loadProp(GetRolePropName(eLastLogin)).c_str(), 0);
    ack.lastlogin   = lastlogin;
    ack.lvl         = role->getLvl();
    ack.inte        = role->getInte();
    ack.phys        = role->getPhys();
    ack.stre        = role->getStre();
    ack.capa        = role->getCapa();
    
    ack.hp          = role->getMaxHp();
    ack.atk         = role->getAtk();
    ack.def         = role->getDef();
    ack.hit         = role->getHit();
    ack.dodge       = role->getDodge();
    ack.cri         = role->getCri();
    
    ack.enchantId   = role->getEnchantId();
    ack.enchantLvl  = role->getEnchantLvl();
    ack.awakeLvl    = role->getRoleAwakeLvl();
    
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

handler_msg(req_cs_get_role_Data, req)
{
    bool isError = true;
    do
    {
        bool shouldDelete = false;
        Role* roledata = SRoleMgr.GetRole(req.roleid);
        if (roledata == NULL)
        {
            shouldDelete = true;
            roledata = Role::create(req.roleid, true);
            
            if (roledata && !roledata->getRolename().empty())
            {
                roledata->onCalPlayerProp();
            }
            else
            {
                shouldDelete = false;
                delete roledata;
                roledata = NULL;
            }
        }
        
        if (roledata) {
            crossServicGetRoleData(sessionid, roledata, req.doReqServerid, req.clientSession);
        }
        
        if (shouldDelete) {
            delete roledata;
        }
    
        isError = false;
        
    } while (0);
    
    if (isError) {
        ack_cs_get_role_Data ack;
        ack.errorcode = 1;
        ack.roleid = req.roleid;
        ack.doReqServerid = req.doReqServerid;
        ack.onReqServerid = req.onReqServerid;
        ack.clientSession = req.clientSession;
        
        sendNetPacket(sessionid, &ack);
    }
}}

hander_msg(req_cs_get_pet_Data, req)
{
    if (req.onReqServerid != Process::env.getInt("server_id")) {
        return;
    }
    
    ack_cs_get_pet_Data ack;
    ack.errorcode = 1;
    ack.clientSession = req.clientSession;
    ack.doReqServerid = req.doReqServerid;
    ack.onReqServerid = req.onReqServerid;
    
    Pet pet(req.petid);
    if (pet.load(get_DbContext()))
    {
        ack.owner = pet.owner;
        ack.petinfo.petid = pet.petid;
        ack.petinfo.petmod = pet.petmod;
        ack.petinfo.growth = pet.getGrowth();
        ack.petinfo.lvl = pet.lvl;
        ack.petinfo.atk = pet.getAtk();
        ack.petinfo.def = pet.getDef();
        ack.petinfo.hp = pet.getMaxHp();
        ack.petinfo.maxHp = pet.getMaxHp();
        ack.petinfo.dodge = pet.getDodge();
        ack.petinfo.hit = pet.getHit();
        ack.petinfo.skills = pet.getSkill().toArray();
        ack.petinfo.capa = pet.mCapa;
        ack.petinfo.inte = pet.mInte;
        ack.petinfo.phy = pet.mPhys;
        ack.petinfo.stre = pet.mStre;
        ack.petinfo.stage = pet.getStage();
        ack.petinfo.cri = pet.getCri();
        ack.petinfo.mergelvl = pet.getMergeLvl();
        ack.petinfo.maxlvladd = pet.getMergeMaxlvlAdd();
    }
    
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_get_csRoleFashion_through_cs, req)
{
    if (req.onReqServerid != Process::env.getInt("server_id")) {
        return;
    }
    
    ack_get_csRoleFashion_through_cs ack;
    ack.doReqServerid = req.doReqServerid;
    ack.onReqServerid = req.onReqServerid;
    ack.clientSession = req.clientSession;
    ack.rankListType  = req.rankListType;
    
    for (int  i = 0; i < req.roleFashionInfos.size(); i ++) {
        obj_crossServiceRoleFashion_info info;
        
        int roleid = req.roleFashionInfos[i].roleid;
        int index  = req.roleFashionInfos[i].index;
        
        RedisResult reply(redisCmd("hmget role:%d weaponfashion bodyfashion homeFashion weaponqua roletype", roleid));
        int weaponfashion   = reply.readHash(0, 0);
        int bodyfashion     = reply.readHash(1, 0);
        int citybodyfashion = reply.readHash(2, 0);
        int weaponquality   = reply.readHash(3, 0);
        int roleType        = reply.readHash(4, 0);
        
        info.roleid = roleid;
        info.weaponFashion      = weaponfashion;
        info.bodyFashion        = bodyfashion;
        info.cityBodyFashion    = citybodyfashion;
        info.weaponquality      = weaponquality;
        info.roleType           = roleType;
        info.serverid           = Process::env.getInt("server_id");
        info.index              = index;
        
        ack.roleFashionInfos.push_back(info);
    }
    
    sendNetPacket(sessionid, &ack);
    
}}

hander_msg(notify_sync_refresh_crossServiceRankList, notify)
{
    notify_sync_fresh_crossServiceRankList notify;
    broadcastPacket(WORLD_GROUP, &notify);
}}





