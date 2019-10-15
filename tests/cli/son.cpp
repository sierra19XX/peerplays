/*
 * Copyright (c) 2019 PBSA, and contributors.
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
#include "cli_fixture.hpp"

#include <boost/test/unit_test.hpp>

///////////////////////
// SON CLI
///////////////////////
BOOST_FIXTURE_TEST_SUITE(son_cli, cli_fixture)

BOOST_AUTO_TEST_CASE( create_sons )
{
   BOOST_TEST_MESSAGE("SON cli wallet tests begin");
   try
   {
      init_nathan();

      graphene::wallet::brain_key_info bki;
      signed_transaction create_tx;
      signed_transaction transfer_tx;
      signed_transaction upgrade_tx;
      account_object son1_before_upgrade, son1_after_upgrade;
      account_object son2_before_upgrade, son2_after_upgrade;
      son_object son1_obj;
      son_object son2_obj;

      // create son1account
      bki = con.wallet_api_ptr->suggest_brain_key();
      BOOST_CHECK(!bki.brain_priv_key.empty());
      create_tx = con.wallet_api_ptr->create_account_with_brain_key(
            bki.brain_priv_key, "son1account", "nathan", "nathan", true
      );
      // save the private key for this new account in the wallet file
      BOOST_CHECK(con.wallet_api_ptr->import_key("son1account", bki.wif_priv_key));
      con.wallet_api_ptr->save_wallet_file(con.wallet_filename);

      // attempt to give son1account some CORE tokens
      BOOST_TEST_MESSAGE("Transferring CORE tokens from Nathan to son1account");
      transfer_tx = con.wallet_api_ptr->transfer(
            "nathan", "son1account", "15000", "1.3.0", "Here are some CORE token for your new account", true
      );

      son1_before_upgrade = con.wallet_api_ptr->get_account("son1account");
      BOOST_CHECK(generate_block());

      // upgrade son1account
      BOOST_TEST_MESSAGE("Upgrading son1account to LTM");
      upgrade_tx = con.wallet_api_ptr->upgrade_account("son1account", true);
      son1_after_upgrade = con.wallet_api_ptr->get_account("son1account");

      // verify that the upgrade was successful
      BOOST_CHECK_PREDICATE(
            std::not_equal_to<uint32_t>(),
            (son1_before_upgrade.membership_expiration_date.sec_since_epoch())
                  (son1_after_upgrade.membership_expiration_date.sec_since_epoch())
      );
      BOOST_CHECK(son1_after_upgrade.is_lifetime_member());

      BOOST_CHECK(generate_block());
      // create son2account
      bki = con.wallet_api_ptr->suggest_brain_key();
      BOOST_CHECK(!bki.brain_priv_key.empty());
      create_tx = con.wallet_api_ptr->create_account_with_brain_key(
            bki.brain_priv_key, "son2account", "nathan", "nathan", true
      );
      // save the private key for this new account in the wallet file
      BOOST_CHECK(con.wallet_api_ptr->import_key("son2account", bki.wif_priv_key));
      con.wallet_api_ptr->save_wallet_file(con.wallet_filename);

      // attempt to give son2account some CORE tokens
      BOOST_TEST_MESSAGE("Transferring CORE tokens from Nathan to son2account");
      transfer_tx = con.wallet_api_ptr->transfer(
            "nathan", "son2account", "15000", "1.3.0", "Here are some CORE token for your new account", true
      );

      son2_before_upgrade = con.wallet_api_ptr->get_account("son2account");
      BOOST_CHECK(generate_block());

      // upgrade son2account
      BOOST_TEST_MESSAGE("Upgrading son2account to LTM");
      upgrade_tx = con.wallet_api_ptr->upgrade_account("son2account", true);
      son2_after_upgrade = con.wallet_api_ptr->get_account("son2account");

      // verify that the upgrade was successful
      BOOST_CHECK_PREDICATE(
            std::not_equal_to<uint32_t>(),
            (son2_before_upgrade.membership_expiration_date.sec_since_epoch())
                  (son2_after_upgrade.membership_expiration_date.sec_since_epoch())
      );
      BOOST_CHECK(son2_after_upgrade.is_lifetime_member());

      BOOST_CHECK(generate_block());

      // create 2 SONs
      BOOST_TEST_MESSAGE("Creating two SONs");
      create_tx = con.wallet_api_ptr->create_son("son1account", "http://son1", "true");
      create_tx = con.wallet_api_ptr->create_son("son2account", "http://son2", "true");
      BOOST_CHECK(generate_maintenance_block());

      son1_obj = con.wallet_api_ptr->get_son("son1account");
      BOOST_CHECK(son1_obj.son_account == con.wallet_api_ptr->get_account_id("son1account"));
      BOOST_CHECK_EQUAL(son1_obj.url, "http://son1");

      son2_obj = con.wallet_api_ptr->get_son("son2account");
      BOOST_CHECK(son2_obj.son_account == con.wallet_api_ptr->get_account_id("son2account"));
      BOOST_CHECK_EQUAL(son2_obj.url, "http://son2");

   } catch( fc::exception& e ) {
      BOOST_TEST_MESSAGE("SON cli wallet tests exception");
      edump((e.to_detail_string()));
      throw;
   }
   BOOST_TEST_MESSAGE("SON cli wallet tests end");
}

BOOST_AUTO_TEST_CASE( cli_update_son )
{
   try
   {
      BOOST_TEST_MESSAGE("Cli get_son and update_son Test");

      init_nathan();

      // create a new account
      graphene::wallet::brain_key_info bki = con.wallet_api_ptr->suggest_brain_key();
      BOOST_CHECK(!bki.brain_priv_key.empty());
      signed_transaction create_acct_tx = con.wallet_api_ptr->create_account_with_brain_key(
            bki.brain_priv_key, "sonmember", "nathan", "nathan", true
      );
      // save the private key for this new account in the wallet file
      BOOST_CHECK(con.wallet_api_ptr->import_key("sonmember", bki.wif_priv_key));
      con.wallet_api_ptr->save_wallet_file(con.wallet_filename);

      // attempt to give sonmember some CORE
      BOOST_TEST_MESSAGE("Transferring CORE from Nathan to sonmember");
      signed_transaction transfer_tx = con.wallet_api_ptr->transfer(
            "nathan", "sonmember", "100000", "1.3.0", "Here are some CORE token for your new account", true
      );

      BOOST_CHECK(generate_block());

      // upgrade sonmember account
      con.wallet_api_ptr->upgrade_account("sonmember", true);
      auto sonmember_acct = con.wallet_api_ptr->get_account("sonmember");
      BOOST_CHECK(sonmember_acct.is_lifetime_member());

      // create son
      con.wallet_api_ptr->create_son("sonmember", "http://sonmember", true);

      // get_son
      auto son_data = con.wallet_api_ptr->get_son("sonmember");
      BOOST_CHECK(son_data.url == "http://sonmember");
      BOOST_CHECK(son_data.son_account == sonmember_acct.get_id());

      // update SON
      con.wallet_api_ptr->update_son("sonmember", "http://sonmember_updated", "", true);
      son_data = con.wallet_api_ptr->get_son("sonmember");
      BOOST_CHECK(son_data.url == "http://sonmember_updated");

   } catch( fc::exception& e ) {
      edump((e.to_detail_string()));
      throw;
   }
}

BOOST_AUTO_TEST_CASE( son_voting )
{
   BOOST_TEST_MESSAGE("SON Vote cli wallet tests begin");
   try
   {
      INVOKE(create_sons);

      BOOST_TEST_MESSAGE("Voting for SONs");

      son_object son1_obj;
      son_object son2_obj;
      signed_transaction vote_son1_tx;
      signed_transaction vote_son2_tx;
      int son1_start_votes, son1_end_votes;
      int son2_start_votes, son2_end_votes;

      son1_obj = con.wallet_api_ptr->get_son("son1account");
      son1_start_votes = son1_obj.total_votes;
      son2_obj = con.wallet_api_ptr->get_son("son2account");
      son2_start_votes = son2_obj.total_votes;

      // Vote for a son1account
      BOOST_TEST_MESSAGE("Voting for son1account");
      vote_son1_tx = con.wallet_api_ptr->vote_for_son("nathan", "son1account", true, true);
      BOOST_CHECK(generate_maintenance_block());

      // Verify that the vote is there
      son1_obj = con.wallet_api_ptr->get_son("son1account");
      son1_end_votes = son1_obj.total_votes;
      BOOST_CHECK(son1_end_votes > son1_start_votes);

      // Vote for a son2account
      BOOST_TEST_MESSAGE("Voting for son2account");
      vote_son2_tx = con.wallet_api_ptr->vote_for_son("nathan", "son2account", true, true);
      BOOST_CHECK(generate_maintenance_block());

      // Verify that the vote is there
      son2_obj = con.wallet_api_ptr->get_son("son2account");
      son2_end_votes = son2_obj.total_votes;
      BOOST_CHECK(son2_end_votes > son2_start_votes);

      // Withdraw vote for a son1account
      BOOST_TEST_MESSAGE("Withdraw vote for a son1account");
      vote_son1_tx = con.wallet_api_ptr->vote_for_son("nathan", "son1account", false, true);
      BOOST_CHECK(generate_maintenance_block());

      // Verify that the vote is removed
      son1_obj = con.wallet_api_ptr->get_son("son1account");
      son1_end_votes = son1_obj.total_votes;
      BOOST_CHECK(son1_end_votes == son1_start_votes);

      // Withdraw vote for a son2account
      BOOST_TEST_MESSAGE("Withdraw vote for a son2account");
      vote_son2_tx = con.wallet_api_ptr->vote_for_son("nathan", "son2account", false, true);
      BOOST_CHECK(generate_maintenance_block());

      // Verify that the vote is removed
      son2_obj = con.wallet_api_ptr->get_son("son2account");
      son2_end_votes = son2_obj.total_votes;
      BOOST_CHECK(son2_end_votes == son2_start_votes);

   } catch( fc::exception& e ) {
      BOOST_TEST_MESSAGE("SON cli wallet tests exception");
      edump((e.to_detail_string()));
      throw;
   }
   BOOST_TEST_MESSAGE("SON Vote cli wallet tests end");
}

BOOST_AUTO_TEST_CASE( delete_son )
{
   BOOST_TEST_MESSAGE("SON delete cli wallet tests begin");
   try
   {
       INVOKE(create_sons);

       BOOST_TEST_MESSAGE("Deleting SONs");
       signed_transaction delete_tx;
       auto _db = app1->chain_database();
       BOOST_CHECK(_db->get_index_type<son_index>().indices().size() == 2);
       delete_tx = con.wallet_api_ptr->delete_son("son1account", "true");
       delete_tx = con.wallet_api_ptr->delete_son("son2account", "true");
       BOOST_CHECK(generate_maintenance_block());
       BOOST_CHECK(_db->get_index_type<son_index>().indices().size() == 0);
   } catch( fc::exception& e ) {
      BOOST_TEST_MESSAGE("SON cli wallet tests exception");
      edump((e.to_detail_string()));
      throw;
   }
   BOOST_TEST_MESSAGE("SON delete cli wallet tests end");
}


BOOST_FIXTURE_TEST_CASE( select_top_fifteen_sons, cli_fixture )
{
   BOOST_TEST_MESSAGE("SON cli wallet tests begin");
   try
   {
      INVOKE(upgrade_nathan_account);

      graphene::wallet::brain_key_info bki;
      signed_transaction create_tx;
      signed_transaction transfer_tx;
      signed_transaction upgrade_tx;
      signed_transaction vote_tx;
      account_object acc_before_upgrade, acc_after_upgrade;
      son_object son_obj;
      global_property_object gpo;

      // create sonaccount01
      bki = con.wallet_api_ptr->suggest_brain_key();
      BOOST_CHECK(!bki.brain_priv_key.empty());
      create_tx = con.wallet_api_ptr->create_account_with_brain_key(
            bki.brain_priv_key, "sonaccount01", "nathan", "nathan", true
      );
      // save the private key for this new account in the wallet file
      BOOST_CHECK(con.wallet_api_ptr->import_key("sonaccount01", bki.wif_priv_key));
      con.wallet_api_ptr->save_wallet_file(con.wallet_filename);

      // attempt to give sonaccount01 some CORE tokens
      BOOST_TEST_MESSAGE("Transferring CORE tokens from Nathan to sonaccount01");
      transfer_tx = con.wallet_api_ptr->transfer(
            "nathan", "sonaccount01", "15000", "1.3.0", "Here are some CORE token for your new account", true
      );

      acc_before_upgrade = con.wallet_api_ptr->get_account("sonaccount01");
      BOOST_CHECK(generate_block(app1));

      // upgrade sonaccount01
      BOOST_TEST_MESSAGE("Upgrading sonaccount01 to LTM");
      upgrade_tx = con.wallet_api_ptr->upgrade_account("sonaccount01", true);
      acc_after_upgrade = con.wallet_api_ptr->get_account("sonaccount01");

      // verify that the upgrade was successful
      BOOST_CHECK_PREDICATE(
            std::not_equal_to<uint32_t>(),
            (acc_before_upgrade.membership_expiration_date.sec_since_epoch())
                  (acc_after_upgrade.membership_expiration_date.sec_since_epoch())
      );
      BOOST_CHECK(acc_after_upgrade.is_lifetime_member());

      BOOST_CHECK(generate_block(app1));



      // create sonaccount02
      bki = con.wallet_api_ptr->suggest_brain_key();
      BOOST_CHECK(!bki.brain_priv_key.empty());
      create_tx = con.wallet_api_ptr->create_account_with_brain_key(
            bki.brain_priv_key, "sonaccount02", "nathan", "nathan", true
      );
      // save the private key for this new account in the wallet file
      BOOST_CHECK(con.wallet_api_ptr->import_key("sonaccount02", bki.wif_priv_key));
      con.wallet_api_ptr->save_wallet_file(con.wallet_filename);

      // attempt to give sonaccount02 some CORE tokens
      BOOST_TEST_MESSAGE("Transferring CORE tokens from Nathan to sonaccount02");
      transfer_tx = con.wallet_api_ptr->transfer(
            "nathan", "sonaccount02", "15000", "1.3.0", "Here are some CORE token for your new account", true
      );

      acc_before_upgrade = con.wallet_api_ptr->get_account("sonaccount02");
      BOOST_CHECK(generate_block(app1));

      // upgrade sonaccount02
      BOOST_TEST_MESSAGE("Upgrading sonaccount02 to LTM");
      upgrade_tx = con.wallet_api_ptr->upgrade_account("sonaccount02", true);
      acc_after_upgrade = con.wallet_api_ptr->get_account("sonaccount02");

      // verify that the upgrade was successful
      BOOST_CHECK_PREDICATE(
            std::not_equal_to<uint32_t>(),
            (acc_before_upgrade.membership_expiration_date.sec_since_epoch())
                  (acc_after_upgrade.membership_expiration_date.sec_since_epoch())
      );
      BOOST_CHECK(acc_after_upgrade.is_lifetime_member());

      BOOST_CHECK(generate_block(app1));



      // create sonaccount03
      bki = con.wallet_api_ptr->suggest_brain_key();
      BOOST_CHECK(!bki.brain_priv_key.empty());
      create_tx = con.wallet_api_ptr->create_account_with_brain_key(
            bki.brain_priv_key, "sonaccount03", "nathan", "nathan", true
      );
      // save the private key for this new account in the wallet file
      BOOST_CHECK(con.wallet_api_ptr->import_key("sonaccount03", bki.wif_priv_key));
      con.wallet_api_ptr->save_wallet_file(con.wallet_filename);

      // attempt to give sonaccount03 some CORE tokens
      BOOST_TEST_MESSAGE("Transferring CORE tokens from Nathan to sonaccount03");
      transfer_tx = con.wallet_api_ptr->transfer(
            "nathan", "sonaccount03", "15000", "1.3.0", "Here are some CORE token for your new account", true
      );

      acc_before_upgrade = con.wallet_api_ptr->get_account("sonaccount03");
      BOOST_CHECK(generate_block(app1));

      // upgrade sonaccount03
      BOOST_TEST_MESSAGE("Upgrading sonaccount03 to LTM");
      upgrade_tx = con.wallet_api_ptr->upgrade_account("sonaccount03", true);
      acc_after_upgrade = con.wallet_api_ptr->get_account("sonaccount03");

      // verify that the upgrade was successful
      BOOST_CHECK_PREDICATE(
            std::not_equal_to<uint32_t>(),
            (acc_before_upgrade.membership_expiration_date.sec_since_epoch())
                  (acc_after_upgrade.membership_expiration_date.sec_since_epoch())
      );
      BOOST_CHECK(acc_after_upgrade.is_lifetime_member());

      BOOST_CHECK(generate_block(app1));



      // create sonaccount04
      bki = con.wallet_api_ptr->suggest_brain_key();
      BOOST_CHECK(!bki.brain_priv_key.empty());
      create_tx = con.wallet_api_ptr->create_account_with_brain_key(
            bki.brain_priv_key, "sonaccount04", "nathan", "nathan", true
      );
      // save the private key for this new account in the wallet file
      BOOST_CHECK(con.wallet_api_ptr->import_key("sonaccount04", bki.wif_priv_key));
      con.wallet_api_ptr->save_wallet_file(con.wallet_filename);

      // attempt to give sonaccount04 some CORE tokens
      BOOST_TEST_MESSAGE("Transferring CORE tokens from Nathan to sonaccount04");
      transfer_tx = con.wallet_api_ptr->transfer(
            "nathan", "sonaccount04", "15000", "1.3.0", "Here are some CORE token for your new account", true
      );

      acc_before_upgrade = con.wallet_api_ptr->get_account("sonaccount04");
      BOOST_CHECK(generate_block(app1));

      // upgrade sonaccount04
      BOOST_TEST_MESSAGE("Upgrading sonaccount04 to LTM");
      upgrade_tx = con.wallet_api_ptr->upgrade_account("sonaccount04", true);
      acc_after_upgrade = con.wallet_api_ptr->get_account("sonaccount04");

      // verify that the upgrade was successful
      BOOST_CHECK_PREDICATE(
            std::not_equal_to<uint32_t>(),
            (acc_before_upgrade.membership_expiration_date.sec_since_epoch())
                  (acc_after_upgrade.membership_expiration_date.sec_since_epoch())
      );
      BOOST_CHECK(acc_after_upgrade.is_lifetime_member());

      BOOST_CHECK(generate_block(app1));



      // create sonaccount05
      bki = con.wallet_api_ptr->suggest_brain_key();
      BOOST_CHECK(!bki.brain_priv_key.empty());
      create_tx = con.wallet_api_ptr->create_account_with_brain_key(
            bki.brain_priv_key, "sonaccount05", "nathan", "nathan", true
      );
      // save the private key for this new account in the wallet file
      BOOST_CHECK(con.wallet_api_ptr->import_key("sonaccount05", bki.wif_priv_key));
      con.wallet_api_ptr->save_wallet_file(con.wallet_filename);

      // attempt to give sonaccount05 some CORE tokens
      BOOST_TEST_MESSAGE("Transferring CORE tokens from Nathan to sonaccount05");
      transfer_tx = con.wallet_api_ptr->transfer(
            "nathan", "sonaccount05", "15000", "1.3.0", "Here are some CORE token for your new account", true
      );

      acc_before_upgrade = con.wallet_api_ptr->get_account("sonaccount05");
      BOOST_CHECK(generate_block(app1));

      // upgrade sonaccount05
      BOOST_TEST_MESSAGE("Upgrading sonaccount05 to LTM");
      upgrade_tx = con.wallet_api_ptr->upgrade_account("sonaccount05", true);
      acc_after_upgrade = con.wallet_api_ptr->get_account("sonaccount05");

      // verify that the upgrade was successful
      BOOST_CHECK_PREDICATE(
            std::not_equal_to<uint32_t>(),
            (acc_before_upgrade.membership_expiration_date.sec_since_epoch())
                  (acc_after_upgrade.membership_expiration_date.sec_since_epoch())
      );
      BOOST_CHECK(acc_after_upgrade.is_lifetime_member());

      BOOST_CHECK(generate_block(app1));



      // create sonaccount06
      bki = con.wallet_api_ptr->suggest_brain_key();
      BOOST_CHECK(!bki.brain_priv_key.empty());
      create_tx = con.wallet_api_ptr->create_account_with_brain_key(
            bki.brain_priv_key, "sonaccount06", "nathan", "nathan", true
      );
      // save the private key for this new account in the wallet file
      BOOST_CHECK(con.wallet_api_ptr->import_key("sonaccount06", bki.wif_priv_key));
      con.wallet_api_ptr->save_wallet_file(con.wallet_filename);

      // attempt to give sonaccount06 some CORE tokens
      BOOST_TEST_MESSAGE("Transferring CORE tokens from Nathan to sonaccount06");
      transfer_tx = con.wallet_api_ptr->transfer(
            "nathan", "sonaccount06", "15000", "1.3.0", "Here are some CORE token for your new account", true
      );

      acc_before_upgrade = con.wallet_api_ptr->get_account("sonaccount06");
      BOOST_CHECK(generate_block(app1));

      // upgrade sonaccount06
      BOOST_TEST_MESSAGE("Upgrading sonaccount06 to LTM");
      upgrade_tx = con.wallet_api_ptr->upgrade_account("sonaccount06", true);
      acc_after_upgrade = con.wallet_api_ptr->get_account("sonaccount06");

      // verify that the upgrade was successful
      BOOST_CHECK_PREDICATE(
            std::not_equal_to<uint32_t>(),
            (acc_before_upgrade.membership_expiration_date.sec_since_epoch())
                  (acc_after_upgrade.membership_expiration_date.sec_since_epoch())
      );
      BOOST_CHECK(acc_after_upgrade.is_lifetime_member());

      BOOST_CHECK(generate_block(app1));



      // create sonaccount07
      bki = con.wallet_api_ptr->suggest_brain_key();
      BOOST_CHECK(!bki.brain_priv_key.empty());
      create_tx = con.wallet_api_ptr->create_account_with_brain_key(
            bki.brain_priv_key, "sonaccount07", "nathan", "nathan", true
      );
      // save the private key for this new account in the wallet file
      BOOST_CHECK(con.wallet_api_ptr->import_key("sonaccount07", bki.wif_priv_key));
      con.wallet_api_ptr->save_wallet_file(con.wallet_filename);

      // attempt to give sonaccount07 some CORE tokens
      BOOST_TEST_MESSAGE("Transferring CORE tokens from Nathan to sonaccount07");
      transfer_tx = con.wallet_api_ptr->transfer(
            "nathan", "sonaccount07", "15000", "1.3.0", "Here are some CORE token for your new account", true
      );

      acc_before_upgrade = con.wallet_api_ptr->get_account("sonaccount07");
      BOOST_CHECK(generate_block(app1));

      // upgrade sonaccount07
      BOOST_TEST_MESSAGE("Upgrading sonaccount07 to LTM");
      upgrade_tx = con.wallet_api_ptr->upgrade_account("sonaccount07", true);
      acc_after_upgrade = con.wallet_api_ptr->get_account("sonaccount07");

      // verify that the upgrade was successful
      BOOST_CHECK_PREDICATE(
            std::not_equal_to<uint32_t>(),
            (acc_before_upgrade.membership_expiration_date.sec_since_epoch())
                  (acc_after_upgrade.membership_expiration_date.sec_since_epoch())
      );
      BOOST_CHECK(acc_after_upgrade.is_lifetime_member());

      BOOST_CHECK(generate_block(app1));



      // create sonaccount08
      bki = con.wallet_api_ptr->suggest_brain_key();
      BOOST_CHECK(!bki.brain_priv_key.empty());
      create_tx = con.wallet_api_ptr->create_account_with_brain_key(
            bki.brain_priv_key, "sonaccount08", "nathan", "nathan", true
      );
      // save the private key for this new account in the wallet file
      BOOST_CHECK(con.wallet_api_ptr->import_key("sonaccount08", bki.wif_priv_key));
      con.wallet_api_ptr->save_wallet_file(con.wallet_filename);

      // attempt to give sonaccount08 some CORE tokens
      BOOST_TEST_MESSAGE("Transferring CORE tokens from Nathan to sonaccount08");
      transfer_tx = con.wallet_api_ptr->transfer(
            "nathan", "sonaccount08", "15000", "1.3.0", "Here are some CORE token for your new account", true
      );

      acc_before_upgrade = con.wallet_api_ptr->get_account("sonaccount08");
      BOOST_CHECK(generate_block(app1));

      // upgrade sonaccount08
      BOOST_TEST_MESSAGE("Upgrading sonaccount08 to LTM");
      upgrade_tx = con.wallet_api_ptr->upgrade_account("sonaccount08", true);
      acc_after_upgrade = con.wallet_api_ptr->get_account("sonaccount08");

      // verify that the upgrade was successful
      BOOST_CHECK_PREDICATE(
            std::not_equal_to<uint32_t>(),
            (acc_before_upgrade.membership_expiration_date.sec_since_epoch())
                  (acc_after_upgrade.membership_expiration_date.sec_since_epoch())
      );
      BOOST_CHECK(acc_after_upgrade.is_lifetime_member());

      BOOST_CHECK(generate_block(app1));



      // create sonaccount09
      bki = con.wallet_api_ptr->suggest_brain_key();
      BOOST_CHECK(!bki.brain_priv_key.empty());
      create_tx = con.wallet_api_ptr->create_account_with_brain_key(
            bki.brain_priv_key, "sonaccount09", "nathan", "nathan", true
      );
      // save the private key for this new account in the wallet file
      BOOST_CHECK(con.wallet_api_ptr->import_key("sonaccount09", bki.wif_priv_key));
      con.wallet_api_ptr->save_wallet_file(con.wallet_filename);

      // attempt to give sonaccount09 some CORE tokens
      BOOST_TEST_MESSAGE("Transferring CORE tokens from Nathan to sonaccount09");
      transfer_tx = con.wallet_api_ptr->transfer(
            "nathan", "sonaccount09", "15000", "1.3.0", "Here are some CORE token for your new account", true
      );

      acc_before_upgrade = con.wallet_api_ptr->get_account("sonaccount09");
      BOOST_CHECK(generate_block(app1));

      // upgrade sonaccount09
      BOOST_TEST_MESSAGE("Upgrading sonaccount09 to LTM");
      upgrade_tx = con.wallet_api_ptr->upgrade_account("sonaccount09", true);
      acc_after_upgrade = con.wallet_api_ptr->get_account("sonaccount09");

      // verify that the upgrade was successful
      BOOST_CHECK_PREDICATE(
            std::not_equal_to<uint32_t>(),
            (acc_before_upgrade.membership_expiration_date.sec_since_epoch())
                  (acc_after_upgrade.membership_expiration_date.sec_since_epoch())
      );
      BOOST_CHECK(acc_after_upgrade.is_lifetime_member());

      BOOST_CHECK(generate_block(app1));



      // create sonaccount10
      bki = con.wallet_api_ptr->suggest_brain_key();
      BOOST_CHECK(!bki.brain_priv_key.empty());
      create_tx = con.wallet_api_ptr->create_account_with_brain_key(
            bki.brain_priv_key, "sonaccount10", "nathan", "nathan", true
      );
      // save the private key for this new account in the wallet file
      BOOST_CHECK(con.wallet_api_ptr->import_key("sonaccount10", bki.wif_priv_key));
      con.wallet_api_ptr->save_wallet_file(con.wallet_filename);

      // attempt to give sonaccount10 some CORE tokens
      BOOST_TEST_MESSAGE("Transferring CORE tokens from Nathan to sonaccount10");
      transfer_tx = con.wallet_api_ptr->transfer(
            "nathan", "sonaccount10", "15000", "1.3.0", "Here are some CORE token for your new account", true
      );

      acc_before_upgrade = con.wallet_api_ptr->get_account("sonaccount10");
      BOOST_CHECK(generate_block(app1));

      // upgrade sonaccount10
      BOOST_TEST_MESSAGE("Upgrading sonaccount10 to LTM");
      upgrade_tx = con.wallet_api_ptr->upgrade_account("sonaccount10", true);
      acc_after_upgrade = con.wallet_api_ptr->get_account("sonaccount10");

      // verify that the upgrade was successful
      BOOST_CHECK_PREDICATE(
            std::not_equal_to<uint32_t>(),
            (acc_before_upgrade.membership_expiration_date.sec_since_epoch())
                  (acc_after_upgrade.membership_expiration_date.sec_since_epoch())
      );
      BOOST_CHECK(acc_after_upgrade.is_lifetime_member());

      BOOST_CHECK(generate_block(app1));



      // create sonaccount11
      bki = con.wallet_api_ptr->suggest_brain_key();
      BOOST_CHECK(!bki.brain_priv_key.empty());
      create_tx = con.wallet_api_ptr->create_account_with_brain_key(
            bki.brain_priv_key, "sonaccount11", "nathan", "nathan", true
      );
      // save the private key for this new account in the wallet file
      BOOST_CHECK(con.wallet_api_ptr->import_key("sonaccount11", bki.wif_priv_key));
      con.wallet_api_ptr->save_wallet_file(con.wallet_filename);

      // attempt to give sonaccount11 some CORE tokens
      BOOST_TEST_MESSAGE("Transferring CORE tokens from Nathan to sonaccount11");
      transfer_tx = con.wallet_api_ptr->transfer(
            "nathan", "sonaccount11", "15000", "1.3.0", "Here are some CORE token for your new account", true
      );

      acc_before_upgrade = con.wallet_api_ptr->get_account("sonaccount11");
      BOOST_CHECK(generate_block(app1));

      // upgrade sonaccount11
      BOOST_TEST_MESSAGE("Upgrading sonaccount11 to LTM");
      upgrade_tx = con.wallet_api_ptr->upgrade_account("sonaccount11", true);
      acc_after_upgrade = con.wallet_api_ptr->get_account("sonaccount11");

      // verify that the upgrade was successful
      BOOST_CHECK_PREDICATE(
            std::not_equal_to<uint32_t>(),
            (acc_before_upgrade.membership_expiration_date.sec_since_epoch())
                  (acc_after_upgrade.membership_expiration_date.sec_since_epoch())
      );
      BOOST_CHECK(acc_after_upgrade.is_lifetime_member());

      BOOST_CHECK(generate_block(app1));



      // create sonaccount12
      bki = con.wallet_api_ptr->suggest_brain_key();
      BOOST_CHECK(!bki.brain_priv_key.empty());
      create_tx = con.wallet_api_ptr->create_account_with_brain_key(
            bki.brain_priv_key, "sonaccount12", "nathan", "nathan", true
      );
      // save the private key for this new account in the wallet file
      BOOST_CHECK(con.wallet_api_ptr->import_key("sonaccount12", bki.wif_priv_key));
      con.wallet_api_ptr->save_wallet_file(con.wallet_filename);

      // attempt to give sonaccount12 some CORE tokens
      BOOST_TEST_MESSAGE("Transferring CORE tokens from Nathan to sonaccount12");
      transfer_tx = con.wallet_api_ptr->transfer(
            "nathan", "sonaccount12", "15000", "1.3.0", "Here are some CORE token for your new account", true
      );

      acc_before_upgrade = con.wallet_api_ptr->get_account("sonaccount12");
      BOOST_CHECK(generate_block(app1));

      // upgrade sonaccount12
      BOOST_TEST_MESSAGE("Upgrading sonaccount12 to LTM");
      upgrade_tx = con.wallet_api_ptr->upgrade_account("sonaccount12", true);
      acc_after_upgrade = con.wallet_api_ptr->get_account("sonaccount12");

      // verify that the upgrade was successful
      BOOST_CHECK_PREDICATE(
            std::not_equal_to<uint32_t>(),
            (acc_before_upgrade.membership_expiration_date.sec_since_epoch())
                  (acc_after_upgrade.membership_expiration_date.sec_since_epoch())
      );
      BOOST_CHECK(acc_after_upgrade.is_lifetime_member());

      BOOST_CHECK(generate_block(app1));



      // create sonaccount13
      bki = con.wallet_api_ptr->suggest_brain_key();
      BOOST_CHECK(!bki.brain_priv_key.empty());
      create_tx = con.wallet_api_ptr->create_account_with_brain_key(
            bki.brain_priv_key, "sonaccount13", "nathan", "nathan", true
      );
      // save the private key for this new account in the wallet file
      BOOST_CHECK(con.wallet_api_ptr->import_key("sonaccount13", bki.wif_priv_key));
      con.wallet_api_ptr->save_wallet_file(con.wallet_filename);

      // attempt to give sonaccount13 some CORE tokens
      BOOST_TEST_MESSAGE("Transferring CORE tokens from Nathan to sonaccount13");
      transfer_tx = con.wallet_api_ptr->transfer(
            "nathan", "sonaccount13", "15000", "1.3.0", "Here are some CORE token for your new account", true
      );

      acc_before_upgrade = con.wallet_api_ptr->get_account("sonaccount13");
      BOOST_CHECK(generate_block(app1));

      // upgrade sonaccount13
      BOOST_TEST_MESSAGE("Upgrading sonaccount13 to LTM");
      upgrade_tx = con.wallet_api_ptr->upgrade_account("sonaccount13", true);
      acc_after_upgrade = con.wallet_api_ptr->get_account("sonaccount13");

      // verify that the upgrade was successful
      BOOST_CHECK_PREDICATE(
            std::not_equal_to<uint32_t>(),
            (acc_before_upgrade.membership_expiration_date.sec_since_epoch())
                  (acc_after_upgrade.membership_expiration_date.sec_since_epoch())
      );
      BOOST_CHECK(acc_after_upgrade.is_lifetime_member());

      BOOST_CHECK(generate_block(app1));



      // create sonaccount14
      bki = con.wallet_api_ptr->suggest_brain_key();
      BOOST_CHECK(!bki.brain_priv_key.empty());
      create_tx = con.wallet_api_ptr->create_account_with_brain_key(
            bki.brain_priv_key, "sonaccount14", "nathan", "nathan", true
      );
      // save the private key for this new account in the wallet file
      BOOST_CHECK(con.wallet_api_ptr->import_key("sonaccount14", bki.wif_priv_key));
      con.wallet_api_ptr->save_wallet_file(con.wallet_filename);

      // attempt to give sonaccount14 some CORE tokens
      BOOST_TEST_MESSAGE("Transferring CORE tokens from Nathan to sonaccount14");
      transfer_tx = con.wallet_api_ptr->transfer(
            "nathan", "sonaccount14", "15000", "1.3.0", "Here are some CORE token for your new account", true
      );

      acc_before_upgrade = con.wallet_api_ptr->get_account("sonaccount14");
      BOOST_CHECK(generate_block(app1));

      // upgrade sonaccount14
      BOOST_TEST_MESSAGE("Upgrading sonaccount14 to LTM");
      upgrade_tx = con.wallet_api_ptr->upgrade_account("sonaccount14", true);
      acc_after_upgrade = con.wallet_api_ptr->get_account("sonaccount14");

      // verify that the upgrade was successful
      BOOST_CHECK_PREDICATE(
            std::not_equal_to<uint32_t>(),
            (acc_before_upgrade.membership_expiration_date.sec_since_epoch())
                  (acc_after_upgrade.membership_expiration_date.sec_since_epoch())
      );
      BOOST_CHECK(acc_after_upgrade.is_lifetime_member());

      BOOST_CHECK(generate_block(app1));



      // create sonaccount15
      bki = con.wallet_api_ptr->suggest_brain_key();
      BOOST_CHECK(!bki.brain_priv_key.empty());
      create_tx = con.wallet_api_ptr->create_account_with_brain_key(
            bki.brain_priv_key, "sonaccount15", "nathan", "nathan", true
      );
      // save the private key for this new account in the wallet file
      BOOST_CHECK(con.wallet_api_ptr->import_key("sonaccount15", bki.wif_priv_key));
      con.wallet_api_ptr->save_wallet_file(con.wallet_filename);

      // attempt to give sonaccount15 some CORE tokens
      BOOST_TEST_MESSAGE("Transferring CORE tokens from Nathan to sonaccount15");
      transfer_tx = con.wallet_api_ptr->transfer(
            "nathan", "sonaccount15", "15000", "1.3.0", "Here are some CORE token for your new account", true
      );

      acc_before_upgrade = con.wallet_api_ptr->get_account("sonaccount15");
      BOOST_CHECK(generate_block(app1));

      // upgrade sonaccount15
      BOOST_TEST_MESSAGE("Upgrading sonaccount15 to LTM");
      upgrade_tx = con.wallet_api_ptr->upgrade_account("sonaccount15", true);
      acc_after_upgrade = con.wallet_api_ptr->get_account("sonaccount15");

      // verify that the upgrade was successful
      BOOST_CHECK_PREDICATE(
            std::not_equal_to<uint32_t>(),
            (acc_before_upgrade.membership_expiration_date.sec_since_epoch())
                  (acc_after_upgrade.membership_expiration_date.sec_since_epoch())
      );
      BOOST_CHECK(acc_after_upgrade.is_lifetime_member());

      BOOST_CHECK(generate_block(app1));



      // create sonaccount16
      bki = con.wallet_api_ptr->suggest_brain_key();
      BOOST_CHECK(!bki.brain_priv_key.empty());
      create_tx = con.wallet_api_ptr->create_account_with_brain_key(
            bki.brain_priv_key, "sonaccount16", "nathan", "nathan", true
      );
      // save the private key for this new account in the wallet file
      BOOST_CHECK(con.wallet_api_ptr->import_key("sonaccount16", bki.wif_priv_key));
      con.wallet_api_ptr->save_wallet_file(con.wallet_filename);

      // attempt to give sonaccount16 some CORE tokens
      BOOST_TEST_MESSAGE("Transferring CORE tokens from Nathan to sonaccount16");
      transfer_tx = con.wallet_api_ptr->transfer(
            "nathan", "sonaccount16", "15000", "1.3.0", "Here are some CORE token for your new account", true
      );

      acc_before_upgrade = con.wallet_api_ptr->get_account("sonaccount16");
      BOOST_CHECK(generate_block(app1));

      // upgrade sonaccount16
      BOOST_TEST_MESSAGE("Upgrading sonaccount16 to LTM");
      upgrade_tx = con.wallet_api_ptr->upgrade_account("sonaccount16", true);
      acc_after_upgrade = con.wallet_api_ptr->get_account("sonaccount16");

      // verify that the upgrade was successful
      BOOST_CHECK_PREDICATE(
            std::not_equal_to<uint32_t>(),
            (acc_before_upgrade.membership_expiration_date.sec_since_epoch())
                  (acc_after_upgrade.membership_expiration_date.sec_since_epoch())
      );
      BOOST_CHECK(acc_after_upgrade.is_lifetime_member());

      BOOST_CHECK(generate_block(app1));



      // create 16 SONs
      BOOST_TEST_MESSAGE("Creating 16 SONs");
      create_tx = con.wallet_api_ptr->create_son("sonaccount01", "http://son01", "true");
      create_tx = con.wallet_api_ptr->create_son("sonaccount02", "http://son02", "true");
      create_tx = con.wallet_api_ptr->create_son("sonaccount03", "http://son03", "true");
      create_tx = con.wallet_api_ptr->create_son("sonaccount04", "http://son04", "true");
      create_tx = con.wallet_api_ptr->create_son("sonaccount05", "http://son05", "true");
      create_tx = con.wallet_api_ptr->create_son("sonaccount06", "http://son06", "true");
      create_tx = con.wallet_api_ptr->create_son("sonaccount07", "http://son07", "true");
      create_tx = con.wallet_api_ptr->create_son("sonaccount08", "http://son08", "true");
      create_tx = con.wallet_api_ptr->create_son("sonaccount09", "http://son09", "true");
      create_tx = con.wallet_api_ptr->create_son("sonaccount10", "http://son10", "true");
      create_tx = con.wallet_api_ptr->create_son("sonaccount11", "http://son11", "true");
      create_tx = con.wallet_api_ptr->create_son("sonaccount12", "http://son12", "true");
      create_tx = con.wallet_api_ptr->create_son("sonaccount13", "http://son13", "true");
      create_tx = con.wallet_api_ptr->create_son("sonaccount14", "http://son14", "true");
      create_tx = con.wallet_api_ptr->create_son("sonaccount15", "http://son15", "true");
      create_tx = con.wallet_api_ptr->create_son("sonaccount16", "http://son16", "true");
      BOOST_CHECK(generate_maintenance_block(app1));

      son_obj = con.wallet_api_ptr->get_son("sonaccount01");
      BOOST_CHECK(son_obj.son_account == con.wallet_api_ptr->get_account_id("sonaccount01"));
      BOOST_CHECK_EQUAL(son_obj.url, "http://son01");
      son_obj = con.wallet_api_ptr->get_son("sonaccount02");
      BOOST_CHECK(son_obj.son_account == con.wallet_api_ptr->get_account_id("sonaccount02"));
      BOOST_CHECK_EQUAL(son_obj.url, "http://son02");
      son_obj = con.wallet_api_ptr->get_son("sonaccount03");
      BOOST_CHECK(son_obj.son_account == con.wallet_api_ptr->get_account_id("sonaccount03"));
      BOOST_CHECK_EQUAL(son_obj.url, "http://son03");
      son_obj = con.wallet_api_ptr->get_son("sonaccount04");
      BOOST_CHECK(son_obj.son_account == con.wallet_api_ptr->get_account_id("sonaccount04"));
      BOOST_CHECK_EQUAL(son_obj.url, "http://son04");
      son_obj = con.wallet_api_ptr->get_son("sonaccount05");
      BOOST_CHECK(son_obj.son_account == con.wallet_api_ptr->get_account_id("sonaccount05"));
      BOOST_CHECK_EQUAL(son_obj.url, "http://son05");
      son_obj = con.wallet_api_ptr->get_son("sonaccount06");
      BOOST_CHECK(son_obj.son_account == con.wallet_api_ptr->get_account_id("sonaccount06"));
      BOOST_CHECK_EQUAL(son_obj.url, "http://son06");
      son_obj = con.wallet_api_ptr->get_son("sonaccount07");
      BOOST_CHECK(son_obj.son_account == con.wallet_api_ptr->get_account_id("sonaccount07"));
      BOOST_CHECK_EQUAL(son_obj.url, "http://son07");
      son_obj = con.wallet_api_ptr->get_son("sonaccount08");
      BOOST_CHECK(son_obj.son_account == con.wallet_api_ptr->get_account_id("sonaccount08"));
      BOOST_CHECK_EQUAL(son_obj.url, "http://son08");
      son_obj = con.wallet_api_ptr->get_son("sonaccount09");
      BOOST_CHECK(son_obj.son_account == con.wallet_api_ptr->get_account_id("sonaccount09"));
      BOOST_CHECK_EQUAL(son_obj.url, "http://son09");
      son_obj = con.wallet_api_ptr->get_son("sonaccount10");
      BOOST_CHECK(son_obj.son_account == con.wallet_api_ptr->get_account_id("sonaccount10"));
      BOOST_CHECK_EQUAL(son_obj.url, "http://son10");
      son_obj = con.wallet_api_ptr->get_son("sonaccount11");
      BOOST_CHECK(son_obj.son_account == con.wallet_api_ptr->get_account_id("sonaccount11"));
      BOOST_CHECK_EQUAL(son_obj.url, "http://son11");
      son_obj = con.wallet_api_ptr->get_son("sonaccount12");
      BOOST_CHECK(son_obj.son_account == con.wallet_api_ptr->get_account_id("sonaccount12"));
      BOOST_CHECK_EQUAL(son_obj.url, "http://son12");
      son_obj = con.wallet_api_ptr->get_son("sonaccount13");
      BOOST_CHECK(son_obj.son_account == con.wallet_api_ptr->get_account_id("sonaccount13"));
      BOOST_CHECK_EQUAL(son_obj.url, "http://son13");
      son_obj = con.wallet_api_ptr->get_son("sonaccount14");
      BOOST_CHECK(son_obj.son_account == con.wallet_api_ptr->get_account_id("sonaccount14"));
      BOOST_CHECK_EQUAL(son_obj.url, "http://son14");
      son_obj = con.wallet_api_ptr->get_son("sonaccount15");
      BOOST_CHECK(son_obj.son_account == con.wallet_api_ptr->get_account_id("sonaccount15"));
      BOOST_CHECK_EQUAL(son_obj.url, "http://son15");
      son_obj = con.wallet_api_ptr->get_son("sonaccount16");
      BOOST_CHECK(son_obj.son_account == con.wallet_api_ptr->get_account_id("sonaccount16"));
      BOOST_CHECK_EQUAL(son_obj.url, "http://son16");



      BOOST_TEST_MESSAGE("Voting for SONs");
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount01", "sonaccount01", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount02", "sonaccount02", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount03", "sonaccount03", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount04", "sonaccount04", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount05", "sonaccount05", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount06", "sonaccount06", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount07", "sonaccount07", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount08", "sonaccount08", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount09", "sonaccount09", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount10", "sonaccount10", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount11", "sonaccount11", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount12", "sonaccount12", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount13", "sonaccount13", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount14", "sonaccount14", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount15", "sonaccount15", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount16", "sonaccount16", true, true);
      BOOST_CHECK(generate_maintenance_block(app1));
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount01", "sonaccount02", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount02", "sonaccount03", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount03", "sonaccount04", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount04", "sonaccount05", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount05", "sonaccount06", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount06", "sonaccount07", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount07", "sonaccount08", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount08", "sonaccount09", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount09", "sonaccount10", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount10", "sonaccount11", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount11", "sonaccount12", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount12", "sonaccount13", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount13", "sonaccount14", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount14", "sonaccount15", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount15", "sonaccount16", true, true);
      gpo = con.wallet_api_ptr->get_global_properties();
      BOOST_TEST_MESSAGE("gpo: " << gpo.active_sons.size());
      BOOST_CHECK(generate_maintenance_block(app1));
      gpo = con.wallet_api_ptr->get_global_properties();
      BOOST_TEST_MESSAGE("gpo: " << gpo.active_sons.size());
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount03", "sonaccount01", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount04", "sonaccount02", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount05", "sonaccount03", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount06", "sonaccount04", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount07", "sonaccount05", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount08", "sonaccount06", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount09", "sonaccount07", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount10", "sonaccount08", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount11", "sonaccount09", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount12", "sonaccount10", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount13", "sonaccount11", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount14", "sonaccount12", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount15", "sonaccount13", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount16", "sonaccount14", true, true);
      gpo = con.wallet_api_ptr->get_global_properties();
      BOOST_TEST_MESSAGE("gpo: " << gpo.active_sons.size());
      BOOST_CHECK(generate_maintenance_block(app1));
      gpo = con.wallet_api_ptr->get_global_properties();
      BOOST_TEST_MESSAGE("gpo: " << gpo.active_sons.size());
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount04", "sonaccount01", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount05", "sonaccount02", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount06", "sonaccount03", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount07", "sonaccount04", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount08", "sonaccount05", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount09", "sonaccount06", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount10", "sonaccount07", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount11", "sonaccount08", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount12", "sonaccount09", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount13", "sonaccount10", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount14", "sonaccount11", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount15", "sonaccount12", true, true);
      vote_tx = con.wallet_api_ptr->vote_for_son("sonaccount16", "sonaccount13", true, true);
      gpo = con.wallet_api_ptr->get_global_properties();
      BOOST_TEST_MESSAGE("gpo: " << gpo.active_sons.size());
      BOOST_CHECK(generate_maintenance_block(app1));

      BOOST_CHECK(gpo.active_sons.size() == 15);

   } catch( fc::exception& e ) {
      BOOST_TEST_MESSAGE("SON cli wallet tests exception");
      edump((e.to_detail_string()));
      throw;
   }
   BOOST_TEST_MESSAGE("SON cli wallet tests end");
}

   BOOST_AUTO_TEST_CASE( update_son_votes_test )
   {
      BOOST_TEST_MESSAGE("SON update_son_votes cli wallet tests begin");
      try
      {
         INVOKE(create_sons);

         BOOST_TEST_MESSAGE("Vote for 2 accounts with update_son_votes");

         son_object son1_obj;
         son_object son2_obj;
         int son1_start_votes, son1_end_votes;
         int son2_start_votes, son2_end_votes;

         // Get votes at start
         son1_obj = con.wallet_api_ptr->get_son("son1account");
         son1_start_votes = son1_obj.total_votes;
         son2_obj = con.wallet_api_ptr->get_son("son2account");
         son2_start_votes = son2_obj.total_votes;

         std::vector<std::string> accepted;
         std::vector<std::string> rejected;
         signed_transaction update_votes_tx;

         // Vote for both SONs
         accepted.clear();
         rejected.clear();
         accepted.push_back("son1account");
         accepted.push_back("son2account");
         update_votes_tx = con.wallet_api_ptr->update_son_votes("nathan", accepted,
                                                                rejected, 15, true);
         BOOST_CHECK(generate_block());
         BOOST_CHECK(generate_maintenance_block());

         // Verify the votes
         son1_obj = con.wallet_api_ptr->get_son("son1account");
         son1_end_votes = son1_obj.total_votes;
         BOOST_CHECK(son1_end_votes > son1_start_votes);
         son1_start_votes = son1_end_votes;
         son2_obj = con.wallet_api_ptr->get_son("son2account");
         son2_end_votes = son2_obj.total_votes;
         BOOST_CHECK(son2_end_votes > son2_start_votes);
         son2_start_votes = son2_end_votes;


         // Withdraw vote for SON 1
         accepted.clear();
         rejected.clear();
         rejected.push_back("son1account");
         update_votes_tx = con.wallet_api_ptr->update_son_votes("nathan", accepted,
                                                                rejected, 15, true);
         BOOST_CHECK(generate_block());
         BOOST_CHECK(generate_maintenance_block());

         // Verify the votes
         son1_obj = con.wallet_api_ptr->get_son("son1account");
         son1_end_votes = son1_obj.total_votes;
         BOOST_CHECK(son1_end_votes < son1_start_votes);
         son1_start_votes = son1_end_votes;
         son2_obj = con.wallet_api_ptr->get_son("son2account");
         // voice distribution changed, SON2 now has all voices
         son2_end_votes = son2_obj.total_votes;
         BOOST_CHECK(son2_end_votes > son2_start_votes);
         son2_start_votes = son2_end_votes;

         // Try to reject incorrect SON
         accepted.clear();
         rejected.clear();
         rejected.push_back("son1accnt");
         BOOST_CHECK_THROW(update_votes_tx = con.wallet_api_ptr->update_son_votes("nathan", accepted,
                                                                rejected, 15, true), fc::exception);
         BOOST_CHECK(generate_block());
         BOOST_CHECK(generate_maintenance_block());

         // Verify the votes
         son1_obj = con.wallet_api_ptr->get_son("son1account");
         son1_end_votes = son1_obj.total_votes;
         BOOST_CHECK(son1_end_votes == son1_start_votes);
         son1_start_votes = son1_end_votes;
         son2_obj = con.wallet_api_ptr->get_son("son2account");
         son2_end_votes = son2_obj.total_votes;
         BOOST_CHECK(son2_end_votes == son2_start_votes);
         son2_start_votes = son2_end_votes;

         // Reject SON2
         accepted.clear();
         rejected.clear();
         rejected.push_back("son2account");
         update_votes_tx = con.wallet_api_ptr->update_son_votes("nathan", accepted,
                                                                rejected, 15, true);
         BOOST_CHECK(generate_block());
         BOOST_CHECK(generate_maintenance_block());

         // Verify the votes
         son1_obj = con.wallet_api_ptr->get_son("son1account");
         son1_end_votes = son1_obj.total_votes;
         BOOST_CHECK(son1_end_votes > son1_start_votes);
         son1_start_votes = son1_end_votes;
         son2_obj = con.wallet_api_ptr->get_son("son2account");
         son2_end_votes = son2_obj.total_votes;
         BOOST_CHECK(son2_end_votes < son2_start_votes);
         son2_start_votes = son2_end_votes;

         // Try to accept and reject the same SON
         accepted.clear();
         rejected.clear();
         rejected.push_back("son1accnt");
         accepted.push_back("son1accnt");
         BOOST_REQUIRE_THROW(update_votes_tx = con.wallet_api_ptr->update_son_votes("nathan", accepted,
                                                                rejected, 15, true), fc::exception);
         BOOST_CHECK(generate_block());
         BOOST_CHECK(generate_maintenance_block());

         // Verify the votes
         son1_obj = con.wallet_api_ptr->get_son("son1account");
         son1_end_votes = son1_obj.total_votes;
         BOOST_CHECK(son1_end_votes == son1_start_votes);
         son1_start_votes = son1_end_votes;
         son2_obj = con.wallet_api_ptr->get_son("son2account");
         son2_end_votes = son2_obj.total_votes;
         BOOST_CHECK(son2_end_votes == son2_start_votes);
         son2_start_votes = son2_end_votes;
      } catch( fc::exception& e ) {
         BOOST_TEST_MESSAGE("SON cli wallet tests exception");
         edump((e.to_detail_string()));
         throw;
      }
      BOOST_TEST_MESSAGE("SON Vote cli wallet tests end");
  }

BOOST_AUTO_TEST_SUITE_END()



