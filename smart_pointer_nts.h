#pragma once

#ifdef SMART_POINTER_NTS_PRINT_LOG
#define SMART_POINTER_NTS_LOG(message) std::cout << message << std::endl
#else
#define SMART_POINTER_NTS_LOG(message) 
#endif

#include <assert.h>
#include <functional>
#include <string>
#include <iostream>


namespace smart_pointer_nts
{

	/// <summary>
	/// reference count container.
	/// this object must be disposed just after not having had owner and observer.
	/// </summary>
	class SharedPtrRefCounter
	{
	public:
		template <class T, class U>friend class shared_ptr;
		template <class T>friend class weak_ptr;

	private:
		/// <summary>
		/// constructor.
		/// </summary>
		SharedPtrRefCounter(void* resource)
			: sref_count(1)
			, wref_count(0)
			, resource(resource)
		{
			SMART_POINTER_NTS_LOG("create counter " + std::to_string((unsigned long)this) + " for " + std::to_string((unsigned long)resource));
		}

		~SharedPtrRefCounter()
		{
			SMART_POINTER_NTS_LOG("delete counter: " + std::to_string((unsigned long)this) + " for " + std::to_string((unsigned long)resource));
		}

		/// <summary>
		/// increase ref count.
		/// </summary>
		int IncreaseOwner()
		{
			++sref_count;
			SMART_POINTER_NTS_LOG("update ref: owner=" + std::to_string(sref_count) + ", observer=" + std::to_string(wref_count) + " for " + std::to_string((unsigned long)resource));
			return sref_count;
		}

		/// <summary>
		/// decrease ref count.
		/// </summary>
		int DecreaseOwner()
		{
			--sref_count;
			SMART_POINTER_NTS_LOG("update ref: owner=" + std::to_string(sref_count) + ", observer=" + std::to_string(wref_count) + " for " + std::to_string((unsigned long)resource));
			return sref_count;
		}

		/// <summary>
		/// increase ref count.
		/// </summary>
		int IncreaseObserver()
		{
			++wref_count;
			SMART_POINTER_NTS_LOG("update ref: owner=" + std::to_string(sref_count) + ", observer=" + std::to_string(wref_count) + " for " + std::to_string((unsigned long)resource));
			return wref_count;
		}

		/// <summary>
		/// decrease ref count.
		/// </summary>
		int DecreaseObserver()
		{
			--wref_count;
			SMART_POINTER_NTS_LOG("update ref: owner=" + std::to_string(sref_count) + ", observer=" + std::to_string(wref_count) + " for " + std::to_string((unsigned long)resource));
			return wref_count;
		}

		/// <summary>
		/// count owners.
		/// </summary>
		long CountOwners() const
		{
			return sref_count;
		}

		/// <summary>
		/// count obesrvers.
		/// </summary>
		long CountObservers() const
		{
			return wref_count;
		}


		/// <summary>
		/// reference count for shared pointer.
		/// </summary>
		int sref_count;

		/// <summary>
		/// refrence count for weak pointer.
		/// </summary>
		int wref_count;

		/// <summary>
		/// pointer managed by smart ptr.
		/// currently used only for log.
		/// </summary>
		const void* const resource;

	};


	/// <summary>
	/// abstract smart pointer class with non thread safe.
	/// </summary>
	template <class T0, class Dt>
	class smart_ptr_nts
	{
	public:
		using T = typename std::remove_extent<T0>::type;

	protected:
		/// <summary>
		/// constructor with nullptr.
		/// </summary>
		smart_ptr_nts()
			: rawPtr(nullptr)
			, deleter(nullptr)
		{
		}

		/// <summary>
		/// constructor.
		/// </summary>
		template<class U>
		smart_ptr_nts(U* ptr, Dt deleter = &Deleter<U>)
			: rawPtr(ptr)
			, deleter(ptr ? deleter : nullptr)
		{
		}

		/// <summary>
		/// copy constructor.
		/// </summary>
		smart_ptr_nts(const smart_ptr_nts& target)
		{
			this->deleter = target.deleter;
			this->rawPtr = target.rawPtr;
		}

		/// <summary>
		/// move constractor.
		/// </summary>
		smart_ptr_nts(smart_ptr_nts&& target) noexcept
		{
			this->deleter = target.deleter;
			this->rawPtr = target.rawPtr;

			target.deleter = nullptr;
			target.rawPtr = nullptr;
		}

		/// <summary>
		/// destructor.
		/// </summary>
		virtual ~smart_ptr_nts()
		{
			Dispose();
		}

