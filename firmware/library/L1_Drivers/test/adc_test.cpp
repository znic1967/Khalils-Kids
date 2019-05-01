#include "L0_LowLevel/LPC40xx.h"
#include "L1_Drivers/adc.hpp"
#include "L4_Testing/testing_frameworks.hpp"

EMIT_ALL_METHODS(Adc);

TEST_CASE("Testing adc", "[adc]")
{
  // Create local version of LPC_ADC
  LPC_ADC_TypeDef local_adc;
  LPC_SC_TypeDef local_sc;
  // Clear local adc registers
  memset(&local_adc, 0, sizeof(local_adc));
  memset(&local_sc, 0, sizeof(local_sc));
  // Set base registers to respective local variables to check
  // for any bit manipulations
  // Any manipulation will be directed to the respective local registers
  Adc::adc_base                              = &local_adc;
  Lpc40xxSystemController::system_controller = &local_sc;
  // Set mock for PinInterface
  Mock<PinInterface> mock_adc;
  Fake(Method(mock_adc, SetAsAnalogMode), Method(mock_adc, SetMode),
       Method(mock_adc, SetPinFunction));
  PinInterface & adc = mock_adc.get();

  // Create ports and pins to test and mock
  Adc channel0_mock(&adc, Adc::Channel::kChannel0);

  constexpr uint8_t kBurstBit = 16;
  SECTION("Initialization")
  {
    // Source "UM10562 LPC408x/7x User Manual" table 678 page 805
    constexpr uint8_t kPowerDownBit       = 21;
    constexpr uint8_t kPowerDown          = 0b1;
    constexpr uint16_t kChannelClkDivMask = 0b1111'1111;
    constexpr uint8_t kChannelClkDivBit   = 8;
    constexpr uint8_t kChannelBit0        = 0b1;
    constexpr uint32_t kAdcClockFrequency = 1'000'000;

    channel0_mock.Initialize(kAdcClockFrequency);
    // Check if bit 0 is set in local_adc.CR
    CHECK(kChannelBit0 == (local_adc.CR & 1));
    Verify(Method(mock_adc, SetPinFunction).Using(Adc::AdcMode::kCh0123Pins),
           Method(mock_adc, SetAsAnalogMode).Using(true),
           Method(mock_adc, SetMode).Using(PinInterface::Mode::kInactive));
    // TODO(#286): This test does not verify that the driver has been powered
    // up.
    // Check if any bits in the clock divider are set
    CHECK(((local_adc.CR >> kChannelClkDivBit) & kChannelClkDivMask) ==
          config::kSystemClockRate / kAdcClockFrequency);
    // Check bit 21 to see if power down bit is set in local_adc.CR
    CHECK(((local_adc.CR >> kPowerDownBit) & 1) == kPowerDown);
  }
  SECTION("Conversion and finished conversion")
  {
    // Source "UM10562 LPC408x/7x User Manual" table 678 page 805
    constexpr uint8_t kStartNoBurst = 0b1;
    constexpr uint8_t kStartBurst   = 0;
    constexpr uint8_t kStartBit     = 24;
    constexpr uint8_t kDone         = 0b1;
    constexpr uint8_t kDoneBit      = 31;

    // Check if bit 24 in local_adc.CR for the start bits is set
    local_adc.GDR |= (1 << kDoneBit);
    channel0_mock.Conversion();
    CHECK(((local_adc.CR >> kStartBit) & 1) == kStartNoBurst);
    channel0_mock.FinishedConversion();
    CHECK(((local_adc.GDR >> kDoneBit) & 1) == kDone);

    // Check if bits 24 to 26 in local_adc.CR are cleared for burst mode
    // conversion
    channel0_mock.BurstMode(true);
    channel0_mock.Conversion();
    CHECK(((local_adc.CR >> kStartBit) & 0b111) == kStartBurst);

    // Check if done bit is set after conversion
    channel0_mock.FinishedConversion();
    CHECK(((local_adc.GDR >> kDoneBit) & 1) == kDone);
  }
  SECTION("Burst mode")
  {
    // Source "UM10562 LPC408x/7x User Manual" table 678 page 805
    constexpr uint8_t kBurstOn  = 0b1;
    constexpr uint8_t kBurstOff = 0b0;

    // Only need to test if burst mode will turn on and off
    // Burst mode applies to all channels that are initialized
    channel0_mock.BurstMode(true);
    CHECK(((local_adc.CR >> kBurstBit) & 1) == kBurstOn);
    channel0_mock.BurstMode(false);
    CHECK(((local_adc.CR >> kBurstBit) & 1) == kBurstOff);
  }
  SECTION("Read result")
  {
    constexpr uint16_t kResultMask = 0xfff;
    constexpr uint8_t kResultBit   = 4;

    // Check if there is any value in the global data reg
    channel0_mock.ReadResult();
    CHECK(((local_adc.GDR >> kResultBit) & kResultMask) == 0);
  }

  Lpc40xxSystemController::system_controller = LPC_SC;
  Adc::adc_base                              = LPC_ADC;
}
