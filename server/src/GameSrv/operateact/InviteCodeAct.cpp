//
//  InviteCodeAct.cpp
//  GameSrv
//
//  Created by hekang on 14-7-14.
//
//

#include "InviteCodeAct.h"

#include <iostream>
#include <string>
#include <string.h>
#include <sstream>
#include "Utils.h"
#include "Role.h"

int my36atoi(char val){
    /*索引表*/
    char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int i=0;
    int j=sizeof(index)/sizeof(index[0]);
    for(;i<j;++i){
        if(index[i]==val){
            return i;
        }
    }
    return -1;
}
char* myitoa(int num,char *str,int radix)
{
    /*索引表*/
    char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    unsigned unum;/*中间变量*/
    int i=0,j,k;
    /*确定unum的值*/
    if(radix==10&&num<0)/*十进制负数*/
    {
        unum=(unsigned)-num;
        str[i++]='-';
    }
    else unum=(unsigned)num;/*其他情况*/
    /*转换*/
    do{
        str[i++]=index[unum%(unsigned)radix];
        unum/=radix;
    }while(unum);
    str[i]='\0';
    /*逆序*/
    if(str[0]=='-')k=1;/*十进制负数*/
    else k=0;
    char temp;
    for(j=k;j<=(i-1)/2;j++)
    {
        temp=str[j];
        str[j]=str[i-1+k-j];
        str[i-1+k-j]=temp;
    }
    return str;
}


char ToLowCase(char val){
    if(val >= 'A' && val <= 'Z'){
        return val|0x20;
    }
    return val;
}
char ToUpCase(char val){
    if(val >= 'a' && val <= 'z'){
        return val-0x20;
    }
    return val;
}
//char* myitoa(char num,char *str,int radix)
//{
//    num=ToUpCase(num);
//    int tmpNum=my36atoi(num);
//    if (tmpNum==-1) {
//        return "\0";
//    }
//    return myitoa(tmpNum,str,radix);
//}

std::string intToUUID8(int ival=0){
    char cUUID[9];
    memset(cUUID,0,9);
    char cbinary[255];
    memset(cbinary,0,255);
    int flag=0x0000000f;
    std::stringstream ss;
    //for(int i=0;i<=7;++i){
    for(int i=7;i>=0;--i){
        int iflag = flag<<(i*4);
        int itmp = ival & iflag;
        itmp = itmp>>(i*4);
        itmp = itmp<<1;
        memset(cbinary,0,255);
        myitoa(itmp,cbinary,36);
        ss<<cbinary;
    }
    std::string sResult;
    std::getline(ss,sResult);
    return sResult;
    
}
int UUID8ToInt(std::string & str){
    std::string::reverse_iterator rbeg=str.rbegin();
    std::string::reverse_iterator rend=str.rend();
    int i=0;
    int result=0;
    char binary[255];
    for(;rbeg!=rend;++rbeg)
    {
        *rbeg=ToUpCase(*rbeg);
        int tmp=my36atoi(*rbeg);
        if (tmp==-1) {
            return -1;
        }
        tmp=tmp>>1;
        memset(binary,0,255);
        tmp=tmp<<i;
        result+=tmp;
        i+=4;
    }
    return result;
}

INSTANTIATE_SINGLETON(InviteCodeActivity)

static const int ROLEIDTOINVITECODE = 1;
static const int INVITECODETOROLEID = 2;
static const int DICTINVITECODETOROLEID = 3;
extern const string INVITECODEMD5INDEXKEY;
const string INVITECODEMD5INDEXKEY = "3";

int CalIndexByCount(int count)
{
    const tagActivityInviteCfg* cfg = ActivityInviteCfgMgr::getcfg();
    typedef tagActivityInviteCfg::MapCountToAward::const_iterator temp_iterator;
    temp_iterator iter = cfg->CountToAward.find(count);
    if (iter != cfg->CountToAward.end()) {
        int i=1;
        temp_iterator beg=cfg->CountToAward.begin();
        temp_iterator end=cfg->CountToAward.end();
        for (; beg!=end; ++beg) {
            if (beg->first == iter->first) {
                break;
            }
            ++i;
        }
        return i;
    }
    return -1;
}

void InviteCodeActivity::init()
{
    
    mRoleIDToInviteCode.load(g_GetRedisName(rnInviteAct), ROLEIDTOINVITECODE);
    mInviteCodeToRoleID.load(g_GetRedisName(rnInviteAct), INVITECODETOROLEID);
    //mDictInviteCodeToRoleID.load(g_GetRedisName(rnInviteAct), DICTINVITECODETOROLEID);
}

