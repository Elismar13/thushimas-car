#pragma once

#include <WebServer.h>

#include "ControlProtocol.h"

// HTTP adapter for the transport-neutral control endpoint. It owns sockets and
// HTTP details; all validation and safety rules stay behind ControlEndpoint.
class HttpApi {
public:
    explicit HttpApi(ControlEndpoint& endpoint);

    void begin();
    void loop();

private:
    EndpointContext context() const;
    void handleControl();
    void handleState();
    void sendResponse(const ControlResponse& response);

    ControlEndpoint& endpoint_;
    WebServer server_;
};
