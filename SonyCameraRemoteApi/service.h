#ifndef SERVICE_H
#define SERVICE_H

#include <QJsonObject>
#include <QJsonDocument>

#include "stream.h"

#include <memory>

namespace Sony {

struct Response {
	struct {
		int code;
		QString err;
	} Error;

	QVariantList Result;
	int Id;

	Response() : Id(0), Error({ 0, "" }) {}
};

class Command : public QObject {
	Q_OBJECT
public:
	typedef std::function<void(Response)> Callback;
	typedef std::shared_ptr<Command> Ptr;

	struct CommandStatus {
		QString descr;
		int code;
	};

	Command(QString, QVariant = QStringList(), QString = "1.0", int = 1);
	~Command();

	void SendTo(QUrl);

signals:
	void onResponse(Response);

private:
	QJsonObject _req;
	QString _version;
	int _id;

	QNetworkReply* _reply;
};

class Service : public QObject {
	Q_OBJECT
public:
	typedef std::shared_ptr<Service> Ptr;

	static Ptr Create(QString k, QString url);

	Service(QString k, QString u);
	~Service();

	void Run(Command::Ptr, const Command::Callback&);

	QString key;
	QUrl url;

signals:
	void terminated(Command::Ptr);

public slots:
	void removeCommand(Command::Ptr);

private:
	QList<Command::Ptr> _pending;
};

}

#endif // SERVICE_H
