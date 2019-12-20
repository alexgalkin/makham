/**
    Copyright 2019 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#pragma once


#include <f5/makham/executor.hpp>

#include <optional>
#include <variant>


namespace f5 {
    namespace detail {
        template<typename... Fs>
        struct visitor_overload : std::remove_reference_t<Fs>... {
            visitor_overload(Fs &&... fs)
            : std::remove_reference_t<Fs>{std::forward<Fs>(fs)}... {}
            using std::remove_reference_t<Fs>::operator()...;
        };
    }
    template<typename V, typename... T>
    decltype(auto) apply_visitor(V &&v, T &&... t) {
        return std::visit(
                detail::visitor_overload<T...>(std::forward<T>(t)...),
                std::forward<V>(v));
    }
}


namespace f5::makham {


    /// Forward declarations
    template<typename R>
    struct promise_type;
    template<typename R, typename P = promise_type<R>>
    class async;


    /// ## Async
    /**
     * A async whose completion can be awaited..
     */
    template<typename R, typename P>
    class async final {
      public:
        using promise_type = P;
        friend promise_type;

        ~async() {
            if (coro) coro.destroy();
        }

        /// Not copyable
        async(async const &) = delete;
        async &operator=(async const &) = delete;
        /// Movable
        async(async &&t) noexcept : coro(t.coro) { t.coro = {}; }
        async &operator=(async &&t) noexcept { swap(coro, t.coro); }

        /// ### Awaitable
        bool await_ready() const { return false; }
        void await_suspend(std::experimental::coroutine_handle<> awaiting) {
            coro.promise().signal(awaiting);
        }
        R await_resume() { return coro.promise().get_value(); }

      private:
        using handle_type = std::experimental::coroutine_handle<promise_type>;
        handle_type coro;

        async(handle_type c) : coro{c} { makham::post(coro); }
    };


    /// An asynchronous promise
    struct async_promise {
        std::atomic<bool> has_value = false;
        std::atomic<std::experimental::coroutine_handle<>> continuation = {};

        void continuation_if_not_run() {
            if (auto h = continuation.exchange({}); h) { h.resume(); }
        }
        void signal(std::experimental::coroutine_handle<> s) {
            if (has_value.exchange(false)) {
                s.resume();
            } else {
                auto const old = continuation.exchange(s);
                if (old) {
                    throw std::invalid_argument{
                            "A async can only have one awaitable"};
                }
                if (has_value.exchange(false)) { continuation_if_not_run(); }
            }
        }
        void value_has_been_set() {
            if (has_value.exchange(true)) {
                throw std::runtime_error{"Coroutine already had a value set"};
            }
            continuation_if_not_run();
        }
        auto initial_suspend() { return std::experimental::suspend_always{}; }
        auto final_suspend() { return std::experimental::suspend_always{}; }
    };


    /// ## The async promise type
    template<typename R>
    struct promise_type final : public async_promise {
        using async_type = async<R, promise_type<R>>;
        using handle_type =
                std::experimental::coroutine_handle<promise_type<R>>;

        std::variant<std::monostate, std::exception_ptr, R> value = {};

        auto get_return_object() {
            return async_type{handle_type::from_promise(*this)};
        }
        auto return_value(R v) {
            value = std::move(v);
            value_has_been_set();
            return std::experimental::suspend_never{};
        }
        void unhandled_exception() {
            value = std::current_exception();
            value_has_been_set();
        }

        R get_value() {
            return apply_visitor(
                    std::move(value),
                    [](std::monostate) -> R {
                        throw std::runtime_error(
                                "The coroutine doesn't have a value");
                    },
                    [](std::exception_ptr e) -> R { std::rethrow_exception(e); },
                    [](R v) { return v; });
        }
    };

    template<>
    struct promise_type<void> final : public async_promise {
        using async_type = async<void, promise_type<void>>;
        using handle_type =
                std::experimental::coroutine_handle<promise_type<void>>;

        std::exception_ptr value;

        auto get_return_object() {
            return async_type{handle_type::from_promise(*this)};
        }
        auto return_void() {
            value_has_been_set();
            return std::experimental::suspend_never{};
        }
        void unhandled_exception() {
            value = std::current_exception();
            value_has_been_set();
        }

        void get_value() {
            if (value) std::rethrow_exception(value);
        }
    };


}