		template <class U>
		static void Deleter(void* obj)
		{
			if constexpr (std::is_array<T0>::value == true)
				delete[] static_cast<U*>(obj);
			else
				delete static_cast<U*>(obj);
		}

		/// <summary>
		/// copy assignment.
		/// </summary>
		smart_ptr_nts& operator=(const smart_ptr_nts& target)
		{
			Dispose();
			this->rawPtr = target.rawPtr;
			this->deleter = target.deleter;

			return *this;
		}

		/// <summary>
		/// move asignment.
		/// </summary>
		smart_ptr_nts& operator=(smart_ptr_nts&& target) noexcept
		{
			Dispose();
			this->rawPtr = target.rawPtr;
			this->deleter = target.deleter;

			target.rawPtr = nullptr;
			target.deleter = nullptr;

			return *this;
		}

		/// <summary>
		/// get rew pointer.
		/// </summary>
		T* Get() const
		{
			return this->rawPtr;
		}

		/// <summary>
		/// dispose current resource, and set null.
		/// </summary>
		void reset()
		{
			Dispose();
		}

		/// <summary>
		/// dispose current resource, and set new resource.
		/// </summary>
		template<class U>
		void reset(U* ptr, std::function<void(void*)> deleter = &Deleter<U>)
		{
			Dispose();
			if (this->rawPtr = ptr)
				this->deleter = deleter;
		}

		/// <summary>
		/// disable deleter for keeping a managing resource.
		/// </summary>
		void DisableDisposing()
		{
			this->deleter = nullptr;
		}

	private:
		/// <summary>
		/// dispose a managing resource.
		/// </summary>
		void Dispose()
		{
			if (this->deleter)
			{
				deleter(this->rawPtr);
				SMART_POINTER_NTS_LOG("release resource: " + std::to_string((unsigned long)this->rawPtr));
			}
			this->rawPtr = nullptr;
			this->deleter = nullptr;
		}

		/// <summary>
		/// rew pointer.
		/// </summary>
		T* rawPtr;

		/// <summary>
		/// reference to deleter.
		/// </summary>
		Dt deleter;

	};


	/// <summary>
	/// non thread safe unique pointer class.
	/// </summary>
	template <class T0, class Dt = std::function<void(void*)>>
	class unique_ptr : public smart_ptr_nts<T0, Dt>
	{
	public:
		using T = typename std::remove_extent<T0>::type;
		using SmartPtrBase = smart_ptr_nts<T0, Dt>;

		/// <summary>
		/// constructor with nullptr.
		/// </summary>
		unique_ptr()
			: SmartPtrBase()
		{
		}

		/// <summary>
		/// constructor.
		/// </summary>
		unique_ptr(T* ptr)
			: SmartPtrBase(ptr)
		{
			if(ptr)
				SMART_POINTER_NTS_LOG("retain resource: " + std::to_string((unsigned long)ptr) + " with unique ptr");
		}

		/// <summary>
		/// constructor with deleter.
		/// </summary>
		unique_ptr(T* ptr, Dt deleter)
			: SmartPtrBase(ptr, deleter)
		{
			if (ptr)
				SMART_POINTER_NTS_LOG("retain resource: " + std::to_string((unsigned long)ptr) + " with unique ptr");
		}

		/// <summary>
		/// copy constractor is disabled.
		/// </summary>
		unique_ptr(unique_ptr&) = delete;

		/// <summary>
		/// move constractor.
		/// </summary>
		unique_ptr(unique_ptr&& target)
			: SmartPtrBase(std::move(target))
		{
		}

		/// <summary>
		/// destructor.
		/// </summary>
		virtual ~unique_ptr()
		{
			Dispose();
		}

		/// <summary>
		/// check if pointer is not null.
		/// </summary>
		explicit operator bool() const
		{
			return this->get();
		}

		/// <summary>
		/// copy assignment is disabled.
		/// </summary>
		unique_ptr& operator=(unique_ptr& target) = delete;

		/// <summary>
		/// move assignment.
		/// </summary>
		unique_ptr& operator=(unique_ptr&& target) noexcept
		{
			assert(this != &target);

			Dispose();
			SmartPtrBase::operator=(std::move(target));

			return *this;
		}

		/// <summary>
		/// calling members of a managing resource.
		/// </summary>
		template <class U = T0>
		auto operator->() -> typename std::enable_if<std::is_same<T0, U>::value && !std::is_array<T0>::value, T*>::type
		{
			return this->get();
		}

