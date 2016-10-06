#include "stream.h"

#include<QImage>

#include <iostream>

using namespace Sony;

LiveView::LiveView(QString u) : _target(QUrl::fromEncoded(u.toUtf8())), _stream(nullptr) {

}

LiveView::~LiveView() {

}

void LiveView::Start(const std::function<void (QImage)>& f) {
	_stream.reset(new Stream(_target));

	connect(_stream.get(), &Stream::refresh, f);
}

void LiveView::Stop() {
	_stream = nullptr;
}

Stream::Stream(QUrl u) : _handle(nullptr), _buf(new Packet) {
	connect(_buf.get(), &Packet::create, this, &Stream::refresh);

	_handle = Network::Get()->get(QNetworkRequest(u));
	if (_handle != nullptr) {
		QObject::connect(_handle, &QNetworkReply::readyRead, this, &Stream::onDataReceived);
		QObject::connect(_handle, &QNetworkReply::finished, [this] {
			_handle->deleteLater();
			_handle = nullptr;
		});
	}
}

Stream::~Stream() {
	if (_handle != nullptr) {
		_handle->abort();
	}
}

void Stream::onDataReceived() {
	try {
		QByteArray data;
		switch (_buf->GetState()) {
		case Packet::COMMON_HEADER:
			data = _handle->read(8);
			break;
		case Packet::PAYLOAD_HEADER:
			data = _handle->read(128);
			break;
		case Packet::PAYLOAD:
			data = _handle->read(_buf->BytesToRead());
			break;
		}
		_buf->Push(data);
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		_handle->abort();
	}
}

QByteArray Packet::HeaderDelim() {
	static QByteArray h;
	if (h.length() == 0) {
		h.push_back(0x24);
		h.push_back(0x35);
		h.push_back(0x68);
		h.push_back(0x79);
	}
	return h;
}

Packet::Packet() : _state(COMMON_HEADER) {
	_commonHeader.reserve(8);
	_payloadHeader.reserve(128);
	_payload.reserve(1024);
}

void Packet::Push(QByteArray b) {
	switch (_state) {
	case COMMON_HEADER:
		HandleCommonHeader(b);
		SetState(PAYLOAD_HEADER);
		break;
	case PAYLOAD_HEADER:
		HandlePayloadHeader(b);
		SetState(PAYLOAD);
		break;
	case PAYLOAD:
		if (HandlePayload(b)) {
			SetState(COMMON_HEADER);
		}
		break;
	}
}

size_t Packet::BytesToRead() const {
	size_t s = _payloadSize - _payload.length();
	return s;
}

void Packet::HandleCommonHeader(QByteArray buf) {
	_commonHeader.clear();
	_commonHeader.append(buf);

	_payloadHeader.clear();
	_payload.clear();
}

void Packet::HandlePayloadHeader(QByteArray buf) {
	_payloadHeader.append(buf);

	if (_payloadHeader.mid(0, 4) != HeaderDelim()) {
		throw std::runtime_error("Corrupted stream..");
	}

	QByteArray size = _payloadHeader.mid(4, 3);
	QByteArray pad = _payloadHeader.mid(7, 1);

	int a = ((unsigned char)_payloadHeader.at(4)) << 16;
	int b = ((unsigned char)_payloadHeader.at(5)) << 8;
	int c = ((unsigned char)_payloadHeader.at(6));
	_payloadSize = a + b + c;
	_payload.reserve(_payloadSize);
}

bool Packet::HandlePayload(QByteArray b) {
	_payload.append(b);
	if (_payload.length() < _payloadSize) {
		return false;
	}
	
	auto img = QImage::fromData(_payload);
	if (!img.isNull()) {
		emit create(img);
	}
	return true;
}