#pragma once

#include <cstdio>
#include <intrin.h>
#include <functional>
#include <unordered_map>
#include <queue>
#include <list>
#include <typeinfo>
#include <ctime>

#define GLM_FORCE_INLINE 
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_SSE2

constexpr uint32_t alloc_vals[] =
{
	0b00000000000000000000000000000001,
	0b00000000000000000000000000000011,
	0b00000000000000000000000000000111,
	0b00000000000000000000000000001111,
	0b00000000000000000000000000011111,
	0b00000000000000000000000000111111,
	0b00000000000000000000000001111111,
	0b00000000000000000000000011111111,
	0b00000000000000000000000111111111,
	0b00000000000000000000001111111111,
	0b00000000000000000000011111111111,
	0b00000000000000000000111111111111,
	0b00000000000000000001111111111111,
	0b00000000000000000011111111111111,
	0b00000000000000000111111111111111,
	0b00000000000000001111111111111111,
	0b00000000000000011111111111111111,
	0b00000000000000111111111111111111,
	0b00000000000001111111111111111111,
	0b00000000000011111111111111111111,
	0b00000000000111111111111111111111,
	0b00000000001111111111111111111111,
	0b00000000011111111111111111111111,
	0b00000000111111111111111111111111,
	0b00000001111111111111111111111111,
	0b00000011111111111111111111111111,
	0b00000111111111111111111111111111,
	0b00001111111111111111111111111111,
	0b00011111111111111111111111111111,
	0b00111111111111111111111111111111,
	0b01111111111111111111111111111111,
	0b11111111111111111111111111111111,
};

#define POOLPAGEERR_LOCKED_PAGE 5
#define POOLPAGEERR_NOT_ALLOCATED_SPACE 6
#define POOLPAGEERR_EMPTY_PAGE 7
#define POOLPAGEERR_NOERR 0

#pragma intrinsic(_BitScanForward)

#define FLAGTEST(A, B) ((A & B) == B)

#define MISS (void*)0xCDCDCDCD

#define POOLPAGE_ZEROMEMORY 1
#define POOLPAGE_LOCK 2
#define POOLPAGE_MEMCOMMIT 4

#define POOLPAGE_LOCKED 1
#define POOLPAGE_FULL 2

class POOLPAGEMEDIUM
{
public:
	POOLPAGEMEDIUM() {};
	POOLPAGEMEDIUM(char poolFlag, uint16_t align_ = 0)
	{
		flags = 0;
		align = align_;
		if (FLAGTEST(poolFlag, POOLPAGE_MEMCOMMIT))
			data = new char[align * 32];
		else
			data = (char*)MISS;
		if (FLAGTEST(poolFlag, POOLPAGE_LOCK))
			flags = flags | POOLPAGE_LOCKED;
		if (FLAGTEST(poolFlag, POOLPAGE_ZEROMEMORY))
			memset(data, 0, align * 32);

		mem = 0xFFFFFFFF;
	}

	bool isAllocated()
	{
		if (data == MISS)
			return false;
		return true;
	}

	bool isPageFull()
	{
		return (mem == 0);
	}

	void* allocPageMemoryAligned()
	{
		if (FLAGTEST(flags, POOLPAGE_LOCKED)) throw "Trying to allocate locked page.";

		unsigned long index;
		if (!_BitScanForward(&index, mem))
		{
			flags = flags | POOLPAGE_FULL;
			return MISS;
		}

		mem = mem ^ (1 << index);
		return data + index * align;
	}

	bool isPageLocked()
	{
		return FLAGTEST(flags, POOLPAGE_LOCKED);
	}

	int freePageMemory(void* addr, size_t size_of)
	{
		if (FLAGTEST(flags, POOLPAGE_LOCKED))
		{
			throw "Trying to allocate locked page.";
			return POOLPAGEERR_LOCKED_PAGE;
		}

		uint32_t bls = size_of / align;
		int32_t index = ((char*)addr - (char*)data) / align;

		if (FLAGTEST(mem, alloc_vals[bls - 1] << index)) // __popcnt can be used
			return POOLPAGEERR_NOT_ALLOCATED_SPACE;

		mem = mem ^ (alloc_vals[bls - 1] << index);
		return POOLPAGEERR_NOERR;
	}

