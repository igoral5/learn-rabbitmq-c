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

#include <boost/lexical_cast.hpp>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>

#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

#include "error.h"

log4cplus::Logger logger = log4cplus::Logger::getInstance("producer");

int
main (int argc, char *argv[])
try
{
    log4cplus::PropertyConfigurator::doConfigure("log4cplus.properties");
    if (argc < 7)
    {
        LOG4CPLUS_ERROR(logger,
                "Usage: producer <host> <port> <exchange> <routing_key> <message> <count>");
        return EXIT_FAILURE;
    }
    const std::string host(argv[1]);
    const int port = boost::lexical_cast<int>(argv[2]);
    const std::string exchange(argv[3]);
    const std::string routing_key(argv[4]);
    const std::string message(argv[5]);
    const size_t count = boost::lexical_cast<size_t>(argv[6]);
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
