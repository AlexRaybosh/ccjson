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
#include <thread>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <exception>
#include <list>
#include <memory>
#include <unordered_set>
#include <set>
#include <chrono>

namespace utils {

template<typename input>
struct copyable_function {
	typedef typename std::decay<input>::type stored_input;
	template<typename ... Args>
	auto operator()(Args &&... args) -> decltype( std::declval<input&>()(std::forward<Args>(args)...)) {
		return (*ptr)(std::forward<Args>(args)...);
	}
	copyable_function(input &&i) : ptr(std::make_shared<stored_input>(std::forward<input>(i))) {}
	copyable_function(copyable_function const&) = default;
private:
	std::shared_ptr<stored_input> ptr;
};
template<typename input>
copyable_function<input> make_copyable_function(input &&i) {return {std::forward<input>(i)};}


class ThreadExecutor {

	std::mutex mutex{};
	std::condition_variable cond{};
	std::condition_variable submitCond{};
	std::list<std::function<void()>> queue;
	std::function<void()> popLastCallable() {
		// under lock
		auto lastIt=std::prev(queue.end());
		auto last=std::move(*lastIt);
		queue.erase(lastIt);
		return last;
	}
	class Worker;
	std::set<Worker*> workers;
	std::list<Worker*> deadWorkers;
	uint64_t timeoutMs{100};
	uint64_t submitTimeoutMs{100};
	size_t coreSize{2};
	size_t maxWorkers{2};
	size_t inFlight{0};
	size_t maxQueueSize{10};

	// allow worker to die
	bool timeoutCheckout(Worker* w) {
		// under lock
		if (workers.size()<=coreSize) return false;
		workers.erase(w);
		deadWorkers.push_back(w);
		return true;
	}
	struct Worker {
		ThreadExecutor* threadExecutor;
		bool running{true};
		std::thread thread;
		Worker(ThreadExecutor* tp) : threadExecutor(tp), thread(&Worker::loop, this) {}
		~Worker() {
			thread.join();
		}
		void loop() {
			std::unique_lock<std::mutex> lock(threadExecutor->mutex);
			for (;;) {
				while (running && threadExecutor->queue.size()==0) {
					//auto time=std::chrono::system_clock::now()+std::chrono::milliseconds(threadExecutor->timeoutMs);
					if (threadExecutor->cond.wait_for(lock, std::chrono::milliseconds(threadExecutor->timeoutMs))==std::cv_status::timeout) {
						if (threadExecutor->timeoutCheckout(this)) return;
					}
				}
				if (threadExecutor->queue.size() > 0) {
					bool submitBlocked=threadExecutor->queue.size() >= threadExecutor->maxQueueSize;
					auto callable=threadExecutor->popLastCallable();
					++threadExecutor->inFlight;
					lock.unlock();
					if (submitBlocked) threadExecutor->submitCond.notify_all();
					callable();
					lock.lock();
					--threadExecutor->inFlight;
				} else if (!running) return;
			}
		}
	};



	/*template <typename R, typename... ARGS> std::function<void()> convert(std::promise<R>&& promise, std::function<R (ARGS ...)>&& f, ARGS&& ... args) {
		auto tt=std::forward_as_tuple(args...);
		std::cerr<<" 1st: " << std::get<0>(tt)<<std::endl;
		auto func = [p=std::move(promise), myf=std::move(f),t=std::move(tt)]() mutable {
			try {
				p.set_value(std::apply(myf, t));
			} catch (...) {
				p.set_exception(std::current_exception());
			}
		};
		return make_copyable_function(std::move(func));
	}
*/
public:

	/*
	template <typename R, typename... ARGS> std::future<R> callWithArgs(std::function<R (ARGS&& ...)> f, ARGS&& ... args) {
		std::shared_ptr<std::promise<R>>  promise=std::make_shared<std::promise<R>>();
		std::future<R> res=promise->get_future();
		auto tt=std::forward_as_tuple(args...);
		std::shared_ptr<decltype(tt)> stt=std::make_shared<decltype(tt)>(std::move(tt));
		//std::function<void()> callable=convert(std::move(promise), std::move(f), std::move(args)... );
		auto callable = [promise, myf=std::move(f),stt]()  mutable {
					try {
						//promise->set_value(std::apply(myf, *stt));
					} catch (...) {
						promise->set_exception(std::current_exception());
					}
				};
		execute(make_copyable_function(std::move(callable)));
		return res;
	}
*/

	/*template <typename F> void call(F&& f) {
		std::shared_ptr<std::promise<R>>  promise=std::make_shared<std::promise<R>>();
		std::future<R> res=promise->get_future();
		auto tt=std::forward_as_tuple(args...);
		std::shared_ptr<decltype(tt)> stt=std::make_shared<decltype(tt)>(std::move(tt));
		//std::function<void()> callable=convert(std::move(promise), std::move(f), std::move(args)... );
		auto callable = [promise, myf=std::move(f), stt]()  {
					try {
						//std::cerr<<"in convert: ";
						//((std::cerr << "|" << args), ...);
						//std::cerr<<std::endl;
						//p.set_value(myf(args...));
						//promise->set_value(std::apply(myf, *stt));
					} catch (...) {
						promise->set_exception(std::current_exception());
					}
				};
		execute(std::move(callable));
		return res;
	}
*/

