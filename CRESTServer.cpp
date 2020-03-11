// Dependencies
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include "HttpMessageHandler.h"
#include "fossa.h"

// Configuration properties
#define CREST2_VERSION "v0.1.0"
#define POLL_TIME_IN_MILLIS 17	// the pcars physics engine runs with 600Hz = 1.67ms interval
#define ESC_KEY 27
#define CREST_API_URL "/crest2/v1/api"

// Constants
#define HTTP_RESPONSE_404 "{\r\n  \"status\": \"404 Not found, please use the correct URL: " CREST_API_URL "\"\r\n}"

// Server variables
static const char *s_http_port = "8180";
static struct ns_serve_http_opts s_http_server_opts;

// Response generator
static HttpMessageHandler httpMessageHandler = HttpMessageHandler();

// Server request handler method
static void ev_handler(struct ns_connection *nc, int ev, void *ev_data) {
	struct http_message *hm = (struct http_message *) ev_data;

	switch (ev) {
	case NS_HTTP_REQUEST:
		// Only handle HTTP requests on the API url
		if (ns_vcmp(&hm->uri, CREST_API_URL) == 0) {
			httpMessageHandler.handle(nc, hm);
		}else{
			// Unknown URI, return a 404
			ns_printf(nc, "HTTP/1.1 404 Not found\r\n"
				"Content-Type: application/json\r\n"
				"Cache-Control: no-cache\r\n"
                "Access-Control-Allow-Origin: *\r\n"
				"Content-Length: %d\r\n\r\n%s",
				(int)strlen(HTTP_RESPONSE_404), HTTP_RESPONSE_404);
		}
		break;
	default:
		break;
	}
}

int main()	{

	// Setup the server
	struct ns_mgr mgr;
	struct ns_connection *nc;
	ns_mgr_init(&mgr, NULL);
	nc = ns_bind(&mgr, s_http_port, ev_handler);
	ns_set_protocol_http_websocket(nc);
	s_http_server_opts.document_root = ".";
	
	// Print some information on the console
	printf("# CREST2 - CARS2 REST API %s\n", CREST2_VERSION);
	printf("# (c) 2015 Lars Rosenquist\n");
	printf("#          updated by Viper\n\n");
	printf("# Server started on port %s\n", s_http_port);
	printf("# API is available at http://localhost:%s%s \n", s_http_port, CREST_API_URL);
	printf("# Press ESC to terminate\n");
    
	// Keep polling until ESC is hit
	while (true)	{

		ns_mgr_poll(&mgr, POLL_TIME_IN_MILLIS);

		if (_kbhit() && _getch() == ESC_KEY)	{
			break;
		}
	}

	// We're done, free up the server and exit
	ns_mgr_free(&mgr);
	return 0;
}
