
//
//  RandName.cpp
//  GameSrv
//
//  Created by cxy on 13-1-30.
//
//

#include "RandName.h"
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <algorithm>

#include "main.h"
#include "redis_helper.h"

using namespace std;

RandNameMgr g_RandNameMgr;

template<class T>
struct NamedValue
{
    string name;
    T value;
};

template <class T>
struct NamedValueLittle{
    bool operator()(const NamedValue<T>& first, const NamedValue<T>& second){
        return first.name < second.name;
    }
};

template<size_t N>
int getKthZero(bitset<N>& bits, int k){
    if (k > N){
        return -1;
    }

    for (int i = 0, count = 0; i < N; i++){
        if (bits.test(i)){
            continue;
        }

        count++;
        if (count == k){
            return i;
        }
    }

    return -1;
}

template<size_t N>
int getOneBitsNum(bitset<N>& bits, int max){
    int count = 0;
    for (int i = 0; i < max; i++){
        if (bits.test()){
            count++;
        }
    }

    return count;
}

void sortAndUniq(vector<string>& arr)
{
    sort(arr.begin(), arr.end());
    
    int i, j;
    for (i = 1, j = 0; i < arr.size(); i++)
    {
        if (arr[i] == arr[j])
        {
            continue;
        }
        
        j++;
        arr[j] = arr[i];
    }
    
    if (arr.size() > 0)
    {
        arr.resize(j + 1);
    }
}


//判断字符串是否能由两个字符串列表组合而成
bool isComposeBy(const string& name, const vector<string>& firstList, const vector<string>& secondList,
                        int& firstIdx, int& secondIdx);



string RandNameMgr::getRandName(bool isMale)
{
    string fullName;

    int& freeNum = isMale ? mMaleFreeNum : mFemaleFreeNum;
    if (freeNum <= 0){
        return fullName;
    }
    int randIdx = range_rand(1, freeNum);

    vector<RandFirstName>& firstNames = isMale ? mMaleFirstNames : mFemaleFirstNames;
    vector<string>& secondNameList = isMale ? mMaleSecondNamesList: mFemaleSecondNamesList;
    for (int i = 0; i < firstNames.size(); i++){

        RandFirstName firstName = firstNames[i];
        if (firstName.mFreeNum >= randIdx){
            int second = getKthZero(firstName.mMask, randIdx);
            fullName = mFirstNamesList[i] + secondNameList[second];
            return fullName;
        }
        else{
            randIdx -= firstName.mFreeNum;
        }
    }

    log_error("should not come here!");
    return fullName;
}

void RandNameMgr::update(bool isMale, const string& name, bool save){

    vector<string>& secondNameList = isMale ? mMaleSecondNamesList : mFemaleSecondNamesList;

    int firstIdx, secondIdx;
    if (!isComposeBy(name, mFirstNamesList, secondNameList, firstIdx, secondIdx)){
        return;
    }

    vector<RandFirstName>& firstNames = isMale ? mMaleFirstNames : mFemaleFirstNames;
    int& genderFreeNum = isMale ? mMaleFreeNum : mFemaleFreeNum;
    if (firstNames[firstIdx].mMask.test(secondIdx)){
        return;
    }

    firstNames[firstIdx].mMask.set(secondIdx);
    firstNames[firstIdx].mFreeNum--;
    genderFreeNum--;

    string formatSecondName;
    for (int i = 0; i < secondNameList.size(); i++){
        if (firstNames[firstIdx].mMask.test(i)){
            formatSecondName.append(secondNameList[i]);
            formatSecondName.append(";");
        }
    }

    if (save) {
        string key = getRandNameKeyByGender(isMale);
        string hkey = mFirstNamesList[firstIdx];
        doRedisCmd("hmset %s %s %s", key.c_str(), hkey.c_str(), formatSecondName.c_str());
    }
}

struct StrPrefix
{
    StrPrefix(const string& src){
        mSrc = src;
    }
    string mSrc;

