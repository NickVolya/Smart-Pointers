#pragma once

#include <exception>

class BadWeakPtr : public std::exception {};

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

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;
