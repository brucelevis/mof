//
//  Worker.cpp
//  MergeDB
//
//  Created by pireszhi on 14-1-22.
//  Copyright (c) 2014年 pireszhi. All rights reserved.
//

#include "Worker.h"
WorkerMgr _WORKERMGR;
#include "log.h"
#include "redis_helper.h"
#include "dbconnector.h"

void Worker::loadPlayerList()
{
    printf("加载玩家账号\n");

    RedisResult result((redisReply*)redisCommand(_conn._dbContextes, "keys player:*"));
    redisReply* reply = (redisReply*)result;

    for (int i = 0; i < reply->elements; i++) {
        if (reply->element[i]->type != REDIS_REPLY_STRING) {
            continue;
        }

        mPlayers.push_back(reply->element[i]->str);
    }
}

bool Worker::getPlayerRoles(vector<string> &player)
{
    for (int i = 0; i < player.size(); i++) {

        redisReply* reply = myRedisGetReply(_conn._dbContextes);
        if (reply == NULL) {
            return false;
        }

        string rolelist = "";
        if (reply->type == REDIS_REPLY_STRING) {
            rolelist = reply->str;
        }

        if (!rolelist.empty()) {

            StringTokenizer token(rolelist, ";");

            for (int index = 0; index < token.count(); index++) {

                int roleid = Utils::safe_atoi(token[index].c_str());
                if (roleid == 0) {
                    continue;
                }

                _WORKERMGR.addPlayerRoles(player[i], roleid);
                mCheckRolesId.insert(roleid);
            }
        }
        else{
            printf("没有角色的账号: %s  \n", player[i].c_str());
        }
        freeReplyObject(reply);
    }
    printf("处理账号[%s][%d]\n", _conn.serverName.c_str(), player.size());
    player.clear();
    return true;
}

bool Worker::rolesIdFliter()
{
    mRoleids.clear();
    vector<int> testRoles;
    testRoles.clear();
    time_t now = time(NULL);

    set<int>::iterator iter = mCheckRolesId.begin();
    printf(" 过滤前:%d\n", mCheckRolesId.size());

    for (; iter != mCheckRolesId.end(); iter++) {

        testRoles.push_back(*iter);

        if (REDIS_OK != redisAppendCommand(_conn._dbContextes, "hmget role:%d lastlogin vipexp rolename lvl", *iter)) {
            log_error("append get lastlogin and vipexp error");
            return false;
        }

        if (testRoles.size() >= 10000) {
            if ( ! onRolesIdFliter(testRoles, now)) {
                return false;
            }
        }
    }

    mCheckRolesId.clear();

    return onRolesIdFliter(testRoles, now);
}

bool Worker::onRolesIdFliter(vector<int> &testRoles, time_t now)
{
    for (int i = 0; i < testRoles.size(); i++)  {

        redisReply* reply = myRedisGetReply(_conn._dbContextes);
        if (reply == NULL) {
            log_error("get role lastlogin vipexp rolename lvl error");
            return false;
        }

        RedisResult result(reply);

        int lastlogin = (int)result.readHash(0, 0);
        int vipexp = (int)result.readHash(1, 0);
        string rolename = result.readHash(2, "");
        int rolelvl = result.readHash(3, 0);

        bool needDel = false;

        if (rolename.empty()) {
            needDel = true;
        }
        
        //不是VIP 且等级少于特定等级
        if ( (vipexp == 0 ) && rolelvl < _WORKERMGR.getValidMinLvl()) {
            
            //N天没登陆
            if ( lastlogin && (now - lastlogin) > (3600 * 24 * _WORKERMGR.getValidDays())) {
                
                needDel = true;
            }
        }
        
        

        if (needDel) {

            _WORKERMGR.mDelRoleIds.insert(testRoles[i]);

//            printf("被删掉的roleid: %d ----------------------------\n", testRoles[i]);

            continue;
        }

        mRoleids.push_back(testRoles[i]);
    }

    testRoles.clear();

    return true;
}

bool Worker::petIdFliter()
{
    mPetsId.clear();
    set<int>::iterator iter = mCheckPetsId.begin();
    for (; iter != mCheckPetsId.end(); iter++) {
        mPetsId.push_back(*iter);
    }

    mCheckPetsId.clear();
    return true;
}

