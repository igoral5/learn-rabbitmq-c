/**
 * @file error.h
 * @author igor
 * @date 30 окт. 2015 г.
 */

#ifndef ERROR_H_
#define ERROR_H_

#include <stdexcept>

#include <amqp.h>

class amqp_runtime_error : public std::runtime_error
{
public:
    amqp_runtime_error(const std::string& message) : std::runtime_error(message) {}
    virtual ~amqp_runtime_error() noexcept {}
};

void
amqp_check_error(amqp_rpc_reply_t x, const std::string& context);

void
check_error(int err, const std::string& context);



#endif /* ERROR_H_ */
