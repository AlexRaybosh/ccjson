//#include <event2/>
#include <iostream>
#include <future>
#include <thread>
#include <unistd.h>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <event2/event.h>
#include <event2/thread.h>
#include <unistd.h>
#include <fcntl.h>
#include <utils.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int setupFd(int fd, bool nonblocking=true) {

	int flags=::fcntl(fd,F_GETFL, fd);
	if (flags==-1) utils::errno_exception("failed to F_GETFL fcntl for "+std::to_string(fd));
	int newFlags=flags | O_CLOEXEC;
	if (nonblocking) newFlags|=O_NONBLOCK;
	else newFlags&=~O_NONBLOCK;
	if (flags != newFlags)
		if (::fcntl(fd,F_SETFL, newFlags)==-1) {
			utils::errno_exception("failed to F_SETFL fcntl for "+std::to_string(fd)+" with flags "+std::to_string(newFlags));
		}
	return fd;
}

struct EventBase {
	struct Init {
		Init() {
			evthread_use_pthreads();
		}
	};
	Init init{};
	struct Debug {
		Debug() {
			auto version=event_get_version();
			std::cerr<<"libevent verion: "<<version<<std::endl;
			event_enable_debug_mode();
			event_enable_debug_logging(EVENT_DBG_ALL);
			evthread_enable_lock_debugging();
		}
	};
	Debug debug{};

	struct event_base* base{NULL};
	std::future<void> end{};
	std::promise<void> propmiseToStart{};
	std::mutex mutex{};
	std::condition_variable cond{};
	bool running{false};
	int controlReadFD{-1};
	int controlWriteFD{-1};
	std::string controlError{};
	struct event* controlReadEvent{NULL};

public:
	void controlReadCallback(bool timeout, bool readable) {
		char cmd;
		for (;;) {
			ssize_t rc=read(controlReadFD, &cmd, 1);
			if (rc==0) break; // closed
			if (rc==-1) {
				if (errno!=EAGAIN) {
					std::lock_guard lock(mutex);
					controlError=strerror_l(errno, uselocale((locale_t)0));
				}
				break;
			}
		}
		event_base_loopexit(base, NULL); // allow to wakeup  and understand, whats going on
		//event_base_loopbreak(base);
	}
	static void controlReadCallbackDispatch(evutil_socket_t fd, short what, void *arg)	{
		EventBase* eventBase = (EventBase*)arg;
		eventBase->controlReadCallback(what&EV_TIMEOUT, what&EV_READ);
	}

	void cleanup() {
		if (controlReadFD>=0) ::close(controlReadFD);
		if (controlWriteFD>=0) ::close(controlWriteFD);
		if (controlReadEvent) event_free(controlReadEvent);
		controlReadFD=-1;
		controlWriteFD=-1;
		controlReadEvent=NULL;
	}
	EventBase() : base(event_base_new()) {
		if (!base) throw std::runtime_error("libevent is not working");
		try {
			int pipefds[2];
			if (pipe(pipefds))
				utils::errno_exception("Failed to setup pipes");
			controlReadFD=setupFd(pipefds[0]);
			controlWriteFD=setupFd(pipefds[0], false);
			controlReadEvent=event_new(base, controlReadFD, EV_TIMEOUT|EV_CLOSED|EV_READ, controlReadCallbackDispatch,this);
			if (-1==event_add(controlReadEvent, NULL)) {
				throw std::runtime_error("Failed to event_add controlReadEvent, fd: "+std::to_string(controlReadFD));
			}

		} catch (const std::exception&  e) {
			cleanup();
			throw std::runtime_error(e.what());
		}
		auto startFuture=propmiseToStart.get_future();
		end=std::async(std::launch::async, &EventBase::run,this);
		startFuture.get();
	}
	void setup() {
		// we are under lock
	}
	void run() {
		std::unique_lock<std::mutex> lock(mutex);
		running=true;
		propmiseToStart.set_value(); // allow constructor to finish
		std::cerr<<"entering the loop"<<std::endl;
		while (running) {
			// check what needs to be done
			setup();
			lock.unlock();
			int ret=event_base_loop(base, EVLOOP_NO_EXIT_ON_EMPTY);//event_base_dispatch(base);
			std::cerr<<"event_base_dispatch: "<<ret<<std::endl;
			lock.lock();
			//if (running) cond.wait_for(lock, std::chrono::milliseconds(1000));
			//std::cerr<<"complete iteration "<<std::endl;
		}
		std::cerr<<"Exiting loop"<<std::endl;
	}
	void stop() {
		if (running) {
			std::lock_guard lock(mutex);
			running=false;
			if (controlWriteFD>=0) {
				::write(controlWriteFD, "S", 1);
				std::cerr<<"wrote stop"<<std::endl;
			}
		}
	}

