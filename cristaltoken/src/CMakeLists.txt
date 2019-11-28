project(cristaltoken)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( cristaltoken cristaltoken cristaltoken.cpp )
target_include_directories( cristaltoken PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( cristaltoken ${CMAKE_SOURCE_DIR}/../ricardian )