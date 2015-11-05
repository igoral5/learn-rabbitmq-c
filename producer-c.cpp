/**
 * @file producer.cpp
 * @author igor
 * @date 30 окт. 2015 г.
 */

#include <iostream>
#include <exception>
#include <cstdlib>
#include <memory>
#include <sstream>
#include <getopt.h>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>

#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

#include "error.h"

log4cplus::Logger logger = log4cplus::Logger::getInstance("producer");

void
usage()
{
    LOG4CPLUS_ERROR(logger, "Usage: producer [-h|--host host] [-p|--port port] [--exchange|-e exchange] [--count|-c count] routing_key message");
}

int
main (int argc, char *argv[])
try
{
    log4cplus::PropertyConfigurator::doConfigure("log4cplus.properties");
    std::string host = "localhost";
    int port = AMQP_PROTOCOL_PORT;
    std::string exchange = "";
    size_t count = 1;
    struct option long_opt[] = {
    	{"host", required_argument, nullptr, 'h'},
    	{"port", required_argument, nullptr, 'p'},
    	{"exchange", required_argument, nullptr, 'e'},
    	{"count", required_argument, nullptr, 'c'},
    	{nullptr, no_argument, nullptr, 0}
    };
    while(true)
    {
    	int c = getopt_long(argc, argv, "h:p:e:c:", long_opt, nullptr);
    	if (c == -1)
    		break;
    	switch(c)
    	{
    	case 'h':
    		host = optarg;
    		break;
    	case 'p':
    		port = atoi(optarg);
    		break;
    	case 'e':
    		exchange = optarg;
    		break;
    	case 'c':
    		count = atol(optarg);
    		break;
    	default:
    		usage();
    		return EXIT_FAILURE;
    	}
    }
    if (argc < optind + 2)
    {
        usage();
        return EXIT_FAILURE;
    }
    const std::string routing_key(argv[optind]);
    const std::string message(argv[optind + 1]);
    amqp_connection_state_t conn = amqp_new_connection();
    amqp_socket_t* socket = amqp_tcp_socket_new(conn);

    if (!socket)
    {
        LOG4CPLUS_ERROR(logger, "error creating socket");
        return EXIT_FAILURE;
    }
    int status = amqp_socket_open(socket, host.c_str(), port);
    if (status)
    {
        LOG4CPLUS_ERROR(logger, "error open socket");
        return EXIT_FAILURE;
    }
    amqp_check_error(amqp_login(conn, "/", AMQP_DEFAULT_MAX_CHANNELS, AMQP_DEFAULT_FRAME_SIZE,
            AMQP_DEFAULT_HEARTBEAT, AMQP_SASL_METHOD_PLAIN, "guest", "guest"), "login");
    amqp_channel_open(conn, 1);
    amqp_check_error(amqp_get_rpc_reply(conn), "open channel");
    for (size_t i = 0; i < count; i++)
    {
        std::ostringstream oss;
        oss << i + 1 << " - " << message;
        check_error(amqp_basic_publish(conn, 1, amqp_cstring_bytes(exchange.c_str()),
                amqp_cstring_bytes(routing_key.c_str()), 0, 0, nullptr,
                amqp_cstring_bytes(oss.str().c_str())), "publish");
        LOG4CPLUS_INFO_FMT(logger, "Send: %s", oss.str().c_str());
    }
    amqp_check_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "closing channel");
    amqp_check_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "closing connection");
    check_error(amqp_destroy_connection(conn), "ending connection");
    return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
    LOG4CPLUS_ERROR(logger, e.what());
    return EXIT_FAILURE;
}
catch (...)
{
    LOG4CPLUS_ERROR(logger, "Unknown exceprion");
    return EXIT_FAILURE;
}
