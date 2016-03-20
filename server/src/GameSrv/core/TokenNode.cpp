#include "TokenNode.h"
#include <string.h>


// 剪切用分割符
const char* __tn_tokens = ";|#~";
int __tn_maxdepth = strlen(__tn_tokens);

// 空对象
TokenNode nullNode;

/////////////////////////////////////
TokenNode::TokenNode()
{
    m_depth = 0;
}

TokenNode::TokenNode(string val)
{
    m_depth = 0;
    *this = val;
}

TokenNode::TokenNode(const char* val)
{
    m_depth = 0;
    *this = val;
}

TokenNode::~TokenNode()
{
    ResetNode();
}

// 下标
TokenNode& TokenNode::operator [] (size_t index)
{
    if ( m_depth >= __tn_maxdepth || m_depth == -1)
    {
        nullNode.m_depth = -1;
        return nullNode;
    }

    size_t cnt = m_children.size();
    size_t needCnt = index + 1;

    if (cnt < needCnt)
    {
        // 根据需要扩容
        for (size_t i = cnt; i < needCnt; i ++)
        {
            TokenNode* node = new TokenNode;
            node->m_depth = m_depth + 1;

            m_children.push_back( node );
        }
    }

    return *m_children[index];
}

// 赋值
void TokenNode::operator = (int val)
{
    if (IsNull()) return;

    char buf[32];
    sprintf(buf, "%d", val);
    string str = buf;
    RecreatNode( str, false );
}

void TokenNode::operator = (float val)
{
    if (IsNull()) return;

    char buf[32];
    sprintf(buf, "%f", val);
    string str = buf;

    RecreatNode( str, false );
}

void TokenNode::operator = (string val)
{
    if (IsNull()) return;

    RecreatNode( val, true );
}

void TokenNode::operator = (const char* val)
{
    if (IsNull()) return;
    if (!val) val = "";

    string v = val;
    RecreatNode( v, true );
}

int TokenNode::AsInt()
{
    if (IsNull()) return 0;

    return atoi( m_value.c_str() );
}

string TokenNode::AsString()
{
    if (IsNull()) return "";

    // 是叶子节点，直接返回值
    if (m_children.size() == 0)
    {
        return m_value;
    }

    // 计算个子类值
    string out;
    size_t count = m_children.size();
    for (size_t i=0; i < count; i++)
    {
        out += m_children[i]->AsString();
        if (i != count-1)
        {
            out += __tn_tokens[m_depth];
        }
    }

    return out;
}

float TokenNode::AsFloat()
{
    if (IsNull()) return 0.0f;

    return atof( m_value.c_str() );
}

void TokenNode::ResetNode()
{
    m_value = "";
    for (size_t i=0; i < m_children.size(); i++)
    {
        delete m_children[i];
    }
    m_children.clear();
}

void TokenNode::RecreatNode(string& value, bool hasChild)
{
    // 重置节点
    ResetNode();

    // 没有子或者深度已经到达最高，不再分割
    if (!hasChild || m_depth >= __tn_maxdepth)
    {
        m_value = value;
        return;
    }

    // 分割
   char tok[2];
   tok[0] = __tn_tokens[m_depth];
   tok[1] = 0;

   StringTokenizer reToken(value, tok);

    // 没有子
    if (reToken.count() < 2)
    {
        m_value = value;
        return;
    }

    // 创建子
    for (size_t i = 0; i < reToken.count(); i++)
    {
        TokenNode* node = new TokenNode;
        node->m_depth = m_depth + 1;
        *node = reToken[i];

        m_children.push_back( node );
    }
}


