#include "http_response.hh"

#include "muduo/buffer.hh"

void HttpResponse::AppendToBuffer(Buffer *output) const
{
    char buf[32];
    snprintf(buf, sizeof buf, "HTTP/1.1 %d ", statusCode_);
    output->Append(buf);
    output->Append(statusMessage_);
    output->Append("\r\n");

    if (closeConnection_) {
        output->Append("Connection: close\r\n");
    } else {
        snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", body_.size());
        output->Append(buf);
        output->Append("Connection: Keep-Alive\r\n");
    }

    for (auto it = headers_.begin();
         it != headers_.end();
         ++it) {
        output->Append(it->first);
        output->Append(": ");
        output->Append(it->second);
        output->Append("\r\n");
    }

    output->Append("\r\n");
    output->Append(body_);
}