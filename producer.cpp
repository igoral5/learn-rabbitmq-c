/**
 * @file producer.cpp
 * @author igor
 * @date 30 окт. 2015 г.
 */

#include <iostream>
#include <exception>
#include <cstdlib>
#include <memory>

#include <boost/lexical_cast.hpp>

#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

#include "error.h"


int
main(int argc, char *argv[])
try
{
    if (argc < 7)
    {
        std::cerr << "Usage: producer <host> <port> <exchange> <routing_key> <message> <count>"
                << std::endl;
        return EXIT_FAILURE;
    }
    const std::string host(argv[1]);
    const int port = boost::lexical_cast<int>(argv[2]);
    const std::string exchange(argv[3]);
    const std::string routing_key(argv[4]);
    const std::string message(argv[5]);
    //const size_t count = boost::lexical_cast<size_t>(argv[6]);
    amqp_connection_state_t conn = amqp_new_connection();
    amqp_socket_t* socket = amqp_tcp_socket_new(conn);

    if (!socket)
    {
        std::cerr << "error creating socket" << std::endl;
        return EXIT_FAILURE;
    }
    int status  = amqp_socket_open(socket, host.c_str(), port);
    if (status)
    {
        std::cerr << "error open socket" << std::endl;
        return EXIT_FAILURE;
    }
    check_error(amqp_login(conn, "/", AMQP_DEFAULT_MAX_CHANNELS,
            AMQP_DEFAULT_FRAME_SIZE, AMQP_DEFAULT_HEARTBEAT, AMQP_SASL_METHOD_PLAIN), "login");
    amqp_channel_open(conn, 1);
    check_error(amqp_get_rpc_reply(conn), "open channel");



	std::cout << "producer" << std::endl;
	return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
	std::cerr << "Exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch(...)
{
	std::cerr << "Unknown exceprion" << std::endl;
    return EXIT_FAILURE;
}
