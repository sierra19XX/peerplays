/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once

#include <fc/exception/exception.hpp>
#include <graphene/chain/protocol/protocol.hpp>

#define GRAPHENE_ASSERT( expr, exc_type, FORMAT, ... )                \
   FC_MULTILINE_MACRO_BEGIN                                           \
   if( !(expr) )                                                      \
      FC_THROW_EXCEPTION( exc_type, FORMAT, __VA_ARGS__ );            \
   FC_MULTILINE_MACRO_END


#define GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( op_name )                \
   FC_DECLARE_DERIVED_EXCEPTION(                                      \
      op_name ## _validate_exception,                                 \
      graphene::chain::operation_validate_exception,                  \
      3040000 + 100 * operation::tag< op_name ## _operation >::value  \
      )                                                               \
   FC_DECLARE_DERIVED_EXCEPTION(                                      \
      op_name ## _evaluate_exception,                                 \
      graphene::chain::operation_evaluate_exception,                  \
      3050000 + 100 * operation::tag< op_name ## _operation >::value  \
      )

#define GRAPHENE_IMPLEMENT_OP_BASE_EXCEPTIONS( op_name )              \
   FC_IMPLEMENT_DERIVED_EXCEPTION(                                    \
      op_name ## _validate_exception,                                 \
      graphene::chain::operation_validate_exception,                  \
      3040000 + 100 * operation::tag< op_name ## _operation >::value, \
      #op_name "_operation validation exception"                      \
      )                                                               \
   FC_IMPLEMENT_DERIVED_EXCEPTION(                                    \
      op_name ## _evaluate_exception,                                 \
      graphene::chain::operation_evaluate_exception,                  \
      3050000 + 100 * operation::tag< op_name ## _operation >::value, \
      #op_name "_operation evaluation exception"                      \
      )

#define GRAPHENE_DECLARE_OP_VALIDATE_EXCEPTION( exc_name, op_name, seqnum, msg ) \
   FC_DECLARE_DERIVED_EXCEPTION(                                      \
      op_name ## _ ## exc_name,                                       \
      graphene::chain::op_name ## _validate_exception,                \
      3040000 + 100 * operation::tag< op_name ## _operation >::value  \
         + seqnum                                                     \
      )

#define GRAPHENE_IMPLEMENT_OP_VALIDATE_EXCEPTION( exc_name, op_name, seqnum, msg ) \
   FC_IMPLEMENT_DERIVED_EXCEPTION(                                    \
      op_name ## _ ## exc_name,                                       \
      graphene::chain::op_name ## _validate_exception,                \
      3040000 + 100 * operation::tag< op_name ## _operation >::value  \
         + seqnum,                                                    \
      msg                                                             \
      )

#define GRAPHENE_DECLARE_OP_EVALUATE_EXCEPTION( exc_name, op_name, seqnum, msg ) \
   FC_DECLARE_DERIVED_EXCEPTION(                                      \
      op_name ## _ ## exc_name,                                       \
      graphene::chain::op_name ## _evaluate_exception,                \
      3050000 + 100 * operation::tag< op_name ## _operation >::value  \
         + seqnum                                                     \
      )

#define GRAPHENE_IMPLEMENT_OP_EVALUATE_EXCEPTION( exc_name, op_name, seqnum, msg ) \
   FC_IMPLEMENT_DERIVED_EXCEPTION(                                    \
      op_name ## _ ## exc_name,                                       \
      graphene::chain::op_name ## _evaluate_exception,                \
      3050000 + 100 * operation::tag< op_name ## _operation >::value  \
         + seqnum,                                                    \
      msg                                                             \
      )

namespace graphene { namespace chain {

   FC_DECLARE_EXCEPTION( chain_exception, 3000000)
   FC_DECLARE_DERIVED_EXCEPTION( database_query_exception,          graphene::chain::chain_exception, 3010000 )
   FC_DECLARE_DERIVED_EXCEPTION( block_validate_exception,          graphene::chain::chain_exception, 3020000)
   FC_DECLARE_DERIVED_EXCEPTION( transaction_exception,             graphene::chain::chain_exception, 3030000)
   FC_DECLARE_DERIVED_EXCEPTION( operation_validate_exception,      graphene::chain::chain_exception, 3040000)
   FC_DECLARE_DERIVED_EXCEPTION( operation_evaluate_exception,      graphene::chain::chain_exception, 3050000)
   FC_DECLARE_DERIVED_EXCEPTION( utility_exception,                 graphene::chain::chain_exception, 3060000)
   FC_DECLARE_DERIVED_EXCEPTION( undo_database_exception,           graphene::chain::chain_exception, 3070000)
   FC_DECLARE_DERIVED_EXCEPTION( unlinkable_block_exception,        graphene::chain::chain_exception, 3080000)
   FC_DECLARE_DERIVED_EXCEPTION( black_swan_exception,              graphene::chain::chain_exception, 3090000)

   FC_DECLARE_DERIVED_EXCEPTION( tx_missing_active_auth,            graphene::chain::transaction_exception, 3030001 )
   FC_DECLARE_DERIVED_EXCEPTION( tx_missing_owner_auth,             graphene::chain::transaction_exception, 3030002 )
   FC_DECLARE_DERIVED_EXCEPTION( tx_missing_other_auth,             graphene::chain::transaction_exception, 3030003 )
   FC_DECLARE_DERIVED_EXCEPTION( tx_irrelevant_sig,                 graphene::chain::transaction_exception, 3030004 )
   FC_DECLARE_DERIVED_EXCEPTION( tx_duplicate_sig,                  graphene::chain::transaction_exception, 3030005 )
   FC_DECLARE_DERIVED_EXCEPTION( invalid_committee_approval,        graphene::chain::transaction_exception, 3030006 )
   FC_DECLARE_DERIVED_EXCEPTION( insufficient_fee,                  graphene::chain::transaction_exception, 3030007 )

