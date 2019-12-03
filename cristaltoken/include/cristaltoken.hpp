#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>

#include <eosio/system.hpp>

#include <cmath>

namespace eosiosystem {
   class system_contract;
}

namespace eosio {

   using std::string;

   /**
    * @defgroup eosiotoken eosio.token
    * @ingroup eosiocontracts
    *
    * eosio.token contract
    *
    * @details eosio.token contract defines the structures and actions that allow users to create, issue, and manage
    * tokens on eosio based blockchains.
    * @{
    */
   class [[eosio::contract("cristaltoken")]] cristaltoken : public contract {
      public:
         using contract::contract;

         /**
          * Create action.
          *
          * @details Allows `issuer` account to create a token in supply of `maximum_supply`.
          * @param issuer - the account that creates the token,
          * @param maximum_supply - the maximum supply set for the token created.
          *
          * @pre Token symbol has to be valid,
          * @pre Token symbol must not be already created,
          * @pre maximum_supply has to be smaller than the maximum supply allowed by the system: 1^62 - 1.
          * @pre Maximum supply must be positive;
          *
          * If validation is successful a new entry in statstable for token symbol scope gets created.
          */
         [[eosio::action]]
         void create( const name&   issuer,
                      const asset&  maximum_supply);
         /**
          * Issue action.
          *
          * @details This action issues to `to` account a `quantity` of tokens.
          *
          * @param to - the account to issue tokens to, it must be the same as the issuer,
          * @param quntity - the amount of tokens to be issued,
          * @memo - the memo string that accompanies the token issue transaction.
          */
         [[eosio::action]]
         void issue( const name& to, const asset& quantity, const string& memo );

         /**
          * Retire action.
          *
          * @details The opposite for create action, if all validations succeed,
          * it debits the statstable.supply amount.
          *
          * @param quantity - the quantity of tokens to retire,
          * @param memo - the memo string to accompany the transaction.
          */
         [[eosio::action]]
         void retire( const asset& quantity, const string& memo );

         /**
          * Transfer action.
          *
          * @details Allows `from` account to transfer to `to` account the `quantity` tokens.
          * One account is debited and the other is credited with quantity tokens.
          *
          * @param from - the account to transfer from,
          * @param to - the account to be transferred to,
          * @param quantity - the quantity of tokens to be transferred,
          * @param memo - the memo string to accompany the transaction.
          */
         [[eosio::action]]
         void transfer( const name&    from,
                        const name&    to,
                        const asset&   quantity,
                        const string&  memo );
         /**
          * Open action.
          *
          * @details Allows `ram_payer` to create an account `owner` with zero balance for
          * token `symbol` at the expense of `ram_payer`.
          *
          * @param owner - the account to be created,
          * @param symbol - the token to be payed with by `ram_payer`,
          * @param ram_payer - the account that supports the cost of this action.
          *
          * More information can be read [here](https://github.com/EOSIO/eosio.contracts/issues/62)
          * and [here](https://github.com/EOSIO/eosio.contracts/issues/61).
          */
         [[eosio::action]]
         void open( const name& owner, const symbol& symbol, const name& ram_payer );

         /**
          * Close action.
          *
          * @details This action is the opposite for open, it closes the account `owner`
          * for token `symbol`.
          *
          * @param owner - the owner account to execute the close action for,
          * @param symbol - the symbol of the token to execute the close action for.
          *
          * @pre The pair of owner plus symbol has to exist otherwise no action is executed,
          * @pre If the pair of owner plus symbol exists, the balance has to be zero.
          */
         [[eosio::action]]
         void close( const name& owner, const symbol& symbol );

         /**
          * Get supply method.
          *
          * @details Gets the supply for token `sym_code`, created by `token_contract_account` account.
          *
          * @param token_contract_account - the account to get the supply for,
          * @param sym_code - the symbol to get the supply for.
          */
         static asset get_supply( const name& token_contract_account, const symbol_code& sym_code )
         {
            stats statstable( token_contract_account, sym_code.raw() );
            const auto& st = statstable.get( sym_code.raw() );
            return st.supply;
         }

