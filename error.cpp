/**
 * @file error.cpp
 * @author igor
 * @date 30 окт. 2015 г.
 */

#include <sstream>
#include <iomanip>

#include "error.h"

void
check_error(amqp_rpc_reply_t x, const std::string& context)
{
    switch (x.reply_type)
    {
    case AMQP_RESPONSE_NORMAL:
        return;
    case AMQP_RESPONSE_NONE:
        throw amqp_runtime_error(context + ": missing RPC reply type!");
    case AMQP_RESPONSE_LIBRARY_EXCEPTION:
        throw amqp_runtime_error(context + ": " + amqp_error_string2(x.library_error));
    case AMQP_RESPONSE_SERVER_EXCEPTION:
        switch(x.reply.id)
        {
        case AMQP_CONNECTION_CLOSE_METHOD:
        {
            amqp_connection_close_t *m = reinterpret_cast<amqp_connection_close_t *>(x.reply.decoded);
            std::ostringstream oss;
            oss << context << ": server connection error " <<  m->reply_code << ", message: ";
            oss.write(reinterpret_cast<char *>(m->reply_text.bytes),  m->reply_text.len);
            throw amqp_runtime_error(oss.str());
        }
        case AMQP_CHANNEL_CLOSE_METHOD:
        {
            amqp_channel_close_t *m = reinterpret_cast<amqp_channel_close_t *>(x.reply.decoded);
            std::ostringstream oss;
            oss << context << ": server channel error " << m->reply_code << ", message: ";
            oss.write(reinterpret_cast<char *>(m->reply_text.bytes),  m->reply_text.len);
            throw amqp_runtime_error(oss.str());
        }
        default:
        {
            std::ostringstream oss;
            oss << context << ": unknown server error, method id 0x"
                   << std::setfill('0') << std::setw(8)  << std::hex <<  x.reply.id;
            throw amqp_runtime_error(oss.str());
        }
        }
    }
}