		/// <summary>
		/// calling members of a managing resource.
		/// </summary>
		template <class U = T0>
		auto operator[](size_t index) -> typename std::enable_if<std::is_same<T0, U>::value && std::is_array<T0>::value, T&>::type
		{
			return *(this->get() + index);
		}

		/// <summary>
		/// get rew pointer.
		/// </summary>
		T* get() const
		{
			return this->Get();
		}

		/// <summary>
		/// dispose current resource, and set null.
		/// </summary>
		void reset()
		{
			Dispose();
			SmartPtrBase::reset();
		}

		/// <summary>
		/// dispose current resource, and set new resource.
		/// </summary>
		template<class U>
		void reset(U* ptr)
		{
			Dispose();
			SmartPtrBase::reset(ptr);
			if (ptr)
				SMART_POINTER_NTS_LOG("retain resource: " + std::to_string((unsigned long)ptr) + " with unique ptr");
		}

		/// <summary>
		/// dispose current resource, and set new resource.
		/// </summary>
		template<class U>
		void reset(U* ptr, Dt deleter)
		{
			Dispose();
			SmartPtrBase::reset(ptr, deleter);
			if (ptr)
				SMART_POINTER_NTS_LOG("retain resource: " + std::to_string((unsigned long)ptr) + " with unique ptr");
		}

	private:
		/// <summary>
		/// dispose resource.
		/// </summary>
		void Dispose()
		{
			// nothing todo.
		}

	};

	/// <summary>
	/// compare managing resources.
	/// </summary>
	template  <class T, class M>
	bool operator==(unique_ptr<T>& target1, unique_ptr<M>& target2)
	{
		return target1.get() == target2.get();
	}

	/// <summary>
	/// check if it is null.
	/// </summary>
	template  <class T>
	bool operator==(const unique_ptr<T>& target, nullptr_t)
	{
		return !target.get();
	}


	/// <summary>
	/// non thread safe shared pointer class.
	/// </summary>
	template <class T0, class Dt = std::function<void(void*)>>
	class shared_ptr : public smart_ptr_nts<T0, Dt>
	{
	public:
		using T = typename std::remove_extent<T0>::type;
		using SmartPtrBase = smart_ptr_nts<T0, Dt>;

		/// <summary>
		/// constructor with nullptr.
		/// </summary>
		shared_ptr()
			: SmartPtrBase()
			, ref_count(nullptr)
		{
		}

		/// <summary>
		/// constructor.
		/// </summary>
		shared_ptr(T * ptr)
			: SmartPtrBase(ptr)
			, ref_count(nullptr)
		{
			if (ptr)
				ref_count = CreateCounter(ptr);
		}

		/// <summary>
		/// constructor.
		/// </summary>
		shared_ptr(T * ptr, Dt deleter)
			: SmartPtrBase(ptr, deleter)
			, ref_count(nullptr)
		{
			if (ptr)
				ref_count = CreateCounter(ptr);
		}

		/// <summary>
		/// copy constructor.
		/// </summary>
		shared_ptr(const shared_ptr& target)
			: SmartPtrBase(target)
			, ref_count(target.ref_count)
		{
			if(ref_count)
				ref_count->IncreaseOwner();
		}

		/// <summary>
		/// move constractor.
		/// </summary>
		shared_ptr(shared_ptr&& target) noexcept
			: SmartPtrBase(std::move(target))
			, ref_count(target.ref_count)
		{
			target.ref_count = nullptr;
		}

		/// <summary>
		/// destructor.
		/// </summary>
		virtual ~shared_ptr()
		{
			Dispose();
		}

		/// <summary>
		/// check if pointer is not null.
		/// </summary>
		explicit operator bool() const
		{
			return this->get();
		}

		/// <summary>
		/// copy assignment.
		/// </summary>
		shared_ptr& operator=(const shared_ptr& target)
		{
			if (this->ref_count == target.ref_count)
				return *this;

			Dispose();
			SmartPtrBase::operator=(target);
			if (this->ref_count = target.ref_count)
			{
				ref_count->IncreaseOwner();
			}
			return *this;
		}

		/// <summary>
		/// move assignment.
		/// </summary>
		shared_ptr& operator=(shared_ptr&& target)
		{
			assert(this != &target);

			Dispose();
			SmartPtrBase::operator=(std::move(target));
			this->ref_count = target.ref_count;

			target.ref_count = nullptr;
			return *this;
		}