         /**
          * Get balance method.
          *
          * @details Get the balance for a token `sym_code` created by `token_contract_account` account,
          * for account `owner`.
          *
          * @param token_contract_account - the token creator account,
          * @param owner - the account for which the token balance is returned,
          * @param sym_code - the token for which the balance is returned.
          */
         static asset get_balance( const name& token_contract_account, const name& owner, const symbol_code& sym_code )
         {
            accounts accountstable( token_contract_account, owner.value );
            const auto& ac = accountstable.get( sym_code.raw() );
            return ac.balance;
         }

         constexpr static   uint32_t     REQUIRED_PERIOD_DURATION   = 30*24*60*60;    // 30 days in second
         constexpr static   uint32_t     DAYS_IN_SECONDS            = 24*60*60;       // 1 day in second

         constexpr static   uint32_t     TYPE_ACCOUNT_PERSONAL      = 1;
         constexpr static   uint32_t     TYPE_ACCOUNT_BUSINESS      = 2;
         constexpr static   uint32_t     TYPE_ACCOUNT_FOUNDATION    = 3;
         constexpr static   uint32_t     TYPE_ACCOUNT_BANK_ADMIN    = 4;

         constexpr static   uint32_t     STATE_ENABLED              = 1;
         constexpr static   uint32_t     STATE_BLOCKED              = 0;

         static inline time_point_sec now() {
           return time_point_sec(current_time_point());
         }

         /**
         * Insert or Update Pre Authorized Payments method
         * @account Permissioner account that allows @provider to widthdraw @price once a month or period from @from to @to
         * @provider
         * @service_id
         * @price
         * @from
         * @to
         * @last_charged
         * @enabled
         */
         [[eosio::action]]
         void upsertpap(const name&             account
                        , const name&           provider
                        , const uint32_t&       service_id
                        , const asset&          price
                        , const uint32_t&       begins_at
                        , const uint32_t&       periods
                        , const uint32_t&       last_charged
                        , const uint32_t&       enabled
                        , const string& memo);

         [[eosio::action]]
         void erasepap(const name&  account
                              , const name&       provider
                              , const uint32_t&   service_id
                              , const string& memo);         
         /**
         * Charge method for @provider to get paid for @service_id provided to @account in the next billable month/period.
         * @account 
         * @provider
         * @service_id
         */
         [[eosio::action]]
         void chargepap(const name&  account
                              , const name&       provider
                              , const uint32_t&   service_id
                              , const string& memo);

         [[eosio::action]]
         void upsertcust(const name&          account
                          , const asset&      fee
                          , const asset&      overdraft
                          , const uint32_t&   account_type
                          , const uint32_t&   state
                          , const string& memo);
         
         [[eosio::action]]
         void erasecust(const name& account
                        , const string& memo);

         using upsertcust_action    = eosio::action_wrapper<"upsertcust"_n, &cristaltoken::upsertcust>;
         using erasecust_action     = eosio::action_wrapper<"erasecust"_n, &cristaltoken::erasecust>;

         using upsertpap_action     = eosio::action_wrapper<"upsertpap"_n, &cristaltoken::upsertpap>;
         using erasepap_action      = eosio::action_wrapper<"erasepap"_n, &cristaltoken::erasepap>;
         using chargepap_action     = eosio::action_wrapper<"chargepap"_n, &cristaltoken::chargepap>;

         using create_action        = eosio::action_wrapper<"create"_n, &cristaltoken::create>;
         using issue_action         = eosio::action_wrapper<"issue"_n, &cristaltoken::issue>;
         using retire_action        = eosio::action_wrapper<"retire"_n, &cristaltoken::retire>;
         using transfer_action      = eosio::action_wrapper<"transfer"_n, &cristaltoken::transfer>;
         using open_action          = eosio::action_wrapper<"open"_n, &cristaltoken::open>;
         using close_action         = eosio::action_wrapper<"close"_n, &cristaltoken::close>;

