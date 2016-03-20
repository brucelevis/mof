//
//  lrucache.h
//  GameSrv
//
//  Created by prcv on 13-9-13.
//
//

#ifndef GameSrv_lrucache_h
#define GameSrv_lrucache_h

/*
template <class Key, class T>
class LRUCache
{
    struct Node{
        Key mKey;
        T mVal;
        Node* prev;
        Node* next;
    };
    
public:
    LRUCache(size_t maxSize) : mMaxSize(maxSize), mCurSize(0){
        mFreeData = new Node[maxSize];
        
        mHead = new Node;
        mTail = new Node;
        mHead->next = mTail;
        mHead->prev = NULL;
        mTail->next = NULL;
        mTail->prev = mHead;
    }
    
    ~LRUCache(){
        delete []mFreeData;
        delete mHead;
        delete mTail;
    }
    
    void put(const Key& key, const T& data){
        //1:获取对应的节点，赋值并移到头部。成功退出，失败下一步
        //2:分配空闲，赋值并移到头部。成功退出，失败下一步
        //3:取出尾部的节点，赋值并移到头部。成功退出，失败下一步
        
        Node* node = getNode(key);
        if (node){
            //如果key已经存在，移动首位
            node->mVal = data;
            moveToFirst(node);
            return;
        }
        
        //获取空闲的
        node = getFree();
        if (node == NULL){
            //如果没有空闲的，取出最后一个
            node = popLast();
        }
        node->mKey = key;
        node->mVal = data;
        setFirst(node);
        mData[key] = node;
    }
    
    bool get(const Key& key, T& val){
        Iterator iter = mData.find(key);
        if (iter == mData.end()){
            return false;
        }
        
        //1:获取对应的节点，赋值并移到头部。
        Node* node = iter->second;
        val = node->mVal;
        moveToFirst(node);
        
        return true;
    }
    
    bool exist(const Key& key){
        Iterator iter = mData.find(key);
        return (iter != mData.end());
    }
    
    int size(){
        return mData.size();
    }
    
    void dump(){
        Node* node = mHead->next;
        while (node != mTail){
            cout << node->mKey << " " << node->mVal << endl;
            node = node->next;
        }
        cout << endl;
    }
    
private:
    
    Node* getNode(const Key& key){
        Iterator iter = mData.find(key);
        if (iter != mData.end()){
            return iter->second;
        }
        
        return NULL;
    }
    
    void moveToFirst(Node* node){
        //remove node from current pos
        node->prev->next = node->next;
        node->next->prev = node->prev;
        
        setFirst(node);
    }
    
    void setFirst(Node* node){
        //set node to the first position
        node->prev = mHead;
        node->next = mHead->next;
        mHead->next->prev = node;
        mHead->next = node;
    }
    
    Node* popLast(){
        assert(mTail->prev != mHead);
        
        Node* node = mTail->prev;
        node->prev->next = mTail;
        mTail->prev = node->prev;
        node->prev = node->next = NULL;
        return node;
    }
    
    Node* getFree(){
        if (mCurSize == mMaxSize){
            return NULL;
        }
        
        Node* node = &mFreeData[mCurSize];
        mCurSize++;
        
        return node;
    }
    
    typedef hash_map<Key, Node*, StrHashFunc, StrHashCompare> HashMap;
    typedef typename hash_map<Key, Node*, StrHashFunc, StrHashCompare>::iterator Iterator;
    
    HashMap mData;
    int mMaxSize;
    int mCurSize;
    Node* mFreeData;
    Node* mHead;
    Node* mTail;
};
*/

#endif
