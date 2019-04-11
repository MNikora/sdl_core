/*
 Copyright (c) 2018, Ford Motor Company
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following
 disclaimer in the documentation and/or other materials provided with the
 distribution.

 Neither the name of the Ford Motor Company nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#include "rc_rpc_plugin/commands/mobile/get_interior_vehicle_data_response.h"
#include "rc_rpc_plugin/rc_module_constants.h"
#include "utils/macro.h"

namespace rc_rpc_plugin {
namespace commands {

GetInteriorVehicleDataResponse::GetInteriorVehicleDataResponse(
    const app_mngr::commands::MessageSharedPtr& message,
    const RCCommandParams& params)
    : application_manager::commands::CommandResponseImpl(
          message,
          params.application_manager_,
          params.rpc_service_,
          params.hmi_capabilities_,
          params.policy_handler_) {}
GetInteriorVehicleDataResponse::~GetInteriorVehicleDataResponse() {}

void GetInteriorVehicleDataResponse::Run() {
  LOG4CXX_AUTO_TRACE(logger_);

  RemoveRedundantGPSDataParams(
      (*message_)[application_manager::strings::msg_params]);
  application_manager_.GetRPCService().SendMessageToMobile(message_);
}

void GetInteriorVehicleDataResponse::RemoveRedundantGPSDataParams(
    smart_objects::SmartObject& msg_params) {
  LOG4CXX_AUTO_TRACE(logger_);
  auto& module_data = msg_params[message_params::kModuleData];

  if (module_data.keyExists(message_params::kRadioControlData)) {
    auto& rc_data = module_data[message_params::kRadioControlData];
    if (rc_data.keyExists(message_params::kSisData) &&
        rc_data[message_params::kSisData].keyExists(
            app_mngr::strings::station_location)) {
      auto& location_data = rc_data[message_params::kSisData]
                                   [app_mngr::strings::station_location];
      smart_objects::SmartObject new_location_data =
          smart_objects::SmartObject(smart_objects::SmartType_Map);
      new_location_data[app_mngr::strings::latitude_degrees] =
          location_data[app_mngr::strings::latitude_degrees];
      new_location_data[app_mngr::strings::longitude_degrees] =
          location_data[app_mngr::strings::longitude_degrees];
      if (location_data.keyExists(app_mngr::strings::altitude)) {
        new_location_data[app_mngr::strings::altitude] =
            location_data[app_mngr::strings::altitude];
      }

      rc_data[message_params::kSisData][app_mngr::strings::station_location] =
          new_location_data;
    }
  }
}

}  // namespace commands
}  // namespace rc_rpc_plugin
