#ifndef STREAM_H
#define STREAM_H

#include "network.h"

#include <functional>
#include <memory>

#include <QImage>

namespace Sony {

class Packet : public QObject {
	Q_OBJECT
public:
	enum State { COMMON_HEADER, PAYLOAD_HEADER, PAYLOAD };

	typedef std::shared_ptr<Packet> Ptr;
	Packet();

	void Push(QByteArray b);
	void HandleCommonHeader(QByteArray);
	void HandlePayloadHeader(QByteArray);
	bool HandlePayload(QByteArray);

	size_t BytesToRead() const;

	static QByteArray HeaderDelim();

	State GetState() const { return _state; }
	void SetState(State s) { _state = s; }

signals:
	void create(QImage);

private:
	State _state;

	QByteArray _commonHeader;
	QByteArray _payloadHeader;

	QByteArray _payload;
	size_t _payloadSize;
};

class Stream : public QObject {
	Q_OBJECT
public:
	typedef std::shared_ptr<Stream> Ptr;

	Stream(QUrl);
	~Stream();

	public slots:
	void onDataReceived();

signals:
	void refresh(QImage);

private:
	Packet::Ptr _buf;
	QNetworkReply* _handle;
};

class LiveView : public QObject {
	Q_OBJECT

public:
	typedef std::function<void(QImage)> Callback;
	typedef std::shared_ptr<LiveView> Ptr;

	LiveView(QString);
	~LiveView();

	void Start(const Callback&);
	void Stop();


private:
	Stream::Ptr _stream;

	QUrl _target;
};

}

#endif // STREAM_H
