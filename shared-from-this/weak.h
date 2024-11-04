#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    template <typename Y>
    friend class SharedPtr;
    template <typename Y>
    friend class WeakPtr;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() noexcept {
        object_ptr_ = nullptr;
        block_ = nullptr;
    };

    WeakPtr(const WeakPtr& other) noexcept {
        if (other.block_ != nullptr) {
            object_ptr_ = other.object_ptr_;
            block_ = other.block_;
            block_->IncrementWeak();
        } else {
            object_ptr_ = nullptr;
            block_ = nullptr;
        }
    };
    template <typename Y>
    WeakPtr(const WeakPtr<Y>& other) noexcept {
        if (other.block_ != nullptr) {
            object_ptr_ = other.object_ptr_;
            block_ = other.block_;
            block_->IncrementWeak();
        } else {
            object_ptr_ = nullptr;
            block_ = nullptr;
        }
    };
    WeakPtr(WeakPtr&& other) noexcept {
        object_ptr_ = nullptr;
        block_ = nullptr;
        this->Swap(other);
        other.Reset();
    };

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    template <typename Y>
    WeakPtr(const SharedPtr<Y>& other) noexcept {
        object_ptr_ = other.object_ptr_;
        block_ = other.block_;
        block_->IncrementWeak();
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) noexcept {
        if (this == &other) {
            return *this;
        }
        if (block_ == other.block_) {
            object_ptr_ = other.object_ptr_;
            return *this;
        }
        if (block_ != nullptr) {
            block_->DecrementWeak();
            if (block_->UseStrongCount() == 0 && block_->UseWeakCount() == 0) {
                delete block_;
            }
        }
        object_ptr_ = other.object_ptr_;
        block_ = other.block_;
        if (block_ != nullptr) {
            block_->IncrementWeak();
        }
        return *this;
    };
    WeakPtr& operator=(WeakPtr&& other) {
        if (this == &other) {
            return *this;
        }
        this->Swap(other);
        other.Reset();
        return *this;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        if (block_ != nullptr) {
            block_->DecrementWeak();
            if (block_->UseStrongCount() == 0 && block_->UseWeakCount() == 0) {
                delete block_;
            }
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() noexcept {
        if (block_ != nullptr) {
            block_->DecrementWeak();
            if (block_->UseStrongCount() == 0 && block_->UseWeakCount() == 0) {
                delete block_;
            }
            block_ = nullptr;
            object_ptr_ = nullptr;
        }
    };
    void Swap(WeakPtr& other) noexcept {
        std::swap(object_ptr_, other.object_ptr_);
        std::swap(block_, other.block_);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const noexcept {
        if (!this->Expired()) {
            return block_->UseStrongCount();
        }
        return 0;
    };
    bool Expired() const noexcept {
        if (block_ != nullptr) {
            return block_->UseStrongCount() == 0;
        }
        return true;
    };
    SharedPtr<T> Lock() const noexcept {
        if (this->Expired()) {
            return SharedPtr<T>();
        }
        return SharedPtr(*this);
    };

    void IncBlockStrong() {
        block_->IncrementStrong();
    }

private:
    T* object_ptr_;
    ControlBlock* block_;
};