    bool operator()(const string& str){
        if (strncmp(mSrc.c_str(), str.c_str(), str.length())){
            return false;
        }

        return true;
    }
};


bool RandNameMgr::loadConfig(const char* cfgFile){
    fstream cfgStream(cfgFile);
    if (!cfgStream.is_open()) {
        return false;
    }
    
    string lineData;
    string section;
    vector<string>* nameListPtr;
    do{
        lineData = "";
        cfgStream >> lineData;

        if (lineData.find("[")!=std::string::npos &&
            lineData.find("]")!=std::string::npos)
        {
            section = lineData;
            if (lineData == "[surname]"){
                nameListPtr = &mFirstNamesList;
            }
            else if (lineData == "[male]"){
                nameListPtr = &mMaleSecondNamesList;
            }
            else if (lineData == "[female]"){
                nameListPtr = &mFemaleSecondNamesList;
            }
            else{
                nameListPtr = NULL;
            }
            continue;
        }

        if ((lineData.empty()) ||
            (lineData[0]==';'&&lineData[1]==';') ||
            (lineData[0]=='/'&&lineData[1]=='/') ||
            (lineData[0]=='#')
            )
        {
            continue;
        }

        if (nameListPtr != NULL){
            nameListPtr->push_back(lineData);
        }
        else{
            log_error("name.conf error");
        }

    }
    while (!cfgStream.eof());

    sortAndUniq(mFirstNamesList);
    sortAndUniq(mMaleSecondNamesList);
    sortAndUniq(mFemaleSecondNamesList);

    return true;
}



static int countTotalFreeNum(const vector<RandFirstName>& randNameState)
{
    int num = 0;
    
    for (int i = 0; i < randNameState.size(); i++)
    {
        num += randNameState[i].mFreeNum;
    }
    
    return num;
}

//标记所有已经用过的随机名
// usedNames       已经使用过的随机名列表 NamedValue中name表示姓，value表示名字列表，以;分隔。已排序
// firstNameList   名字库的姓列表。已排序
// secondNameList  名字库的名列表。已排序
// randNameState   随机名使用状态
// freeNum         可用的随机名数量
static void markUsedNames(const vector<NamedValue<string> >& usedNames, const vector<string>& firstNameList, const vector<string>& secondNameList,
                   vector<RandFirstName>& randNameState)
{
    vector<string>::const_iterator iter;
    int i, j;
    
    //类似归并的处理方式
    for (i = 0, j = 0; i < firstNameList.size() && j < usedNames.size();){
        RandFirstName first;
        
        first.mIndex = i;
        
        if (usedNames[j].name < firstNameList[i])
        {
            j++;
            continue;
        }
        else if (usedNames[j].name == firstNameList[i])
        {
            int usedNum = 0;
            vector<string> secondNames = StrSpilt(usedNames[j].value, ";");
            for (int k = 0; k < secondNames.size(); k++){
                iter = find(secondNameList.begin(), secondNameList.end(), secondNames[k]);
                if (iter != secondNameList.end()){
                    first.mMask.set(iter - secondNameList.begin());
                    usedNum++;
                }
            }
            first.mFreeNum = secondNameList.size() - usedNum;
            
            i++, j++;
        }
        else
        {
            first.mFreeNum = secondNameList.size();
            i++;
        }
        randNameState.push_back(first);
    }
    
    
    while (i < firstNameList.size()){
        RandFirstName first;
        first.mIndex = i;
        first.mFreeNum = secondNameList.size();
        randNameState.push_back(first);
        
        i++;
    }
}




