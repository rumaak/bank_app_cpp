#include <string>

#ifndef CONNECTION_MANAGER_H_
#define CONNECTION_MANAGER_H_

/**
* A "static" class responsible for network communication.
*/
class ConnectionManager
{
public:
	// Important constants
	static const char SEPARATOR = ';';
	static const char END = '\n';
	static const std::string ACCEPTED;
	static const std::string REJECTED;

	/**
	 * Attempt to send a request to server and read response.
	 * @param[in]	message		Request to be sent
	 * @returns					String representation of server response
	 */
	static std::string sendMessage(const std::string& message);

	/**
	 * Take the beginning of response up until a sepator and move it to field.
	 * @param[in]	response	Server response
	 * @param[out]	field		String to store the response substring
	 * @param[in]	separator	Substring separator
	 */
	static void fillField(std::string& response, std::string& field, char separator);
};

#endif
