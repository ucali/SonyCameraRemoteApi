#include "functions.h"

using namespace Sony;

void GrabberService::Take(const ImageCallback& c){
	Command::Ptr cmd(new Command("actTakePicture"));
	_cam->Run(cmd, [c](Response r) {
		if (r.Error.code == 0 && r.Result.length() > 0) {
			auto url = r.Result.at(0).toStringList();
			if (url.length() > 0) {
				auto u = url.at(0);
				auto decoded = QUrl::fromEncoded(u.toUtf8());
				auto r = Network::Get()->get(QNetworkRequest(decoded));
				QObject::connect(r, &QNetworkReply::finished, [r, c] {
					QImage im;
					if (r->error() == QNetworkReply::NoError) {
						auto bytes = r->readAll();
						im = QImage::fromData(bytes, "JPEG");
					}

					c(im);
					r->deleteLater();
				});
			}
		}
	});
}

void GrabberService::Take(const BytesCallback& c){
	Command::Ptr cmd(new Command("actTakePicture"));
	_cam->Run(cmd, [c](Response r) {
		if (r.Error.code == 0 && r.Result.length() > 0) {
			auto url = r.Result.at(0).toStringList();
			if (url.length() > 0) {
				auto u = url.at(0);
				auto decoded = QUrl::fromEncoded(u.toUtf8());
				auto r = Network::Get()->get(QNetworkRequest(decoded));
				QObject::connect(r, &QNetworkReply::finished, [r, c] {
					auto bytes = r->readAll();
					c(bytes);
					r->deleteLater();
				});
			}
		}
	});
}

void GrabberService::Take(QString p, const ResultCallback& c){
	Command::Ptr cmd(new Command("actTakePicture"));
	_cam->Run(cmd, [p, c](Response r) {
		if (r.Error.code == 0 && r.Result.length() > 0) {
			auto url = r.Result.at(0).toStringList();
			if (url.length() > 0) {
				auto u = url.at(0);
				auto decoded = QUrl::fromEncoded(u.toUtf8());
				auto r = Network::Get()->get(QNetworkRequest(decoded));

				std::shared_ptr<QFile> file(new QFile(p));
				file->open(QIODevice::WriteOnly);

				QObject::connect(r, &QNetworkReply::readyRead, [r, file] {
					auto bytes = r->readAll();
					file->write(bytes);
				});

				QObject::connect(r, &QNetworkReply::finished, [r, c, file] {
					auto bytes = r->readAll();
					file->write(bytes); 
					file->close();

					c(file->exists());

					r->deleteLater();
				});
			}
		}
	});
}

void GrabberService::GetStillSize(const SizeCallback& c){
	Command::Ptr cmd(new Command("getStillSize"));
	_cam->Run(cmd, [c](Response r) {
		if (r.Error.code == 0 && r.Result.length() > 0) {
			c(r.Result.at(0).toMap());
		}
	});
}


void GrabberService::GetSupportedStillSize(const SizeCallback& c) {
	Command::Ptr cmd(new Command("getSupportedStillSize"));
	_cam->Run(cmd, [c](Response r) {
		if (r.Error.code == 0 && r.Result.length() > 0) {
			c(r.Result.at(0).toMap());
		}
	});
}

void GrabberService::GetAvailableStillSize(const SizeCallback& c) {
	Command::Ptr cmd(new Command("getAvailableStillSize"));
	_cam->Run(cmd, [c](Response r) {
		if (r.Error.code == 0 && r.Result.length() > 0) {
			c(r.Result.at(0).toMap());
		}
	});
}

void GrabberService::SetStillSize(QStringList params, const ResultCallback& c) {
	Command::Ptr cmd(new Command("setStillSize", params));
	_cam->Run(cmd, [c](Response r) {
		c(r.Error.code == 0 && r.Result.length() > 0);
	});
}

void GrabberService::setPostviewImageSize(QStringList params, const ResultCallback& c) {
	Command::Ptr cmd(new Command("setPostviewImageSize", params));
	_cam->Run(cmd, [c](Response r) {
		c(r.Error.code == 0 && r.Result.length() > 0);
	});
}

void ZoomerService::ZoomIn(const ResultCallback& c) {
	Command::Ptr cmd(new Command("actZoom", QStringList() << "in" << "1shot"));
	_cam->Run(cmd, [c](Response r) {
		c(r.Error.code == 0 && r.Result.length() > 0);
	});
}

void ZoomerService::ZoomOut(const ResultCallback& c) {
	Command::Ptr cmd(new Command("actZoom", QStringList() << "out" << "1shot"));
	_cam->Run(cmd, [c](Response r) {
		c(r.Error.code == 0 && r.Result.length() > 0);
	});
}

void ZoomerService::Wide(const ResultCallback& c) {
	Command::Ptr cmd(new Command("actZoom", QStringList() << "out" << "start"));
	_cam->Run(cmd, [c](Response r) {
		c(r.Error.code == 0 && r.Result.length() > 0);
	});
}

void ZoomerService::Tele(const ResultCallback& c) {
	Command::Ptr cmd(new Command("actZoom", QStringList() << "in" << "start"));
	_cam->Run(cmd, [c](Response r) {
		c(r.Error.code == 0 && r.Result.length() > 0);
	});
}

void ExposureService::GetExposureMode(const ValueCallback& c) {
	Command::Ptr cmd(new Command("getExposureMode"));
	_cam->Run(cmd, [c](Response r) {
		if (r.Error.code == 0 && r.Result.length() > 0) {
			c(r.Result.at(0).toString());
		}
	});
}

void ExposureService::GetSupportedExposureMode(const ListCallback& c) {
	Command::Ptr cmd(new Command("getSupportedExposureMode"));
	_cam->Run(cmd, [c](Response r) {
		if (r.Error.code == 0 && r.Result.length() > 0) {
			c(r.Result.at(0).toStringList());
		}
	});
}

void ExposureService::GetAvailableExposureMode(const ListCallback& c) {
	Command::Ptr cmd(new Command("getAvailableExposureMode"));
	_cam->Run(cmd, [c](Response r) {
		if (r.Error.code == 0 && r.Result.length() > 0) {
			c(r.Result.at(0).toStringList());
		}
	});
}

void ExposureService::SetExposureMode(QString val, const ResultCallback& c) {
	Command::Ptr cmd(new Command("setExposureMode", QStringList(val)));
	_cam->Run(cmd, [c](Response r) {
		c(r.Error.code == 0 && r.Result.length() > 0);
	});
}

void HalfPressShutterService::Press() {
	Command::Ptr cmd(new Command("actHalfPressShutter"));
	_cam->Run(cmd, [](Response r) {});
}

void HalfPressShutterService::Release() {
	Command::Ptr cmd(new Command("cancelHalfPressShutter"));
	_cam->Run(cmd, [](Response r) {});
}

void LiveViewService::Start(const ValueCallback& c) {
	Command::Ptr cmd(new Command("startLiveview"));
	_cam->Run(cmd, [c](Response r) {
		if (r.Error.code == 0 && r.Result.length() > 0) {
			c(r.Result.at(0).toString());
		}
	});
}

void LiveViewService::Stop(const ResultCallback& c) {
	Command::Ptr cmd(new Command("stopLiveview"));
	_cam->Run(cmd, [c](Response r) {
		c(r.Error.code == 0 && r.Result.length() > 0);
	});
}
