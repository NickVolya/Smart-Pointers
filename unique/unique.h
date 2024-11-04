#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t
#include <type_traits>

template <typename T>
struct Slug {
    Slug() = default;
    template <class U>
    Slug(T&& other){};
    template <class U>
    Slug(Slug<U> other){};
    ~Slug() = default;
    template <typename U>
    void operator()(U&& other) {
        delete other;
    }
};

template <typename T>
struct Slug<T[]> {
    Slug() = default;
    template <class U>
    Slug(T&& other){};
    template <class U>
    Slug(Slug<U> other){};
    ~Slug() = default;
    template <typename U>
    void operator()(U&& other) {
        delete[] other;
    }
};

// Primary template
template <typename T, typename Deleter = Slug<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) noexcept {
        pair_.GetFirst() = ptr;
    };
    UniquePtr(T* ptr, Deleter deleter) noexcept {
        pair_.GetFirst() = ptr;
        pair_.GetSecond() = std::forward<Deleter>(deleter);
    };

    UniquePtr(UniquePtr&& other) noexcept {
        this->Swap(other);
        other.Reset();
    };

    template <class U, class B>
    UniquePtr(UniquePtr<U, B>&& other) noexcept {
        pair_.GetFirst() = other.Release();
        other.Reset();
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        this->Swap(other);
        other.Reset();
        return *this;
    };
    template <class U, class B>
    UniquePtr& operator=(UniquePtr<U, B>&& other) noexcept {
        pair_.GetSecond()(pair_.GetFirst());
        pair_.GetFirst() = other.Release();
        other.Reset();
        return *this;
    };
    UniquePtr& operator=(std::nullptr_t) noexcept {
        pair_.GetSecond()(pair_.GetFirst());
        pair_.GetFirst() = nullptr;
        return *this;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        pair_.GetSecond()(pair_.GetFirst());
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() noexcept {
        T* ret = pair_.GetFirst();
        pair_.GetFirst() = nullptr;
        return ret;
    };
    void Reset(T* ptr = nullptr) noexcept {
        T* el = pair_.GetFirst();
        pair_.GetFirst() = ptr;
        pair_.GetSecond()(el);
    };
    void Swap(UniquePtr& other) noexcept {
        std::swap(pair_, other.pair_);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const noexcept {
        return pair_.GetFirst();
    };
    Deleter& GetDeleter() noexcept {
        return pair_.GetSecond();
    };
    const Deleter& GetDeleter() const noexcept {
        return pair_.GetSecond();
    };
    explicit operator bool() const noexcept {
        if (pair_.GetFirst() != nullptr) {
            return true;
        }
        return false;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    T& operator*() const noexcept {
        return *pair_.GetFirst();
    };
    T* operator->() const noexcept {
        return pair_.GetFirst();
    };

private:
    CompressedPair<T*, Deleter> pair_;
};

template <typename Deleter>
class UniquePtr<void, Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(void* ptr = nullptr) noexcept {
        pair_.GetFirst() = ptr;
    };
    UniquePtr(void* ptr, Deleter deleter) noexcept {
        pair_.GetFirst() = ptr;
        pair_.GetSecond() = std::forward<Deleter>(deleter);
    };

    UniquePtr(UniquePtr&& other) noexcept {
        this->Swap(other);
        other.Reset();
    };

    template <class U, class B>
    UniquePtr(UniquePtr<U, B>&& other) noexcept {
        pair_.GetFirst() = other.Release();
        other.Reset();
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        this->Swap(other);
        other.Reset();
        return *this;
    };
    template <class U, class B>
    UniquePtr& operator=(UniquePtr<U, B>&& other) noexcept {
        pair_.GetSecond()(pair_.GetFirst());
        pair_.GetFirst() = other.Release();
        other.Reset();
        return *this;
    };
    UniquePtr& operator=(std::nullptr_t) noexcept {
        if (pair_.GetFirst() != nullptr) {
            pair_.GetSecond()(pair_.GetFirst());
        }
        pair_.GetFirst() = nullptr;
        return *this;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        pair_.GetSecond()(pair_.GetFirst());
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void* Release() noexcept {
        void* ret = pair_.GetFirst();
        pair_.GetFirst() = nullptr;
        return ret;
    };
    void Reset(void* ptr = nullptr) noexcept {
        void* el = pair_.GetFirst();
        pair_.GetFirst() = ptr;
        pair_.GetSecond()(el);
    };
    void Swap(UniquePtr& other) noexcept {
        std::swap(pair_, other.pair_);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    void* Get() const noexcept {
        return pair_.GetFirst();
    };
    Deleter& GetDeleter() noexcept {
        return pair_.GetSecond();
    };
    const Deleter& GetDeleter() const noexcept {
        return pair_.GetSecond();
    };
    explicit operator bool() const noexcept {
        if (pair_.GetFirst() != nullptr) {
            return true;
        }
        return false;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators
    void* operator->() const noexcept {
        return pair_.GetFirst();
    };

private:
    CompressedPair<void*, Deleter> pair_;
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) noexcept {
        pair_.GetFirst() = ptr;
    };
    UniquePtr(T* ptr, Deleter deleter) noexcept {
        pair_.GetFirst() = ptr;
        pair_.GetSecond() = std::forward<Deleter>(deleter);
    };

    UniquePtr(UniquePtr&& other) noexcept {
        this->Swap(other);
        other.Reset();
    };

    template <class U, class B>
    UniquePtr(UniquePtr<U, B>&& other) noexcept {
        pair_.GetFirst() = other.Release();
        other.Reset();
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        this->Swap(other);
        other.Reset();
        return *this;
    };
    template <class U, class B>
    UniquePtr& operator=(UniquePtr<U, B>&& other) noexcept {
        pair_.GetSecond()(pair_.GetFirst());
        pair_.GetFirst() = other.Release();
        other.Reset();
        return *this;
    };
    UniquePtr& operator=(std::nullptr_t) noexcept {
        if (pair_.GetFirst() != nullptr) {
            pair_.GetSecond()(pair_.GetFirst());
        }
        pair_.GetFirst() = nullptr;
        return *this;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        pair_.GetSecond()(pair_.GetFirst());
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() noexcept {
        T* ret = pair_.GetFirst();
        pair_.GetFirst() = nullptr;
        return ret;
    };
    void Reset(T* ptr = nullptr) noexcept {
        T* el = pair_.GetFirst();
        pair_.GetFirst() = ptr;
        pair_.GetSecond()(el);
    };
    void Swap(UniquePtr& other) noexcept {
        std::swap(pair_, other.pair_);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const noexcept {
        return pair_.GetFirst();
    };
    Deleter& GetDeleter() noexcept {
        return pair_.GetSecond();
    };
    const Deleter& GetDeleter() const noexcept {
        return pair_.GetSecond();
    };
    explicit operator bool() const noexcept {
        if (pair_.GetFirst() != nullptr) {
            return true;
        }
        return false;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    T& operator*() const noexcept {
        return *pair_.GetFirst();
    };
    T* operator->() const noexcept {
        return pair_.GetFirst();
    };
    T& operator[](size_t ind) const noexcept {
        return pair_.GetFirst()[ind];
    }

private:
    CompressedPair<T*, Deleter> pair_;
};
