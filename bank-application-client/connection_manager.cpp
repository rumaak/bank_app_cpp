#include <SDKDDKVer.h>
#include "connection_manager.h"

#define ASIO_STANDALONE
#include <asio.hpp>
#include <iostream>

using tcp = asio::ip::tcp;

const std::string ConnectionManager::ACCEPTED = "SUC";
const std::string ConnectionManager::REJECTED = "ERR";

std::string ConnectionManager::sendMessage(const std::string& message)
{
    std::string response = "";

 	try {
 		asio::io_context io_context;
 
        std::string host = "127.0.0.1";
        std::string port = "13";
 		tcp::resolver resolver(io_context);
 		tcp::resolver::results_type endpoints = resolver.resolve(host, port);

 		tcp::socket socket(io_context);
 		asio::connect(socket, endpoints);

        socket.write_some(asio::buffer(message.data(), message.length()));

 		for (;;) {
 			char buf[128];
 			asio::error_code error;
 		 	size_t len = socket.read_some(asio::buffer(buf), error);
 	
 		 	if (error == asio::error::eof)
 		 		break;
 		 	else if (error)
 		 		throw asio::system_error(error);
 	
 		 	// std::cout.write(buf, len);
            response.append(buf, len);
 		}
 	}
 	catch (std::exception& e)
 	{
        response = REJECTED;
        response += "an error occurred during issuing a request (";
        response += e.what();
        response += ")";
 		// std::cout << e.what() << std::endl;
 	}
    catch (...) {
        response = REJECTED;
        response += "an unknown error occurred during issuing a request";
    }

    return response;
}

void ConnectionManager::fillField(std::string& response, std::string& field, char separator)
{
	int i = 0;
	for (;;) {
		if (response[i] == separator) {
			break;
		}
		field += response[i];
		i++;
	}
	response.erase(0, i+1);
}
