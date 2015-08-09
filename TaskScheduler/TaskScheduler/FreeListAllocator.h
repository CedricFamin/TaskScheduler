#ifndef FREE_LIST_ALLOCATOR_H_
#define FREE_LIST_ALLOCATOR_H_

#include <vector>
#include <cassert>

// O(1) Create new task
// O(1) Delete task
// Avoid memory fragmentation

class Id
{
public:
	static unsigned int const IndexMask = 0x0000FFFF;
	static unsigned int const GenMask   = 0xFFFF0000;
	static unsigned int const IndexShift = 0;
	static unsigned int const GenShift = 16;

	static Id InvalidId() { return Id(); }
	Id()
		:_id(0)
	{
	}

	Id(unsigned int parIdx, unsigned int parGen)
	{
		assert((parIdx & ~(IndexMask >> IndexShift)) == 0);
		assert((parGen & ~(GenMask >> GenShift)) == 0);
		_id = (IndexMask & (parIdx << IndexShift)) | (GenMask & (parGen << GenShift));
	}

	unsigned int Index() const
	{
		return (_id & IndexMask) >> IndexShift;
	}

	unsigned int Gen() const
	{
		return (_id & GenMask) >> GenShift;
	}

	bool operator==(Id const & parOther) const
	{
		return _id == parOther._id;
	}

	bool operator!=(Id const & parOther) const
	{
		return !(*this == parOther);
	}
private:
	int _id;
};

template<typename T>
class FreeListAllocator
{
public:
	typedef T data_type;
private:
	struct ObjectWNext
	{
		ObjectWNext()
			: Generation(0)
			, Next(0)
		{
			assert(this == reinterpret_cast<void*>(&Data)); // maybe on some obscure plateforms this could be false ?
		}
		data_type Data;
		unsigned int  Generation;
		ObjectWNext * Next;
	};
public:
	FreeListAllocator(unsigned int parPoolSize)
		: _memoryPool(parPoolSize)
		, _nbAllocation(0)
	{
		this->_next = reinterpret_cast<ObjectWNext*>(this->_memoryPool.data());

		ObjectWNext * prev = this->_next;
		ObjectWNext * current = this->_next + 1;
		while (current < &(this->_memoryPool.back()))
		{
			prev->Next = current;
			prev = current;
			current = current + 1;
		}
		prev->Next = nullptr;
	}

	~FreeListAllocator()
	{
		assert(_nbAllocation == 0);
	}

	std::pair<Id, data_type *> GetObject()
	{
		if (this->_next == nullptr)
			return std::make_pair(Id(0, 0), nullptr);
		++this->_next->Generation;
		Id id(this->_next - this->_memoryPool.data(), this->_next->Generation);
		data_type * newObject = new (&this->_next->Data) data_type;
		this->_next = this->_next->Next;
		++_nbAllocation;
		return std::make_pair(id, newObject);
	}

	data_type* GetObject(Id const & parId)
	{
		unsigned int const index = parId.Index();
		unsigned int const gen = parId.Gen();
		assert(_memoryPool[index].Generation == gen);
		return &_memoryPool[index].Data;
	}

	void ReleaseObject(data_type * parObject)
	{
		parObject->~data_type();
		ObjectWNext * objAlloc = reinterpret_cast<ObjectWNext*>(parObject);
		objAlloc->Next = this->_next;
		this->_next = objAlloc;
		assert(_nbAllocation > 0);
		--_nbAllocation;
		++this->_next->Generation;
	}

	bool IsAlive(Id const & parId) const
	{
		unsigned int const index = parId.Index();
		unsigned int const gen = parId.Gen();
		return _memoryPool[index].Generation == gen;
	}

private:
	std::vector<ObjectWNext> _memoryPool; // TODO maybe this should be const ? But I don't want to use a const cast to initialize _next
	ObjectWNext * _next;

	// debug
	unsigned int _nbAllocation;
};

#endif