	int freePageMemoryAligned(void* addr)
	{
		if (FLAGTEST(flags, POOLPAGE_LOCKED))
		{
			throw "Trying to allocate locked page.";
			return POOLPAGEERR_LOCKED_PAGE;
		}

		int32_t index = (((char*)addr - (char*)data) / align) + 1;

		if (FLAGTEST(mem, index))
			return POOLPAGEERR_NOT_ALLOCATED_SPACE;

		mem = mem ^ index;
		return POOLPAGEERR_NOERR;
	}

	void pageUnlock()
	{
		if (FLAGTEST(flags, POOLPAGE_LOCK))
			flags = flags ^ POOLPAGE_LOCK;
	}

	void pageLock()
	{
		flags = flags | POOLPAGE_LOCK;
	}

	void commitMemory(uint32_t align_)
	{
		align = align_;
		if (data == MISS)
			data = new char[align_ * 32];
		else
			throw "Trying to commit commited memory.";
	}

	int allocPageMemoryNext(size_t allocSize, uint32_t _srm = 0xFFFFFFFF)
	{
		if (_srm == 0)
			return -1;

		unsigned long index;
		if (!_BitScanForward(&index, mem & (~_srm)))
		{
			flags = flags | POOLPAGE_FULL;
			return -1;
		}

		uint32_t allocValue = alloc_vals[(allocSize / align) - 1] << index;

		int ptr = index;
		if ((mem & allocValue) != allocValue)
			ptr = allocPageMemoryNext(allocSize, _srm ^ allocValue);

		return ptr;
	}

	void* allocPageMemory(size_t allocSize)
	{
		if (FLAGTEST(flags, POOLPAGE_LOCKED)) throw "Trying to allocate locked page.";

		unsigned long index;
		if (!_BitScanForward(&index, mem))
		{
			flags = flags | POOLPAGE_FULL;
			return MISS;
		}

		uint32_t sx = index * align;
		if (allocSize > (32 - index)* align)
			return MISS;

		uint32_t allocValue = alloc_vals[(allocSize / align) - 1] << index;

		int ptr = index;
		if ((mem & allocValue) != allocValue)
		{
			ptr = allocPageMemoryNext(allocSize, allocValue);
			if (ptr == -1)
				return MISS;
			allocValue = alloc_vals[(allocSize / align) - 1] << ptr;
			sx = ptr * align;
		}

		mem = mem ^ allocValue;

		return data + sx;
	}

	uint32_t pageAlign()
	{
		return align;
	}

private:
	uint8_t flags;
	uint16_t align;
	uint32_t mem;

	char* data;
};

class POOLPAGELARGE
{
public:
	POOLPAGELARGE() {};
	POOLPAGELARGE(char poolFlag, uint16_t align_ = 0)
	{
		flags = 0;
		align = align_;
		if (FLAGTEST(poolFlag, POOLPAGE_MEMCOMMIT))
			data = new char[align * 64];
		else
			data = (char*)MISS;
		if (FLAGTEST(poolFlag, POOLPAGE_LOCK))
			flags = flags | POOLPAGE_LOCKED;
		if (FLAGTEST(poolFlag, POOLPAGE_ZEROMEMORY))
			memset(data, 0, align * 64);

		mem = 0xFFFFFFFFFFFFFFFF;
	}

	bool isAllocated()
	{
		if (data == MISS)
			return false;
		return true;
	}

	bool isPageFull()
	{
		return (mem == 0);
	}

	void* allocPageMemoryAligned()
	{
		if (FLAGTEST(flags, POOLPAGE_LOCKED)) throw "Trying to allocate locked page.";

		unsigned long index;
		if (!_BitScanForward(&index, mem))
		{
			flags = flags | POOLPAGE_FULL;
			return MISS;
		}

		mem = mem ^ (1 << index);
		return data + index * align;
	}

	bool isPageLocked()
	{
		return FLAGTEST(flags, POOLPAGE_LOCKED);
	}

	int freePageMemory(void* addr, size_t size_of)
	{
		if (FLAGTEST(flags, POOLPAGE_LOCKED))
		{
			throw "Trying to allocate locked page.";
			return POOLPAGEERR_LOCKED_PAGE;
		}

		uint32_t bls = size_of / align;
		int32_t index = ((char*)addr - (char*)data) / align;

		if (FLAGTEST(mem, alloc_vals[bls - 1] << index)) // __popcnt can be used
			return POOLPAGEERR_NOT_ALLOCATED_SPACE;

		mem = mem ^ (alloc_vals[bls - 1] << index);
		return POOLPAGEERR_NOERR;
	}

