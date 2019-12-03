#include <cristaltoken.hpp>

namespace eosio {

  void cristaltoken::create( const name&   issuer,
                      const asset&  maximum_supply )
  {
      require_auth( get_self() );

      auto sym = maximum_supply.symbol;
      check( sym.is_valid(), "invalid symbol name" );
      check( maximum_supply.is_valid(), "invalid supply");
      check( maximum_supply.amount > 0, "max-supply must be positive");

      stats statstable( get_self(), sym.code().raw() );
      auto existing = statstable.find( sym.code().raw() );
      check( existing == statstable.end(), "token with symbol already exists" );

      statstable.emplace( get_self(), [&]( auto& s ) {
         s.supply.symbol = maximum_supply.symbol;
         s.max_supply    = maximum_supply;
         s.issuer        = issuer;
      });
  }


  void cristaltoken::issue( const name& to, const asset& quantity, const string& memo )
  {
      auto sym = quantity.symbol;
      check( sym.is_valid(), "invalid symbol name" );
      check( memo.size() <= 256, "memo has more than 256 bytes" );

      stats statstable( get_self(), sym.code().raw() );
      auto existing = statstable.find( sym.code().raw() );
      check( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
      const auto& st = *existing;
      // HACK
      // check( to == st.issuer, "tokens can only be issued to issuer account" );

      require_auth( st.issuer );
      check( quantity.is_valid(), "invalid quantity" );
      check( quantity.amount > 0, "must issue positive quantity" );

      check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
      check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

      statstable.modify( st, same_payer, [&]( auto& s ) {
         s.supply += quantity;
      });

      // HACK
      // add_balance( st.issuer, quantity, st.issuer );
      add_balance( st.issuer, quantity, st.issuer );

      if( to != st.issuer ) {
        SEND_INLINE_ACTION( *this, transfer, { {st.issuer, "active"_n} },
                            { st.issuer, to, quantity, memo }
        );
      }

  }

  void cristaltoken::retire( const asset& quantity, const string& memo )
  {
      auto sym = quantity.symbol;
      check( sym.is_valid(), "invalid symbol name" );
      check( memo.size() <= 256, "memo has more than 256 bytes" );

      stats statstable( get_self(), sym.code().raw() );
      auto existing = statstable.find( sym.code().raw() );
      check( existing != statstable.end(), "token with symbol does not exist" );
      const auto& st = *existing;

      require_auth( st.issuer );
      check( quantity.is_valid(), "invalid quantity" );
      check( quantity.amount > 0, "must retire positive quantity" );

      check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

      statstable.modify( st, same_payer, [&]( auto& s ) {
         s.supply -= quantity;
      });

      sub_balance( st.issuer, quantity );
  }

  void cristaltoken::transfer( const name&    from,
                        const name&    to,
                        const asset&   quantity,
                        const string&  memo )
  {
      
      require_auth( from );
      transfer_impl( from, to, quantity, memo );
  }

  void cristaltoken::sub_balance( const name& owner, const asset& value ) {
     accounts from_acnts( get_self(), owner.value );

     const auto& from = from_acnts.get( value.symbol.code().raw(), "no balance object found" );
     check( from.balance.amount >= value.amount, "overdrawn balance" );

     // from_acnts.modify( from, owner, [&]( auto& a ) {
     from_acnts.modify( from, get_self(), [&]( auto& a ) {
           a.balance -= value;
        });
  }

  void cristaltoken::add_balance( const name& owner, const asset& value, const name& ram_payer )
  {
     accounts to_acnts( get_self(), owner.value );
     auto to = to_acnts.find( value.symbol.code().raw() );
     if( to == to_acnts.end() ) {
        // to_acnts.emplace( ram_payer, [&]( auto& a ){
        to_acnts.emplace( get_self(), [&]( auto& a ){
          a.balance = value;
        });
     } else {
        // to_acnts.modify( to, same_payer, [&]( auto& a ) {
        to_acnts.modify( to, get_self(), [&]( auto& a ) {
          a.balance += value;
        });
     }
  }

  void cristaltoken::open( const name& owner, const symbol& symbol, const name& ram_payer )
  {
     require_auth( ram_payer );

     check( is_account( owner ), "owner account does not exist" );

     auto sym_code_raw = symbol.code().raw();
     stats statstable( get_self(), sym_code_raw );
     const auto& st = statstable.get( sym_code_raw, "symbol does not exist" );
     check( st.supply.symbol == symbol, "symbol precision mismatch" );

     accounts acnts( get_self(), owner.value );
     auto it = acnts.find( sym_code_raw );
     if( it == acnts.end() ) {
        acnts.emplace( ram_payer, [&]( auto& a ){
        // acnts.emplace( get_self(), [&]( auto& a ){
          a.balance = asset{0, symbol};
        });
     }
  }

  void cristaltoken::close( const name& owner, const symbol& symbol )
  {
     require_auth( owner );
     accounts acnts( get_self(), owner.value );
     auto it = acnts.find( symbol.code().raw() );
     check( it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect." );
     check( it->balance.amount == 0, "Cannot close because the balance is not zero." );
     acnts.erase( it );
  }

  void cristaltoken::upsertpap(const name&      from
                        , const name&           to
                        , const uint32_t&       service_id
                        , const asset&          price
                        , const uint32_t&       begins_at
                        , const uint32_t&       periods
                        , const uint32_t&       last_charged
                        , const uint32_t&       enabled
                        , const string& memo)
  {

      // require_auth(get_self());
      customers customers_idx(get_self(), get_first_receiver().value);
      auto iter_account = customers_idx.find(from.value);
      
      check( iter_account != customers_idx.end(), "Customer account not exists." );
      check( memo.size() <= 256, "memo has more than 256 bytes" );
      
      auto iter_account_obj = iter_account;
      
      check( iter_account_obj->state == STATE_ENABLED, "Customer account is not enabled." );

      auto iter_provider = customers_idx.find(to.value);
      check( iter_provider != customers_idx.end(), "Provider account not exists." );
      // auto& iter_provider_obj = *iter_provider;
      auto iter_provider_obj = iter_provider;
      check( iter_provider_obj->state == STATE_ENABLED, "Provider account is not enabled." );
      check( iter_provider_obj->account_type == TYPE_ACCOUNT_BUSINESS || iter_provider_obj->account_type == TYPE_ACCOUNT_BANK_ADMIN, "Provider account is not BIZ neither ADMIN." );

      auto idxKey = pap::_by_account_service_provider(from, to, service_id);
      paps pap_list(get_self(), get_first_receiver().value);
      auto cidx = pap_list.get_index<"byall"_n>();
      auto it = cidx.find(idxKey);
      if( it == cidx.end())
      {
        require_auth( from );

        check(to != from, "Customer and provider should be different accounts");
        check( periods>0, "periods is less than 1" );


        auto sym = price.symbol;
        check( sym.is_valid(), "invalid price symbol name" );
        stats statstable( get_self(), sym.code().raw() );
        auto existing = statstable.find( sym.code().raw() );
        check( existing != statstable.end(), "price token symbol does not exist" );
        const auto& st = *existing;
        check( price.is_valid(), "invalid price quantity" );
        check( price.amount > 0, "must set positive price quantity" );
        check( price.symbol == st.supply.symbol, "price symbol precision mismatch" );
        
        pap_list.emplace(get_self(), [&]( auto& row ) {
          row.id              = pap_list.available_primary_key();
          row.account         = from; //account;
          row.provider        = to;   //provider;
          row.service_id      = service_id;
          row.price           = price;
          row.begins_at       = time_point_sec(begins_at);
          row.periods         = periods;
          row.last_charged    = 0;
          row.enabled         = STATE_ENABLED;

          row.provider_account          = row.by_provider_account();
          row.account_service           = row.by_account_service();
          row.provider_service          = row.by_provider_service();
          row.account_service_provider  = row.by_account_service_provider();

        });

      }
      else {
        
        check( has_auth(get_self()) || has_auth(to), "Missing required authority of admin or provider");
        check( enabled==STATE_ENABLED || enabled==STATE_BLOCKED, "Invalid enabled argument.");
        
        // cidx.modify(it, same_payer, [&]( auto& row ) {  
        cidx.modify(it, get_self(), [&]( auto& row ) {
          row.enabled           = enabled;
        });
      }

  }
  
  void cristaltoken::erasepap(const name&         from
                              , const name&       to
                              , const uint32_t&   service_id
                              , const string& memo) {
      
      check( has_auth(get_self()) || has_auth(to), "Missing required authority of admin or provider");
      check( memo.size() <= 256, "memo has more than 256 bytes" );
      
      auto idxKey = pap::_by_account_service_provider(from, to, service_id);
      paps pap_list(get_self(), get_first_receiver().value);
      auto cidx = pap_list.get_index<"byall"_n>();
      auto it = cidx.find(idxKey);

      check( it != cidx.end(), "PAP (Account-Provider-Service) not found");
      
      cidx.erase(it);
  }

  void cristaltoken::chargepap(const name&        from
                              , const name&       to
                              , const uint32_t&   service_id
                              , const asset&      quantity
                              , const string&     memo) {
      
    check( memo.size() <= 256, "memo has more than 256 bytes" );
    check( has_auth(get_self()) || has_auth(to), "Missing required authority of admin or provider");

    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    
    // Check pap exists
    auto idxKey = pap::_by_account_service_provider(from, to, service_id);
    paps pap_list(get_self(), get_first_receiver().value);
    auto cidx = pap_list.get_index<"byall"_n>();
    auto it = cidx.find(idxKey);
    
    check( it != cidx.end(), "PAP (Account-Provider-Service) not found");
    
    auto& pap = *it;
    
    check( pap.enabled==STATE_ENABLED, "PAP is not enabled");
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.symbol == pap.price.symbol, "symbol mismatch" );
    check( quantity.amount == pap.price.amount , "quantity differs from agreed price");


    time_point_sec current_time       = now();
    time_point_sec last_charged_time  = time_point_sec(pap.begins_at.sec_since_epoch() + (pap.last_charged * REQUIRED_PERIOD_DURATION)); 
    
    if ( current_time.sec_since_epoch() < (last_charged_time.sec_since_epoch() + REQUIRED_PERIOD_DURATION))
    {
      auto remaining = std::floor((( last_charged_time.sec_since_epoch() + REQUIRED_PERIOD_DURATION ) - current_time.sec_since_epoch()) / DAYS_IN_SECONDS);
      std::string err = "Cannot charge yet, You still have "
                    + std::to_string( remaining )
                    + " days remaining";

      check( false, err.c_str());
    
    }
    
    auto period = pap.last_charged+1;

    check( period <= pap.periods, "Sorry, contract has ended!");

    // action{
    //   permission_level{get_self(), "active"_n},
    //   "cristaltoken"_n,
    //   "transfer"_n,
    //   std::make_tuple(pap.account, pap.provider, pap.price, memo)
    // }.send();
    // SEND_INLINE_ACTION( *this, transfer, { {get_self(), "active"_n} },
    //                     { pap.account, pap.provider, pap.price, memo }
    // );
    
    transfer_impl( pap.account, pap.provider, pap.price, memo );

    auto enabled = 1;
    if(period == pap.periods)
      enabled = 0;
    cidx.modify(it, get_self(), [&]( auto& row ) {
    // cidx.modify(it, same_payer, [&]( auto& row ) {
      row.last_charged = period;
      row.enabled      = enabled; 
    });

  }

  void cristaltoken::transfer_impl( const name&    from,
                        const name&    to,
                        const asset&   quantity,
                        const string&  memo  ){
     
    
    check( from != to, "cannot transfer to self" );
    // check( has_auth(from) || has_auth(get_self()), "Missing required authority of owner or admin");
    
    check( is_account( to ), "to account does not exist");
    
    auto sym = quantity.symbol.code();
    stats statstable( get_self(), sym.raw() );
    const auto& st = statstable.get( sym.raw() );

    require_recipient( from );
    require_recipient( to );
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must transfer positive quantity" );
    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    // auto payer = has_auth( to ) ? to : from;
    auto payer = get_self();

    sub_balance( from, quantity );
    add_balance( to, quantity, payer );
    
  }


  void cristaltoken::upsertcust(const name&       to
                              , const asset&      fee
                              , const asset&      overdraft
                              , const uint32_t&   account_type
                              , const uint32_t&   state
                              , const string& memo) {
      
    check( memo.size() <= 256, "memo has more than 256 bytes" );
    require_auth(get_self());
    customers idx(get_self(), get_first_receiver().value);
    auto iterator = idx.find(to.value);
    if( iterator == idx.end() )
    {
      idx.emplace(get_self(), [&]( auto& row ) {
        row.key               = to;
        row.fee               = fee;
        row.overdraft         = overdraft;
        row.account_type      = account_type;
        row.state             = state;
      });

      if(overdraft.amount>0)
      {
        std::string memo = "oft|create";
        SEND_INLINE_ACTION( *this, issue, { {get_self(), "active"_n} },
                            { to, overdraft, memo }
        );
      }
    }
    else {
      // update
      idx.modify(iterator, get_self(), [&]( auto& row ) {
        // row.key               = to;
        row.fee               = fee;
        row.overdraft         = overdraft; // We should withdraw tokens if new overdraft is minor than old one.
        row.account_type      = account_type;
        row.state             = state;
      });
    }
  }

  void cristaltoken::erasecust(const name& to
                              , const string& memo) {
      
    check( memo.size() <= 256, "memo has more than 256 bytes" );
    require_auth(get_self());
    customers idx(get_self(), get_first_receiver().value);
    auto iterator = idx.find(to.value);
    check(iterator != idx.end(), "Account does not exist");
    idx.erase(iterator);
    
  }

  
} /// namespace eosio
