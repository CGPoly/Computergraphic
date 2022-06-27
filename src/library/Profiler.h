#pragma once

#include <chrono>
#include <map>

template<class T>
class Profiler {
public:
	void begin(T type);
	void end(T type);
	void commit(T type);

	[[nodiscard]] std::chrono::milliseconds getTime(T type);

private:
	struct Entry {
		std::chrono::time_point<std::chrono::system_clock> startTime{};
		std::chrono::milliseconds accumulator{};
		std::chrono::milliseconds time{};
	};

	std::map<T, Entry> entries{};
};

template<class T>
void Profiler<T>::begin(T type) {
	entries[type].startTime = std::chrono::system_clock::now();
}

template<class T>
void Profiler<T>::end(T type) {
	auto& entry = entries[type];
	auto now = std::chrono::system_clock::now();
	entry.accumulator += std::chrono::duration_cast<std::chrono::milliseconds>(now - entry.startTime);
}

template<class T>
void Profiler<T>::commit(T type) {
	auto& entry = entries[type];
	entry.time = entry.accumulator;
	entry.accumulator = std::chrono::milliseconds(0);
}

template<class T>
std::chrono::milliseconds Profiler<T>::getTime(T type) {
	return entries[type].time;
}