#pragma once
#include <unordered_map>
#include <string>
#include <iostream>
#include <concepts>

// used by:
// GL<Texture>
// GL<Buffer>
// GL<Shader>
// GL<Program>
// GL<VAO>
template<class T, class Handle_t = unsigned int>
class GL {
public:
	GL() = default;
	~GL() {
		destroyRef(m_handle);
	}
	GL(const GL<T, Handle_t>& other) {
		createRef(other.m_handle);
		m_handle = other.m_handle;
	}
	GL<T, Handle_t>& operator=(const GL<T, Handle_t>& other) {
		if (this->m_handle == other.m_handle)
			return *this;

		destroyRef(m_handle);
		createRef(other.m_handle);
		m_handle = other.m_handle;
		return *this;
	}

	GL(GL<T, Handle_t>&& other) {
		m_handle = other.m_handle;
		other.m_handle = 0;
	}
	GL<T, Handle_t>& operator=(GL<T, Handle_t>&& other) {
		if (this->m_handle == other.m_handle)
			return *this;

		destroyRef(m_handle);
		m_handle = other.m_handle;
		other.m_handle = 0;

		return *this;
	}


	Handle_t getHandle() const { return m_handle; }

protected:
	Handle_t m_handle = 0;

private:
	static void createRef(Handle_t handle) {
		if (handle == 0) return;

		auto it = m_ref_count.find(handle);
		if (it == m_ref_count.end())
			m_ref_count.emplace_hint(it, handle, 1);
		else
			it->second++;
	}

	static void destroyRef(Handle_t handle) {
		if (handle == 0) return;

		auto it = m_ref_count.find(handle);
		if (it == m_ref_count.end())
			T::destroy(handle);
		else if (--it->second == 0)
			m_ref_count.erase(it);
	}

	inline static std::unordered_map<Handle_t, unsigned int> m_ref_count;
};


// used by:
// FileManager<Texture>
template<class T>
class FileManager {
public:
	static void drop(std::string filepath)
	{
		m_file_cache.erase(filepath);
	}

	static T get(std::string filepath)
	{
		auto it = m_file_cache.find(filepath);
		if (it != m_file_cache.end())
			return T(it->second);

		std::cerr << "[Resource Error] - " << "failed to get " << typeid(T).name() << " from '" << filepath << "'" << std::endl;
		throw std::exception();
	}

	/* gets value associated with 'filepath' or creates new value from args if none is found */
	template<typename ... Ts> requires std::constructible_from<T, Ts...>
	static T get_or_default(std::string filepath, Ts ... args)
	{
		auto it = m_file_cache.find(filepath);
		if (it != m_file_cache.end())
			return T(it->second);

		T value = T(std::move(args)...);
		m_file_cache.emplace(filepath, value);
		return value;
	}

	/* gets value associated with 'filepath' or creates new value from filepath if none is found */
	static T load(std::string filepath)
	{
		return get_or_default(filepath, filepath);
	}

private:
	inline static std::unordered_map<std::string, T> m_file_cache;
};