		/// <summary>
		/// calling members of a managing resource.
		/// </summary>
		template <class U = T0>
		auto operator->() -> typename std::enable_if<std::is_same<T0, U>::value && !std::is_array<T0>::value, T*>::type
		{
			return this->get();
		}

		/// <summary>
		/// calling members of a managing resource.
		/// </summary>
		template <class U = T0>
		auto operator[](size_t index) -> typename std::enable_if<std::is_same<T0, U>::value && std::is_array<T0>::value, T&>::type
		{
			return *(this->get() + index);
		}

		/// <summary>
		/// get rew pointer.
		/// </summary>
		T* get() const
		{
			return this->Get();
		}

		/// <summary>
		/// dispose current resource, and set null.
		/// </summary>
		void reset()
		{
			Dispose();
			SmartPtrBase::reset();
		}

		/// <summary>
		/// dispose current resource, and set new resource.
		/// </summary>
		template<class U>
		void reset(U* ptr)
		{
			Dispose();
			SmartPtrBase::reset(ptr);
			if (ptr)
				this->ref_count = CreateCounter(ptr);
		}

		/// <summary>
		/// dispose current resource, and set new resource.
		/// </summary>
		template<class U>
		void reset(U* ptr, Dt deleter)
		{
			Dispose();
			SmartPtrBase::reset(ptr, deleter);
			if (ptr)
				this->ref_count = CreateCounter(ptr);
		}

		/// <summary>
		/// get reference count.
		/// </summary>
		long use_count() const
		{
			long result = 0;
			if (ref_count)
				result = ref_count->CountOwners();

			return result;
		}

		/// <summary>
		/// accesser class for weak pointer.
		/// </summary>
		class AccesserForWeakPtr
		{
			template <class U>
			friend class weak_ptr;

		private:
			static SharedPtrRefCounter* GetRefCounter(shared_ptr<T0>& obj)
			{
				return obj.ref_count;
			}
			static shared_ptr<T0> CreateFrom(T* raw_ptr, SharedPtrRefCounter* ref_count)
			{
				if (raw_ptr && ref_count && ref_count->CountOwners())
					return shared_ptr<T0>(raw_ptr, ref_count);
				else
					return shared_ptr<T0>();
			}

		};
		friend class AccesserForWeakPtr;

	private:
		/// <summary>
		/// contractor from pointer
		/// </summary>
		shared_ptr(T* raw_ptr, SharedPtrRefCounter* ref_count)
			: SmartPtrBase(raw_ptr)
			, ref_count(ref_count)
		{
			assert(raw_ptr && ref_count && ref_count->CountOwners());
			ref_count->IncreaseOwner();
		}

		/// <summary>
		/// dispose a managing resource if needed.
		/// </summary>
		void Dispose()
		{
			if (ref_count)
			{
				if (ref_count->DecreaseOwner() == 0)
				{
					if (ref_count->CountObservers() == 0)
						delete this->ref_count;

					// a managing resource will be disposed by base disposer.
				}
				else
				{
					// has owner(s), thus not disposing yet.
					this->DisableDisposing();
				}
				this->ref_count = nullptr;
			}
		}

		/// <summary>
		/// create counter object.
		/// </summary>
		static SharedPtrRefCounter* CreateCounter(void* resource)
		{
			SMART_POINTER_NTS_LOG("retain resource: " + std::to_string((unsigned long)resource) + " with shared ptr");
			return new SharedPtrRefCounter(resource);
		}

		/// <summary>
		/// reference counter.
		/// </summary>
		SharedPtrRefCounter* ref_count;

	};

	/// <summary>
	/// compare managing resources.
	/// </summary>
	template  <class T, class M>
	bool operator==(shared_ptr<T>& target1, shared_ptr<M>& target2)
	{
		return target1.get() == target2.get();
	}

	/// <summary>
	/// check if it is null.
	/// </summary>
	template  <class T>
	bool operator==(shared_ptr<T>& target, nullptr_t)
	{
		return !target.get();
	}
	
	
	/// <summary>
	/// non thread safe weak pointer class.
	/// </summary>
	template <class T0>
	class weak_ptr : public smart_ptr_nts<T0, std::function<void(void*)>>
	{
	public:
		using T = typename std::remove_extent<T0>::type;
		using SmartPtrBase = smart_ptr_nts<T0, std::function<void(void*)>>;

		/// <summary>
		/// constructor with nullptr.
		/// </summary>
		weak_ptr()
			: SmartPtrBase()
			, ref_count(nullptr)
		{
		}

