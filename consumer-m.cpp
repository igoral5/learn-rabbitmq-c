/**
 * @file consumer-m.cpp
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

log4cplus::Logger logger = log4cplus::Logger::getInstance("consumer-m");

void
usage()
{
    LOG4CPLUS_ERROR(logger, "Usage: consumer-m [-h|--host host] [-p|--port port] [-w|--wait milliseconds] queue");
}

size_t wait = 0;

int
onMessage(AMQPMessage* message)
{
    uint32_t len;
    std::string message_str = message -> getMessage(&len);
    LOG4CPLUS_INFO_FMT(logger, "Receive: %s, length: %d, tag: %d", message_str.c_str(), len, message -> getDeliveryTag());
    message -> getQueue() -> Ack(message -> getDeliveryTag());
    if (wait)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(wait));
    }
    return 0;
}

int
main(int argc, char *argv[])
try
{
    log4cplus::PropertyConfigurator::doConfigure("log4cplus.properties");
    std::string host = "localhost";
    int port = AMQP_PROTOCOL_PORT;
    struct option long_opt[] = {
        {"host", required_argument, nullptr, 'h'},
        {"port", required_argument, nullptr, 'p'},
        {"wait", required_argument, nullptr, 'w'},
        {nullptr, no_argument, nullptr, 0}
    };
    while(true)
    {
        int c = getopt_long(argc, argv, "h:p:w:", long_opt, nullptr);
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
            case 'w':
                wait = atol(optarg);
                break;
            default:
                usage();
                return EXIT_FAILURE;
        }
    }
    if (argc < optind + 1)
    {
        usage();
        return EXIT_FAILURE;
    }
    const std::string name_queue(argv[optind]);
    std::ostringstream connection_str;
    connection_str << "guest:guest@" << host << ":" << port << "/";
    AMQP amqp(connection_str.str());
    AMQPQueue *queue = amqp.createQueue(name_queue);
    queue -> Declare(name_queue, AMQP_DURABLE);
    queue -> addEvent(AMQP_MESSAGE, onMessage);
    //queue -> Qos(0, 1, 0);
    LOG4CPLUS_INFO(logger, "Ожидание входящих сообщений. Для выхода нажмите CTRL+C");
    queue -> Consume();
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
