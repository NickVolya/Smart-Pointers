#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    size_t IncRef() noexcept {
        return ++count_;
    };
    size_t DecRef() noexcept {
        return --count_;
    };
    size_t RefCount() const noexcept {
        return count_;
    };

private:
    size_t count_ = 0;
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    // Increase reference counter.
    void IncRef() {
        counter_.IncRef();
    };

    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef() {
        if (counter_.RefCount() < 2) {
            if (counter_.RefCount() == 1) {
                counter_.DecRef();
            }
            Deleter::Destroy(static_cast<Derived*>(this));
            return;
        }
        counter_.DecRef();
    };

    // Get current counter value (the number of strong references).
    size_t RefCount() const {
        return counter_.RefCount();
    };

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    // Constructors
    IntrusivePtr() noexcept {
        object_ptr_ = nullptr;
    };
    IntrusivePtr(std::nullptr_t) noexcept {
        object_ptr_ = nullptr;
    };
    IntrusivePtr(T* ptr) noexcept {
        object_ptr_ = ptr;
        if (ptr != nullptr) {
            object_ptr_->IncRef();
        }
    };

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) noexcept {
        object_ptr_ = other.object_ptr_;
        if (object_ptr_ != nullptr) {
            object_ptr_->IncRef();
        }
    };

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) noexcept {
        object_ptr_ = other.object_ptr_;
        other.object_ptr_ = nullptr;
    };

    IntrusivePtr(const IntrusivePtr& other) noexcept {
        if (object_ptr_ != other.object_ptr_) {
            object_ptr_ = other.object_ptr_;
            if (object_ptr_ != nullptr) {
                object_ptr_->IncRef();
            }
        }
    };
    IntrusivePtr(IntrusivePtr&& other) noexcept {
        object_ptr_ = other.object_ptr_;
        other.object_ptr_ = nullptr;
    };

    // `operator=`-s
    IntrusivePtr& operator=(const IntrusivePtr& other) noexcept {
        if (this == &other) {
            return *this;
        }
        if (object_ptr_ == other.object_ptr_) {
            return *this;
        }
        if (object_ptr_ != nullptr) {
            object_ptr_->DecRef();
        }
        object_ptr_ = other.object_ptr_;
        if (object_ptr_ != nullptr) {
            object_ptr_->IncRef();
        }
        return *this;
    };
    IntrusivePtr& operator=(IntrusivePtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        this->Swap(other);
        other.Reset();
        return *this;
    };

    // Destructor
    ~IntrusivePtr() {
        if (object_ptr_ != nullptr) {
            object_ptr_->DecRef();
        }
    };

    // Modifiers
    void Reset() noexcept {
        if (object_ptr_ != nullptr) {
            object_ptr_->DecRef();
            object_ptr_ = nullptr;
        }
    };
    void Reset(T* ptr) noexcept {
        if (object_ptr_ != nullptr) {
            object_ptr_->DecRef();
        }
        object_ptr_ = ptr;
        if (object_ptr_ != nullptr) {
            object_ptr_->IncRef();
        }
    };
    void Swap(IntrusivePtr& other) noexcept {
        std::swap(object_ptr_, other.object_ptr_);
    };

    // Observers
    T* Get() const noexcept {
        return object_ptr_;
    };
    T& operator*() const noexcept {
        return *object_ptr_;
    };
    T* operator->() const noexcept {
        return object_ptr_;
    };
    size_t UseCount() const {
        if (object_ptr_ != nullptr) {
            return object_ptr_->RefCount();
        }
        return 0;
    };
    explicit operator bool() const noexcept {
        return object_ptr_ != nullptr;
    };

private:
    T* object_ptr_;
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    T* ptr = new T(std::forward<Args>(args)...);
    return IntrusivePtr(ptr);
}