bool Worker::onMerge()
{
    //先迁移公会
    if ( !mergeGuild() )
        return false;

    //读取所有的角色id
    vector<string> player_roles;
    player_roles.clear();

    for (int i = 0; i < mPlayers.size(); i++) {

        if (REDIS_OK != redisAppendCommand(_conn._dbContextes, "hget %s roleids", mPlayers[i].c_str())) {
            return false;
        }

        player_roles.push_back(mPlayers[i]);

        //一定数目后立即读取缓冲区
        if (player_roles.size() >= 5000) {

            if(!getPlayerRoles(player_roles))
            {
                return false;
            }
        }
    }

    if (!getPlayerRoles(player_roles))
    {
        return false;
    }

    log_info("玩家账号总数:"<<mPlayers.size()<<"-------------------------------");

    rolesIdFliter();

    log_info("有角色的账号总数:"<<_WORKERMGR.mPlayers.size()<<"-------------------------------");

    log_info("读取全服角色id完成 共 :"<<mRoleids.size());

    //合并存档
    if (!mergeRoles())
    {
        return false;
    }

    log_info("角色存档搬迁完成");

    //修改角色名
    if (!roleRename()) {
        return false;
    }
    log_info("角色重命名完成");

    //修改公会id
    if (!resetRoleGuildId()) {
        return false;
    }
    log_info("角色公会id修改完成");

    //删除一些没用的字段
    if(!removeRoleProps())
    {
        return false;
    }
    log_info("删除没用的属性完成");

    //加载全服宠物列表并修改角色的RolePropPetList属性
    if (!loadPetsId()) {
        return false;
    }
    petIdFliter();
    log_info("加载宠物id完成");

    if (!mergePets()) {
        return false;
    }
    log_info("迁移宠物存档完成");

    return true;
}

/***********************************  分界线  ***********************************/

bool Worker::loadAllGuildid()
{
    mGuildAllId.clear();
    RedisResult result(redisCmd(_conn._dbContextes, "get gen_guildid"));
    int genGuildid = (int)result.readInt();
    vector<int> tempId;
    tempId.clear();

    for (int i = 1; i <= genGuildid; i++) {
        if (REDIS_OK != redisAppendCommand(_conn._dbContextes, "exists guild:%d", i)) {
            log_error("append exist guild error");
            return false;
        }

        tempId.push_back(i);

        if (tempId.size() >= 5000) {

            if( !onLoadAllGuildId(tempId) )
            {
                return false;
            }
        }
    }

    return onLoadAllGuildId(tempId);
}

bool Worker::onLoadAllGuildId(vector<int>& tempId)
{
    for (int i = 0; i < tempId.size(); i++) {
        redisReply* reply = myRedisGetReply(_conn._dbContextes);
        if (reply == NULL) {
            log_error("get exists guild error");
            return false;
        }

        RedisResult result(reply);
        int isExist = (int)result.readInt();

        if (isExist) {
            mGuildAllId.push_back(tempId[i]);
        }
    }

    tempId.clear();

    return true;
}

bool Worker::mergeGuild()
{

    if ( !loadAllGuildid()) {
        return false;
    }

    vector<int> mergeGuild;
    mergeGuild.clear();

    for (int i = 0 ; i < mGuildAllId.size(); i++)
    {
        int guildid = mGuildAllId[i];

        if (!dumpGuildKeys(guildid)) {
            return false;
        }
        if (guildid == 0) {
            printf("sds\n");
        }
        mergeGuild.push_back(guildid);

        if (mergeGuild.size() >= 5000) {
            if (!restoreGuild(mergeGuild)) {
                return false;
            }
        }
    }

    if (!restoreGuild(mergeGuild)) {
        return false;
    }

    stringstream cmd;
    cmd<<"hmset guildallid:0";
    for (int i = 0; i < mGuildAllId.size(); i++) {

        int newid = mGuildAllId[i] + _WORKERMGR.getGeneralGuildId();
        cmd<<" ";
        cmd<<newid; //key
        cmd<<" ";
        cmd<<newid; //val
    }

    doRedisCmd(g_DBMgr.mMaster._dbContextes, cmd.str().c_str());
    mGuildAllId.clear();
    return true;
}

bool Worker::dumpGuildKeys(int guildid)
{
    for (int i = 0; i < _WORKERMGR.mGuildKeysName.size(); i++) {

        if (REDIS_OK != redisAppendCommand(_conn._dbContextes, "dump %s:%d", _WORKERMGR.mGuildKeysName[i].c_str(), guildid)) {

            log_error("append "<<_WORKERMGR.mGuildKeysName[i]<<"dump data error roleid:"<<guildid);
            return false;
        }
    }

    if (REDIS_OK != redisAppendCommand(_conn._dbContextes, "hget guild:%d guildname", guildid)) {
        log_error("append get guildname error");
        return false;
    }

    return true;
}
bool Worker::onRestoreGuild(int guildid, vector<KeysInfo> &guildinfo)
{
    KeysInfo info;
    info.ownerId = guildid;

    int newGuildid = guildid + _WORKERMGR.getGeneralGuildId();

    //get and append 公会keys
    for (int i = 0; i < _WORKERMGR.mGuildKeysName.size(); i++) {

        redisReply* reply = myRedisGetReply(_conn._dbContextes);
        if ( reply == NULL){
            log_error("get "<<_WORKERMGR.mGuildKeysName[i]<<":"<<guildid<<" dump data error");
            return false;
        }
        else if(reply->type == REDIS_REPLY_NIL){
            log_info("lose key -- "<<_WORKERMGR.mGuildKeysName[i]<<":"<<guildid);
            continue;
        }

        string keyStr = Utils::makeStr("%s:%d", _WORKERMGR.mGuildKeysName[i].c_str(), newGuildid);

        if (reply->type == REDIS_REPLY_STRING) {
            if (REDIS_OK != redisAppendCommand(g_DBMgr.mMaster._dbContextes, "restore %s 0 %b ", keyStr.c_str(), reply->str, (size_t)reply->len)) {
                log_error("append "<<keyStr<<"restore data error !");
                return false;
            }

            info.mEffectKeysIndexs.push_back(i);

        }
        freeReplyObject(reply);
    }

    redisReply* reply = myRedisGetReply(_conn._dbContextes);
    if (reply == NULL) {
        log_error("get guildname error");
        return false;
    }

    //append 更改公会名称命令
    RedisResult result(reply);
    string oldname = result.readStr();
    string newname = oldname;
    newname.append("(");
    newname.append(_conn.serverName.c_str());
    newname.append(")");

    if (REDIS_OK != redisAppendCommand(g_DBMgr.mMaster._dbContextes, "hset guild:%d guildname %s", newGuildid, newname.c_str()))
    {
        log_error("append set guild newname error");
        return false;
    }

    guildinfo.push_back(info);

    return true;
}

