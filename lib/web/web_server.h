#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>

/**
 * @brief The global Arduino WebServer instance (defined in web_server.cpp).
 * Exposed here so other translation units (eg. `src/main.cpp`) can call
 * `server.handleClient()` without redefining the server.
 */
extern WebServer server;

/**
 * @brief Starts the Arduino WebServer and registers all handlers.
 */
void start_webserver(void);

#endif
