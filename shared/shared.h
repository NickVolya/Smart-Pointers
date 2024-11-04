#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <type_traits>

class ControlBlock {
public:
    virtual void IncrementWeak() = 0;
    virtual void DecrementWeak() = 0;
    virtual void IncrementStrong() = 0;
    virtual void DecrementStrong() = 0;
    virtual size_t UseStrongCount() = 0;
    virtual size_t UseWeakCount() = 0;
    virtual ~ControlBlock() = default;
};

template <typename T>
class ControlBlockWithObject : public ControlBlock {
public:
    size_t UseStrongCount() override {
        return strong_counter_;
    }
    size_t UseWeakCount() override {
        return weak_counter_;
    }
    void IncrementStrong() override {
        ++strong_counter_;
    }
    void DecrementStrong() override {
        --strong_counter_;
        if (strong_counter_ == 0) {
            if (Get() != nullptr) {
                Get()->~T();
            }
        }
    }
    void IncrementWeak() override {
        ++weak_counter_;
    }
    void DecrementWeak() override {
        --weak_counter_;
    }
    T* Get() {
        return reinterpret_cast<T*>(&object_);
    }
    template <typename... Args>
    ControlBlockWithObject(Args&&... args) {
        strong_counter_ = 1;
        weak_counter_ = 0;
        ::new (&object_) T(std::forward<Args>(args)...);
    }
    ~ControlBlockWithObject(){};

private:
    size_t weak_counter_{};
    size_t strong_counter_{};
    std::aligned_storage_t<sizeof(T), alignof(T)> object_;
};

template <typename T>
class ControlBlockWithPtr : public ControlBlock {
public:
    size_t UseStrongCount() override {
        return strong_counter_;
    }
    size_t UseWeakCount() override {
        return weak_counter_;
    }
    void IncrementStrong() override {
        ++strong_counter_;
    }
    void DecrementStrong() override {
        --strong_counter_;
        if (strong_counter_ == 0) {
            delete object_;
        }
    }
    void IncrementWeak() override {
        ++weak_counter_;
    }
    void DecrementWeak() override {
        --weak_counter_;
    }
    ControlBlockWithPtr(T* ptr) {
        strong_counter_ = 1;
        weak_counter_ = 0;
        object_ = ptr;
    }
    ~ControlBlockWithPtr(){};

private:
    size_t strong_counter_{};
    size_t weak_counter_{};
    T* object_;
};

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    template <typename Y>
    friend class SharedPtr;
    template <typename Y>
    friend class WeakPtr;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() noexcept {
        object_ptr_ = nullptr;
        block_ = nullptr;
    };
    SharedPtr(std::nullptr_t) noexcept {
        object_ptr_ = nullptr;
        block_ = nullptr;
    };
    explicit SharedPtr(T* ptr) {
        object_ptr_ = ptr;
        if (ptr != nullptr) {
            block_ = new ControlBlockWithPtr<T>(ptr);
        }
    };
    template <typename Y>
    explicit SharedPtr(Y* ptr) {
        object_ptr_ = ptr;
        if (ptr != nullptr) {
            block_ = new ControlBlockWithPtr<Y>(ptr);
        }
    };
    template <typename Y>
    SharedPtr(ControlBlockWithObject<Y>* block, T* ptr) {
        object_ptr_ = ptr;
        block_ = block;
    }

    SharedPtr(const SharedPtr& other) noexcept {
        if (other) {
            object_ptr_ = other.object_ptr_;
            block_ = other.block_;
            block_->IncrementStrong();
        } else {
            object_ptr_ = nullptr;
            block_ = nullptr;
        }
    };
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other) noexcept {
        if (other) {
            object_ptr_ = other.object_ptr_;
            block_ = other.block_;
            block_->IncrementStrong();
        } else {
            object_ptr_ = nullptr;
            block_ = nullptr;
        }
    };
    SharedPtr(SharedPtr&& other) noexcept {
        object_ptr_ = nullptr;
        block_ = nullptr;
        this->Swap(other);
        other.Reset();
    };
    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other) {
        object_ptr_ = other.object_ptr_;
        block_ = other.block_;
        other.object_ptr_ = nullptr;
        other.block_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) {
        if (other) {
            object_ptr_ = ptr;
            block_ = other.block_;
            block_->IncrementStrong();
        } else {
            block_ = nullptr;
            object_ptr_ = nullptr;
        }
    };

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.Expired()) {
            throw BadWeakPtr();
        } else {
            object_ptr_ = other.object_ptr_;
            block_ = other.block_;
            block_->IncrementStrong();
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) noexcept {
        if (this == &other) {
            return *this;
        }
        if (block_ == other.block_) {
            object_ptr_ = other.object_ptr_;
            return *this;
        }
        if (*this) {
            block_->DecrementStrong();
            if (block_->UseStrongCount() == 0 && block_->UseWeakCount() == 0) {
                delete block_;
            }
        }
        object_ptr_ = other.object_ptr_;
        block_ = other.block_;
        if (other) {
            block_->IncrementStrong();
        }
        return *this;
    };
    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        this->Swap(other);
        other.Reset();
        return *this;
    };
    template <typename Y>
    SharedPtr& operator=(SharedPtr<Y>&& other) noexcept {
        this->Reset();
        object_ptr_ = other.object_ptr_;
        block_ = other.block_;
        other.object_ptr_ = nullptr;
        other.block_ = nullptr;
        return *this;
    };
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        if (*this) {
            block_->DecrementStrong();
            if (block_->UseStrongCount() == 0 && block_->UseWeakCount() == 0) {
                delete block_;
            }
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() noexcept {
        if (*this) {
            block_->DecrementStrong();
            if (block_->UseStrongCount() == 0 && block_->UseWeakCount() == 0) {
                delete block_;
            }
            block_ = nullptr;
            object_ptr_ = nullptr;
        }
    };
    void Reset(T* ptr) {
        if (*this) {
            block_->DecrementStrong();
            if (block_->UseStrongCount() == 0 && block_->UseWeakCount() == 0) {
                delete block_;
            }
        }
        object_ptr_ = ptr;
        block_ = new ControlBlockWithPtr(ptr);
    };
    template <typename Y>
    void Reset(Y* ptr) {
        if (*this) {
            block_->DecrementStrong();
            if (block_->UseStrongCount() == 0 && block_->UseWeakCount() == 0) {
                delete block_;
            }
        }
        object_ptr_ = ptr;
        block_ = new ControlBlockWithPtr(ptr);
    };
    void Swap(SharedPtr& other) noexcept {
        std::swap(object_ptr_, other.object_ptr_);
        std::swap(block_, other.block_);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
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
    size_t UseCount() const noexcept {
        if (*this) {
            return block_->UseStrongCount();
        }
        return 0;
    };
    explicit operator bool() const noexcept {
        return object_ptr_ != nullptr;
    };

private:
    T* object_ptr_;
    ControlBlock* block_;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right);

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    ControlBlockWithObject<T>* block = new ControlBlockWithObject<T>(std::forward<Args>(args)...);
    return SharedPtr(block, block->Get());
};

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};