bool Worker::restoreGuild(vector<int>& guilds)
{
    printf("合并公会:%d\n", (int)guilds.size());

    vector<KeysInfo> guildsInfo;
    for (int i = 0 ; i < guilds.size(); i++) {

        if(!onRestoreGuild(guilds[i], guildsInfo))
        {
            return false;
        }
    }

    //提交到目标库
    for (int i = 0; i < guildsInfo.size(); i++) {

        for (int j = 0; j < guildsInfo[i].mEffectKeysIndexs.size(); j++) {

            redisReply* reply = myRedisGetReply(g_DBMgr.mMaster._dbContextes);

            if( reply != NULL && strcmp(reply->str, "OK") != 0)
            {
                log_error(" ERROR: "<<reply->str);
                return false;
            }
            freeReplyObject(reply);
        }

        redisReply* reply = myRedisGetReply(g_DBMgr.mMaster._dbContextes);
        if (reply == NULL) {
            log_error("commit set guild newname  error");
            return false;
        }
        freeReplyObject(reply);
    }

    printf("move guild %d------------------------------------------------\n", (int)guilds.size());

    guilds.clear();

    return true;
}

bool Worker::mergeRoles()
{
    vector<int> mergeRoles;
    mergeRoles.clear();

    for (int i = 0; i < mRoleids.size(); i++) {

        if ( !dumpRoles(mRoleids[i]) )
        {
            return false;
        }

        mergeRoles.push_back(mRoleids[i]);

        if(mergeRoles.size() >= 5000)
        {
            if(!reStoreRoles(mergeRoles))
            {
                return false;
            }
        }
    }

    return reStoreRoles(mergeRoles);
}

bool Worker::mergePets(int roleid)
{
    vector<int> newPetIds;
    const char* props[] =
    {
        "RolePropPetList",
        "activepetid"
    };

    string allprops = StrJoin(props, &props[sizeof(props) / sizeof(const char*) - 1], " ");
    string cmd = Utils::makeStr("hmget role:%d ", roleid);
    cmd.append(allprops.c_str());

    RedisResult result((redisReply*)redisCommand(_conn._dbContextes, cmd.c_str()));

    string petsStr = result.readHash(0, "");
    int activePet = result.readHash(1, 0);

    vector<string> petids = StrSpilt(petsStr, ";,");

    //从被合服务器读取存档
    for (int i = 0; i < petids.size(); i++) {
        int oldeid = Utils::safe_atoi(petids[i].c_str());
        if (!oldeid) {
            continue;
        }

        if (REDIS_OK != redisAppendCommand(_conn._dbContextes, "dump pet:%d", oldeid)) {
            log_error("dump petdata error:petid "<<oldeid);
            return false;
        }
        int newpetid = oldeid + _WORKERMGR.getGeneralPetId();
        newPetIds.push_back(newpetid);
    }

    //重置到目标服务器，并以新的宠物id存储
    for (int i = 0; i < newPetIds.size(); i++) {

        redisReply* reply = myRedisGetReply(_conn._dbContextes);
        if(reply == NULL)
        {
            log_error("get dump petdata return error");
            return false;
        }

        if (REDIS_OK != redisAppendCommand(g_DBMgr.mMaster._dbContextes, "restore pet:%d 0 %b ", newPetIds[i], reply->str, (size_t)reply->len)) {
            log_error("append pet restore data error petid:"<<newPetIds[i]);
            return false;
        }
        freeReplyObject(reply);
    }

    stringstream newPetlist;
    //piple提交到目标数据库
    for (int i = 0; i < newPetIds.size(); i++) {
        redisReply* reply = myRedisGetReply(g_DBMgr.mMaster._dbContextes);
        if (reply == NULL) {
            log_error("commit pet restore data error petid:"<<newPetIds[i]);
            return false;
        }
        freeReplyObject(reply);

        newPetlist<<newPetIds[i];
        newPetlist<<",";
    }

    if (activePet) {
        activePet += _WORKERMGR.getGeneralPetId();
    }

    //保存新的宠物列表
    doRedisCmd(g_DBMgr.mMaster._dbContextes, "hmset role:%d activepetid %d RolePropPetList %s", roleid, activePet, newPetlist.str().c_str());

    return true;
}

