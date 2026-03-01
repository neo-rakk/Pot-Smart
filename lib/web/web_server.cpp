#include "web_server.h"
#include "api.h"
#include <WebServer.h>
#include <SPIFFS.h>

WebServer server(80);

static void spiffs_handler() {
    String path = server.uri();
    if (path == "/") path = "/index.html";

    String dataType = "text/html";
    if (path.endsWith(".css")) dataType = "text/css";
    else if (path.endsWith(".js")) dataType = "application/javascript";

    if (SPIFFS.exists(path)) {
        File f = SPIFFS.open(path, "r");
        server.streamFile(f, dataType);
        f.close();
    } else {
        server.send(404, "text/plain", "Not Found");
    }
}

void start_webserver(void) {
    register_api_handlers();
    server.onNotFound(spiffs_handler);
    server.begin();
    Serial.println("Arduino WebServer started");
}
