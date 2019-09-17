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

#include <graphene/chain/database.hpp>

#include <graphene/chain/chain_property_object.hpp>
#include <graphene/chain/witness_schedule_object.hpp>
#include <graphene/chain/special_authority_object.hpp>
#include <graphene/chain/operation_history_object.hpp>
#include <graphene/chain/protocol/fee_schedule.hpp>

#include <fc/io/fstream.hpp>

#include <fstream>
#include <functional>
#include <iostream>

namespace graphene { namespace chain {

database::database() :
   _random_number_generator(fc::ripemd160().data())
{
   initialize_indexes();
   initialize_evaluators();
}

database::~database()
{
   clear_pending();
}

void database::reindex(fc::path data_dir, const genesis_state_type& initial_allocation)
{ try {
   ilog( "reindexing blockchain" );
   wipe(data_dir, false);
   open(data_dir, [&initial_allocation]{return initial_allocation;});

   auto start = fc::time_point::now();
   auto last_block = _block_id_to_block.last();
   if( !last_block ) {
      elog( "!no last block" );
      edump((last_block));
      return;
   }

   const auto last_block_num = last_block->block_num();

   ilog( "Replaying blocks..." );
   // Right now, we leave undo_db enabled when replaying when the bookie plugin is 
   // enabled.  It depends on new/changed/removed object notifications, and those are 
   // only fired when the undo_db is enabled
   if (!_slow_replays)
      _undo_db.disable();
   for( uint32_t i = 1; i <= last_block_num; ++i )
   {
      if( i == 1 || 
          i % 10000 == 0 ) 
         std::cerr << "   " << double(i*100)/last_block_num << "%   "<< i << " of " <<last_block_num<<"   \n";
      fc::optional< signed_block > block = _block_id_to_block.fetch_by_number(i);
      if( !block.valid() )
      {
         wlog( "Reindexing terminated due to gap:  Block ${i} does not exist!", ("i", i) );
         uint32_t dropped_count = 0;
         while( true )
         {
            fc::optional< block_id_type > last_id = _block_id_to_block.last_id();
            // this can trigger if we attempt to e.g. read a file that has block #2 but no block #1
            if( !last_id.valid() )
               break;
            // we've caught up to the gap
            if( block_header::num_from_id( *last_id ) <= i )
               break;
            _block_id_to_block.remove( *last_id );
            dropped_count++;
         }
         wlog( "Dropped ${n} blocks from after the gap", ("n", dropped_count) );
         break;
      }
      if (_slow_replays)
         push_block(*block, skip_fork_db |
                            skip_witness_signature |
                            skip_transaction_signatures |
                            skip_transaction_dupe_check |
                            skip_tapos_check |
                            skip_witness_schedule_check |
                            skip_authority_check);
      else
         apply_block(*block, skip_witness_signature |
                             skip_transaction_signatures |
                             skip_transaction_dupe_check |
                             skip_tapos_check |
                             skip_witness_schedule_check |
                             skip_authority_check);
   }
   if (!_slow_replays)
     _undo_db.enable();
   auto end = fc::time_point::now();
   ilog( "Done reindexing, elapsed time: ${t} sec", ("t",double((end-start).count())/1000000.0 ) );
} FC_CAPTURE_AND_RETHROW( (data_dir) ) }

void database::wipe(const fc::path& data_dir, bool include_blocks)
{
   ilog("Wiping database", ("include_blocks", include_blocks));
   if (_opened) {
     close(false);
   }
   object_database::wipe(data_dir);
   if( include_blocks )
      fc::remove_all( data_dir / "database" );
}

void database::open(
   const fc::path& data_dir,
   std::function<genesis_state_type()> genesis_loader)
{
   try
   {
      object_database::open(data_dir);

      _block_id_to_block.open(data_dir / "database" / "block_num_to_block");

      if( !find(global_property_id_type()) )
         init_genesis(genesis_loader());
      else
      {
         _p_core_asset_obj = &get( asset_id_type() );
         _p_core_dynamic_data_obj = &get( asset_dynamic_data_id_type() );
         _p_global_prop_obj = &get( global_property_id_type() );
         _p_chain_property_obj = &get( chain_property_id_type() );
         _p_dyn_global_prop_obj = &get( dynamic_global_property_id_type() );
         _p_witness_schedule_obj = &get( witness_schedule_id_type() );
      }

      fc::optional<signed_block> last_block = _block_id_to_block.last();
      if( last_block.valid() )
      {
         _fork_db.start_block( *last_block );
         if( last_block->id() != head_block_id() )
         {
              FC_ASSERT( head_block_num() == 0, "last block ID does not match current chain state",
                         ("last_block->id", last_block->id())("head_block_num",head_block_num()) );
         }
      }
      _opened = true;
   }
   FC_CAPTURE_LOG_AND_RETHROW( (data_dir) )
}

void database::close(bool rewind)
{
   if (!_opened)
      return;
      
   // TODO:  Save pending tx's on close()
   clear_pending();

   // pop all of the blocks that we can given our undo history, this should
   // throw when there is no more undo history to pop
   if( rewind )
   {
      try
      {
         uint32_t cutoff = get_dynamic_global_properties().last_irreversible_block_num;

         while( head_block_num() > cutoff )
         {
         //   elog("pop");
            block_id_type popped_block_id = head_block_id();
            pop_block();
            _fork_db.remove(popped_block_id); // doesn't throw on missing
            try
            {
               _block_id_to_block.remove(popped_block_id);
            }
            catch (const fc::key_not_found_exception&)
            {
            }
         }
      }
      catch ( const fc::exception& e )
      {
         wlog( "Database close unexpected exception: ${e}", ("e", e) );
      }
   }

   // Since pop_block() will move tx's in the popped blocks into pending,
   // we have to clear_pending() after we're done popping to get a clean
   // DB state (issue #336).
   clear_pending();

   object_database::flush();
   object_database::close();

   if( _block_id_to_block.is_open() )
      _block_id_to_block.close();

   _fork_db.reset();

   _opened = false;
}

void database::force_slow_replays()
{
   ilog("enabling slow replays");
   _slow_replays = true;
}

void database::check_ending_lotteries()
{
   try {
      const auto& lotteries_idx = get_index_type<asset_index>().indices().get<active_lotteries>();      
      for( auto checking_asset: lotteries_idx )
      {
         FC_ASSERT( checking_asset.is_lottery() );
         FC_ASSERT( checking_asset.lottery_options->is_active );
         FC_ASSERT( checking_asset.lottery_options->end_date != time_point_sec() );
         if( checking_asset.lottery_options->end_date > head_block_time() ) continue;
         checking_asset.end_lottery(*this);
      }
   } catch( ... ) {}
}

void database::check_lottery_end_by_participants( asset_id_type asset_id )
{
   try {
      asset_object asset_to_check = asset_id( *this );
      auto asset_dyn_props = asset_to_check.dynamic_data( *this );
      FC_ASSERT( asset_dyn_props.current_supply == asset_to_check.options.max_supply );
      FC_ASSERT( asset_to_check.is_lottery() );
      FC_ASSERT( asset_to_check.lottery_options->ending_on_soldout );
      asset_to_check.end_lottery( *this );
   } catch( ... ) {}
}

} }