bool Worker::dumpRoles(int roleid)
{
    //append各字段名
    for (int i = 0; i < _WORKERMGR.mRoleKeysName.size(); i++) {

        if (REDIS_OK != redisAppendCommand(_conn._dbContextes, "dump %s:%d", _WORKERMGR.mRoleKeysName[i].c_str(), roleid)) {

            log_error("append "<<_WORKERMGR.mRoleKeysName[i]<<"dump data error roleid:"<<roleid);
            return false;
        }
    }

    return true;
}

bool Worker::reStoreRoles(vector<int>& roles)
{
    vector<KeysInfo> rolesInfo;
    for (int i = 0 ; i < roles.size(); i++) {

        if(!onRestoreRoles(roles[i], rolesInfo))
        {
            return false;
        }
    }

    //提交到目标库
    for (int i = 0; i < rolesInfo.size(); i++) {

        for (int j = 0; j < rolesInfo[i].mEffectKeysIndexs.size(); j++) {

            redisReply* reply = myRedisGetReply(g_DBMgr.mMaster._dbContextes);

            string roleKey = _WORKERMGR.mRoleKeysName[rolesInfo[i].mEffectKeysIndexs[j]];
            
            if( reply != NULL && strcmp(reply->str, "OK") != 0)
            {
                log_error("RESTORE KEY "<<roleKey<<":"<<rolesInfo[i].ownerId<<" ERROR: "<<reply->str);
                return false;
            }
            freeReplyObject(reply);
        }
    }

    printf("move roleid %d------------------------------------------------\n", (int)roles.size());

    roles.clear();

    return true;
}

bool Worker::onRestoreRoles(int roleid, vector<KeysInfo>& roleinfo)
{
    KeysInfo info;
    info.ownerId = roleid;

    //读取个字段的dump数据
    for (int i = 0; i < _WORKERMGR.mRoleKeysName.size(); i++) {

        redisReply* reply = myRedisGetReply(_conn._dbContextes);

        if (reply == NULL) {
            log_error("get "<<_WORKERMGR.mRoleKeysName[i]<<" dump data error roleid: "<<roleid);
            return false;
        }

        string keyStr = Utils::makeStr("%s:%d", _WORKERMGR.mRoleKeysName[i].c_str(), roleid);

        if (reply->type == REDIS_REPLY_STRING) {

            if(REDIS_OK != redisAppendCommand(g_DBMgr.mMaster._dbContextes, "restore %s 0 %b ", keyStr.c_str(), reply->str, (size_t)reply->len)) {
                log_error("append "<<keyStr<<"restore data error !");
                return false;
            }
            info.mEffectKeysIndexs.push_back(i);
        }
        freeReplyObject(reply);
    }

    roleinfo.push_back(info);

    return true;
}

bool Worker::roleRename()
{
    vector<int> mergeRoles;
    mergeRoles.clear();

    for (int i = 0; i < mRoleids.size(); i++) {
        if (REDIS_OK != redisAppendCommand(_conn._dbContextes, "hget role:%d rolename", mRoleids[i])) {
            log_error("get role:"<<mRoleids[i]<<" name error");
            return false;
        }
        mergeRoles.push_back(mRoleids[i]);

        if (mergeRoles.size() >= 1000) {
            if (!onRoleRename(mergeRoles))
            {
                return false;
            }
        }
    }
    return onRoleRename(mergeRoles);
}

bool Worker::onRoleRename(vector<int>& roles)
{
    for (int i = 0; i < roles.size(); i++) {
        redisReply* reply = myRedisGetReply(_conn._dbContextes);

        string name = "";
        if (reply && reply->str) {
            name = reply->str;
        }

        name.append("(");
        name.append(_conn.serverName.c_str());
        name.append(")");

        if (REDIS_OK != redisAppendCommand(g_DBMgr.mMaster._dbContextes, "hset role:%d rolename %s", roles[i], name.c_str()) ||
            REDIS_OK != redisAppendCommand(g_DBMgr.mMaster._dbContextes, "set rolename:%s %d", name.c_str(), roles[i]))
        {
            log_error("append name error !");
            return false;
        }
        freeReplyObject(reply);
    }

    for (int i = 0; i < roles.size(); i++) {
        redisReply* reply = NULL;

        reply = myRedisGetReply(g_DBMgr.mMaster._dbContextes);
        freeReplyObject(reply);

        redisGetReply(g_DBMgr.mMaster._dbContextes, (void**)&reply);
        if (reply && strcmp(reply->str, "OK") != 0) {
            log_error("rename error roleid:"<<roles[i]);
            return false;
        }
        freeReplyObject(reply);
    }
    roles.clear();
    return true;
}

