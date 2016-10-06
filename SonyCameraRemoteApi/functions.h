#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "service.h"

namespace Sony {

class GrabberService {
public:
	typedef std::function<void(QImage)> ImageCallback;
	typedef std::function<void(QByteArray)> BytesCallback;
	typedef std::function<void(QVariantMap)> SizeCallback;
	typedef std::function<void(bool)> ResultCallback;


	typedef std::shared_ptr<GrabberService> Ptr;

	GrabberService(Service::Ptr c) : _cam(c) {}

	void Take(const ImageCallback& c);
	void Take(const BytesCallback& c);
	void Take(QString, const ResultCallback&);

	void GetStillSize(const SizeCallback&);
	void GetSupportedStillSize(const SizeCallback&);
	void GetAvailableStillSize(const SizeCallback&);

	void SetStillSize(QStringList, const ResultCallback&);
	void setPostviewImageSize(QStringList, const ResultCallback&);

private:
	Service::Ptr _cam;
};

class ZoomerService {
public:
	typedef std::function<void(bool)> ResultCallback;
	typedef std::shared_ptr<ZoomerService> Ptr;

	ZoomerService(Service::Ptr c) : _cam(c) {}

	void ZoomIn(const ResultCallback&);
	void ZoomOut(const ResultCallback&);

	void Wide(const ResultCallback&);
	void Tele(const ResultCallback&);

private:
	Service::Ptr _cam;
};

class ExposureService {
public:
	typedef std::function<void(bool)> ResultCallback;
	typedef std::function<void(QStringList)> ListCallback;
	typedef std::function<void(QString)> ValueCallback;
	typedef std::shared_ptr<ExposureService> Ptr;

	ExposureService(Service::Ptr c) : _cam(c) {}

	void GetExposureMode(const ValueCallback&);
	void GetSupportedExposureMode(const ListCallback&);
	void GetAvailableExposureMode(const ListCallback&);

	void SetExposureMode(QString, const ResultCallback&);

private:
	Service::Ptr _cam;
};

class HalfPressShutterService {
public:
	typedef std::function<void(bool)> ResultCallback;
	typedef std::shared_ptr<HalfPressShutterService> Ptr;

	HalfPressShutterService(Service::Ptr c) : _cam(c) {}

	void Press();
	void Release();

private:
	Service::Ptr _cam;
};

//UNTESTED
class LiveViewService {
public:
	typedef std::function<void(QString)> ValueCallback;
	typedef std::function<void(bool)> ResultCallback;
	typedef std::shared_ptr<LiveViewService> Ptr;

	LiveViewService(Service::Ptr c, LiveView::Ptr p) : _cam(c), _view(p) {}

	void Start(const ValueCallback&);
	void Stop(const ResultCallback&);

private:
	Service::Ptr _cam;
	LiveView::Ptr _view;
};

}

#endif // FUNCTIONS_H
