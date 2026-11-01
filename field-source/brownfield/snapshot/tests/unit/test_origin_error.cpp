#include "preloader_l/protocol/origin_error.hpp"
#include <string_view>
#include <cstring>
using namespace preloader_l::protocol;
int main(){
    if(origin_error_value(OriginError::success)!=0) return 1;
    if(origin_error_value(OriginError::pending)!=1) return 2;
    if(!origin_error_is_success(OriginError::success)||!origin_error_is_success(OriginError::pending)) return 3;
    if(!origin_error_is_pending(OriginError::pending)||origin_error_is_pending(OriginError::success)) return 4;
    if(origin_error_is_success(OriginError::not_implemented)||!origin_error_is_error(OriginError::not_implemented)) return 5;
    if(origin_error_value(OriginError::not_implemented)!=-1593835518) return 6;
    if(origin_error_value(OriginError::proxy_bad_request)!=-1576468080) return 7;
    if(origin_error_value(OriginError::no_service)!=-1577058293) return 8;
    if(origin_error_area(OriginError::proxy_bad_request)!=OriginErrorArea::proxy) return 9;
    if(origin_error_level(OriginError::proxy_bad_request)!=OriginErrorLevel::l2) return 10;
    if(origin_error_local_code(OriginError::proxy_bad_request)!=400) return 11;
    if(origin_error_area(OriginError::lsx_no_response)!=OriginErrorArea::lsx) return 12;
    if(origin_error_local_code(OriginError::lsx_no_response)!=1) return 13;
    if(std::strcmp(origin_error_name(OriginError::pending),"Pending")!=0) return 14;
    if(std::strcmp(origin_error_name(OriginError::not_implemented),"NotImplemented")!=0) return 15;
    if(std::strcmp(origin_error_name(OriginError::lsx_no_response),"LsxNoResponse")!=0) return 16;
    if(!origin_error_is_warning(OriginError::success_proxy_ok)||origin_error_is_error(OriginError::success_proxy_ok)) return 17;
    return 0;
}