bool Worker::resetRoleGuildId()
{
    vector<int> mergeRoles;
    mergeRoles.clear();

    for (int i = 0; i <mRoleids.size(); i++) {

        if (REDIS_OK != redisAppendCommand(_conn._dbContextes, "hget roleguild:%d guild", mRoleids[i])) {
            log_error("get role:"<<mRoleids[i]<<" name error");
            return false;
        }
        mergeRoles.push_back(mRoleids[i]);
        if (mergeRoles.size() >= 10000) {
            if (!onResetRoleGuildId(mergeRoles)) {
                return false;
            }
        }
    }

    return onResetRoleGuildId(mergeRoles);
}

bool Worker::onResetRoleGuildId(vector<int> &roles)
{
    int commandCount = 0;

    for (int i = 0; i < roles.size(); i++) {
        redisReply* reply = myRedisGetReply(_conn._dbContextes);

        if (reply == NULL || reply->type == REDIS_REPLY_NIL) {
            continue;
        }

        RedisResult result(reply);
        int guildid = (int)result.readInt(0);

        if (guildid) {
            guildid += _WORKERMGR.getGeneralGuildId();
            if (REDIS_OK != redisAppendCommand(g_DBMgr.mMaster._dbContextes, "hset roleguild:%d guild %d", roles[i], guildid))
            {
                return false;
            }
            commandCount++;
        }
    }

    for (int i = 0; i < commandCount; i++) {

        redisReply* reply = myRedisGetReply(g_DBMgr.mMaster._dbContextes);

        freeReplyObject(reply);
    }

    printf("reset roleguildid %d ----------------\n", (int)roles.size());
    roles.clear();

    return true;
}

bool Worker::removeRoleProps()
{
    vector<int> mergeRoles;
    mergeRoles.clear();

    std::vector<string> props;
    props.clear();
    
    props.push_back("roleassistpets");
    
    props.push_back("petpvpform");
    props.push_back("petpvpwintimes");
    props.push_back("petpvplosetimes");
    props.push_back("petpvpformszie");
    props.push_back("petpvpcups");
    
    string cmdProp = "";
    
    for (int i = 0; i < props.size(); i++) {
        cmdProp.append(" ");
        cmdProp.append(props[i]);
    }
    
    if (cmdProp.size() <= 1) {
        return true;
    }
    
    for (int i = 0; i < mRoleids.size(); i++) {
        
        string cmd = Utils::makeStr("hdel role:%d%s", mRoleids[i], cmdProp.c_str());
        
        if(REDIS_OK != redisAppendCommand(g_DBMgr.mMaster._dbContextes, "hdel role:%d petpvpform roleassistpets", mRoleids[i]))
        {
            return false;
        }

        mergeRoles.push_back(mRoleids[i]);

        if (mergeRoles.size() >= 10) {
            if (!onRemoveRolePeops(mergeRoles)) {
                return false;
            }
        }
    }
    return onRemoveRolePeops(mergeRoles);
}

bool Worker::onRemoveRolePeops(vector<int> &roles)
{
    for (int i = 0; i < roles.size(); i++) {
        redisReply* reply = myRedisGetReply(g_DBMgr.mMaster._dbContextes);
        if ( reply == NULL ){
            log_error("commite remove role keys error");
            return false;
        }
        freeReplyObject(reply);
    }

    roles.clear();
    return true;
}

bool Worker::loadPetsId()
{
    vector<int> mergeRoles;
    mergeRoles.clear();

    for (int i = 0; i < mRoleids.size(); i++) {

        if (REDIS_OK != redisAppendCommand(_conn._dbContextes, "hget role:%d RolePropPetList", mRoleids[i])) {
            log_error("append loadpetids command error");
            return false;
        }

        mergeRoles.push_back(mRoleids[i]);

        if (mergeRoles.size() >= 1000) {
            if (!onLoadPetsId(mergeRoles)) {
                return false;
            }
        }
    }
    return onLoadPetsId(mergeRoles);
}

bool Worker::onLoadPetsId(vector<int> &roles)
{
    for (int i = 0; i < roles.size(); i++) {
        redisReply* reply = myRedisGetReply(_conn._dbContextes);
        if (reply == NULL) {
            return false;
        }

        RedisResult result(reply);
        string petlist = result.readStr();

        string newlist = makeNewPetlist(petlist);

        if (REDIS_OK != redisAppendCommand(g_DBMgr.mMaster._dbContextes, "hset role:%d RolePropPetList %s", roles[i], newlist.c_str())) {
            log_error("append newpetlist erro");
            return false;
        }
    }

    for (int i = 0; i < roles.size(); i++) {
        redisReply* reply = myRedisGetReply(g_DBMgr.mMaster._dbContextes);

        if (reply == NULL) {
            log_error("commit newpetlist error");
            return false;
        }

        freeReplyObject(reply);
    }

    printf("load and recode petid %d -----------------\n", (int)roles.size());
    roles.clear();
    return true;
}

