// Define either here or when building
// #define CURL_STATICLIB

#include <iostream>
#include "mail_client.h"
#include "bank_server.h"
#include "bank_exception.h"

/**
 * Application entry point.
 */
int main(int argc, char* argv[])
{
	// Server listening
	BankServer server;
	server.run(argv[0]);

	return 0;
}
