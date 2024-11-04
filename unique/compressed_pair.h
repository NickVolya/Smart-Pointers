#pragma once

#pragma once

#include <type_traits>
#include <utility>

// Me think, why waste time write lot code, when few code do trick.
template <typename F, typename S, bool = std::is_empty_v<F> && !std::is_final_v<F>,
          bool = std::is_empty_v<S> && !std::is_final_v<S>, bool = std::is_same_v<F, S>>
class CompressedPair {
public:
    CompressedPair() {
        first_ = F();
        second_ = S();
    };
    CompressedPair(F& first, S&& second) : first_(first), second_(std::move(second)) {
    }
    CompressedPair(F&& first, S&& second) : first_(std::move(first)), second_(std::move(second)) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    S& GetSecond() {
        return second_;
    }

    const S& GetSecond() const {
        return second_;
    };

private:
    F first_;
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, true, true, false> : public F, public S {
    auto GetFirst() {
        return *this;
    }

    auto GetFirst() const {
        return *this;
    }

    auto GetSecond() {
        return *this;
    }

    auto GetSecond() const {
        return *this;
    };
};

template <typename F, typename S>
class CompressedPair<F, S, true, true, true> : public F {
    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    auto GetSecond() {
        return *this;
    }

    auto GetSecond() const {
        return *this;
    };

private:
    F first_;
};

template <typename F, typename S>
class CompressedPair<F, S, true, false, true> : public F {
public:
    CompressedPair() {
        second_ = S();
    };
    CompressedPair(F& first, S&& second) : second_(std::move(second)) {
    }
    CompressedPair(F&& first, S&& second) : second_(std::move(second)) {
    }

    auto GetFirst() {
        return *this;
    }

    auto GetFirst() const {
        return *this;
    }

    S& GetSecond() {
        return second_;
    }

    const S& GetSecond() const {
        return second_;
    };

private:
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, true, false, false> : public F {
public:
    CompressedPair() {
        second_ = S();
    };
    CompressedPair(F& first, S&& second) : second_(std::move(second)) {
    }
    CompressedPair(F&& first, S&& second) : second_(std::move(second)) {
    }

    auto GetFirst() {
        return *this;
    }

    auto GetFirst() const {
        return *this;
    }

    S& GetSecond() {
        return second_;
    }

    const S& GetSecond() const {
        return second_;
    };

private:
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, false, true, true> : public S {
public:
    CompressedPair() {
        first_ = F();
    };
    CompressedPair(F& first, S&& second) : first_(first) {
    }
    CompressedPair(F&& first, S&& second) : first_(std::move(first)) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    auto GetSecond() {
        return *this;
    }

    auto GetSecond() const {
        return *this;
    };

private:
    F first_;
};

template <typename F, typename S>
class CompressedPair<F, S, false, true, false> : public S {
public:
    CompressedPair() {
        first_ = F();
    };
    CompressedPair(F& first, S&& second) : first_(first) {
    }
    CompressedPair(F&& first, S&& second) : first_(std::move(first)) {
    }

    F& GetFirst() {
        return first_;
    }

    const F& GetFirst() const {
        return first_;
    }

    auto GetSecond() {
        return *this;
    }

    auto GetSecond() const {
        return *this;
    };

private:
    F first_;
};
