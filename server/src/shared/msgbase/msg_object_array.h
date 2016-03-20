#ifndef SHARED_MSG_OBJECT_ARRAY_H
#define SHARED_MSG_OBJECT_ARRAY_H

class MsgObject;
#include <vector>
using namespace std;


template<class T>
class MsgObjectArray
{
public:
    MsgObjectArray()
    {
    }
    
    ~MsgObjectArray()
    {
        clear();
    }
    
    void clear()
    {
        for (int i = 0; i < mData.size(); i++) {
            delete mData[i];
        }
    }
    
    void push_back(const T& val)
    {
        MsgObject* pData = val.clone();
        mData.push_back(pData);
    }
    
    T& operator[](int index)
    {
        return *((T*)(mData[index]));
    }
    
    
    T& operator[](int index) const
    {
        return *((T*)(mData[index]));
    }
    
    MsgObject** data()
    {
        return mData.data();
    }
    
    int size() const
    {
        return mData.size();
    }
    
    MsgObjectArray& operator=(const vector<T>& datas)
    {
        for (int i = 0; i < datas.size(); i++) {
            push_back(datas[i]);
        }
        return *this;
    }
    
private:
    
    vector<MsgObject*> mData;
};

#endif