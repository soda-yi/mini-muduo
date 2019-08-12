#ifndef SAMPLE_HTTP_HTTPCONTEXT_HH
#define SAMPLE_HTTP_HTTPCONTEXT_HH

#include "http_context.hh"

#include "http_request.hh"

class Buffer;

class HttpContext
{
public:
    enum HttpRequestParseState {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll,
    };

    HttpContext()
        : state_(kExpectRequestLine)
    {
    }

    bool ParseRequest(Buffer *buf);

    bool GotAll() const
    {
        return state_ == kGotAll;
    }

    void Reset()
    {
        state_ = kExpectRequestLine;
        HttpRequest dummy;
        request_ = dummy;
    }

    const HttpRequest &GetRequest() const
    {
        return request_;
    }

    HttpRequest &GetRequest()
    {
        return request_;
    }

private:
    bool ProcessRequestLine(const char *begin, const char *end);

    HttpRequestParseState state_;
    HttpRequest request_;
};

#endif