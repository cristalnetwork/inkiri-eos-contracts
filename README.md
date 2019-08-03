Inkiri EOS Smart Contracts
==========================

# inkiri-eos-contracts #
EOS Smart Contracts for Inkiri Bank

## Setup Dev environment ##
1. Follow EOS tutorial steps from scratch up to point 2.2 at [this link](https://developers.eos.io/eosio-home/docs/10-big-picture). After this step you would have achieved:
	1. Run `cleos` at your local machine. 
	2. You will have a local wallet and several accounts.
	3. You will be able to compile and deploy a Smart Contract.
2. Create 2 accounts on [EOS Jungle Testnet](https://api.monitor.jungletestnet.io/)
	1. Create 2 key pairs following [this link](https://api.monitor.jungletestnet.io/#createKey)
	2. Create 2 accounts using the public keys created on the step before, following [this link](https://api.monitor.jungletestnet.io/#account)
	3. Get some EOS testnet tokens using the faucet [at this link](https://api.monitor.jungletestnet.io/#faucet)
3. Import created accounts to your local wallet, so you can use Testnet Blockchain without the need of running a local node.
	1. Take a look at (this link)[https://developers.eos.io/eosio-cleos/reference#cleos-wallet-import] to import private keys created.

## Create INK Inkiri Token ##

#### Step 1: Obtain Contract Source ####
```bash
cd /local/directory/for/eos/contracts
git clone https://github.com/EOSIO/eosio.contracts --branch v1.5.2 --single-branch
cd eosio.contracts/eosio.token
```

#### Step 2: Compile the Contract ####
```bash
eosio-cpp -I include -o eosio.token.wasm src/eosio.token.cpp --abigen
```

#### Step 3: Deploy the Token Contract ####
```bash
cleos -u http://jungle2.cryptolions.io:80 set contract youmasteracc /local/directory/for/eos/contracts/eosio.contracts/eosio.token --abi eosio.token.abi -p youmasteracc@active
```

#### Step 4: Create the Token ####
```bash
cleos -u http://jungle2.cryptolions.io:80 push action youmasteracc create '[ "youmasteracc", "1000000000.0000 XXX"]' -p youmasteracc@active
```
> Replace XXX with your preferred token name.


## Issue INK Inkiri Token ##
The account named `youmasteracc` will now issue some tokens to the other account named `simpleaccoun` created on the first step by running the following command:

```bash
cleos -u http://jungle2.cryptolions.io:80 push action youmasteracc issue '[ "simpleaccoun", "50.0000 XXX", "memo"]' -p youmasteracc@active
```
If you have troubles running this command, you may have some missing permission.
Just run:
```bash
cleos -u http://jungle2.cryptolions.io:80 set account permission youmasteracc active '{"threshold": 1,"keys": [{"key": "YOUR-youmasteracc-PUBLIC-KEY","weight": 1}], "accounts": [{"permission":{"actor":"youmasteracc","permission":"eosio.code"},"weight":1}]}' -p youmasteracc@owner
```
## Issue InkiriBank Smart Contract ##