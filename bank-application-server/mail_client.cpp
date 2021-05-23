#include <curl/curl.h>
#include "mail_client.h"
#include <iostream>
#include <fstream>
#include "bank_exception.h"

void MailClient::sendEmail(const std::string& to, const std::string& message)
{
	CURL *curl;
	CURLcode res = CURLE_OK;
	struct curl_slist *recipients = NULL;

	// A random filename
	std::string filename = std::tmpnam(nullptr);
	filename += ".txt";

	// Write the email into a file
	std::ofstream file;
	try {
		file.open(filename);
		file << "Subject: account status update\n";
		file << "\n";
		file << message;
		file.close();
	}
	catch (std::ofstream::failure& e) {
		file.close();
		std::string message = e.what();
		throw mail_exception("error occured during writing email file (" + message + ")");
	}

	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_USERNAME, USR.c_str());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, PSWD.c_str());
		curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com");

		#ifdef SKIP_PEER_VERIFICATION
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		#endif

		#ifdef SKIP_HOSTNAME_VERIFICATION
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		#endif
 
		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM.c_str());
	 
		recipients = curl_slist_append(recipients, to.c_str());
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

		FILE *p = fopen(filename.c_str(), "r");
		if (p == NULL) {
			throw mail_exception("failed to load email file");
		}

		curl_easy_setopt(curl, CURLOPT_READDATA, p);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
 
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	 
		res = curl_easy_perform(curl);
 
		std::string error_message = "";
		if (res != CURLE_OK) {
			error_message = curl_easy_strerror(res);
		}
	 
		curl_slist_free_all(recipients);
	 
		curl_easy_cleanup(curl);
		std::remove(filename.c_str());

		if (res != CURLE_OK) {
			std::string message = "curl_easy_perform() failed (";
			message += error_message;
			message += ")";
			throw mail_exception(message);
		}
	}
	else {
		std::remove(filename.c_str());
		throw mail_exception("curl failed to initialize");
	}
}
