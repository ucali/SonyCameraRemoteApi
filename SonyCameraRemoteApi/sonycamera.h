#ifndef SONYCAMERA_H
#define SONYCAMERA_H

#include "functions.h"

#include <QObject>
#include <QMap>
#include <QByteArray>
#include <QImage>

#include <QUdpSocket>

namespace Sony {

class Camera : public QObject {
	Q_OBJECT
public:
	typedef std::shared_ptr<Camera> Ptr;
	typedef std::function<void(bool)> DiscoverCallback;

	Camera(); 
	~Camera();

	quint16 Ping();
	void DiscoverAsync(int timeout, const DiscoverCallback&);
	void DiscoverSync(int timeout, const DiscoverCallback&);

	const QByteArray GetDiscoveryInfo(const QByteArray&) const;
	void AddServiceHandler(QString k, QString url);
	void AddImagingDeviceHandler(QString defaultStatus, QString liveUrl);

	LiveView::Ptr Live() const { return _stream; }
	Service::Ptr Get(QString k) const { return _handlers.value(k); }

	GrabberService::Ptr Grabber();
	ZoomerService::Ptr Zoomer();
	ExposureService::Ptr Exposure();
	HalfPressShutterService::Ptr Shutter();

	bool IsInitialized() const { return _handlers.size() != 0; }

signals:
	void cameraReady(bool);

public slots:
	void onCameraReady(bool);

private:
	void _askForDeviceDescription();

	DiscoverCallback _callback;

	QUdpSocket server;
	QTimer _timeout;

	LiveView::Ptr _stream;
	GrabberService::Ptr _grabber;
	ZoomerService::Ptr _zoomer;
	ExposureService::Ptr _exposure;
	HalfPressShutterService::Ptr _shutter;

	QMap<QByteArray, QByteArray> _discovery;
	QMap<QString, Service::Ptr> _handlers;
};

}

#endif // SONYCAMERA_H
