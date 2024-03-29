#include "http_context.hh"

#include <algorithm>

#include "muduo/buffer.hh"

bool HttpContext::ProcessRequestLine(const char *begin, const char *end)
{
    bool succeed = false;
    const char *start = begin;
    const char *space = std::find(start, end, ' ');
    if (space != end && request_.SetMethod(start, space)) {
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end) {
            const char *question = std::find(start, space, '?');
            if (question != space) {
                request_.SetPath(start, question);
                request_.SetQuery(question, space);
            } else {
                request_.SetPath(start, space);
            }
            start = space + 1;
            succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
            if (succeed) {
                if (*(end - 1) == '1') {
                    request_.SetVersion(HttpRequest::kHttp11);
                } else if (*(end - 1) == '0') {
                    request_.SetVersion(HttpRequest::kHttp10);
                } else {
                    succeed = false;
                }
            }
        }
    }
    return succeed;
}

// return false if any error
bool HttpContext::ParseRequest(Buffer *buf)
{
    bool ok = true;
    bool hasMore = true;
    std::string crlfs("\r\n");
    while (hasMore) {
        if (state_ == kExpectRequestLine) {
            const char *crlf = buf->Find(crlfs);
            if (crlf) {
                ok = ProcessRequestLine(buf->Peek(), crlf);
                if (ok) {
                    //request_.SetReceiveTime(receiveTime);
                    buf->Retrieve(crlf + 2 - buf->Peek());
                    state_ = kExpectHeaders;
                } else {
                    hasMore = false;
                }
            } else {
                hasMore = false;
            }
        } else if (state_ == kExpectHeaders) {
            const char *crlf = buf->Find(crlfs);
            if (crlf) {
                const char *colon = std::find(buf->Peek(), crlf, ':');
                if (colon != crlf) {
                    request_.AddHeader(buf->Peek(), colon, crlf);
                } else {
                    // empty line, end of header
                    // FIXME:
                    state_ = kGotAll;
                    hasMore = false;
                }
                buf->Retrieve(crlf + 2 - buf->Peek());
            } else {
                hasMore = false;
            }
        } else if (state_ == kExpectBody) {
            // FIXME:
        }
    }
    return ok;
}