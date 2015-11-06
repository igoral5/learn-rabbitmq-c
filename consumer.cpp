/**
 * @file consumer.cpp
 * @author igor
 * @date 06 нояб. 2015 г.
 */

#include <iostream>
#include <exception>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <getopt.h>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>

#include <amqp.h>
#include <amqp_framing.h>

#include <SimpleAmqpClient/SimpleAmqpClient.h>

log4cplus::Logger logger = log4cplus::Logger::getInstance("consumer");

void
usage()
{
    LOG4CPLUS_ERROR(logger, "Usage: consumer [-h|--host host] [-p|--port port] [-w|--wait milliseconds] queue");
}

int
main(int argc, char *argv[])
try
{
    log4cplus::PropertyConfigurator::doConfigure("log4cplus.properties");
    std::string host = "localhost";
    int port = AMQP_PROTOCOL_PORT;
    size_t wait = 0;
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
    AmqpClient::Channel::ptr_t channel = AmqpClient::Channel::Create(host, port);
    channel -> DeclareQueue(name_queue, false, true, false, false);
    std::ostringstream consumer_tag;
    consumer_tag << "consumer-simple-" << getpid();
    channel -> BasicConsume(name_queue, consumer_tag.str(), true, false, false, 1);
    channel -> BasicQos(consumer_tag.str(), 1);
    LOG4CPLUS_INFO(logger, "Ожидание входящих сообщений. Для выхода нажмите CTRL+C");
    while(true)
    {
        AmqpClient::Envelope::ptr_t envelope;
        if (channel -> BasicConsumeMessage(envelope))
        {
            LOG4CPLUS_INFO_FMT(logger, "Receive: %s", envelope -> Message() -> Body().c_str());
            channel -> BasicAck(envelope);
            if (wait)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(wait));
            }
        }
        else
        {
            LOG4CPLUS_ERROR(logger, "error received message");
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
