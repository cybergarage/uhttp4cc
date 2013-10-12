/******************************************************************
*
* uHTTP for C++
*
* Copyright (C) Satoshi Konno 2002-2012
*
* This is licensed under BSD-style license, see file COPYING.
*
******************************************************************/

#include <string>
#include <iostream>

#include <stdlib.h>
#include <unistd.h>

#include <uhttp/HTTP.h>

using namespace std;
using namespace uHTTP;

void usage(char *argv[])
{
    std::string programName = argv[0];
    size_t lastPathIndex = programName.find_last_of("/");
    if (lastPathIndex != std::string::npos)
        programName = programName.substr((lastPathIndex + 1));

    cout << "Usage: " << programName << " [options...] <url>" << endl;
    cout << " -0, Use HTTP 1.0" << endl;
    cout << " -1, Use HTTP 1.1" << endl;
    cout << " -h, This help text" << endl;
    cout << " -v, Enable verbose mode" << endl;
}

int main(int argc, char *argv[]) 
{
    const char *httpVersion = HTTP::VER_11;
    bool verboseMode = false;
    
    int ch;
    while ((ch = getopt(argc, argv, "01vh")) != -1) {
        switch (ch) {
        case '0':
            {
                httpVersion = HTTP::VER_10;
            }
            break;
        case '1':
            {
                httpVersion = HTTP::VER_11;
            }
            break;
        case 'v':
            {
                verboseMode = true;
            }
            break;
        case 'h':
        default:
            {
                usage(argv);
                exit(EXIT_SUCCESS);
            }
        }
    }
    
    argc -= optind;
    argv += optind;

	if (argc < 1) {
		usage((argv-optind));
        return EXIT_FAILURE;
	}
    
	URL uri(argv[0]);
    
    if (!uri.isValid()) {
        if (!uri.hasPath()) {
            uri.setPath("/");
        }
        if (!uri.isValid()) {
            cout << "ERR : Invalid URL <" << argv[0] << ">";
            return EXIT_FAILURE;
        }
    }
    
	HTTPRequest httpReq;
	httpReq.setVersion(httpVersion);
	httpReq.setMethod(HTTP::GET);
	httpReq.setURL(&uri);

    if (verboseMode) {
        string firstHeader;
        cout << "> " << httpReq.getRequestLine(firstHeader) <<  endl;
        for (HTTPHeaderList::iterator header = httpReq.getHeaders().begin(); header != httpReq.getHeaders().end(); header++) {
            cout << "> " << (*header)->getName() << " : " << (*header)->getValue() << endl;
        }
    }
    
	HTTPResponse *httpRes = httpReq.post();

    if (verboseMode) {
        cout << "< " << httpRes->getFirstLine() << endl;
        for (HTTPHeaderList::iterator header = httpRes->getHeaders().begin(); header != httpRes->getHeaders().end(); header++) {
            cout << "< " << (*header)->getName() << " : " << (*header)->getValue() << endl;
        }
    }

	if (!httpRes->isSuccessful()) {
		return EXIT_FAILURE;
	}

	string httpContent = httpRes->getContent();

	cout << httpContent << endl;
	
	return EXIT_SUCCESS;
}