string Worker::makeNewPetlist(string petlist)
{
    stringstream newlist;
    StringTokenizer token(petlist, ",");
    for (int i = 0; i < token.count(); i++) {
        int oldid = Utils::safe_atoi(token[i].c_str());
        int newid = oldid + _WORKERMGR.getGeneralPetId();

        mCheckPetsId.insert(oldid);
        newlist<<newid<<",";
    }
    return newlist.str();
}

bool Worker::mergePets()
{
    vector<int> mergePets;
    mergePets.clear();

    for (int i = 0; i < mPetsId.size(); i++) {
        if (REDIS_OK != redisAppendCommand(_conn._dbContextes, "dump pet:%d", mPetsId[i])) {
            log_error("append dump pet:* error");
            return false;
        }

        mergePets.push_back(mPetsId[i]);
        if (mergePets.size() >= 10000) {
            if (!onMergePets(mergePets)) {
                return false;
            }
        }
    }
    return onMergePets(mergePets);
}

bool Worker::onMergePets(vector<int>& pets)
{
    vector<int> handledPets;
    for (int i = 0; i < pets.size(); i++) {
        redisReply* reply = myRedisGetReply(_conn._dbContextes);
        if (reply == NULL) {
            log_error("get pet dump data error");
            return false;
        } else if (reply->type == REDIS_REPLY_NIL) {
            log_error("get pet dump data error with pet:" << pets[i]);
            continue;
        }
        
        handledPets.push_back(pets[i]);
        
        int newpetid = pets[i] + _WORKERMGR.getGeneralPetId();

        if (REDIS_OK != redisAppendCommand(g_DBMgr.mMaster._dbContextes, "restore pet:%d 0 %b", newpetid, reply->str, reply->len)) {
            log_error("append restore pet data error");
            return false;
        }
        freeReplyObject(reply);
    }

    for (int i = 0; i < handledPets.size(); i++) {
        redisReply* reply = myRedisGetReply(g_DBMgr.mMaster._dbContextes);
        if (reply == NULL || strcmp(reply->str, "OK") != 0) {
            log_error("commit restore pet:" << handledPets[i] << "'s data error:" << reply->str);
            return false;
        }
        freeReplyObject(reply);
    }
    printf("restor pet data %d ----------------------------\n", (int)pets.size());
    pets.clear();
    return true;
}

bool Worker::updateGeneralIds()
{
    RedisResult guildResult(redisCmd(_conn._dbContextes, "get gen_guildid"));
    int genGuildid = (int)guildResult.readInt();
    _WORKERMGR.setGeneralGuildid(genGuildid);

    RedisResult petResult(redisCmd(_conn._dbContextes, "get pet:incr_id"));
    int genPetid = (int)petResult.readInt(0);
    _WORKERMGR.setGeneralPetid(genPetid);

    RedisResult roleResult(redisCmd(_conn._dbContextes, "get gen_roleid"));
    int genRoleid = (int)roleResult.readInt();
    _WORKERMGR.setGeneralRoleid(genRoleid);

    return true;
}

bool Worker::mergeActivityData()
{
    std::map<string, string> content;
    
    int keysCount = _WORKERMGR.mActivityKeys.size();
    
    for (int i = 0; i < keysCount; i++) {
        string keyName = _WORKERMGR.mActivityKeys[i];
        
        printf("开始合并 %s \n", keyName.c_str());
        
        if(!onMergeActivityData(keyName))
        {
            return false;
        }
        
        printf("合并 %s 成功\n", keyName.c_str());
    }
    return true;
}

void Worker::loadActivityKeys(string configkey, vector<string> &keys)
{
    redisReply* reply = (redisReply*)redisCommand(_conn._dbContextes, "keys %s*", configkey.c_str());
    
    for (int i = 0; i < reply->elements; i++) {
        
        if (reply->element[i]->str) {
            keys.push_back(reply->element[i]->str);
        }
        
    }
    
    freeReplyObject(reply);
}

bool Worker::onMergeActivityData(string keyName)
{
    vector<string> realkeys;
    realkeys.clear();
    
    loadActivityKeys(keyName, realkeys);
    
    int keysCount = realkeys.size();
    
    for (int i = 0; i < keysCount; i++) {
        if(!onMergeAcivityKey(realkeys[i]))
        {
            log_error("合并 "<<realkeys[i]<<"错误!");
            return false;
        }
    }
    return true;
}

bool Worker::onMergeAcivityKey(string realKey)
{
    vector<string> tmp;
    tmp.clear();
    
    vector<const char*> args;
    args.clear();
    
    args.push_back("hmset");
    args.push_back(realKey.c_str());
    
    bool isEmpty = true;
    
    redisReply* reply = (redisReply*)redisCommand(_conn._dbContextes, "hgetall %s", realKey.c_str());
    
    for (int i = 0; i < reply->elements/2; i++) {
        string key = (reply->element[i*2])->str;
        string val = (reply->element[i*2+1])->str;
        
        tmp.push_back(key);
        tmp.push_back(val);
        
        isEmpty = false;
        
    }
    
    freeReplyObject(reply);
    
    int tmpsize = tmp.size();
    
    for (int i = 0; i < tmpsize; i++) {
        args.push_back(tmp[i].c_str());
    }
    
    int ret = 0;
    
    if (!isEmpty) {
        
        reply = (redisReply*)redisCommandArgv(g_DBMgr.mMaster._dbContextes, args.size(), args.data(), NULL);
        
        if (reply != NULL) {
            ret = strcasecmp("OK", reply->str);
        }
        
        freeReplyObject(reply);
    }

    return ret == 0;
}