		/// <summary>
		/// constructor with shared pointer.
		/// </summary>
		template <class Dt>
		weak_ptr(shared_ptr<T0, Dt>& sharedPtr)
			: SmartPtrBase(sharedPtr.get(), nullptr)
			, ref_count(shared_ptr<T0, Dt>::AccesserForWeakPtr::GetRefCounter(sharedPtr))
		{
			if (ref_count)
				ref_count->IncreaseObserver();
		}

		/// <summary>
		/// copy constructor.
		/// </summary>
		weak_ptr(weak_ptr& another)
			: SmartPtrBase(another)
			, ref_count(another.ref_count)
		{
			if (ref_count)
				ref_count->IncreaseObserver();
		}

		/// <summary>
		/// move constructor.
		/// </summary>
		weak_ptr(weak_ptr&& another) noexcept
			: SmartPtrBase(std::move(another))
			, ref_count(another.ref_count)
		{
			another.ref_count = nullptr;
		}

		/// <summary>
		/// destructor.
		/// </summary>
		virtual ~weak_ptr()
		{
			Dispose();
		}

		/// <summary>
		/// copy assignment.
		/// </summary>
		weak_ptr& operator=(weak_ptr& target)
		{
			if (this->ref_count == target.ref_count)
				return *this;

			Dispose();
			SmartPtrBase::operator=(target);

			if (this->ref_count = target.ref_count)
				ref_count->IncreaseObserver();

			return *this;
		}

		/// <summary>
		/// copy assignment from shared ptr.
		/// </summary>
		weak_ptr& operator=(shared_ptr<T0>& target)
		{
			weak_ptr<T0> tmpForCopy(target);
			operator=(std::move(tmpForCopy));

			return *this;
		}

		/// <summary>
		/// move assignment.
		/// </summary>
		weak_ptr& operator=(weak_ptr&& target) noexcept
		{
			assert(this != &target);

			Dispose();
			SmartPtrBase::operator=(std::move(target));
			this->ref_count = target.ref_count;
			target.ref_count = nullptr;

			return *this;
		}

		/// <summary>
		/// dispose current resource, and set null.
		/// </summary>
		void reset()
		{
			Dispose();
			SmartPtrBase::reset();
		}

		/// <summary>
		/// try locking an observing pointer.
		/// </summary>
		shared_ptr<T0> lock()
		{
			return shared_ptr<T0>::AccesserForWeakPtr::CreateFrom(
				this->Get(), this->ref_count);
		}

		/// <summary>
		/// check if managing pointer is expired or not.
		/// </summary>
		bool expired() const
		{
			bool result = true;
			if (ref_count && ref_count->CountOwners())
				result = false;

			return result;
		}

	private:
		/// <summary>
		/// dispose.
		/// </summary>
		void Dispose()
		{
			if (ref_count)
			{
				if (ref_count->DecreaseObserver() == 0)
				{
					if (ref_count->CountOwners() == 0)
						delete this->ref_count;
				}
				this->ref_count = nullptr;
			}
		}

		/// <summary>
		/// reference counter.
		/// </summary>
		SharedPtrRefCounter* ref_count;

	};
}

/// <summary>
/// hash class implementation for nts shared ptr.
/// </summary>
template <class T>
struct std::hash<smart_pointer_nts::unique_ptr<T>>
{
	std::size_t operator()(
		const smart_pointer_nts::unique_ptr<T>& target) const noexcept
	{
		return std::hash<T*>()(target.get());
	}
};

/// <summary>
/// equal_to class implementation for nts shared ptr.
/// </summary>
template <class T>
struct std::equal_to<smart_pointer_nts::unique_ptr<T>>
{
	constexpr bool operator ()(
		smart_pointer_nts::unique_ptr<T>& target1,
		smart_pointer_nts::unique_ptr<T>& target2) const
	{
		return target1.get() == target2.get();
	}
};

/// <summary>
/// hash class implementation for nts shared ptr.
/// </summary>
template <class T>
struct std::hash<smart_pointer_nts::shared_ptr<T>>
{
	std::size_t operator()(
		const smart_pointer_nts::shared_ptr<T>& target) const noexcept
	{
		return std::hash<T*>()(target.get());
	}
};

/// <summary>
/// equal_to class implementation for nts shared ptr.
/// </summary>
template <class T>
struct std::equal_to<smart_pointer_nts::shared_ptr<T>>
{
	constexpr bool operator ()(
		const smart_pointer_nts::shared_ptr<T>& target1,
		const smart_pointer_nts::shared_ptr<T>& target2) const
	{
		return target1.get() == target2.get();
	}
};