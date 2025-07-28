#ifndef _PMR_TYPES_H
#define _PMR_TYPES_H

// @see https://github.com/endurodave/pmr_allocator
// David Lafreniere

#include <map>
#include <list>
#include <queue>
#include <set>
#include <string>
#include <iostream>
#include "pmr_allocator.h"

std::pmr::string make_pmr_string(const char* s) {
	static pmr_allocator static_resource;
	return std::pmr::string(s, std::pmr::polymorphic_allocator<char>(&static_resource));
}

std::pmr::wstring make_pmr_wstring(const wchar_t* s) {
	static pmr_allocator static_resource;
	return std::pmr::wstring(s, std::pmr::polymorphic_allocator<wchar_t>(&static_resource));
}

template<typename T>
std::pmr::list<T> make_pmr_list() {
	static pmr_allocator static_resource; // one allocator per type
	return std::pmr::list<T>(std::pmr::polymorphic_allocator<T>(&static_resource));
}

template<typename K, typename V>
std::pmr::map<K, V> make_pmr_map() {
	using pair_type = std::pair<const K, V>;
	static pmr_allocator static_resource;

	return std::pmr::map<K, V>(
		std::less<K>(),
		std::pmr::polymorphic_allocator<pair_type>(&static_resource)
	);
}

template<typename T>
std::queue<T, std::pmr::deque<T>> make_pmr_queue() {
	static pmr_allocator static_resource;
	return std::queue<T, std::pmr::deque<T>>(
		std::pmr::deque<T>(std::pmr::polymorphic_allocator<T>(&static_resource))
	);
}

template<typename T>
std::pmr::set<T> make_pmr_set() {
	static pmr_allocator static_resource;
	return std::pmr::set<T>(
		std::less<T>(),
		std::pmr::polymorphic_allocator<T>(&static_resource)
	);
}

#endif