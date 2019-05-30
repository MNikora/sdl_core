#include "vehicle_info_plugin/vehicle_info_app_extension.h"
#include "application_manager/application_manager.h"
#include "application_manager/mock_application.h"
#include "application_manager/mock_rpc_plugin.h"
#include "gtest/gtest.h"
#include "vehicle_info_plugin/vehicle_info_plugin.h"

namespace test {
namespace components {
namespace vehicle_info_plugin {

using ::application_manager::plugin_manager::MockRPCPlugin;
using test::components::application_manager_test::MockApplication;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::vehicle_info_plugin::VehicleInfoAppExtension;
using ::vehicle_info_plugin::VehicleInfoPlugin;

typedef mobile_apis::VehicleDataType::eType VehicleDataType;

const VehicleDataType speed = VehicleDataType::VEHICLEDATA_SPEED;
const VehicleDataType rpm = VehicleDataType::VEHICLEDATA_RPM;

class VehicleInfoAppExtensionTest : public ::testing::Test {
 public:
  VehicleInfoAppExtensionTest() : vi_ext(vi_plugin_, mock_app_) {}

 protected:
  virtual void SetUp() OVERRIDE {}
  VehicleInfoPlugin vi_plugin_;
  MockApplication mock_app_;
  VehicleInfoAppExtension vi_ext;
};

TEST_F(VehicleInfoAppExtensionTest, Subscribe_SUCCESS) {
  bool ret = vi_ext.subscribeToVehicleInfo(speed);
  EXPECT_TRUE(ret);
}

TEST_F(VehicleInfoAppExtensionTest, DoubleSubscribe_FAILURE) {
  bool ret = vi_ext.subscribeToVehicleInfo(speed);
  EXPECT_TRUE(ret);
  ret = vi_ext.subscribeToVehicleInfo(speed);
  EXPECT_FALSE(ret);
}

TEST_F(VehicleInfoAppExtensionTest, IsSubscribed_SUCCESS) {
  bool ret = vi_ext.subscribeToVehicleInfo(speed);
  EXPECT_TRUE(ret);
  ret = vi_ext.isSubscribedToVehicleInfo(speed);
  EXPECT_TRUE(ret);
  ret = vi_ext.isSubscribedToVehicleInfo(rpm);
  EXPECT_FALSE(ret);
}

TEST_F(VehicleInfoAppExtensionTest, UnsubscribeFromAllData_SUCCESS) {
  bool ret = vi_ext.subscribeToVehicleInfo(speed);
  EXPECT_TRUE(ret);
  ret = vi_ext.subscribeToVehicleInfo(rpm);
  EXPECT_TRUE(ret);
  vi_ext.unsubscribeFromVehicleInfo();
  ret = vi_ext.isSubscribedToVehicleInfo(speed);
  EXPECT_FALSE(ret);
  ret = vi_ext.isSubscribedToVehicleInfo(rpm);
  EXPECT_FALSE(ret);
}

TEST_F(VehicleInfoAppExtensionTest, UnsubscribeFromSpeed_SUCCESS) {
  bool ret = vi_ext.subscribeToVehicleInfo(speed);
  EXPECT_TRUE(ret);
  ret = vi_ext.subscribeToVehicleInfo(rpm);
  EXPECT_TRUE(ret);
  vi_ext.unsubscribeFromVehicleInfo(speed);
  ret = vi_ext.isSubscribedToVehicleInfo(speed);
  EXPECT_FALSE(ret);
  ret = vi_ext.isSubscribedToVehicleInfo(rpm);
  EXPECT_TRUE(ret);
}
}  // namespace vehicle_info_plugin
}  // namespace components
}  // namespace test