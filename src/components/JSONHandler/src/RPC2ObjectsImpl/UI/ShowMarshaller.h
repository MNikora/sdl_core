#ifndef RPC2COMMUNICATION_UI_SHOWMARSHALLER_INCLUDE
#define RPC2COMMUNICATION_UI_SHOWMARSHALLER_INCLUDE

#include <string>
#include <json/json.h>

#include "../include/JSONHandler/RPC2Objects/UI/Show.h"

namespace RPC2Communication
{
  namespace UI
  {

    struct ShowMarshaller
    {
      static bool checkIntegrity(Show& e);
      static bool checkIntegrityConst(const Show& e);
    
      static bool fromString(const std::string& s,Show& e);
      static const std::string toString(const Show& e);
    
      static bool fromJSON(const Json::Value& s,Show& e);
      static Json::Value toJSON(const Show& e);
    };
  }
}

#endif
