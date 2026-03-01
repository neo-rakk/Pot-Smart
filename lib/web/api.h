#ifndef API_H
#define API_H

#include "esp_http_server.h"
// Register API handlers with the server. The implementation in api.cpp uses the
// global WebServer instance, so no parameter is required.
void register_api_handlers(void);

#endif