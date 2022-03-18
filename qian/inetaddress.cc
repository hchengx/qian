#include "inetaddress.h"

#include <netinet/in.h>
#include <string.h>
#include <strings.h>

namespace qian {

InetAddress::InetAddress(uint16_t port, std::string ip)
{
	::bzero(&addr_, sizeof addr_);
	addr_.sin_family = AF_INET;
	addr_.sin_port = ::htons(port);
	if (ip.empty()) {
		addr_.sin_addr.s_addr = htonl(INADDR_ANY);
	} else {
		inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
	}
}

std::string InetAddress::toIp() const
{
	char buf[64] = { 0 };
	::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
	return buf;
}

	// ip:port
std::string InetAddress::toIpPort() const
{
	char buf[64] = { 0 };
	::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
	size_t end = ::strlen(buf);
	uint16_t port = ::ntohs(addr_.sin_port);
	sprintf(buf+end, ":%u", port);
	return buf;
}

uint16_t InetAddress::toPort() const
{
	return ::ntohs(addr_.sin_port);
}

} // namespace qian

