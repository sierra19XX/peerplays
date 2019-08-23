/*
 * Copyright (c) 2017 Cryptonomex, Inc., and contributors.
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

#include <boost/test/unit_test.hpp>

#include <graphene/app/database_api.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE(database_api_tests, database_fixture)

  BOOST_AUTO_TEST_CASE(is_registered) {
      try {
          /***
           * Arrange
           */
          auto nathan_private_key = generate_private_key("nathan");
          public_key_type nathan_public = nathan_private_key.get_public_key();

          auto dan_private_key = generate_private_key("dan");
          public_key_type dan_public = dan_private_key.get_public_key();

          auto unregistered_private_key = generate_private_key("unregistered");
          public_key_type unregistered_public = unregistered_private_key.get_public_key();


          /***
           * Act
           */
          create_account("dan", dan_private_key.get_public_key()).id;
          create_account("nathan", nathan_private_key.get_public_key()).id;
          // Unregistered key will not be registered with any account


          /***
           * Assert
           */
          graphene::app::database_api db_api(db);

          BOOST_CHECK(db_api.is_public_key_registered((string) nathan_public));
          BOOST_CHECK(db_api.is_public_key_registered((string) dan_public));
          BOOST_CHECK(!db_api.is_public_key_registered((string) unregistered_public));

      } FC_LOG_AND_RETHROW()
  }

BOOST_AUTO_TEST_CASE(get_account_limit_orders)
{ try {

   ACTORS((seller));

   const auto& bitcny = create_bitasset("CNY");
   const auto& core   = asset_id_type()(db);

   int64_t init_balance(10000000);
   transfer(committee_account, seller_id, asset(init_balance));
   BOOST_CHECK_EQUAL( 10000000, get_balance(seller, core) );

   /// Create 250 versatile orders
   for (size_t i = 0 ; i < 50 ; ++i) {
     BOOST_CHECK(create_sell_order(seller, core.amount(100), bitcny.amount(250)));
   }

   for (size_t i = 1 ; i < 101 ; ++i) {
     BOOST_CHECK(create_sell_order(seller, core.amount(100), bitcny.amount(250 + i)));
     BOOST_CHECK(create_sell_order(seller, core.amount(100), bitcny.amount(250 - i)));
   }

   graphene::app::database_api db_api(db);
   std::vector<limit_order_object> results;
   limit_order_object o;

   // query with no constraint, expected:
   // 1. up to 101 orders returned
   // 2. orders were sorted by price desendingly
   results = db_api.get_account_limit_orders(seller.name, "BTS", "CNY");
   BOOST_CHECK(results.size() == 101);
   for (size_t i = 0 ; i < results.size() - 1 ; ++i) {
     BOOST_CHECK(results[i].sell_price >= results[i+1].sell_price);
   }
   results.clear();

   // query with specified limit, expected:
   // 1. up to specified amount of orders returned
   // 2. orders were sorted by price desendingly
   results = db_api.get_account_limit_orders(seller.name, "BTS", "CNY", 50);
   results = db_api.get_account_limit_orders(seller.name, "BTS", "CNY", 50);
   BOOST_CHECK(results.size() == 50);
   for (size_t i = 0 ; i < results.size() - 1 ; ++i) {
     BOOST_CHECK(results[i].sell_price >= results[i+1].sell_price);
   }

   o = results.back();
   results.clear();

   // query with specified order id and limit, expected:
   // same as before, but also the first order's id equal to specified
   results = db_api.get_account_limit_orders(seller.name, "BTS", "CNY", 100,
       limit_order_id_type(o.id));
   BOOST_CHECK(results.size() == 100);
   BOOST_CHECK(results.front().id == o.id);
   for (size_t i = 0 ; i < results.size() - 1 ; ++i) {
     BOOST_CHECK(results[i].sell_price >= results[i+1].sell_price);
   }

   o = results.back();
   results.clear();

   // query with specified price and an not exists order id, expected:
   // 1. the canceled order should not exists in returned orders and first order's
   //    id should greater than specified
   // 2. returned orders sorted by price desendingly
   // 3. the first order's sell price equal to specified
   cancel_limit_order(o); // NOTE 1: this canceled order was in scope of the
                          // first created 50 orders, so with price 2.5 BTS/CNY
   results = db_api.get_account_limit_orders(seller.name, "BTS", "CNY", 101,
       limit_order_id_type(o.id), o.sell_price);
   BOOST_CHECK(results.size() <= 101);
   BOOST_CHECK(results.front().id > o.id);
   // NOTE 2: because of NOTE 1, here should be equal
   BOOST_CHECK(results.front().sell_price == o.sell_price);
   for (size_t i = 0 ; i < results.size() - 1 ; ++i) {
     BOOST_CHECK(results[i].sell_price >= results[i+1].sell_price);
   }

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()
