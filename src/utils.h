#ifndef UTILS_H_
#define UTILS_H_




#include <string>
#include <vector>
#include <unistd.h>
#include <chrono>
#include <thread>


namespace utils {
	std::string errno_string();
	void errno_string(std::string& str);
	void errno_exception(const std::string& msg);

	std::string slurpTextFile(const std::string& fileName);
	std::vector<char> slurpBinFile(const std::string& fileName);

	const std::vector<std::string>& getArgs();
	int sh(const char* cmd, std::string *out, std::string* err);
	inline int sh(const char* cmd, std::string & log,std::string & err) {
		return sh(cmd, &log, &err);
	}
	void dumpToFile(const std::string& fileName, const void* buffer, size_t len);
	inline void dumpToFile(const std::string& fileName, const std::string& s) {
		dumpToFile(fileName, s.data(), s.length());
	}
	void splitDirBasename(const std::string& name, std::string& dir,std::string& base);
	bool isFileSystemObject(const std::string& name);

	bool isRegularFile(const std::string& name, ssize_t* p_size);
	inline bool isRegularFile(const std::string& name) {
		return isRegularFile(name, nullptr);
	}
	bool isDirectory(const std::string& name);
	void mkdir_p(const std::string& name);

	struct FD {
		int fd;
		inline FD() : fd(-1) {}
		inline FD(int _fd) : fd(_fd) {}
		FD(const FD & ) = delete;
		FD(FD && ) = delete;
		FD& operator =(const FD & ) =delete;
		FD& operator =(FD && ) =delete;
		inline operator bool() {return fd>=0;}
		inline operator int() {return fd;}
		inline FD& operator =(int v) {
			if (fd>=0) ::close(fd);
			fd=v;
			return *this;
		}
		inline ~FD() {if (fd>=0) ::close(fd);}
	};

	uint64_t currentTimeMilliseconds();
	uint64_t currentTimeMicroseconds();

	inline std::chrono::time_point<std::chrono::system_clock> clock() noexcept {return std::chrono::system_clock::now();}
	inline uint64_t microseconds(const std::chrono::time_point<std::chrono::system_clock> & startClock) noexcept {
		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - startClock);
		return duration.count();
	}

}

#endif /* UTILS_H_ */
