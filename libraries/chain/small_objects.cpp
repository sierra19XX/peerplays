/*
 * Copyright (c) 2019 BitShares Blockchain Foundation, and contributors.
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

#include <graphene/chain/protocol/fee_schedule.hpp>

#include <graphene/chain/balance_object.hpp>
#include <graphene/chain/block_summary_object.hpp>
#include <graphene/chain/budget_record_object.hpp>
#include <graphene/chain/buyback_object.hpp>
#include <graphene/chain/chain_property_object.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/confidential_object.hpp>
#include <graphene/chain/fba_object.hpp>
#include <graphene/chain/global_property_object.hpp>
#include <graphene/chain/market_object.hpp>
#include <graphene/chain/operation_history_object.hpp>
#include <graphene/chain/special_authority_object.hpp>
#include <graphene/chain/transaction_object.hpp>
#include <graphene/chain/withdraw_permission_object.hpp>
#include <graphene/chain/witness_object.hpp>
#include <graphene/chain/witness_schedule_object.hpp>
#include <graphene/chain/worker_object.hpp>

#include <fc/io/raw.hpp>

GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::balance_object )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::block_summary_object )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::budget_record )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::budget_record_object )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::buyback_object )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::immutable_chain_parameters )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::chain_property_object )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::committee_member_object )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::blinded_balance_object )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::fba_accumulator_object )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::dynamic_global_property_object )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::global_property_object )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::operation_history_object )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::account_transaction_history_object )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::special_authority_object )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::transaction_object )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::withdraw_permission_object )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::witness_object )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::witness_schedule_object )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::witness_scheduler )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::worker_object )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::limit_order_object )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::call_order_object )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::chain::force_settlement_object )
