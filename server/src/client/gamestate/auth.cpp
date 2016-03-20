#include "auth.h"

void AuthState::loginRaw(StringArray& sa)
{
    if (sa.size() < 2)
    {
        return;
    }
}


void AuthState::loginXinyou(StringArray &sa)
{
    /*
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t ms = tv.tv_usec / 1000 + tv.tv_sec * 1000;
    //    char _time[255];
    //    sprintf(_time, "%llu",ms);
    string _time=Utils::makeStr("%llu",ms);
    string _key = GameCfg::xinyouKey + _time;
    MD5 _md5  = MD5(_key);
    _key = _md5.hexdigest();
    string strURL = GameCfg::xinyouServerURL;
    string post =  "do=login&uname="+account + "&pwd="+ pass +"&time="+ _time + "&sign=" + _key;
    createRequestThreadPost(strURL , post,tmt_RequestXinyouLogin, delegate);
    */
}