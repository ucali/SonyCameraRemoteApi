#include "service.h"

using namespace Sony;

Command::Command(QString m, QVariant p, QString v, int id) : _version(v), _id(id) {
	_req["method"] = m;
	_req["params"] = QJsonValue::fromVariant(p);
	_req["version"] = _version;
	_req["id"] = _id;
}

Command::~Command() {
	if (_reply && !_reply->isFinished()) {
		_reply->abort();
	}
}

void Command::SendTo(QUrl url) {
	QJsonDocument doc(_req);
	QByteArray body = doc.toJson();
	QNetworkRequest req(url);
	req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	req.setHeader(QNetworkRequest::ContentLengthHeader, body.length());
	_reply = Network::Get()->post(req, body);
	if (_reply != nullptr) {
		QObject::connect(_reply, &QNetworkReply::finished, [this] {
			Response data;
			if (!_reply->error()) {
				QByteArray json = _reply->readAll();
				auto doc = QJsonDocument::fromJson(json);
				auto resp = doc.object();

				auto res = resp["result"];
				if (res.isNull() || res.isUndefined()) {
					res = resp["results"];
				}
				data.Result = res.toArray().toVariantList();
				data.Id = resp["id"].toInt();

				auto errJson = resp["error"];
				if (!errJson.isNull() && !errJson.isUndefined()) {
					auto err = errJson.toArray().toVariantList();
					if (err.length() == 2) {
						data.Error.code = err.at(0).toInt();
						data.Error.err = err.at(1).toString();
					}
				}

			} else {
				data.Error.code = _reply->error();
				data.Error.err = _reply->errorString();
			}
			emit onResponse(data);
			_reply->deleteLater();
		});
	}
}

Service::Service(QString k, QString u) : key(k), url(u + "/" + k) {
	connect(this, &Service::terminated, this, &Service::removeCommand, Qt::QueuedConnection);
}

Service::~Service() { 

}

Service::Ptr Service::Create(QString k, QString url) {
	return Ptr(new Service(k, url));
}

void Service::Run(Command::Ptr cmd, const Command::Callback& f) {
	auto ptr = cmd.get();
	connect(ptr, &Command::onResponse, f);

	connect(ptr, &Command::onResponse, [this, ptr] {
		for (int i = 0; i < _pending.size(); i++) {
			if (_pending.at(i).get() == ptr) {
				emit terminated(_pending.at(i));
				return;
			}
		}
	});

	cmd->SendTo(url);
	_pending.push_back(cmd);
}

void Service::removeCommand(Command::Ptr i) {
    _pending.remove(i);
}
