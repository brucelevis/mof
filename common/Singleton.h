/*
*  单例泛型类
sample:
*  class AcountMgr;
*  #define sAcountMgr Singleton<AcountMgr>::Instance()
*  sAcountMgr.*
in cpp:
*
*  INSTANTIATE_SINGLETON(AcountMgr);
*/
#ifndef __SINGLETON_H
#define __SINGLETON_H

/**
 * @brief class Singleton
 */

template<typename T>
    class Singleton
{
    public:
        static T& Instance();

    protected:
        Singleton() {};

    private:

        // Prohibited actions...this does not prevent hijacking.
        Singleton(const Singleton &);
        Singleton& operator=(const Singleton &);
};

#define INSTANTIATE_SINGLETON(TYPE) \
    template<> TYPE& Singleton<TYPE>::Instance() \
    {static TYPE si_instance;return si_instance;}

#endif