	int freePageMemoryAligned(void* addr)
	{
		if (FLAGTEST(flags, POOLPAGE_LOCKED))
		{
			throw "Trying to allocate locked page.";
			return POOLPAGEERR_LOCKED_PAGE;
		}

		int32_t index = (((char*)addr - (char*)data) / align) + 1;

		if (FLAGTEST(mem, index))
			return POOLPAGEERR_NOT_ALLOCATED_SPACE;

		mem = mem ^ index;
		return POOLPAGEERR_NOERR;
	}
	
	void pageUnlock()
	{
		if (FLAGTEST(flags, POOLPAGE_LOCK))
			flags = flags ^ POOLPAGE_LOCK;
	}

	void pageLock()
	{
		flags = flags | POOLPAGE_LOCK;
	}

	void commitMemory(uint32_t align_)
	{
		align = align_;
		if (data == MISS)
			data = new char[align_ * 64];
		else
			throw "Trying to commit commited memory.";
	}

	int allocPageMemoryNext(size_t allocSize, uint64_t _srm = 0xFFFFFFFFFFFFFFFF)
	{
		if (_srm == 0)
			return -1;

		unsigned long index;
		if (!_BitScanForward(&index, mem & (~_srm)))
		{
			flags = flags | POOLPAGE_FULL;
			return -1;
		}

		uint32_t allocValue = alloc_vals[(allocSize / align) - 1] << index;

		int ptr = index;
		if ((mem & allocValue) != allocValue)
			ptr = allocPageMemoryNext(allocSize, _srm ^ allocValue);

		return ptr;
	}

	void* allocPageMemory(size_t allocSize)
	{
		if (FLAGTEST(flags, POOLPAGE_LOCKED)) throw "Trying to allocate locked page.";

		unsigned long index;
		if (!_BitScanForward(&index, mem))
		{
			flags = flags | POOLPAGE_FULL;
			return MISS;
		}

		uint32_t sx = index * align;
		if (allocSize > (32 - index)* align)
			return MISS;

		uint32_t allocValue = alloc_vals[(allocSize / align) - 1] << index;

		int ptr = index;
		if ((mem & allocValue) != allocValue)
		{
			ptr = allocPageMemoryNext(allocSize, allocValue);
			if (ptr == -1)
				return MISS;
			allocValue = alloc_vals[(allocSize / align) - 1] << ptr;
			sx = ptr * align;
		}

		mem = mem ^ allocValue;

		return data + sx;
	}

	uint32_t pageAlign()
	{
		return align;
	}

private:
	uint8_t flags;
	uint32_t align;
	uint64_t mem;

	char* data;
};

extern uint32_t POOLPAGE_MEDIUM_NEW_ALLOCATE;

class POOLMEDIUMPAGES
{
private:
	POOLPAGEMEDIUM* poolPages;
	uint32_t pagesAmount;
	uint32_t pagesReserved;

	POOLPAGEMEDIUM* pageAct;
	uint8_t poolFlag;


public:
	POOLMEDIUMPAGES(uint32_t res, uint32_t ss, uint32_t align)
	{
		poolPages = new POOLPAGEMEDIUM[res];
		for (int i = 0; i < res; i++)
			poolPages[i] = POOLPAGEMEDIUM(0);
		for (int i = 0; i < ss; i++)
			poolPages[i].commitMemory(align);

		pageAct = poolPages;
		pagesAmount = ss;
		pagesReserved = res;
	}

	void loadFirstFreePage()
	{
		if ((pageAct->isPageFull() == false) && (pageAct->isPageLocked() == false))
			return;
		for (int i = 0; i < pagesAmount; i++)
		{
			if ((poolPages[i].isPageFull() == false) && (poolPages[i].isPageLocked() == false))
			{
				pageAct = poolPages + i;
				break;
			}
		}
		pageAct = (POOLPAGEMEDIUM*)MISS;
	}

	POOLPAGEMEDIUM* getActivePage()
	{
		return pageAct;
	}

