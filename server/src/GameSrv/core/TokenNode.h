#ifndef DATATOSTRING_H
#define DATATOSTRING_H

#include "Utils.h"


// 数据节点
class TokenNode
{
public:
    TokenNode();
    TokenNode(string val);
    TokenNode(const char* val);
    virtual ~TokenNode();

    // 下标
    TokenNode& operator [] (size_t index);

    // 赋值
    void operator = (int val);
    void operator = (float val);
    void operator = (string val);
    void operator = (const char* val);

    int AsInt();
    string AsString();
    float AsFloat();

    // 判断是否空
    bool IsNull(){return m_depth == -1;}

    // 获取子的数量
    bool Size(){ return m_children.size(); }

protected:
    void ResetNode();
    void RecreatNode(string& value, bool hasChild);

private:
    int m_depth; // 递归深度
    string m_value; // 当前值
    vector<TokenNode*> m_children;// 子容器
};

#endif // DATATOSTRING_H
