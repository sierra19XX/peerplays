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

#include <graphene/chain/protocol/chain_parameters.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/chain/immutable_chain_parameters.hpp>

#include <fc/crypto/sha256.hpp>

#include <string>
#include <vector>

namespace graphene { namespace chain {
using std::string;
using std::vector;

struct genesis_state_type {
   struct initial_account_type {
      initial_account_type(const string& name = string(),
                           const public_key_type& owner_key = public_key_type(),
                           const public_key_type& active_key = public_key_type(),
                           bool is_lifetime_member = false)
         : name(name),
           owner_key(owner_key),
           active_key(active_key == public_key_type()? owner_key : active_key),
           is_lifetime_member(is_lifetime_member)
      {}
      string name;
      public_key_type owner_key;
      public_key_type active_key;
      bool is_lifetime_member = false;
   };
   struct initial_bts_account_type {
      struct initial_authority {
         uint32_t                               weight_threshold;
         flat_map<string, weight_type>          account_auths; // uses account name instead of account id
         flat_map<public_key_type, weight_type> key_auths;
         flat_map<address, weight_type>         address_auths;
      };
      struct initial_cdd_vesting_policy {
         uint32_t vesting_seconds;
         fc::uint128_t coin_seconds_earned;
         fc::time_point_sec start_claim;
         fc::time_point_sec coin_seconds_earned_last_update;
      };
      struct initial_linear_vesting_policy {
         fc::time_point_sec begin_timestamp;
         uint32_t vesting_cliff_seconds;
         uint32_t vesting_duration_seconds;
         share_type begin_balance;
      };
      struct initial_vesting_balance {
         string asset_symbol;
         share_type amount;
         std::string policy_type;  // either "linear" or "cdd"
         fc::variant policy; // either an initial_cdd_vesting_policy or initial_linear_vesting_policy
      };
      initial_bts_account_type(const string& name = string(),
                               const initial_authority& owner_authority = initial_authority(),
                               const initial_authority& active_authority = initial_authority(),
                               const share_type& core_balance = share_type())
         : name(name),
           owner_authority(owner_authority),
           active_authority(active_authority),
           core_balance(core_balance)
      {}
      string name;
      initial_authority owner_authority;
      initial_authority active_authority;
      share_type core_balance;
      fc::optional<std::vector<initial_vesting_balance> > vesting_balances;
   };
   struct initial_asset_type {
      struct initial_collateral_position {
         address owner;
         share_type collateral;
         share_type debt;
      };

      string symbol;
      string issuer_name;

      string description;
      uint8_t precision = GRAPHENE_BLOCKCHAIN_PRECISION_DIGITS;

      share_type max_supply;
      share_type accumulated_fees;

      bool is_bitasset = false;
      vector<initial_collateral_position> collateral_records;
   };
   struct initial_balance_type {
      address owner;
      string asset_symbol;
      share_type amount;
   };
   struct initial_vesting_balance_type {
      address owner;
      string asset_symbol;
      share_type amount;
      time_point_sec begin_timestamp;
      fc::optional<uint32_t> vesting_cliff_seconds;
      uint32_t vesting_duration_seconds = 0;
      share_type begin_balance;
   };
   struct initial_witness_type {
      /// Must correspond to one of the initial accounts
      string owner_name;
      public_key_type block_signing_key;
   };
   struct initial_committee_member_type {
      /// Must correspond to one of the initial accounts
      string owner_name;
   };
   struct initial_worker_type {
      /// Must correspond to one of the initial accounts
      string owner_name;
      share_type daily_pay;
   };

   time_point_sec                           initial_timestamp;
   share_type                               max_core_supply = GRAPHENE_MAX_SHARE_SUPPLY;
   chain_parameters                         initial_parameters;
   immutable_chain_parameters               immutable_parameters;
   vector<initial_bts_account_type>         initial_bts_accounts;
   vector<initial_account_type>             initial_accounts;
   vector<initial_asset_type>               initial_assets;
   vector<initial_balance_type>             initial_balances;
   vector<initial_vesting_balance_type>     initial_vesting_balances;
   uint64_t                                 initial_active_witnesses = GRAPHENE_DEFAULT_MIN_WITNESS_COUNT;
   vector<initial_witness_type>             initial_witness_candidates;
   vector<initial_committee_member_type>    initial_committee_candidates;
   vector<initial_worker_type>              initial_worker_candidates;

