#include <iostream>
#include <string>
#include <libgen.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>
#include <chrono>
#include <thread>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <exception>
#include <list>
#include <memory>



namespace utils {
class ThreadExecutor;

class WorkThread {
	friend class ThreadExecutor;
	ThreadExecutor* threadExecutor;
	bool running{false};
	std::mutex mutex{};
	std::condition_variable cond{};
	std::future<void> exit;
	std::promise<void> promiseToStart{};
	std::function<void ()> func{};


	void loop();

public:
	WorkThread(ThreadExecutor* te) : threadExecutor(te), exit(std::async(std::launch::async, &WorkThread::loop, this)) {
		promiseToStart.get_future().get();
	}
	~WorkThread() {
		{
			std::lock_guard lock(mutex);
			running=false;
		}
		cond.notify_one();
		try {exit.get();} catch (...) {}

	}

};

class ThreadExecutor {
	friend class WorkThread;
	std::list<WorkThread*> freeWorkers{};
public:
	template <typename R, typename... ARGS> std::future<R> call(const std::function<R (const ARGS& ...)>& f, const ARGS& ... args) {
		WorkThread* workThread=freeWorkers.empty() ? new WorkThread(this) : *freeWorkers.erase(std::prev(freeWorkers.end()));
		std::promise<R> promise;
		std::future<R> res=promise.get_future();
		auto func = [p=std::move(promise), f, args ...]() mutable {
			try {
				p.set_value(f(args...));
			} catch (...) {
				p.set_exception(std::current_exception());
			}
		};
		func();
		return res;
	}
	void free(WorkThread* wt) {
		freeWorkers.push_back(wt);
	}
};

void WorkThread::loop() {
	std::unique_lock<std::mutex> lock(mutex);
	running=true;
	promiseToStart.set_value();
	for (;;) {
		while (!func && running) cond.wait(lock);
		if (!running) break;
		//lock.unlock();
		func();
		func=nullptr;
		threadExecutor->free(this);
		//lock.lock();
	}
}


}



int main() {
	utils::ThreadExecutor te;

	auto l= [](std::string arg)->int {
		std::cerr<<"got "<<arg<<std::endl;
		return 42;
	};
	std::string s("blah");
	auto r=te.call<int,std::string>( std::function< int (std::string) >(l),s);

	auto l1= []()->int {
		std::cerr<<"got void"<<std::endl;
		return 55;
	};

	auto r1=te.call<int>(
			std::function<int()>([]()->int {
		std::cerr<<"got void"<<std::endl;
		return 55;
		})
		);

	std::cerr<<"called"<<std::endl;
	std::cerr<<r.get()<<std::endl;
	return 0;
}


