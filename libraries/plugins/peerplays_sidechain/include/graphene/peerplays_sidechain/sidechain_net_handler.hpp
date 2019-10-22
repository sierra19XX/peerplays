#pragma once

#include <boost/program_options.hpp>

namespace graphene { namespace peerplays_sidechain {

class sidechain_net_handler {
public:
    sidechain_net_handler(const boost::program_options::variables_map& options);
    virtual ~sidechain_net_handler();
protected:
    virtual void handle_block( const std::string& block_hash ) = 0;

private:

};

} } // graphene::peerplays_sidechain