   /**
    * Temporary, will be moved elsewhere.
    */
   chain_id_type                            initial_chain_id;

   /**
    * Get the chain_id corresponding to this genesis state.
    *
    * This is the SHA256 serialization of the genesis_state.
    */
   chain_id_type compute_chain_id() const;
};

} } // namespace graphene::chain

FC_REFLECT_TYPENAME( graphene::chain::genesis_state_type::initial_account_type )
FC_REFLECT_TYPENAME( graphene::chain::genesis_state_type::initial_asset_type )
FC_REFLECT_TYPENAME( graphene::chain::genesis_state_type::initial_asset_type::initial_collateral_position )
FC_REFLECT_TYPENAME( graphene::chain::genesis_state_type::initial_balance_type )
FC_REFLECT_TYPENAME( graphene::chain::genesis_state_type::initial_vesting_balance_type )
FC_REFLECT_TYPENAME( graphene::chain::genesis_state_type::initial_witness_type )
FC_REFLECT_TYPENAME( graphene::chain::genesis_state_type::initial_committee_member_type )
FC_REFLECT_TYPENAME( graphene::chain::genesis_state_type::initial_worker_type )
FC_REFLECT_TYPENAME( graphene::chain::genesis_state_type::initial_bts_account_type::initial_authority )
FC_REFLECT_TYPENAME( graphene::chain::genesis_state_type::initial_bts_account_type::initial_cdd_vesting_policy )
FC_REFLECT_TYPENAME( graphene::chain::genesis_state_type::initial_bts_account_type::initial_linear_vesting_policy )
FC_REFLECT_TYPENAME( graphene::chain::genesis_state_type::initial_bts_account_type::initial_vesting_balance )
FC_REFLECT_TYPENAME( graphene::chain::genesis_state_type::initial_bts_account_type )
FC_REFLECT_TYPENAME( graphene::chain::genesis_state_type )

GRAPHENE_EXTERNAL_SERIALIZATION( extern, graphene::chain::genesis_state_type::initial_account_type )
GRAPHENE_EXTERNAL_SERIALIZATION( extern, graphene::chain::genesis_state_type::initial_asset_type )
GRAPHENE_EXTERNAL_SERIALIZATION( extern, graphene::chain::genesis_state_type::initial_asset_type::initial_collateral_position )
GRAPHENE_EXTERNAL_SERIALIZATION( extern, graphene::chain::genesis_state_type::initial_balance_type )
GRAPHENE_EXTERNAL_SERIALIZATION( extern, graphene::chain::genesis_state_type::initial_vesting_balance_type )
GRAPHENE_EXTERNAL_SERIALIZATION( extern, graphene::chain::genesis_state_type::initial_witness_type )
GRAPHENE_EXTERNAL_SERIALIZATION( extern, graphene::chain::genesis_state_type::initial_committee_member_type )
GRAPHENE_EXTERNAL_SERIALIZATION( extern, graphene::chain::genesis_state_type::initial_worker_type )
GRAPHENE_EXTERNAL_SERIALIZATION( extern, graphene::chain::genesis_state_type::initial_bts_account_type::initial_authority )
GRAPHENE_EXTERNAL_SERIALIZATION( extern, graphene::chain::genesis_state_type::initial_bts_account_type::initial_cdd_vesting_policy )
GRAPHENE_EXTERNAL_SERIALIZATION( extern, graphene::chain::genesis_state_type::initial_bts_account_type::initial_linear_vesting_policy )
GRAPHENE_EXTERNAL_SERIALIZATION( extern, graphene::chain::genesis_state_type::initial_bts_account_type::initial_vesting_balance )
GRAPHENE_EXTERNAL_SERIALIZATION( extern, graphene::chain::genesis_state_type::initial_bts_account_type )
GRAPHENE_EXTERNAL_SERIALIZATION( extern, graphene::chain::genesis_state_type )
