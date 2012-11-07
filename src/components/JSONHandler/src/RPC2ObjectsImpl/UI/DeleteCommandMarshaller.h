#ifndef RPC2COMMUNICATION_UI_DELETECOMMANDMARSHALLER_INCLUDE
#define RPC2COMMUNICATION_UI_DELETECOMMANDMARSHALLER_INCLUDE

#include <string>
#include <json/json.h>

#include "../include/JSONHandler/RPC2Objects/UI/DeleteCommand.h"

namespace RPC2Communication
{
  namespace UI
  {

    struct DeleteCommandMarshaller
    {
      static bool checkIntegrity(DeleteCommand& e);
      static bool checkIntegrityConst(const DeleteCommand& e);
    
      static bool fromString(const std::string& s,DeleteCommand& e);
      static const std::string toString(const DeleteCommand& e);
    
      static bool fromJSON(const Json::Value& s,DeleteCommand& e);
      static Json::Value toJSON(const DeleteCommand& e);
    };
  }
}

#endif
