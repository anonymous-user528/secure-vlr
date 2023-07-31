#pragma once

#include <functional>
#include <future>
#include <thread>
#include <vector>

// behaves just like std::future< std::vector<T> >
template <typename T>
class FutureVector
{
  protected:
    std::vector<std::future<T>> _futures;

  public:
    FutureVector() = default;
    ~FutureVector() = default;
    FutureVector(FutureVector &&) = default;
    FutureVector(const FutureVector &) = delete;
    FutureVector &operator=(FutureVector &&) = default;
    FutureVector &operator=(const FutureVector &) = delete;

    void wait() const;

    template <typename Rep, typename Period>
    std::future_status wait_for(std::chrono::duration<Rep, Period> const &dur) const;

    template <typename Clock, typename Duration>
    std::future_status wait_until(std::chrono::time_point<Clock, Duration> const &tp) const;

    std::conditional_t<std::is_void_v<T>,
        void, std::vector<T>
    > get();

    void emplace_back(std::future<T> &&f);
};


template <typename T>
class PromiseVector
{

  protected:
    std::vector<std::promise<T>> _promises;

  public:
    using size_type = std::size_t;

    PromiseVector() = default;
    ~PromiseVector() = default;
    PromiseVector(PromiseVector &&) = default;
    PromiseVector(const PromiseVector &) = delete;
    PromiseVector &operator=(PromiseVector &&) = default;
    PromiseVector &operator=(const PromiseVector &) = delete;

    PromiseVector(size_type n) : _promises(n) {}

    std::promise<T>& at(size_type pos);

    FutureVector<T> get_future();
};

// wait on future
// until ready and return the value
// or until timeout and throw std::runtime_error(error_message)
template <typename FutureType, typename Rep, typename Period>
auto get_or_throw(
    FutureType &future,
    std::chrono::duration<Rep, Period> timeout,
    std::string const &error_message
);

/************************ future vector ************************/

template <typename T>
void FutureVector<T>::emplace_back(std::future<T> &&f)
{
    _futures.emplace_back(std::move(f));
}

template <typename T>
void FutureVector<T>::wait() const
{
    for (auto &f : _futures)
        f.wait();
}

template <typename T>
template <typename Rep, typename Period>
std::future_status FutureVector<T>::wait_for(std::chrono::duration<Rep, Period> const &dur) const
{
    return this->wait_until(
        std::chrono::steady_clock::now() + dur);
}

template <typename T>
template <typename Clock, typename Duration>
std::future_status FutureVector<T>::wait_until(std::chrono::time_point<Clock, Duration> const &tp) const
{
    std::future_status ret;

    for (auto &f : _futures) {

        auto status = f.wait_until(tp);

        switch (status) {
            case std::future_status::ready:
                continue;
                break;
            case std::future_status::deferred:
            case std::future_status::timeout:
                return status;
                break;
        }
    }
    return std::future_status::ready;
}

template <typename T>
std::conditional_t<std::is_void_v<T>,
    void,
    std::vector<T>
> FutureVector<T>::get() {
    if constexpr (std::is_void_v<T>) {
        for (auto &f : _futures)
            f.get();
    } else {
        std::vector<T> res;
        for (auto &f : _futures) {
            res.emplace_back(f.get());
        }
        return res;
    }
}

/************************ promise vector ************************/

template <typename T>
FutureVector<T> PromiseVector<T>::get_future()
{
    FutureVector<T> future_vec;
    for (auto &p : _promises) {
        future_vec.emplace_back(p.get_future());
    }
    return future_vec;
}

template <typename T>
std::promise<T> & PromiseVector<T>::at(size_type pos) {
    return this->_promises.at(pos);
}

/************************ helper function ************************/

template <typename FutureType, typename Rep, typename Period>
auto get_or_throw(
    FutureType &future,
    std::chrono::duration<Rep, Period> timeout,
    std::string const &error_message)
{
    auto status = future.wait_for(timeout);
    switch (status) {
        case std::future_status::timeout:
        case std::future_status::deferred:
            throw std::runtime_error(error_message);
            break;
        case std::future_status::ready:
            break;
    }

    return future.get();
}