bool Worker::mergeGlobalMaxVal()
{
    for (int i = 0; i < _WORKERMGR.mMergeMaxKey.size(); i++) {
        string keyName = _WORKERMGR.mMergeMaxKey[i];
        
        redisReply* reply = (redisReply*)redisCommand(_conn._dbContextes, "type %s", keyName.c_str());
        
        string replyType = "";
        
        if (reply)
        {
            replyType = reply->str;
        }
        
        freeReplyObject(reply);
        
        if ( 0 == strcmp(replyType.c_str(), "string") ) {
            RedisResult result((redisReply*)redisCommand(_conn._dbContextes, "get %s", keyName.c_str()));
            
            int val = result.readInt();
            
            _WORKERMGR.updateGlobalMaxVal(keyName, val);
        }
    }
    return true;
}

bool Worker::mergeGlobalMinVal()
{
    for (int i = 0; i < _WORKERMGR.mMergeMinKey.size(); i++) {
        string keyName = _WORKERMGR.mMergeMinKey[i];
        
        redisReply* reply = (redisReply*)redisCommand(_conn._dbContextes, "type %s", keyName.c_str());
        
        string replyType = "";
        
        if (reply)
        {
            replyType = reply->str;
        }
        
        freeReplyObject(reply);
        
        if ( 0 == strcmp(replyType.c_str(), "string") ) {
            RedisResult result((redisReply*)redisCommand(_conn._dbContextes, "get %s", keyName.c_str()));
            
            int val = result.readInt();
            
            _WORKERMGR.updateGlobalMinVal(keyName, val);
        }
    }
    
    return true;
}

/*************
   WorkerMgr
 ************/

bool WorkerMgr::init()
{
    return loadConfig();
}

bool WorkerMgr::loadConfig()
{
    string exePath = GetAppPath();
    string fullpath = exePath + "/merge_keys.ini";
    IniFile inifile(fullpath.c_str());

    mValidDays = inifile.getValueT("common", "valid_days", 0);
    mValidMinLvl = inifile.getValueT("common", "min_lvl", 0);

    for(int i = 1;; i++)
    {
        string keyNum = Utils::makeStr("key%d", i);
        string keysVal = inifile.getValue("RoleKeys", keyNum.c_str());

        if (keysVal.empty()) {
            break;
        }
        
        log_info("要合并的角色字段-"<<i<<"---------"<<keysVal);
        mRoleKeysName.push_back(keysVal);
    }

    for(int i = 1;; i++)
    {
        string keyNum = Utils::makeStr("key%d", i);
        string keysVal = inifile.getValue("GuildKeys", keyNum.c_str());

        if (keysVal.empty()) {
            break;
        }
        
        log_info("要合并的公会字段-"<<i<<"---------"<<keysVal);
        mGuildKeysName.push_back(keysVal);
    }

    if (mRoleKeysName.size() == 0 || mGuildKeysName.size() == 0) {
        log_error("角色和公会的keys名字不能为空");
        return false;
    }
    
    for(int i = 1;; i++)
    {
        string keyNum = Utils::makeStr("key%d", i);
        string keysVal = inifile.getValue("activityKeys", keyNum.c_str());
        
        if (keysVal.empty()) {
            break;
        }
        
        mActivityKeys.push_back(keysVal);
    }
    
    for(int i = 1;; i++)
    {
        string keyNum = Utils::makeStr("key%d", i);
        string keysVal = inifile.getValue("merge_max_val", keyNum.c_str());
        
        if (keysVal.empty()) {
            break;
        }
        
        mMergeMaxKey.push_back(keysVal);
    }

    for(int i = 1;; i++)
    {
        string keyNum = Utils::makeStr("key%d", i);
        string keysVal = inifile.getValue("merge_min_val", keyNum.c_str());
        
        if (keysVal.empty()) {
            break;
        }
        
        mMergeMinKey.push_back(keysVal);
    }
    
    return true;
}

bool WorkerMgr::addPlayerRoles(string playerName, int roleid)
{
    map<string, RoleList>::iterator iter = mPlayers.find(playerName);

    if (iter == mPlayers.end()) {
        RoleList newlist;
        newlist.clear();
        newlist.push_back(roleid);
        mPlayers.insert(make_pair(playerName, newlist));
    }
    else{
        iter->second.push_back(roleid);
    }
    return true;
}

