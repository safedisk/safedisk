#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <map>
#include <array>
#include <vector>
#include <list>
#include <memory>
#include <string>
#include <algorithm>
#include <queue>
#include <tuple>

using std::array;
using std::vector;
using std::map;
using std::multimap;
using std::queue;
using std::deque;
using std::list;
using std::max;
using std::min;
using std::shared_ptr;
using std::unique_ptr;
using std::string;
using std::swap;
using std::tuple;
using std::make_shared;

// Basic type declarations
typedef uint8_t byte;
typedef uint64_t htime_t;  // Time in microsecs from unix epoch

const htime_t t_millisecs = 1000;
const htime_t t_seconds = 1000 * t_millisecs;
const htime_t t_minutes = 60 * t_seconds;
const htime_t t_hours = 60 * t_minutes;
const htime_t t_days = 24 * t_hours;

// Walk over all entries in a collection and call functor, which may alter entry
// If functor returns false, remove entry, presumes stable iterators
template<typename Collection, class Functor>
void walk_remove(Collection& col, Functor func) {
	for (auto it = col.begin(); it != col.end(); ) {
		bool keep = func(*it);
		if (!keep) {
			auto it2 = it;
			++it;
			col.erase(it2);
		} else {
			++it;
		}
	}
}

// Std forgot this
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
