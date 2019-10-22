#include <graphene/peerplays_sidechain/sidechain_net_handler_bitcoin.hpp>

#include <thread>

#include <boost/algorithm/hex.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <fc/crypto/base64.hpp>
#include <fc/logger/log.hpp>
#include <fc/network/ip.hpp>

namespace graphene { namespace peerplays_sidechain {

// =============================================================================

bitcoin_rpc_client::bitcoin_rpc_client( std::string _ip, uint32_t _rpc, std::string _user, std::string _password ):
                      ip( _ip ), rpc_port( _rpc ), user( _user ), password( _password )
{
   authorization.key = "Authorization";
   authorization.val = "Basic " + fc::base64_encode( user + ":" + password );
}

std::string bitcoin_rpc_client::receive_full_block( const std::string& block_hash )
{
   fc::http::connection conn;
   conn.connect_to( fc::ip::endpoint( fc::ip::address( ip ), rpc_port ) );

   const auto url = "http://" + ip + ":" + std::to_string( rpc_port ) + "/rest/block/" + block_hash + ".json";

   const auto reply = conn.request( "GET", url );
   if ( reply.status != 200 )
      return "";

   ilog( "Receive Bitcoin block: ${hash}", ( "hash", block_hash ) );
   return std::string( reply.body.begin(), reply.body.end() );
}

int32_t bitcoin_rpc_client::receive_confirmations_tx( const std::string& tx_hash )
{
   const auto body = std::string("{\"jsonrpc\": \"1.0\", \"id\":\"curltest\", \"method\": \"getrawtransaction\", \"params\": [") +
                      std::string("\"") + tx_hash + std::string("\"") + ", " + "true" + std::string("] }");

   const auto reply = send_post_request( body );

   if ( reply.status != 200 )
      return 0;

   const auto result = std::string( reply.body.begin(), reply.body.end() );

   std::stringstream ss( result );
   boost::property_tree::ptree tx;
   boost::property_tree::read_json( ss, tx );

   if( tx.count( "result" ) ) {
      if( tx.get_child( "result" ).count( "confirmations" ) ) {
         return tx.get_child( "result" ).get_child( "confirmations" ).get_value<int64_t>();
      }
   }
   return 0;
}

bool bitcoin_rpc_client::receive_mempool_entry_tx( const std::string& tx_hash )
{
   const auto body = std::string("{\"jsonrpc\": \"1.0\", \"id\":\"curltest\", \"method\": \"getmempoolentry\", \"params\": [") +
                      std::string("\"") + tx_hash + std::string("\"") + std::string("] }");

   const auto reply = send_post_request( body );

   if ( reply.status != 200 )
        return false;

    return true;
}

uint64_t bitcoin_rpc_client::receive_estimated_fee()
{
   static const auto confirmation_target_blocks = 6;

   const auto body = std::string("{\"jsonrpc\": \"1.0\", \"id\":\"estimated_feerate\", \"method\": \"estimatesmartfee\", \"params\": [") +
                     std::to_string(confirmation_target_blocks) + std::string("] }");

   const auto reply = send_post_request( body );

   if( reply.status != 200 )
      return 0;

   std::stringstream ss( std::string( reply.body.begin(), reply.body.end() ) );
   boost::property_tree::ptree json;
   boost::property_tree::read_json( ss, json );

   if( json.count( "result" ) )
      if ( json.get_child( "result" ).count( "feerate" ) ) {
         auto feerate_str = json.get_child( "result" ).get_child( "feerate" ).get_value<std::string>();
         feerate_str.erase( std::remove( feerate_str.begin(), feerate_str.end(), '.' ), feerate_str.end() );
         return std::stoll( feerate_str );
      }
   return 0;
}

void bitcoin_rpc_client::send_btc_tx( const std::string& tx_hex )
{
   const auto body = std::string("{\"jsonrpc\": \"1.0\", \"id\":\"send_tx\", \"method\": \"sendrawtransaction\", \"params\": [") +
                     std::string("\"") + tx_hex + std::string("\"") + std::string("] }");

   const auto reply = send_post_request( body );

   if( reply.body.empty() )
      return;

   std::string reply_str( reply.body.begin(), reply.body.end() );

   std::stringstream ss(reply_str);
   boost::property_tree::ptree json;
   boost::property_tree::read_json( ss, json );

   if( reply.status == 200 ) {
      idump(( tx_hex ));
      return;
   } else if( json.count( "error" ) && !json.get_child( "error" ).empty() ) {
      const auto error_code = json.get_child( "error" ).get_child( "code" ).get_value<int>();
      if( error_code == -27 ) // transaction already in block chain
         return;

      wlog( "BTC tx is not sent! Reply: ${msg}", ("msg", reply_str) );
   }
}

bool bitcoin_rpc_client::connection_is_not_defined() const
{
   return ip.empty() || rpc_port == 0 || user.empty() || password.empty();
}

fc::http::reply bitcoin_rpc_client::send_post_request( std::string body )
{
   fc::http::connection conn;
   conn.connect_to( fc::ip::endpoint( fc::ip::address( ip ), rpc_port ) );

   const auto url = "http://" + ip + ":" + std::to_string( rpc_port );

   return conn.request( "POST", url, body, fc::http::headers{authorization} );
}

// =============================================================================

zmq_listener::zmq_listener( std::string _ip, uint32_t _zmq ): ip( _ip ), zmq_port( _zmq ), ctx( 1 ), socket( ctx, ZMQ_SUB ) {
   std::thread( &zmq_listener::handle_zmq, this ).detach();
}

std::vector<zmq::message_t> zmq_listener::receive_multipart() {
   std::vector<zmq::message_t> msgs;

   int32_t more;
   size_t more_size = sizeof( more );
   while ( true ) {
      zmq::message_t msg;
      socket.recv( &msg, 0 );
      socket.getsockopt( ZMQ_RCVMORE, &more, &more_size );

      if ( !more )
         break;
      msgs.push_back( std::move(msg) );
   }

   return msgs;
}

void zmq_listener::handle_zmq() {
   socket.setsockopt( ZMQ_SUBSCRIBE, "hashblock", 0 );
   socket.connect( "tcp://" + ip + ":" + std::to_string( zmq_port ) );

   while ( true ) {
      auto msg = receive_multipart();
      const auto header = std::string( static_cast<char*>( msg[0].data() ), msg[0].size() );
      const auto hash = boost::algorithm::hex( std::string( static_cast<char*>( msg[1].data() ), msg[1].size() ) );

      block_received( hash );
   }
}

// =============================================================================

sidechain_net_handler_bitcoin::sidechain_net_handler_bitcoin(const boost::program_options::variables_map& options) :
      sidechain_net_handler(options) {
   ip = options.at("bitcoin-node-ip").as<std::string>();
   zmq_port = options.at("bitcoin-node-zmq-port").as<uint32_t>();
   rpc_port = options.at("bitcoin-node-rpc-port").as<uint32_t>();
   rpc_user = options.at("bitcoin-node-rpc-user").as<std::string>();
   rpc_password = options.at("bitcoin-node-rpc-password").as<std::string>();

   fc::http::connection conn;
   try {
      conn.connect_to( fc::ip::endpoint( fc::ip::address( ip ), rpc_port ) );
   } catch ( fc::exception e ) {
      elog( "No BTC node running at ${ip} or wrong rpc port: ${port}", ("ip", ip) ("port", rpc_port) );
      FC_ASSERT( false );
   }

   listener = std::unique_ptr<zmq_listener>( new zmq_listener( ip, zmq_port ) );
   bitcoin_client = std::unique_ptr<bitcoin_rpc_client>( new bitcoin_rpc_client( ip, rpc_port, rpc_user, rpc_password ) );
   //db = _db;

   listener->block_received.connect([this]( const std::string& block_hash ) {
      std::thread( &sidechain_net_handler_bitcoin::handle_block, this, block_hash ).detach();
   } );

   //db->send_btc_tx.connect([this]( const sidechain::bitcoin_transaction& trx ) {
   //   std::thread( &sidechain_net_handler_bitcoin::send_btc_tx, this, trx ).detach();
   //} );
}

sidechain_net_handler_bitcoin::~sidechain_net_handler_bitcoin() {
}

void sidechain_net_handler_bitcoin::handle_block( const std::string& block_hash ) {
   ilog("peerplays sidechain plugin:  sidechain_net_handler_bitcoin::handle_block");
   ilog("                             block_hash: ${block_hash}", ("block_hash", block_hash));
}

// =============================================================================

} } // graphene::peerplays_sidechain

