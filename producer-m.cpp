/**
 * @file producer-m.cpp
 * @author igor
 * @date 10 нояб. 2015 г.
 */

#include <iostream>
#include <exception>
#include <cstdlib>
#include <sstream>
#include <thread>
#include <chrono>
#include <getopt.h>


#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>

#include <AMQPcpp.h>

log4cplus::Logger logger = log4cplus::Logger::getInstance("producer-m");

void
usage()
{
    LOG4CPLUS_ERROR(logger, "Usage: producer-m [-h|--host host] [-p|--port port] [--exchange|-e exchange] [--count|-c count] [-w|--wait milliseconds] routing_key message");
}

int
main(int argc, char *argv[])
try
{
    log4cplus::PropertyConfigurator::doConfigure("log4cplus.properties");
    std::string host = "localhost";
    int port = AMQP_PROTOCOL_PORT;
    std::string exchange = "";
    size_t count = 1;
    size_t wait = 0;
    struct option long_opt[] = {
        {"host", required_argument, nullptr, 'h'},
        {"port", required_argument, nullptr, 'p'},
        {"exchange", required_argument, nullptr, 'e'},
        {"count", required_argument, nullptr, 'c'},
        {"wait", required_argument, nullptr, 'w'},
        {nullptr, no_argument, nullptr, 0}
    };
    while(true)
    {
        int c = getopt_long(argc, argv, "h:p:e:c:w:", long_opt, nullptr);
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
        case 'w':
            wait = atol(optarg);
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
    std::ostringstream connection_str;
    connection_str << "guest:guest@" << host << ":" << port << "/";
    AMQP amqp(connection_str.str());
    AMQPExchange * ex = amqp.createExchange(exchange);
    for(size_t i = 0; i < count; i++)
    {
        std::ostringstream oss;
        oss << i + 1 << " - " << message;
        ex -> setHeader("Content-type", "text/plain");
        ex -> setHeader("Content-encoding", "UTF-8");
        ex -> setHeader("Delivery-mode", 2);
        ex -> Publish(oss.str(), routing_key);
        LOG4CPLUS_INFO_FMT(logger, "Send: %s", oss.str().c_str());
        if (wait)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(wait));
        }
    }
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
