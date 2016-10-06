#include "sonycamera.h"

#include <QImage>

#include <QXmlStreamReader>
#include <QTimer>

using namespace Sony;

Camera::Camera() { 
	connect(this, &Camera::cameraReady, this, &Camera::onCameraReady);

	_stream = nullptr;
	_grabber = nullptr;
	_zoomer = nullptr;
	_exposure = nullptr;
	_shutter = nullptr;

	qRegisterMetaType<Command::Ptr>("Command::Ptr");
}

Camera::~Camera() {
	_stream = nullptr;
	_grabber = nullptr;
	_zoomer = nullptr;
	_exposure = nullptr;
	_shutter = nullptr;

	_handlers.clear();
}

quint16 Camera::Ping() {
	QHostAddress h("239.255.255.250");
	QUdpSocket sock;

	QByteArray datagram("M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nMAN: \"ssdp:discover\"\r\nMX: 4\r\nST: urn:schemas-sony-com:service:ScalarWebAPI:1\r\n\r\n");
	sock.writeDatagram(datagram.data(), datagram.size(), h, 1900);

	auto port = sock.localPort();
	sock.close();
	return port;
}

void Camera::DiscoverSync(int timeout, const DiscoverCallback& c) {
	_callback = c;

    auto b = server.bind(Ping());
    if (!b) {
		_callback(false);
        return;
    }

	if (!server.waitForReadyRead(timeout * 1000)) {
		_callback(false);
        return;
    }

	if (server.hasPendingDatagrams()) {
		QByteArray resp;
		resp.resize(server.pendingDatagramSize());

		server.readDatagram(resp.data(), resp.size());

		resp = resp.mid(resp.indexOf('\n') + 1).trimmed();
		for (QByteArray line : resp.split('\n')) {
			int colon = line.indexOf(':');

			QByteArray headerName = line.left(colon).trimmed();
			QByteArray headerValue = line.mid(colon + 1).trimmed();

			_discovery.insertMulti(headerName.toLower(), headerValue);
		}
		server.close();

		bool result = _discovery.size() != 0;
		if (result) {
			_askForDeviceDescription();
		}
	}
}


void Camera::DiscoverAsync(int timeout, const DiscoverCallback& c) {
	_callback = c;

	auto b = server.bind(Ping());
	if (!b) { 
		_callback(false);
		return; 
	}

	_timeout.setSingleShot(true);
	_timeout.start(timeout*1000);
	QObject::connect(&_timeout, &QTimer::timeout, this, [this] {
			server.close();
			_callback(false);
	}, Qt::UniqueConnection);

	connect(&server, &QUdpSocket::readyRead, [this] {
		_timeout.stop();
		if (server.hasPendingDatagrams()) { 
			QByteArray resp;
			resp.resize(server.pendingDatagramSize());

			server.readDatagram(resp.data(), resp.size());

			resp = resp.mid(resp.indexOf('\n') + 1).trimmed();
			for (QByteArray line : resp.split('\n')) {
				int colon = line.indexOf(':');

				QByteArray headerName = line.left(colon).trimmed();
				QByteArray headerValue = line.mid(colon + 1).trimmed();

				_discovery.insertMulti(headerName.toLower(), headerValue);
			}
			server.close();

			bool result = _discovery.size() != 0;
			if (result) {
				_askForDeviceDescription();
			}
		}
	});

}

void Camera::onCameraReady(bool b) {
	_callback(b);
}

const QByteArray Camera::GetDiscoveryInfo(const QByteArray& k) const {
    return _discovery.value(k);
}

void Camera::_askForDeviceDescription() {
	_handlers.clear();
    auto reply = Network::Get()->get(QNetworkRequest(QUrl(GetDiscoveryInfo("location"))));
    if (reply != nullptr) {
        QObject::connect(reply, &QNetworkReply::finished, [this, reply] {
			bool success = reply->error() == QNetworkReply::NoError;
			if (success) {
				QXmlStreamReader reader(reply);
				while (!reader.atEnd()) {
					reader.readNext();
					QString name = reader.name().toString();
					if (reader.isStartElement()) {
						if (name == "X_ScalarWebAPI_Service") {

							reader.readNext();
							QString type = reader.readElementText();
							reader.readNext();
							QString url = reader.readElementText();

							AddServiceHandler(type, url);

						}
						else if (name == "X_ScalarWebAPI_ImagingDevice") {

							reader.readNext();
							QString live = reader.readElementText();
							reader.readNext();
							QString defaultVal = reader.readElementText();

							AddImagingDeviceHandler(defaultVal, live);

						}
					}
				}
				if (reader.hasError()) { /*TODO: Handle errors in xml from camera*/ }
			}

			emit cameraReady(success);
			reply->deleteLater();
        });

    }
}

void Camera::AddServiceHandler(QString k, QString url) {
	_handlers.insert(k, Service::Create(k, url));
}

void Camera::AddImagingDeviceHandler(QString defaultStatus, QString liveUrl) {
	_stream.reset(new LiveView(liveUrl));
}

GrabberService::Ptr Camera::Grabber() {
	if (!_grabber) {
		_grabber.reset(new GrabberService(Get("camera")));
	}
	return _grabber;
}

ZoomerService::Ptr Camera::Zoomer() {
	if (!_zoomer) {
		_zoomer.reset(new ZoomerService(Get("camera")));
	}
	return _zoomer;
}

ExposureService::Ptr Camera::Exposure() {
	if (!_exposure) {
		_exposure.reset(new ExposureService(Get("camera")));
	}
	return _exposure;
}

HalfPressShutterService::Ptr Camera::Shutter() {
	if (!_shutter) {
		_shutter.reset(new HalfPressShutterService(Get("camera")));
	}
	return _shutter;
}