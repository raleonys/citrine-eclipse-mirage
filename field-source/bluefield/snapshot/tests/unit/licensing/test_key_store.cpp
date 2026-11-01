// SPDX-License-Identifier: GPL-3.0-only
#include "src/licensing/key_store.hpp"
#include "src/application/environment.hpp"
#include "src/crypto/crypto.hpp"
#include "src/tokens/constants.hpp"
#include <chrono>
#include <iostream>
int main(){try{
 auto root=std::filesystem::temp_directory_path()/("preloader_l_key_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 const auto paths=preloader_l::RuntimePaths::fixture(root/"host.exe",root/"surge.cfg",root,root/"programdata");
 preloader_l::configuration::State state{};state.license_directory=preloader_l::model_path(paths.program_data()/"Electronic Arts/EA Services/License/fixture-content");state.content_id="fixture-content";state.user_id="fixture-user";
 preloader_l::licensing::store_cipher_key(state,"SYNTHETIC_KEY_FIXTURE",paths);auto path=state.license_directory+u".dlf";const auto initial=preloader_l::read_file(path,true);auto plain=initial;
 if(plain.size()<=65||static_cast<unsigned char>(plain[0])!=0x30)throw std::runtime_error("key file header");
 preloader_l::crypto::Cipher cipher;cipher.set_key_and_iv_042760(preloader_l::tokens::constants::key_084a50,preloader_l::tokens::constants::iv_084a60);
 if(!cipher.decrypt_with_offset(plain,65)||plain.find("<CipherKey>SYNTHETIC_KEY_FIXTURE</CipherKey>")==std::string::npos||plain.find("<UserId>fixture-user</UserId>")==std::string::npos)throw std::runtime_error("key file CBC/XML");
 preloader_l::licensing::store_cipher_key(state,"OTHER_SYNTHETIC_KEY",paths);if(preloader_l::read_file(path,true)!=initial)throw std::runtime_error("existing key file replaced");
 std::filesystem::remove_all(root);std::cout<<"PASS key store header/CBC/XML/identity/no-overwrite\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
