#ifndef PROMISE_H_
#define PROMISE_H_

#include <type_traits>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <cassert>

namespace promise_impl
{
	template<typename T>
	class DelayedInit
	{
	public:
		DelayedInit()
		{
		}

		~DelayedInit()
		{
			if (this->_hasBeenInit)
				this->_hasObjectPtr->~T();
		}

		DelayedInit(DelayedInit const &) = delete;
		DelayedInit & operator=(DelayedInit const &) = delete;

		template <typename... Args>
		void Init(Args&&... args)
		{
			this->_hasObjectPtr = new (&this->_storage) T(args...);
			this->_hasBeenInit = true;
		}

		bool HasBeenInit() const
		{
			return this->_hasBeenInit;
		}

		T & operator*()
		{
			assert(this->_hasBeenInit);
			return *_hasObjectPtr;
		}

		T const& operator*() const
		{
			assert(this->_hasBeenInit);
			return _hasObjectPtr;
		}

		T * operator->()
		{
			assert(this->_hasBeenInit);
			return _hasObjectPtr;
		}

		T const* operator->() const
		{
			assert(this->_hasBeenInit);
			return _hasObjectPtr;
		}

	private:
		bool _hasBeenInit = false;
		typename std::aligned_storage<sizeof(T)>::type _storage;
		T * _hasObjectPtr = nullptr;
	};

	template<typename T>
	class SharedObject
	{
	public:
		void Wait() const
		{
			std::unique_lock<std::mutex> ul(this->_mutex);
			this->_notifier.wait(ul, 
			[&]
			{
				return this->_sharedObject.HasBeenInit();
			});
		}

		T& GetValue()
		{
			if (this->_sharedObject.HasBeenInit())
				return *this->_sharedObject;
			throw std::runtime_error("Shared Object not initialized");
		}

		template <typename U>
		void SetValue(U & value)
		{
			{
				std::lock_guard<std::mutex> lg(this->_mutex);
				this->_sharedObject.Init(value);
			}
			this->_notifier.notify_all();
		}
	private:
		mutable std::mutex _mutex;
		mutable std::condition_variable _notifier;
		DelayedInit<T> _sharedObject;
	};
}

template<typename T>
class Future
{
	typedef T data_type;

	typedef promise_impl::SharedObject<data_type> shared_data_type;
	typedef std::shared_ptr<shared_data_type> shared_data_type_ptr;
public:
	Future() = default;

	Future(shared_data_type_ptr const & parSharedPtr)
		: _sharedObject(parSharedPtr)
	{

	}

	data_type const & GetValue()
	{
		this->_sharedObject->Wait();
		return this->_sharedObject->GetValue();
	}

private:
	shared_data_type_ptr _sharedObject;
};

template<typename T>
class Promise
{
	typedef T data_type;
	typedef promise_impl::SharedObject<data_type> shared_data_type;
	typedef std::shared_ptr<shared_data_type> shared_data_type_ptr;
public:

	Promise<T>& operator=(data_type const & parValue)
	{
		for (auto & sharedState : this->_sharedObject)
		{
			sharedState->SetValue(parValue);
		}
		return *this;
	}

	void SetValue(data_type & parValue)
	{
		for (auto & sharedState : this->_sharedObject)
		{
			sharedState->SetValue(parValue);
		}
	}

	Future<data_type> GetFuture()
	{
		shared_data_type_ptr sharedObject = std::make_shared<shared_data_type>();
		this->_sharedObject.push_back(sharedObject);
		return Future<data_type>(this->_sharedObject.back());
	}

private:
	std::vector<shared_data_type_ptr> _sharedObject;
};


#endif // !PROMISE_H_
