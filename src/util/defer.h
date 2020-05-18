#pragma once
//
// This header was shamelessly stolen from "brhsiao" on GitHub.
// https://gist.github.com/brhsiao/
#include <utility>

template <typename F>
struct Defer {
  Defer(F f) : f(f) {}
  ~Defer() { f(); }
  F f;
};

template <typename F>
Defer<F> make_defer(F f) {
  return Defer<F>(f);
};

#define __defer(line) _DEFER_MACRO_ ## line
#define _defer(line) __defer(line)

struct defer_dummy {};

template<typename F>
Defer<F> operator<<(defer_dummy, F&& f) {
  return make_defer<F>(std::forward<F>(f));
}

#define defer auto _defer(__LINE__) = defer_dummy() << [&]()
