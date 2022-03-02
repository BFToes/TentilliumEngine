#pragma once
#include <map>
#include <string>
#include <iostream>
// special pointers???
// resources never destructed until static map destroyed
// shared pointer is exactly what i want
// when destructed, invalidating pointers???
template<class t>
struct Resource
{
private:
	inline static std::map<std::string, std::weak_ptr<t>> cache;

public:
	Resource() { std::cout << "resource is being allocated" << std::endl; }
	~Resource() { std::cout << "resource is being deleted" << std::endl; }


	const static std::shared_ptr<t> Get(std::string filepath)
	{
		auto it = cache.find(filepath);
		if (it != cache.end() && !(it->second.expired()))
			return std::shared_ptr<t>(it->second);

		auto ptr = std::shared_ptr<t>(new t(filepath));
		cache.emplace(filepath, ptr);

		return ptr;
	}

	
};