// SPDX-License-Identifier: GPL-3.0-only
#include "src/licensing/key_store.hpp"
#include "src/application/environment.hpp"
#include "src/crypto/crypto.hpp"
#include "src/files/atomic_io.hpp"
#include "src/tokens/constants.hpp"
namespace preloader_l::licensing {
void store_cipher_key(const configuration::State& state,std::string_view key,const RuntimePaths& paths){
 if(key.empty())return;
 auto path=state.license_directory;path+=filesystem_path(".dlf");
 if(std::filesystem::is_regular_file(native_path(path)))return;
 std::string bytes(0x41,'\0');
 const unsigned char prefix[]={0x30,0x06,0x02,0x01,0x00,0x02,0x01,0x00};
 for(size_t i=0;i<sizeof prefix;++i)bytes[i]=static_cast<char>(prefix[i]);
 bytes+="<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><License xmlns=\"http://ea.com/license\"><CipherKey>";
 bytes.append(key);bytes+="</CipherKey><MachineHash>1</MachineHash><ContentId>";bytes+=state.content_id;
 bytes+="</ContentId><UserId>";bytes+=state.user_id;
 bytes+="</UserId><GrantTime>2024-01-01T00:00:00Z</GrantTime><StartTime>2024-01-01T00:00:00Z</StartTime></License>";
 crypto::Cipher cipher;cipher.set_key_and_iv_042760(tokens::constants::key_084a50,tokens::constants::iv_084a60);
 if(!cipher.encrypt_with_offset(bytes,0x41))throw std::runtime_error("license-key encryption failed");
 atomic_write(native_path(path),bytes,&paths,true);
}
}