	int getPageAmount()
	{
		return pagesAmount;
	}

	void loadFirstFreePageAligned(uint32_t align_, uint32_t offset = 0)
	{
		if ((pageAct->isPageFull() == false) && (pageAct->isPageLocked() == false) && (pageAct->pageAlign() == align_))
			return;
		for (int i = offset; i < pagesAmount; i++)
		{
			if ((poolPages[i].isPageFull() == false) && (poolPages[i].isPageLocked() == false) && (pageAct->pageAlign() == align_))
			{
				pageAct = poolPages + i;
				break;
			}
		}
		pageAct = (POOLPAGEMEDIUM*)MISS;
	}

	int getReservedPage(int sindex)
	{
		for (int i = sindex; i < pagesReserved; i++)
		{
			if (!(poolPages + i)->isAllocated())
				return i;
		}
	}

	void reservePages(uint32_t amount)
	{
		POOLPAGEMEDIUM* temp = poolPages;
		poolPages = new POOLPAGEMEDIUM[amount + pagesReserved];
		memcpy(poolPages, temp, sizeof(POOLPAGEMEDIUM) * pagesReserved);
		delete[] temp;
		for (int i = 0; i < amount; i++)
			*(poolPages + pagesReserved + i) = POOLPAGEMEDIUM(0);
		pagesReserved += amount;
	}

	int commitPages(uint32_t amount, uint32_t align_, uint32_t offset = 0)
	{
		uint32_t off = offset;
		for (int i = 0; i < amount; i++)
		{
			off = getReservedPage(off);
			(poolPages + off)->commitMemory(align_);
		}
		pagesAmount += amount;
		return off;
	}

	int allocPages(uint32_t amount, uint32_t align_)
	{
		if (amount + pagesAmount > pagesReserved)
		{
			reservePages(amount);
		}
		return commitPages(amount, align_);
	}

	void* allocAligned(uint32_t align_)
	{
		loadFirstFreePageAligned(align_);
		if (pageAct == MISS)
		{
			int newpg = allocPages(POOLPAGE_MEDIUM_NEW_ALLOCATE, align_);
			pageAct = poolPages + newpg;
		}
		return pageAct->allocPageMemoryAligned();
	}

	void* alloc(uint32_t size, uint32_t align_) // do not use, in progress
	{
		if (size > align_ * 32)
		{
			throw "Allocation cannot fit in one page.";
			return nullptr;
		}
		loadFirstFreePageAligned(align_);
		if (pageAct == MISS)
		{
			int newpg = allocPages(POOLPAGE_MEDIUM_NEW_ALLOCATE, align_);
			pageAct = poolPages + newpg;
		}
		void* mm = pageAct->allocPageMemory(size);
		return mm;
	}

};

template<typename T>
class STACK
{
public:

	uint32_t c_size;
	T* base_ptr;

	STACK() {}

	STACK(size_t e_size)
	{
		base_ptr = new T[e_size];
		c_size = 0;
	}

	void put(T o)
	{
		*(base_ptr + c_size) = o;
		c_size++;
	}

	T* alloc()
	{
		c_size++;
		return base_ptr + (c_size - 1);
	}

	T fget()
	{
		if (c_size == 0)
			return *base_ptr;
		return *(base_ptr + (c_size - 1));
	}

	void pop()
	{
		c_size--;
	}

	size_t getSize()
	{
		return c_size;
	}
	
};

template<typename T>
class DynamicList
{
public:
	T* base_ptr;
	uint32_t c_size;
	size_t reserved;

	DynamicList() {}

	DynamicList(size_t _size)
	{
		c_size = 0;
		reserved = _size;
		base_ptr = new T[_size];
	}

	void push_back(T obj)
	{
		if (c_size == reserved)
		{
			reserve(reserved * 2);
		}
		base_ptr[c_size] = obj;
		c_size++;
	}

	T at(size_t index)
	{
		return base_ptr[index];
	}

	T* atp(size_t index)
	{
		return base_ptr + index;
	}

	void reserve(size_t n_size)
	{
		T* o_base_ptr = base_ptr;
		base_ptr = new T[n_size];
		memcpy(base_ptr, o_base_ptr, c_size * sizeof(T));
		delete[] o_base_ptr;
	}

	void del_last()
	{
		c_size--;
	}
};

