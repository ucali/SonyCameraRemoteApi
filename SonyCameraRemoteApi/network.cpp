#include "network.h"

using namespace Sony;

std::unique_ptr<QNetworkAccessManager> Network::_manager = nullptr;

QNetworkAccessManager* Network::Get() {
	if (_manager.get() == nullptr) {
		_manager.reset(new QNetworkAccessManager);
	}
	return _manager.get();
}


void Network::Release() {
	_manager->deleteLater();
}