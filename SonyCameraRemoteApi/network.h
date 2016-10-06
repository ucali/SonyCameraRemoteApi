#ifndef SONY_NETWORK_H
#define SONY_NETWORK_H

#include <QtNetwork>
#include <memory>

namespace Sony {

class Network {
public:
	static QNetworkAccessManager* Get();
	static void Release();

private:
	static std::unique_ptr<QNetworkAccessManager> _manager;

};

}

#endif
