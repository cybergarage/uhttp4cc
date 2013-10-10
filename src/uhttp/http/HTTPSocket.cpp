/******************************************************************
*
* uHTTP for C++
*
* Copyright (C) Satoshi Konno 2002-2012
*
* This is licensed under BSD-style license, see file COPYING.
*
******************************************************************/

#include <uhttp/http/HTTPSocket.h>
#include <uhttp/http/HTTPResponse.h>
#include <uhttp/util/StringUtil.h>

using namespace std;
using namespace uHTTP;
using namespace uHTTP;
using namespace uHTTP;
using namespace uHTTP;

static const int DEFAULT_CHUNK_SIZE = 512 * 1024;

////////////////////////////////////////////////
//	Constructor
////////////////////////////////////////////////

HTTPSocket::HTTPSocket(uHTTP::Socket *socket)
{
	setSocket(socket);
	open();
}

HTTPSocket::HTTPSocket(HTTPSocket *socket)
{
	setSocket((socket != NULL) ? socket->getSocket() : NULL);
}
	
HTTPSocket::~HTTPSocket()
{
	close();
}
	
////////////////////////////////////////////////
//	open/close
////////////////////////////////////////////////

bool HTTPSocket::open()
{
	return true;
}

bool HTTPSocket::close()
{
	Socket *sock = getSocket();
	if (sock != NULL) {
		sock->close();
		setSocket(NULL);
	}
	return true;
}

////////////////////////////////////////////////
//	post
////////////////////////////////////////////////
	
bool HTTPSocket::post(HTTPResponse *httpRes, const std::string &content, int contentOffset, int contentLength, bool isOnlyHeader, bool isChunked)
{
	HTTPDate now;
	httpRes->setDate(&now);

	Socket *sock = getSocket();
	
	httpRes->setContentLength(contentLength);

	string header;
	sock->send(httpRes->getHeader(header));
	sock->send(HTTP::CRLF);

	if (isOnlyHeader == true)
		return true;

	if (isChunked == true) {
		string chunSizeBuf;
		Integer2HexString(contentLength, chunSizeBuf);
		sock->send(chunSizeBuf.c_str());
		sock->send(HTTP::CRLF);
	}

	sock->send((content.c_str() + contentOffset), contentLength);

	if (isChunked == true) {
		sock->send(HTTP::CRLF);
		sock->send("0");
		sock->send(HTTP::CRLF);
	}
		
	return true;
}

bool HTTPSocket::post(HTTPResponse *httpRes, InputStream *in, long contentOffset, long contentLength, bool isOnlyHeader, bool isChunked)
{
	HTTPDate now;
	httpRes->setDate(&now);

	Socket *sock = getSocket();

	httpRes->setContentLength(contentLength);

	string header;
	sock->send(httpRes->getHeader(header));
	sock->send(HTTP::CRLF);

	if (isOnlyHeader == true)
		return true;

	if (0 < contentOffset)
		in->skip(contentOffset);

	int chunkSize = HTTP::GetChunkSize();
	char *chunkBuf = new char[chunkSize+1];

	string readBuf;
	string chunSizeBuf;
	long readCnt = 0;
	long readSize = (chunkSize < contentLength) ? chunkSize : contentLength;
	int readLen = in->read(chunkBuf, (int)readSize);
	while (0 < readLen && readCnt < contentLength) {
		if (isChunked == true) {
			Integer2HexString(readLen, chunSizeBuf);
			sock->send(chunSizeBuf.c_str());
			sock->send(HTTP::CRLF);
		}
		if (sock->send(chunkBuf, readLen) <=0)
			break;
		if (isChunked == true)
			sock->send(HTTP::CRLF);
		readCnt += readLen;
		readBuf = "";
		readSize = (chunkSize < (contentLength-readCnt)) ? chunkSize : (contentLength-readCnt);
		readLen = in->read(chunkBuf, (int)readSize);
	}

	if (isChunked == true) {
		sock->send("0");
		sock->send(HTTP::CRLF);
	}
	
	delete []chunkBuf;

	return true;
}

bool HTTPSocket::post(HTTPResponse *httpRes, int contentOffset, int contentLength, bool isOnlyHeader, bool isChunked)
{
	if (httpRes->hasContentInputStream() == true)
		return post(httpRes,httpRes->getContentInputStream(), contentOffset, contentLength, isOnlyHeader, isChunked);
	return post(httpRes,httpRes->getContent(), contentOffset, contentLength, isOnlyHeader, isChunked);
}