#ifndef PTHREADS_PP
#define PTHREADS_PP

#include <pthread.h>

class PThread {
private:
	pthread_t handle;
	pthread_attr_t attr;

protected:
	void exit();

public:
	friend void *pthread_run(void *data);

	virtual void main() = 0;
	void run();
	void join();

	PThread(bool const joinable = false);
	virtual ~PThread();
};

class PMutex {
private:
	 pthread_mutex_t handle;

public:
	void lock();
	void unlock();
	bool trylock();

	PMutex();
	~PMutex();
};

#endif