int countNumByStartLvl(std::map<int,int> &roleIdToLvl,int startLvl)
{
    std::map<int,int>::iterator beg = roleIdToLvl.begin();
    std::map<int,int>::iterator end=roleIdToLvl.end();
    int i=0;
    for (;beg!=end ;++beg ) {
        if (beg->second >=startLvl) {
            ++i;
        }
    }
    return i;
}
void InviteCodeActivity::sendStatus(int roleid)
{
    Role* role = SRoleMgr.GetRole(roleid);
    xyassert(role);
    
    const tagActivityInviteCfg* cfg = ActivityInviteCfgMgr::getcfg();
    typedef tagActivityInviteCfg::MapCountToLvl::const_iterator temp_iterator;
    temp_iterator beg=cfg->CountToLvl.begin();
    temp_iterator end=cfg->CountToLvl.end();
    ack_invite_status ack;
    // 首先进行：判断玩家的等级是否符合要求
    if(!mRoleIDToInviteCode.exist(roleid)){
        
        mRoleIDToInviteCode[roleid]["roleid"]=roleid;
        mRoleIDToInviteCode[roleid]["hasInputOtherCode"]=0;
        mRoleIDToInviteCode[roleid]["otherCode"]="";
        for (; beg!=end; ++beg) {
            mRoleIDToInviteCode[roleid][Utils::makeStr("%d",beg->first)]=0;
        }
        string codeDigest =intToUUID8(roleid);
        mRoleIDToInviteCode[roleid]["code"]=codeDigest;
        mRoleIDToInviteCode.save();
//        ack.code = mRoleIDToInviteCode[roleid]["code"].asString();
//        ack.errorcode = eInviteResult_Succ;
//        role->send(&ack);
//        return;
    }
    
    // 再进行：发送信息逻辑
    Json::Value jvalRoleIDToCode = this->mRoleIDToInviteCode[roleid];
    Json::Value inputCodeRoleIDs = mInviteCodeToRoleID[jvalRoleIDToCode["code"].asString()];
    ack.errorcode = eInviteResult_Succ;
    ack.code=this->mRoleIDToInviteCode[roleid]["code"].asString();
    beg=cfg->CountToLvl.begin();
    end=cfg->CountToLvl.end();
    //拿出输入此邀请码玩家的等级
    int inputNum = inputCodeRoleIDs.getMemberNames().size();
    std::map<int,int> roleIdToLvl;
    for (int i=0;i< inputNum;++i){
        RedisResult result(redisCmd("hmget role:%d lvl ", inputCodeRoleIDs[Utils::makeStr("%d",i)].asInt()));
        int lvl = result.readHash(0,0);
        roleIdToLvl.insert(std::map<int,int>::value_type(inputCodeRoleIDs[Utils::makeStr("%d" ,i)].asInt(),lvl));
    }
    for (; beg!=end;++beg) {
        
            //对自己的奖励特别考虑
            if (beg->first%10000 == 0) {
                obj_invite_code_num objNum;
                objNum.index = beg->first;
                
                if (role->getLvl() >= beg->second  && jvalRoleIDToCode["hasInputOtherCode"].asInt() ==1) {
                    if (mRoleIDToInviteCode[roleid][Utils::makeStr("%d",beg->first)].asInt() ==0) {
                    ack.canGet.push_back(beg->first);
                    objNum.count = 0;
                    }
                    else
                        objNum.count = 1;
                }
                ack.totalNum.push_back(objNum);
                continue;
            }
            int startLvl = beg->second;
            int count = countNumByStartLvl(roleIdToLvl,startLvl);
            obj_invite_code_num objNum;
            objNum.index = beg->first;
            objNum.count = count;
            ack.totalNum.push_back(objNum);
            if (beg->first % 10000 <= count) {
                if (mRoleIDToInviteCode[roleid][Utils::makeStr("%d",beg->first)].asInt() ==0) {
                    ack.canGet.push_back(beg->first);
                    continue;
                }
            }
    }
    role->send(&ack);
    return;
}


void InviteCodeActivity::inputCode(int roleid,std::string & codeStr) // 输入激活码
{
    Role* role = SRoleMgr.GetRole(roleid);
    xyassert(role);
    ack_invite_input_code ack;
    ack.code = codeStr;
    send_if(codeStr=="", eInviteResult_GetAwardError);
    
    std::string::iterator codebeg=codeStr.begin();
    std::string::iterator codeend=codeStr.end();
    for (;codebeg!=codeend ; ++codebeg) {
        *codebeg = ToUpCase(*codebeg);
    }
    RedisResult result;
    result.setData(redisCmd("hmget invite_code_act:%s %s",INVITECODEMD5INDEXKEY.c_str(),codeStr.c_str()));
    int elementnum = result.readHash(0,0);
    if(elementnum==0){
        int targetRoleID = UUID8ToInt(codeStr);
        //判断该该ID的玩家是否存在
        result.setData(redisCmd("exists role:%d", targetRoleID));
        if (result.readInt() == 1){
            doRedisCmd("hmset invite_code_act:%s %s %d",INVITECODEMD5INDEXKEY.c_str(),codeStr.c_str(), targetRoleID);
            elementnum=targetRoleID;
        }
    }
    send_if(elementnum==0, eInviteResult_GetAwardError);
    int targetRoleID = UUID8ToInt(codeStr);
    send_if(targetRoleID == roleid, eInviteResult_IsSelf);
    send_if(this->mRoleIDToInviteCode[roleid]["hasInputOtherCode"].asInt()==1, eInviteResult_GetAwardError);
    
    this->mRoleIDToInviteCode[roleid]["hasInputOtherCode"]=1;
    this->mRoleIDToInviteCode[roleid]["otherCode"]=codeStr;
    this->mRoleIDToInviteCode.save();
    Json::Value thisCodeStrToRoleIDs = this->mInviteCodeToRoleID[codeStr];
    int index = thisCodeStrToRoleIDs.getMemberNames().size();
    this->mInviteCodeToRoleID[codeStr][Utils::makeStr("%d" ,index)]=roleid;
    this->mInviteCodeToRoleID.save();
    role->send(&ack);
}

