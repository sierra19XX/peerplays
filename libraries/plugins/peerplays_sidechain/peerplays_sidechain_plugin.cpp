#include <graphene/peerplays_sidechain/peerplays_sidechain_plugin.hpp>

#include <fc/log/logger.hpp>
#include <graphene/peerplays_sidechain/sidechain_net_manager.hpp>

namespace bpo = boost::program_options;

namespace graphene { namespace peerplays_sidechain {

namespace detail
{

class peerplays_sidechain_plugin_impl
{
   public:
      peerplays_sidechain_plugin_impl(peerplays_sidechain_plugin& _plugin)
         : _self( _plugin )
      { }
      virtual ~peerplays_sidechain_plugin_impl();

      peerplays_sidechain_plugin& _self;

      peerplays_sidechain::sidechain_net_manager _net_manager;

};

peerplays_sidechain_plugin_impl::~peerplays_sidechain_plugin_impl()
{
   return;
}

} // end namespace detail

peerplays_sidechain_plugin::peerplays_sidechain_plugin() :
   my( new detail::peerplays_sidechain_plugin_impl(*this) )
{
}

peerplays_sidechain_plugin::~peerplays_sidechain_plugin()
{
   return;
}

std::string peerplays_sidechain_plugin::plugin_name()const
{
   return "peerplays_sidechain";
}

void peerplays_sidechain_plugin::plugin_set_program_options(
   boost::program_options::options_description& cli,
   boost::program_options::options_description& cfg
   )
{
   cli.add_options()
         //("bitcoin-node-ip", bpo::value<string>()->implicit_value("127.0.0.1"), "IP address of Bitcoin node")
         //("bitcoin-node-zmq-port", bpo::value<uint32_t>()->implicit_value(28332), "ZMQ port of Bitcoin node")
         //("bitcoin-node-rpc-port", bpo::value<uint32_t>()->implicit_value(18332), "RPC port of Bitcoin node")
         //("bitcoin-node-rpc-user", bpo::value<string>(), "Bitcoin RPC user")
         //("bitcoin-node-rpc-password", bpo::value<string>(), "Bitcoin RPC password")
         ("bitcoin-node-ip", bpo::value<string>()->implicit_value("99.79.189.95"), "IP address of Bitcoin node")
         ("bitcoin-node-zmq-port", bpo::value<uint32_t>()->implicit_value(11111), "ZMQ port of Bitcoin node")
         ("bitcoin-node-rpc-port", bpo::value<uint32_t>()->implicit_value(22222), "RPC port of Bitcoin node")
         ("bitcoin-node-rpc-user", bpo::value<string>()->implicit_value("1"), "Bitcoin RPC user")
         ("bitcoin-node-rpc-password", bpo::value<string>()->implicit_value("1"), "Bitcoin RPC password")
         ;
   cfg.add(cli);
}

void peerplays_sidechain_plugin::plugin_initialize(const boost::program_options::variables_map& options)
{
   ilog("peerplays sidechain plugin:  plugin_initialize()");

   if( options.count( "bitcoin-node-ip" ) && options.count( "bitcoin-node-zmq-port" ) && options.count( "bitcoin-node-rpc-port" )
      && options.count( "bitcoin-node-rpc-user" ) && options.count( "bitcoin-node-rpc-password" ) )
   {
      my->_net_manager.create_handler(networks::bitcoin, options);
   } else {
      wlog("Haven't set up bitcoin sidechain parameters");
   }
}

void peerplays_sidechain_plugin::plugin_startup()
{
   ilog("peerplays sidechain plugin:  plugin_startup()");
}

} } // graphene::peerplays_sidechain

