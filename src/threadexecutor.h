#ifndef THREADEXECUTOR_H_
#define THREADEXECUTOR_H_
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
	ThreadExecutor* threadExecutor;
	bool running{false};
	std::mutex mutex{};
	std::condition_variable cond;
	std::future<void> exit;

	void loop() {

	}

public:
	WorkThread(ThreadExecutor* te) : threadExecutor(te), exit(std::async(std::launch::async, &WorkThread::loop, this)) {}


};

class ThreadExecutor {
	std::list<std::shared_ptr<WorkThread>> freeWorkers{};

};

}




#endif /* THREADEXECUTOR_H_ */
