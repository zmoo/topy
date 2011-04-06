#include "client_thread.hh"

ClientThread::ClientThread(Client *_client) : client(_client) {
	client->ref();
}

ClientThread::~ClientThread() {
	client->unref();
}


