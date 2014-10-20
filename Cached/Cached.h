//
//  @ File Name : Cached.h 
//  @ Date : 2014/6/5
//  @ Author : bin.li@flyingwings.cn
//

#ifndef __CACHED__H__
#define __CACHED__H__

#include "cache.h"

using namespace std;

struct memcached_st;

typedef map<string, string> StatusMap;
typedef map<string, StatusMap> ServerStatusMap;

struct CachedRetType {
	enum {
		SUCCESS = 0L,			// �����ɹ�
		NORMAL_FAILED,			// ����ʧ��
		EXCEPT_FAILED			// �쳣ʧ��
	};
};

class Cached
{
	friend class CacheMgr;

public:
	Cached(void);
	virtual ~Cached(void);

	bool GetStats(ServerStatusMap& statsMap);
	//bool Exist(const string& cacheKey);
	bool Get(const string& cacheKey, char*& itemValue, int64& itemValueLength);
	bool Set(const string& cacheKey, const char* itemValue, int64 itemValueLength);
	bool Add(const string& cacheKey, const char* itemValue, int64 itemValueLength);
	bool Replace(const string& cacheKey, const char* cacheValue, int64 cacheValueLength);
	//bool Append(const string& cacheKey, const char* itemValue, int64 itemValueLength);
	bool Delete(const string& cacheKey);


	/**
	* Clean all the contents of memcached servers.
	*
	* @return true if succeed; false otherwise
	*/
	bool Flush(void);


public:

	inline bool SetBehavior(CACHED_BEHAVIOR flag, uint64 data)
	{
		return memcached_success(memcached_behavior_set(_memc, flag, data));
	}

	inline bool AddServer(const string& hostIP, const uint32 port)
	{
		return memcached_success(memcached_server_add(_memc, hostIP.c_str(), port));
	}

	inline uint32 GetServerCount(void)
	{
		return (uint32)memcached_server_count(_memc);
	}

	inline int CacheAdd(const string& cacheKey, const char* cacheValue, int64 cacheValueLength, time_t expiration /*= 0*/)
	{
		return CachedGetRetValueType(memcached_add(_memc, cacheKey.c_str(), cacheKey.length(), cacheValue, cacheValueLength, expiration, 0));
	}

	inline int CacheGet(const string& cacheKey, char*& cacheValue, int64& cacheValueLength)
	{
		uint32 flags = 0;
		memcached_return_t rc;

		cacheValue = memcached_get(_memc, cacheKey.c_str(), cacheKey.length(), (size_t *)&cacheValueLength, &flags, &rc);
		return CachedGetRetValueType(rc);
	}

	inline int CacheSet(const string& cacheKey, const char* cacheValue, int64 cacheValueLength, time_t expiration /*= 0*/)
	{
		return CachedGetRetValueType(memcached_set(_memc, cacheKey.c_str(), cacheKey.length(), cacheValue, cacheValueLength, expiration, 0));
	}

	inline int CacheReplace(const string& cacheKey, const char* cacheValue, int64 cacheValueLength, time_t expiration /*= 0*/)
	{
		return CachedGetRetValueType(memcached_replace(_memc, cacheKey.c_str(), cacheKey.length(), cacheValue, cacheValueLength, expiration, 0));
	}

	inline int CacheFlush(time_t expiration /*= 0*/)
	{
		return CachedGetRetValueType(memcached_flush(_memc, expiration));
	}

	inline int CacheDelete(const string& cacheKey, time_t expiration /*= 0*/)
	{
		return CachedGetRetValueType(memcached_delete(_memc, cacheKey.c_str(), cacheKey.length(), expiration));
	}

	inline int CachedGetRetValueType(int retVal)
	{
		/*
		* �����ɹ�����memcached������صĴ�������memcached_success����true������Ϊ�����ɹ���
		* ����ʧ�ܣ���memcached������صĴ�������memcached_success����false��ͬʱ��memcached_fatal����false������Ϊ����ʧ�ܣ�
		* �쳣ʧ�ܣ���memcached������صĴ�������memcached_success����false������memcached_fatal����true������Ϊ�쳣ʧ�ܡ�
		*/
		if (memcached_success((memcached_return_t)retVal)) {
			return CachedRetType::SUCCESS;
		}
		else if (!memcached_fatal((memcached_return_t)retVal)) {
			return CachedRetType::NORMAL_FAILED;
		}
		return CachedRetType::EXCEPT_FAILED;
	}

private:
	memcached_st* _memc;
};

#endif //__CACHED__H__