	~EventBase() {
		std::cerr<<"~EventBase"<<std::endl;
		stop();
		//cond.notify_one();
		try {end.get();} catch(...) {} // wait for exit loop
		std::cerr<<"stopped"<<std::endl;
		cleanup();
		if (base) event_base_free(base);
		//libevent_global_shutdown();
	}

};



#include <errno.h>
void cb_func(evutil_socket_t fd, short what, void *arg)
{
        const char *data = (const char *)arg;
        std::cerr<<"GOT EVENT "<<std::hex << std::uppercase << what << std::nouppercase << std::dec<<std::endl;

        printf("Got an event on socket %d:%s|%s|%s|%s [%s]\n",
            (int) fd,
            (what&EV_TIMEOUT) ? " timeout" : "",
            (what&EV_READ)    ? " read" : "",
            (what&EV_WRITE)   ? " write" : "",
            (what&EV_SIGNAL)  ? " signal" : "",
            data);
        if (what&EV_READ) {
			char b[1024];
			int rr=read(fd, b, 1024 );
			std::cerr<<"rr "<<rr<<std::endl;
			if (rr==-1) {
				try {
					std::cerr<<"errno "<<errno<<std::endl;
					utils::errno_exception("failed read");
				} catch(std::exception& ee) {
					std::cerr<<"ee "<<ee.what()<<std::endl;
				}
			}
        }
        if (what&EV_WRITE) {
			char b[1024];
			int rr=write(fd, b, 1024 );
			std::cerr<<"ww "<<rr<<std::endl;
			if (rr==-1) {
				try {
					std::cerr<<"errno "<<errno<<std::endl;
					utils::errno_exception("failed write");
				} catch(std::exception& ee) {
					std::cerr<<"wee "<<ee.what()<<std::endl;
				}
			}
        }
}

void cb(evutil_socket_t fd, short what, void *ptr)
{
    /* We pass 'NULL' as the callback pointer for the heap allocated
     * event, and we pass the event itself as the callback pointer
     * for the stack-allocated event. */
    struct event *ev = (struct event *)ptr;
    std::cerr<<"GOT EVENT "<<std::hex << std::uppercase << what << std::nouppercase << std::dec<<std::endl;
}

int cc(const std::string n) {
	struct hostent * h=gethostbyname(n.c_str());
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sin;

    sin.sin_family = AF_INET;
    sin.sin_port = htons(3306);
    sin.sin_addr = *(struct in_addr*)h->h_addr;


	int flags=::fcntl(fd,F_GETFL, fd);
	if (flags==-1) utils::errno_exception("zzz");
	if (::fcntl(fd,F_SETFL, flags | O_NONBLOCK)==-1) utils::errno_exception("zzz1");

    if (-1==connect(fd, (struct sockaddr*) &sin, sizeof(sin))) {
    	if (errno != EINPROGRESS)
    		utils::errno_exception("connect");
    }
    struct event_base *base = event_base_new();
    struct event* ev1=event_new(base, fd, EV_TIMEOUT|EV_READ, cb_func,
               (char*)"my read");
    struct event* ev2=event_new(base, fd, EV_TIMEOUT|EV_WRITE, cb_func,
               (char*)"my write");
    struct event* ev3=event_new(base, fd, EV_TIMEOUT|EV_CLOSED, cb_func,
               (char*)"my close");
    struct timeval five_seconds = {500,0};

    event_add(ev1, &five_seconds);
    event_add(ev2, &five_seconds);
    event_add(ev3, &five_seconds);
    event_base_dispatch(base);

	return fd;
}

int main() {
	try {
		//EventBase eventBase;

	    int fd=cc("10.20.30.40");
	    std::this_thread::sleep_for(std::chrono::milliseconds(600000));
	    if (1==1) return 1;
	    EventBase eventBase;
		struct event* e = event_new(eventBase.base, fd, EV_READ | EV_WRITE, cb, NULL);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		event_add(e, NULL);
		std::this_thread::sleep_for(std::chrono::milliseconds(6000));
		event_free(e);
	} catch (const std::exception& e) {
		std::cerr<<"error: "<<e.what()<<std::endl;
	}

	return 0;
}