void WorkerMgr::doMerge()
{
    redisReply* reply = (redisReply*)redisCommand(g_DBMgr.mMaster._dbContextes, "flushdb");
    if ( strcmp(reply->str, "OK") != 0) {
        return;
    }

    for (int i = 0; i < g_DBMgr.mSlaves.size(); i++) {

        Worker slaveWorker(g_DBMgr.mSlaves[i]);

        slaveWorker.loadPlayerList();

        if(!slaveWorker.onMerge())
        {
            log_error("domerge faild!");
            return;
        }

        slaveWorker.updateGeneralIds();
        
        //搬迁活动数据
        slaveWorker.mergeActivityData();
        
        //一些全局的set值
        slaveWorker.mergeGlobalMaxVal();
        
        slaveWorker.mergeGlobalMinVal();

        log_info("move slave["<<i + 1<<"] successfully! \n");
    }

    savePlayerRoleList();
    saveGeneralIds();
    
    saveMaxVal();
    saveMinVal();
    
    log_info("domerge successfully !");
    return;
}

bool WorkerMgr::savePlayerRoleList()
{
    map<string, RoleList>::iterator iter = mPlayers.begin();
    int playerCount = 0;

    for (; iter != mPlayers.end(); iter++) {
        stringstream list;
        for (int i = 0; i < iter->second.size(); i++) {
            set<int>::iterator delIter = mDelRoleIds.find(iter->second[i]);

            if (delIter != mDelRoleIds.end()) {
                mDelRoleIds.erase(delIter);
                continue;
            }

            list<<iter->second[i];
            list<<";";
        }

        string rolelist = list.str();
        if (rolelist.empty()) {
            continue;
        }

        if (REDIS_OK != redisAppendCommand(g_DBMgr.mMaster._dbContextes, "hset %s roleids %s", iter->first.c_str(), list.str().c_str())) {
            log_error("append save player rolelist error");
        }

        playerCount++;

        if (playerCount >= 10000) {
            if (!onSavePlayerRoleList(playerCount)) {
                return false;
            }
        }
    }

    return onSavePlayerRoleList(playerCount);
}

bool WorkerMgr::onSavePlayerRoleList(int &playerCount)
{
    for (int i = 0; i < playerCount; i++) {
        redisReply* reply = myRedisGetReply(g_DBMgr.mMaster._dbContextes);
        if (reply == NULL) {
            log_error("commit save player rolelist error");
            return false;
        }

        freeReplyObject(reply);
    }

    playerCount = 0;
    return true;
}

void WorkerMgr::saveGeneralIds()
{
    doRedisCmd(g_DBMgr.mMaster._dbContextes, "set gen_roleid %d", gen_roleid);
    doRedisCmd(g_DBMgr.mMaster._dbContextes, "set gen_guildid %d", gen_guildid);
    doRedisCmd(g_DBMgr.mMaster._dbContextes, "set pet:incr_id %d", gen_petid);
}

void WorkerMgr::updateGlobalMaxVal(string keyName, int val)
{
    map<string, int>::iterator iter = mMaxVal.find(keyName);
    
    if (iter == mMaxVal.end()) {
        mMaxVal.insert(make_pair(keyName, val));
        return;
    }
    else
    {
        if (iter->second < val) {
            iter->second = val;
        }
    }
}

void WorkerMgr::updateGlobalMinVal(string keyName, int val)
{
    map<string, int>::iterator iter = mMinVal.find(keyName);
    
    if (iter == mMinVal.end()) {
        mMinVal.insert(make_pair(keyName, val));
        return;
    }
    else
    {
        if (iter->second > val) {
            iter->second = val;
        }
    }
}

void WorkerMgr::saveMaxVal()
{
    map<string, int>::iterator iter = mMaxVal.begin();
    
    //append各字段名
    for (; iter != mMaxVal.end(); iter++) {
        
        if (REDIS_OK != redisAppendCommand(g_DBMgr.mMaster._dbContextes, "set %s %d", iter->first.c_str(), iter->second)) {
            
            log_error("append "<<iter->first<<"reset globalMaxVal error val:"<<iter->second);
            return;
        }
    }
    
    
    for (iter = mMaxVal.begin(); iter != mMaxVal.end(); iter++) {
        
        redisReply* reply = myRedisGetReply(g_DBMgr.mMaster._dbContextes);
        
        if (reply == NULL) {
            log_error("reset "<<iter->first<<" error!");
            return;
        }
        
        freeReplyObject(reply);
    }

}

void WorkerMgr::saveMinVal()
{
    map<string, int>::iterator iter = mMinVal.begin();
    
    //append各字段名
    for (; iter != mMinVal.end(); iter++) {
        
        if (REDIS_OK != redisAppendCommand(g_DBMgr.mMaster._dbContextes, "set %s %d", iter->first.c_str(), iter->second)) {
            
            log_error("append "<<iter->first<<"reset globalMinVal error val:"<<iter->second);
            return;
        }
    }
    
    for (iter = mMinVal.begin(); iter != mMinVal.end(); iter++) {
        
        redisReply* reply = myRedisGetReply(g_DBMgr.mMaster._dbContextes);
        
        if (reply == NULL) {
            log_error("reset "<<iter->first<<" error!");
            return;
        }
        
        freeReplyObject(reply);
    }
}