	void execute(const std::function<void()>& f) {
		execute(std::function<void()>(f));
	}
	void execute(std::function<void()>&& f) {
		{
			std::unique_lock<std::mutex> lock(mutex);
			while (queue.size()>=maxQueueSize) {
				if (submitCond.wait_for(lock, std::chrono::milliseconds(submitTimeoutMs))==std::cv_status::timeout) {
					throw std::runtime_error("timeout");
				}
			}
			queue.emplace_front(std::move(f));
			if (workers.size()< maxWorkers && workers.size()-inFlight < queue.size()) {
				workers.insert(new Worker(this));
			}
		}
		cond.notify_one();
	}

	template <typename R> struct C {
		std::promise<R> promise;
		std::function<R()> func;
		C(std::promise<R>&& p, std::function<R()>&& f) :  promise(std::move(p)), func(std::move(f)) {}
		C(C&& cc) : promise(std::move(cc.promise)), func(std::move(cc.func)) {}
		C(const C& cc) {throw std::runtime_error("callable wrapper copy shell not be made");}
		void operator()() {
			try {
				promise.set_value(func());
			} catch (...) {
				promise.set_exception(std::current_exception());
			}
		}
	};

	template <typename R>
	std::future<R> call(std::function<R()>&& f) {
		std::promise<R> promise{};
		std::future<R> res=promise.get_future();
		C<R> c(std::move(promise), std::move(f));
		execute(std::move(c));
		return res;
	}
	template <typename R>
	std::future<R> call(const std::function<R()>& f) {
		return call(std::function<R()>(f));
	}

	template <typename R, typename ...Args> struct CA {
		std::tuple<Args...> tuple;
		std::promise<R> promise;
		std::function<R(const Args&...)> func;
		CA(std::tuple<Args...>&& t, std::promise<R>&& p, std::function<R(const Args&...)>&& f) : tuple(std::move(t)), promise(std::move(p)), func(std::move(f)) {}
		CA(CA&& cc) : tuple(std::move(cc.tuple)),promise(std::move(cc.promise)), func(std::move(cc.func)) {}
		CA(const CA& cc) {throw std::runtime_error("callable wrapper copy shell not be made");}
		void operator()() {
			try {
				promise.set_value(std::apply(func, tuple));
			} catch (...) {
				promise.set_exception(std::current_exception());
			}
		}
	};


	template <typename R, typename ... Args>
	std::future<R> call(std::function<R(const Args&...)>&& f,Args&& ... args) {
		std::promise<R> promise{};
		std::future<R> res=promise.get_future();
		auto tt=std::make_tuple(std::move(args)...);
		CA<R,Args...> ca(std::move(tt),std::move(promise),std::move(f));
		execute(std::move(ca));
		return res;
	}
	template <typename R, typename ... Args>
	std::future<R> call(const std::function<R(const Args&...)>& f,Args&& ... args) {
		std::function<R(const Args&...)> fclone=f;
		return call(std::move(fclone), std::move(args)...);
	}
	template <typename R, typename ... Args>
	std::future<R> call(const std::function<R(const Args&...)>& f,const Args& ... args) {
		std::promise<R> promise{};
		std::future<R> res=promise.get_future();
		auto tt=std::make_tuple(args...);
		std::function<R(const Args&...)> fclone=f;
		CA<R,Args...> ca(std::move(tt),std::move(promise),std::move(fclone));
		execute(std::move(ca));
		return res;
	}

	~ThreadExecutor() {
		std::set<Worker*> alive;
		std::list<Worker*> dead;
		{
			std::unique_lock<std::mutex> lock(mutex);
			for (auto w : workers) {
				w->running=false;
			}
			std::swap(alive,workers);
			std::swap(dead,deadWorkers);
		}
		cond.notify_all();
		for (auto w : dead) delete w;
		dead.clear();
		for (auto w : alive) delete w;
		alive.clear();
	}
};




}

int main() {
	utils::ThreadExecutor te;


	std::string x("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz");
	std::cerr<<"x "<<x<<" "<<((void*)x.c_str())<<std::endl;

	std::function<int(const std::string&)> zzz([](const std::string& a) -> int {
						std::cerr<<"xa "<<a<<" "<<((void*)a.c_str())<<std::endl;
						return 55;
		});

	auto a=te.call<int, std::string>(
			zzz,
			std::move(x));
	std::cerr<<a.get()<<std::endl;
	std::string s1("GRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRraaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaablahzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzdfdddddddddddddddddddddddddddddddddddddddddddddd");
	std::cerr<<"s1 "<<s1<<" "<<((void*)s1.c_str())<<std::endl;

	std::string s=std::move(s1);
	std::cerr<<"s "<<s<<" "<<((void*)s.c_str())<<std::endl;
	std::cerr<<"s1 "<<s1<<" "<<((void*)s1.c_str())<<std::endl;

	/*auto l= [mys=std::move(s), s1]() ->int  {
		::sleep(1);
		std::cerr<<"arg ["<<mys<<"] "<<((void*)mys.c_str())<<std::endl;
		::sleep(1);
		return 42;
	};
	*/
	//std::cerr<<"size "<<( sizeof(l)  )<<std::endl;
	//l();
	auto l=[mys=std::move(s), s1]() ->int  {
		::sleep(1);
		std::cerr<<"arg ["<<mys<<"] "<<((void*)mys.c_str())<<std::endl;
		::sleep(1);
		return 42;
	};
	auto r=te.call<int>(std::move(l));
	//auto r1=te.call<int>(std::move(l));
	auto n=r.get();
	std::cerr<<"result "<<n<<std::endl;
	return 0;
}
/*
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
*/