   FC_DECLARE_DERIVED_EXCEPTION( invalid_pts_address,               graphene::chain::utility_exception, 3060001 )
   FC_DECLARE_DERIVED_EXCEPTION( insufficient_feeds,                graphene::chain::chain_exception, 37006 )

   FC_DECLARE_DERIVED_EXCEPTION( pop_empty_chain,              undo_database_exception, 3070001 )

   GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( transfer );
   GRAPHENE_DECLARE_OP_EVALUATE_EXCEPTION( from_account_not_whitelisted, transfer, 1, "owner mismatch" )
   GRAPHENE_DECLARE_OP_EVALUATE_EXCEPTION( to_account_not_whitelisted, transfer, 2, "owner mismatch" )
   GRAPHENE_DECLARE_OP_EVALUATE_EXCEPTION( restricted_transfer_asset, transfer, 3, "restricted transfer asset" )

   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( limit_order_create );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( limit_order_cancel );
   GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( call_order_update );
   GRAPHENE_DECLARE_OP_EVALUATE_EXCEPTION( unfilled_margin_call, call_order_update, 1, "Updating call order would trigger a margin call that cannot be fully filled" )

   GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( account_create );
   GRAPHENE_DECLARE_OP_EVALUATE_EXCEPTION( max_auth_exceeded, account_create, 1, "Exceeds max authority fan-out" )
   GRAPHENE_DECLARE_OP_EVALUATE_EXCEPTION( auth_account_not_found, account_create, 2, "Auth account not found" )
   GRAPHENE_DECLARE_OP_EVALUATE_EXCEPTION( buyback_incorrect_issuer, account_create, 3, "Incorrect issuer specified for account" )
   GRAPHENE_DECLARE_OP_EVALUATE_EXCEPTION( buyback_already_exists, account_create, 4, "Cannot create buyback for asset which already has buyback" )
   GRAPHENE_DECLARE_OP_EVALUATE_EXCEPTION( buyback_too_many_markets, account_create, 5, "Too many buyback markets" )

   GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( account_update );
   GRAPHENE_DECLARE_OP_EVALUATE_EXCEPTION( max_auth_exceeded, account_update, 1, "Exceeds max authority fan-out" )
   GRAPHENE_DECLARE_OP_EVALUATE_EXCEPTION( auth_account_not_found, account_update, 2, "Auth account not found" )

   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( account_whitelist );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( account_upgrade );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( account_transfer );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( asset_create );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( asset_update );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( asset_update_bitasset );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( asset_update_feed_producers );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( asset_issue );

   GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( asset_reserve );
   GRAPHENE_DECLARE_OP_EVALUATE_EXCEPTION( invalid_on_mia, asset_reserve, 1, "invalid on mia" )

   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( asset_fund_fee_pool );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( asset_settle );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( asset_global_settle );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( asset_publish_feed );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( committee_member_create );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( witness_create );

   GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( proposal_create );
   GRAPHENE_DECLARE_OP_EVALUATE_EXCEPTION( review_period_required, proposal_create, 1, "review_period required" )
   GRAPHENE_DECLARE_OP_EVALUATE_EXCEPTION( review_period_insufficient, proposal_create, 2, "review_period insufficient" )

   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( proposal_update );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( proposal_delete );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( withdraw_permission_create );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( withdraw_permission_update );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( withdraw_permission_claim );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( withdraw_permission_delete );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( fill_order );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( global_parameters_update );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( vesting_balance_create );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( vesting_balance_withdraw );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( worker_create );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( custom );
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( assert );

   GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( balance_claim );
   GRAPHENE_DECLARE_OP_EVALUATE_EXCEPTION( claimed_too_often, balance_claim, 1, "balance claimed too often" )
   GRAPHENE_DECLARE_OP_EVALUATE_EXCEPTION( invalid_claim_amount, balance_claim, 2, "invalid claim amount" )
   GRAPHENE_DECLARE_OP_EVALUATE_EXCEPTION( owner_mismatch, balance_claim, 3, "owner mismatch" )

   GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( override_transfer );
   GRAPHENE_DECLARE_OP_EVALUATE_EXCEPTION( not_permitted, override_transfer, 1, "not permitted" )

   GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( blind_transfer );
   GRAPHENE_DECLARE_OP_EVALUATE_EXCEPTION( unknown_commitment, blind_transfer, 1, "Attempting to claim an unknown prior commitment" );

   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( transfer_from_blind_operation )
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( asset_claim_fees_operation )
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( bid_collateral_operation )
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( asset_claim_pool_operation )
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( asset_update_issuer_operation )
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( htlc_create_operation )
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( htlc_redeem_operation )
   //GRAPHENE_DECLARE_OP_BASE_EXCEPTIONS( htlc_extend_operation )

   FC_DECLARE_EXCEPTION(no_transition, 100000)

   #define GRAPHENE_RECODE_EXC( cause_type, effect_type ) \
      catch( const cause_type& e ) \
      { throw( effect_type( e.what(), e.get_log() ) ); }

} } // graphene::chain