      private:
         
         struct [[eosio::table]] account {
            asset    balance;

            uint64_t primary_key()const { return balance.symbol.code().raw(); }
         };

         struct [[eosio::table]] currency_stats {
            asset    supply;
            asset    max_supply;
            name     issuer;

            uint64_t primary_key()const { return supply.symbol.code().raw(); }
         };

         typedef eosio::multi_index< "accounts"_n, account > accounts;
         typedef eosio::multi_index< "stat"_n, currency_stats > stats;

         void sub_balance( const name& owner, const asset& value );
         void add_balance( const name& owner, const asset& value, const name& ram_payer );
         void transfer_impl( const name&    from,
                        const name&    to,
                        const asset&   quantity,
                        const string&  memo  );
         void send_summary(const name& user, const string& message);

        // Pre Authorized Payments
        struct [[eosio::table]] pap {
          uint64_t        id;
          name            account;
          name            provider;
          uint32_t        service_id;
          asset           price;
          time_point_sec  begins_at;
          uint32_t        periods;
          
          uint32_t        last_charged;

          uint32_t        enabled;
          
          uint128_t       provider_account;
          uint128_t       account_service;
          uint128_t       provider_service;
          checksum256     account_service_provider;

          uint64_t primary_key() const { return id; }
          
          uint128_t by_provider_account() const {
            return _by_provider_account(provider, account);
          }
          static uint128_t _by_provider_account(name provider, name account) {

            return (uint128_t{provider.value}<<64) | (uint64_t)account.value;
            
          }

          uint128_t by_account_service() const {
            return _by_account_service(account, service_id);
          }

          static uint128_t _by_account_service(name account, uint32_t service_id) {

            return (uint128_t{account.value}<<64) | (uint64_t)service_id;
            
          }

          uint128_t by_provider_service() const {
            return _by_provider_service(provider, service_id);
          }
          static uint128_t _by_provider_service(name provider, uint32_t service_id) {
            return (uint128_t{provider.value}<<64) | (uint64_t)service_id;
          }


          checksum256 by_account_service_provider() const {
            return _by_account_service_provider(account, provider, service_id);
          }
          static checksum256 _by_account_service_provider(name account, name provider, uint32_t service_id) {
            return checksum256::make_from_word_sequence<uint64_t>(
                0ULL, account.value, provider.value, (uint64_t)service_id
              );
          }
          // EOSLIB_SERIALIZE( pap, ( id )( account )( provider )( service_id ) )
        };


        typedef eosio::multi_index<
          "pap"_n, pap,
          indexed_by<"byall"_n,       const_mem_fun<pap, checksum256, &pap::by_account_service_provider>>,
          indexed_by<"byprovserv"_n,  const_mem_fun<pap, uint128_t,   &pap::by_provider_service>>,
          indexed_by<"byprovacc"_n,   const_mem_fun<pap, uint128_t,   &pap::by_provider_account>>,
          indexed_by<"byaccserv"_n,   const_mem_fun<pap, uint128_t,   &pap::by_account_service>>
          >
          paps;

        struct [[eosio::table]] customer {
          name         key;
          asset        fee;
          asset        overdraft;
          uint32_t     account_type; 
          uint32_t     state;
          
          uint64_t primary_key() const { return key.value;}
        };

        // typedef eosio::multi_index
        //   <
        //       "customer_n",
        //       indexed_by
        //       <
        //           // sort by customer::operator<
        //           ordered_unique<identity<customer>>,
        //           // sort by string's < on customer::name member
        //           ordered_unique<member<customer, std::string, &CryptoCurrency::name>>
        //       >
        //   > customers;

        typedef eosio::multi_index<"customer"_n, customer> customers;
        

   };
   /** @}*/ // end of @defgroup eosiotoken eosio.token
} /// namespace eosio
