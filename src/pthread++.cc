
#include "pthread++.hh"

void PThread::exit() {
	delete this;
	pthread_exit(NULL);
}

void *pthread_run(void *data) {
	PThread *thread = (PThread *) data;
	thread->main();
	thread->exit();
	return NULL;
}	

void PThread::run() {
       pthread_create(&handle, &attr, pthread_run, this);
}

void PThread::join() {
	pthread_join(handle, NULL);
}

PThread::PThread(bool const joinable) {
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, (joinable) ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED);
}

PThread::~PThread() {
	pthread_attr_destroy(&attr);
}

bool PMutex::trylock() {
	return (pthread_mutex_trylock(&handle) == 0);
}

void PMutex::lock() {
	pthread_mutex_lock(&handle);
}

void PMutex::unlock() {
	pthread_mutex_unlock(&handle);
}

PMutex::PMutex() {
	pthread_mutex_init(&handle, NULL);
}

PMutex::~PMutex() {
	pthread_mutex_destroy(&handle);
}

