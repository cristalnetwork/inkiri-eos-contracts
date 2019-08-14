#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include "eosio.token.hpp"

using namespace eosio;

class [[eosio::contract("inkiribank")]] inkiribank : public eosio::contract {

  public:
  	inkiribank(name receiver, name code, datastream<const char*> ds):contract(receiver, code, ds) {}

		// [[eosio::action]]
	 //  void init(){
  // 		require_auth(get_self());
  // 		ikaccount_type_index account_types(get_self(), get_first_receiver().value);
		//   auto iterator = account_types.find('personal');
	 // }

  	[[eosio::action]]
	  void setoverdraft(name user, double overdraft){
			require_auth(get_self());
		  ikaccount_index accounts(get_self(), get_first_receiver().value);
		  auto iterator = accounts.find(user.value);
	  	check(iterator != accounts.end(), "Account does not exist");
	  	check(iterator->state!=1, "Account is not enabled");

  		// const auto& ikaccount = accounts.get( value.symbol.code().raw(), "no balance object found" );
  		// eosio_assert( ikaccount->amount >= value.amount, "overdrawn balance" );

	  	 if(overdraft>0.0)
		    {
		    	symbol ink_symbol = symbol(symbol_code("INK"), 4);
					// asset quantity = asset( overdraft, symbolvalue );
					asset quantity = asset( int64_t(overdraft), ink_symbol );
					// action(
					//     permission_level{get_self(), "active"_n},
					//     N(inkiritoken1), N(transfer),
					//     std::make_tuple(_self, user, quantity, string("oft|create"))
					// ).send();

					action(
		        permission_level{get_self(),"active"_n},
		        "inkiritoken1"_n,
		        "issue"_n,
		        std::make_tuple(_self, user, quantity, string("oft|create"))
		      ).send();

		    }

  	}

  	
  	[[eosio::action]]
	  void upsertikacc(name user, double fee, double overdraft, uint64_t account_type, uint64_t state){

		  // require_auth( user );
		  require_auth(get_self());
		  ikaccount_index accounts(get_self(), get_first_receiver().value);
		  auto iterator = accounts.find(user.value);
		  if( iterator == accounts.end() )
		  {
		    accounts.emplace(get_self(), [&]( auto& row ) {
		      row.key  							= user;
		      row.locked_amount 		= 0;
					row.deposits_counter	= 0;
					row.withdraw_amount 	= 0;
					row.withdraw_counter 	= 0;
					row.xchg_amount 			= 0;
					row.xchg_counter 			= 0;
					row.fee 							= fee;
					row.overdraft 				= overdraft;
					row.account_type 			= account_type;
		      row.state 						= state;
		    });

		    send_summary(user, "successfully created inkiri account");

		   	if(overdraft>0)
		   	{
		   		// setoverdraft(user, overdraft);		
		   		// send_summary(user, "successfully set overdraft");
		   	}
		  }
		  else {
		    // update
		  	
			}
		}


		[[eosio::action]]
  	void erase(name user) {
	    // require_auth(user);
	    require_auth(get_self());
	    ikaccount_index accounts(get_self(), get_first_receiver().value);
	    auto iterator = accounts.find(user.value);
	    check(iterator != accounts.end(), "Account does not exist");
	    accounts.erase(iterator);
	    send_summary(user, "Successfully erased inkiri account");
	  }
		
		// May I notify the contracts owner and define receiver name so I can log actions for deployer and filter by 
		// receiver?
		// En config.ini se puede definir el parametro --filter-on y pasarle el nombre -> alice::. En vez de alice
		// se le podria pasar el del deployer.

	  [[eosio::action]]
	  void notify(name user, std::string msg) {
	    require_auth(get_self());
	    require_recipient(user);
	  }


  private:
    // struct [[eosio::table]] ikaccount_type {
    //   uint64_t     	id;
    //   std::string 	description;
    //   uint128_t 	  fee;
    //   uint128_t			overdraft;
    // 	uint64_t 			owners_limit;
    // 	uint64_t 			visualizars_limit;
    // 	uint64_t 			pos_limit;

    //   auto primary_key() const { return id; }
    // };
    // typedef eosio::multi_index<"ikaccount_type"_n, ikaccount_type> ikaccount_type_index;    

    
    struct [[eosio::table]] ikaccount {
      name 					key;
      double 				locked_amount;
	    uint64_t 			deposits_counter;
	    double 				withdraw_amount;
	    uint64_t 			withdraw_counter;
	    double 				xchg_amount;
	    uint64_t 			xchg_counter;
	    double 	  		fee;
			double				overdraft;
      uint64_t  		account_type; 
      /*
				1 personal
				2 business
				3 foundation
				4 bank admin
      */

      uint64_t state;
			/*
				1 ok
				2 blocked
      */      

      uint64_t primary_key() const { return key.value;}
    };
    typedef eosio::multi_index<"ikaccounts"_n, ikaccount> ikaccount_index;

    struct [[eosio::table]] ikrequest {
    	uint64_t     	id;
      name 					user;
      double 				amount;
	    uint64_t  		req_type; 
      /*
				1 deposit
				2 withdraw
				3 foundation
				4 bank admin
      */

      uint64_t state;
			/*
				1 requested
				2 pending
				3 canceled
				4 done
      */      

      auto primary_key() const { return id; }
      uint64_t get_secondary_1() const { return name;}
    };
    typedef eosio::multi_index<"ikrequests"_n, ikrequest, indexed_by<"byname"_n, const_mem_fun<ikrequest, name, &ikrequest::get_secondary_1>>> ikrequest_index;


    void send_summary(name user, std::string message){
      action(
        permission_level{get_self(),"active"_n},
        get_self(),
        "notify"_n,
        std::make_tuple(user, name{user}.to_string() + " " + message)
      ).send();
    }
};