// 发送奖励，成功返回true
bool InviteCodeActivity::sendAward(Role* role ,  const string& award)
{
    if (award.empty())
    {
        xyerr("邀请码奖励出错%s",award.c_str());
        return false;
    }
    vector<string> awards = StrSpilt(award.c_str(), ";");
    ItemArray items;
    return role->addAwards(awards, items, "激活码领取奖励");
}

// 领奖
void InviteCodeActivity::getAward(int roleid, int index)
{
    Role* role = SRoleMgr.GetRole(roleid);
    xyassert(role);
    ack_invite_get_award ack;
    ack.index = index;
    
    int key = CalIndexByCount(index);
    send_if(key==-1, eInviteResult_GetAwardError);
    const tagActivityInviteCfg* cfg = ActivityInviteCfgMgr::getcfg();
    // 再进行：发送信息逻辑
    Json::Value jvalRoleIDToCode = this->mRoleIDToInviteCode[roleid];
    Json::Value inputCodeRoleIDs = mInviteCodeToRoleID[jvalRoleIDToCode["code"].asString()];
    typedef tagActivityInviteCfg::MapCountToLvl::const_iterator temp_iterator;
    temp_iterator beg=cfg->CountToLvl.begin();
    temp_iterator end=cfg->CountToLvl.end();
    //拿出输入此邀请码玩家的等级
    int inputNum = inputCodeRoleIDs.getMemberNames().size();
    std::map<int,int> roleIdToLvl;
    
    bool canGetFlag=false;
    for (int i=0;i< inputNum;++i){
        RedisResult result(redisCmd("hmget role:%d lvl ", inputCodeRoleIDs[Utils::makeStr("%d",i)].asInt()));
        int lvl = result.readHash(0,0);
        roleIdToLvl.insert(std::map<int,int>::value_type(inputCodeRoleIDs[Utils::makeStr("%d" ,i)].asInt(),lvl));
    }

    for (; beg!=end;++beg) {
        if (beg->first != index) {
            continue;
        }
        if (mRoleIDToInviteCode[roleid][Utils::makeStr("%d",beg->first)].asInt() ==0) {
            //对自己的奖励特别考虑
            if (beg->first%10000 == 0) {
                if (role->getLvl() >= beg->second  && jvalRoleIDToCode["hasInputOtherCode"].asInt() ==1) {
                    canGetFlag=true;
                    continue;
                }
            }
            int startLvl = beg->second;
            int count = countNumByStartLvl(roleIdToLvl,startLvl);
            if (beg->first % 10000 <= count) {
                canGetFlag=true;
            }
        }
    }
    
    send_if(canGetFlag==false, eInviteResult_GetAwardError);
    
    bool isBagFull = ! sendAward(role , cfg->CountToAward.find(index)->second);
    send_if(isBagFull, eInviteResult_BagFull);
    
    this->mRoleIDToInviteCode[roleid][Utils::makeStr("%d", index)]=1;
    this->mRoleIDToInviteCode.save();
    Xylog log(eLogName_Invite_Code,roleid);
    //日志格式，玩家ID，玩家角色名字，领取的INDEX(经过计算后的)，奖励，四个字段
    log<<roleid<<role->getRolename()<<index<<cfg->CountToAward.find(index)->second;
    send_if(true,eInviteResult_Succ);
}

static void AfterServerStartInitInviteCodeActivity(void * param)
{
    SInviteCodeMgr.init();
}


static void OnlyDoOnceByDataIsemptyInInviteCodeActivity(void * param)
{
    RedisResult result;
    result.setData(redisCmd("exists invite_code_act:%s", INVITECODEMD5INDEXKEY.c_str()));
    if (result.readInt() == 1)
    {
        return;
    }
    result.setData(redisCmd("keys role:*"));
    int elementnum = result.getHashElements();
    for (int i = 0; i < elementnum; ++i)
    {
        string strtmp=result.readHash(i, "");
        StringArray tmpStrArray = StrSpilt(strtmp.c_str(), ":");
        if (tmpStrArray.size() !=2) {
            continue;
        }
        int roleid = Utils::safe_atoi(tmpStrArray[1].c_str(), 0);
        string codeDigest =intToUUID8(roleid);
        doRedisCmd("hmset invite_code_act:%s %s %d",INVITECODEMD5INDEXKEY.c_str(),codeDigest.c_str(), roleid);
    }
}
addTimerDef(OnlyDoOnceByDataIsemptyInInviteCodeActivity, NULL, 5, 1);
addTimerDef(AfterServerStartInitInviteCodeActivity, NULL, 15, 1);
