/**
 * @file consumer.cpp
 * @author igor
 * @date 30 окт. 2015 г.
 */

#include <iostream>
#include <exception>
#include <cstdlib>
#include <string>
#include <getopt.h>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>

#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

#include "error.h"

log4cplus::Logger logger = log4cplus::Logger::getInstance("consumer");

void
usage()
{
    LOG4CPLUS_ERROR(logger, "Usage: consumer [-h|--host host] [-p|--port port] queue");
}

int
main(int argc, char *argv[])
try
{
    log4cplus::PropertyConfigurator::doConfigure("log4cplus.properties");
    int c;
    std::string host = "localhost";
    int port = 5672;
    while(true)
    {
        static struct option long_opt[] = {
                 {"host", required_argument, nullptr, 'h'},
                 {"port", required_argument, nullptr, 'p'},
                 {nullptr, no_argument, nullptr, 0}
        };
        c = getopt_long(argc, argv, "h:p:", long_opt, nullptr);
        if (c == -1)
            break;
        switch (c)
        {
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            default:
                usage();
                return EXIT_FAILURE;
        }
    }
	if (optind == argc)
	{
	    usage();
	    return EXIT_FAILURE;
	}
	const std::string name_queue(argv[optind]);
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
	amqp_check_error(amqp_get_rpc_reply(conn), "channel open");
	amqp_bytes_t queue = amqp_cstring_bytes(name_queue.c_str());
	amqp_queue_declare(conn, 1, queue, 0, 1, 0, 0, amqp_empty_table);
	amqp_check_error(amqp_get_rpc_reply(conn), "declare queue");
	amqp_basic_consume(conn, 1, queue, amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
	amqp_check_error(amqp_get_rpc_reply(conn), "consuming");
	LOG4CPLUS_INFO(logger, "Ожидание входящих сообщений. Для выхода нажмите CTRL+C");
	while(true)
	{
	    amqp_maybe_release_buffers(conn);
	    amqp_envelope_t envelope;
	    amqp_rpc_reply_t reply =  amqp_consume_message(conn, &envelope, nullptr, 0);
	    if (reply.reply_type != AMQP_RESPONSE_NORMAL)
	        break;
	    std::string message(reinterpret_cast<char *>(envelope.message.body.bytes), envelope.message.body.len);
	    LOG4CPLUS_INFO_FMT(logger, "Получено: %s", message.c_str());
	    amqp_destroy_envelope(&envelope);
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
catch(...)
{
	LOG4CPLUS_ERROR(logger, "Unknown exceprion");
    return EXIT_FAILURE;
}