bool RandNameMgr::loadMaleData()
{
    mMaleFreeNum = 0;
    mMaleFirstNames.clear();
    //load database data
    RedisResult result(redisCmd("hgetall randname:male"));
    vector<NamedValue<string> > usedNames;
    for (int i = 0; i < result.getHashElements(); i += 2){
        NamedValue<string> usedName;

        usedName.name = result.readHash(i);
        usedName.value = result.readHash(i + 1);
        usedNames.push_back(usedName);
    }
    sort(usedNames.begin(), usedNames.end(), NamedValueLittle<string>());

    
    markUsedNames(usedNames, mFirstNamesList, mMaleSecondNamesList, mMaleFirstNames);
    mMaleFreeNum = countTotalFreeNum(mMaleFirstNames);

    return true;
}


bool RandNameMgr::loadFemaleData()
{
    mFemaleFreeNum = 0;
    mFemaleFirstNames.clear();
    //load database data
    RedisResult result(redisCmd("hgetall randname:female"));
    vector<NamedValue<string> > usedNames;
    for (int i = 0; i < result.getHashElements(); i += 2){
        NamedValue<string> usedName;

        usedName.name = result.readHash(i);
        usedName.value = result.readHash(i + 1);
        usedNames.push_back(usedName);
    }
    sort(usedNames.begin(), usedNames.end(), NamedValueLittle<string>());
    
    markUsedNames(usedNames, mFirstNamesList, mFemaleSecondNamesList, mFemaleFirstNames);
    mFemaleFreeNum = countTotalFreeNum(mFemaleFirstNames);

    return true;
}

bool RandNameMgr::load(const string& cfgFile)
{
    if (!loadConfig(cfgFile.c_str())) {
        return false;
    }
    loadMaleData();
    loadFemaleData();
    return true;
}

void RandNameMgr::saveMaleData()
{
    vector<RandFirstName>& firstNames = mMaleFirstNames;
    vector<string>& secondNameList = mMaleSecondNamesList;
    
    for (int i = 0; i < firstNames.size(); i++) {
        
        string formatSecondName;
        for (int j = 0; j < secondNameList.size(); j++){
            if (firstNames[i].mMask.test(j)){
                formatSecondName.append(secondNameList[j]);
                formatSecondName.append(";");
            }
        }
        
        string key = "randname:male";
        string hkey = mFirstNamesList[i];
        doRedisCmd("hmset %s %s %s", key.c_str(), hkey.c_str(), formatSecondName.c_str());
    }
    
}

void RandNameMgr::saveFemaleData()
{
    vector<RandFirstName>& firstNames = mMaleFirstNames;
    vector<string>& secondNameList = mFemaleSecondNamesList;
    
    for (int i = 0; i < firstNames.size(); i++) {
        
        string formatSecondName;
        for (int j = 0; j < secondNameList.size(); j++){
            if (firstNames[i].mMask.test(j)){
                formatSecondName.append(secondNameList[j]);
                formatSecondName.append(";");
            }
        }
        
        string key = "randname:female";
        string hkey = mFirstNamesList[i];
        doRedisCmd("hmset %s %s %s", key.c_str(), hkey.c_str(), formatSecondName.c_str());
    }
    
}

void RandNameMgr::save()
{
    saveMaleData();
    saveFemaleData();
}


//判断字符串是否能由两个字符串列表组合而成
bool isComposeBy(const string& name, const vector<string>& firstList, const vector<string>& secondList,
                 int& firstIdx, int& secondIdx){
    //查找第一个字符串列表，是否为前缀
    StrPrefix prefix(name);
    vector<string>::const_iterator fiter = firstList.begin();
    vector<string>::const_iterator siter = firstList.begin();
    for (;;){
        fiter = find_if(fiter, firstList.end(), prefix);
        if (fiter == firstList.end()){
            return false;
        }
        firstIdx = fiter - firstList.begin();
        
        //查找第二个字符串列表
        string remainStr = name.substr(fiter->size());
        siter = find(secondList.begin(), secondList.end(), remainStr);
        if (siter == secondList.end()){
            fiter++;
            continue;
        }
        
        secondIdx = siter - secondList.begin();
        break;
    }
    return true;
}
