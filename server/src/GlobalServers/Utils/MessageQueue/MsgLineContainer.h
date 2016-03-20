/*-------------------------------------------------------------------------
	created:	2010/08/16  16:33
	filename: 	e:\Project_SVN\Server\Utils\MessageQueue\MsgLineContainer.h
	author:		Deeple
	
	purpose:     读取字符串消息中的一行
---------------------------------------------------------------------------*/


class RecvQueue
{
	struct MemBlock
	{
	public:
		char* pData;
		char* pReadPtr;
		int nLen;
		int space()
		{
			return nLen - (int)(pReadPtr - pData);
		}
	};
public:
	RecvQueue();
	~RecvQueue();
public:
	void put(void* data, int nLen);
	void put2(void* data, int nLen);
	bool get(char * buffer, int nLen);
private:
	int TotleBytes();
private:
	std::vector<MemBlock> m_Data;
};

RecvQueue::RecvQueue()
{
}

RecvQueue::~RecvQueue()
{
	for (size_t i=0; i<m_Data.size(); i++)
	{
		delete [] m_Data[i].pData;
	}
}

void RecvQueue::put(void* data, int nLen)
{		
	MemBlock mb;
	mb.nLen = nLen;
	mb.pData = new char[nLen];
	mb.pReadPtr = mb.pData;
	::memcpy(mb.pData, data, nLen);
	m_Data.push_back(mb);
}

void RecvQueue::put2(void* data, int nLen)
{
	MemBlock mb;
	mb.nLen = nLen;
	mb.pData = (char*)data;
	mb.pReadPtr = mb.pData;
	m_Data.push_back(mb);
}

bool RecvQueue::get(char * buffer, int nLen)
{
	if (nLen < 1 || TotleBytes() < nLen)
	{
		return false;
	}

	int nRead = 0;
	while (nRead < nLen)
	{
		int nBytesReq = nLen - nRead;
		int nBytesToRead = min(nBytesReq, m_Data[0].space());
		memcpy(buffer + nRead, m_Data[0].pReadPtr, nBytesToRead);
		m_Data[0].pReadPtr += nBytesToRead;
		nRead += nBytesToRead;
		if (m_Data[0].space() == 0)
		{
			delete [] m_Data[0].pData;
			m_Data.erase(m_Data.begin());
		}
	}
	return true;
}

int RecvQueue::TotleBytes()
{
	int iRet =0;
	for (size_t i=0; i<m_Data.size();i++)
	{
		iRet += m_Data[i].space();
	}
	return iRet;
}

// class MsgDataCont
// -------------------------------------------------------------------------
class MsgDataCont
{
public:
	MsgDataCont()
	{
		m_pWritePtr = m_Line;
	}
	~MsgDataCont()
	{

	}
public:
	void put(void* pData, int nLen)
	{
		if (pData && nLen > 0)
		{
			m_DataQueue.put(pData, nLen);
		}
	}

	int GetLine(string& str)
	{
		if (m_pWritePtr - m_Line > 8 * 1024 - 10)
		{
			return -1;
		}

		while (m_DataQueue.get(m_pWritePtr, 1))
		{
			if (*m_pWritePtr == 0x0A || *m_pWritePtr == 0x0D)
			{
				// 读到结束符
				*m_pWritePtr = 0;
				str = m_Line;
				m_pWritePtr = m_Line;
				return 1;
			}
			m_pWritePtr ++;
		}

		return 0;
	}
private:
	RecvQueue m_DataQueue;
	char m_Line[128 * 1024];
	char* m_pWritePtr;
};