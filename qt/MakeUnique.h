#ifndef MAKEUNIQUE_H
#define MAKEUNIQUE_H

#include <memory>
#include <utility>

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

#endif // MAKEUNIQUE_H
