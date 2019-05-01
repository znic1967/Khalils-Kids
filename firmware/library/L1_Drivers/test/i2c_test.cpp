#include <cstdint>
#include "L0_LowLevel/interrupt.hpp"
#include "L0_LowLevel/LPC40xx.h"
#include "L1_Drivers/i2c.hpp"
#include "L4_Testing/testing_frameworks.hpp"
#include "utility/enum.hpp"
#include "utility/status.hpp"

EMIT_ALL_METHODS(I2c);

TEST_CASE("Testing I2C", "[i2c]")
{
  Mock<PinInterface> mock_sda_pin;
  Fake(Method(mock_sda_pin, SetPinFunction),
       Method(mock_sda_pin, SetAsOpenDrain), Method(mock_sda_pin, SetMode));
  PinInterface & sda = mock_sda_pin.get();

  Mock<PinInterface> mock_scl_pin;
  Fake(Method(mock_scl_pin, SetPinFunction),
       Method(mock_scl_pin, SetAsOpenDrain), Method(mock_scl_pin, SetMode));
  PinInterface & scl = mock_scl_pin.get();

  // Dummy address used by test sections
  constexpr uint8_t kAddress = 0x33;
  // Create a local register
  LPC_I2C_TypeDef local_i2c;
  // Clear local i2c registers
  memset(&local_i2c, 0, sizeof(local_i2c));
  LPC_SC_TypeDef local_sc;
  Lpc40xxSystemController::system_controller = &local_sc;
  // Setting i2c register to local_i2c
  constexpr uint8_t kI2cPort = util::Value(I2c::Port::kI2c0);
  I2c::i2c[kI2cPort]         = &local_i2c;

  I2c test_subject(I2c::Port::kI2c0, &sda, &scl, false);

  SECTION("Initialize")
  {
    // Source: "UM10562 LPC408x/407x User manual" table 84 page 443
    constexpr uint32_t kExpectedControlClear =
        I2c::Control::kAssertAcknowledge | I2c::Control::kStart |
        I2c::Control::kStop | I2c::Control::kInterrupt;
    test_subject.Initialize();

    constexpr float kScll =
        ((config::kSystemClockRate / 75'000.0f) / 2.0f) * 0.7f;
    constexpr float kSclh =
        ((config::kSystemClockRate / 75'000.0f) / 2.0f) * 1.3f;

    constexpr uint32_t kLow  = static_cast<uint32_t>(kScll);
    constexpr uint32_t kHigh = static_cast<uint32_t>(kSclh);
    CHECK(kLow == local_i2c.SCLL);
    CHECK(kHigh == local_i2c.SCLH);
    CHECK(local_i2c.CONCLR == kExpectedControlClear);
    CHECK(local_i2c.CONSET == I2c::Control::kInterfaceEnable);
    CHECK(dynamic_isr_vector_table[I2c::kIrq[kI2cPort] + kIrqOffset] ==
          I2c::kHandlers[kI2cPort]);

    Verify(Method(mock_sda_pin, SetPinFunction).Using(I2c::kI2cPort2Function));
    Verify(Method(mock_sda_pin, SetAsOpenDrain));
    Verify(Method(mock_sda_pin, SetMode).Using(PinInterface::Mode::kInactive));

    Verify(Method(mock_scl_pin, SetPinFunction).Using(I2c::kI2cPort2Function));
    Verify(Method(mock_scl_pin, SetAsOpenDrain));
    Verify(Method(mock_scl_pin, SetMode).Using(PinInterface::Mode::kInactive));
  }
  SECTION("Transaction test")
  {
    constexpr uint32_t kFakeDeviceAddress = 0x22;
    I2cInterface::Transaction_t transaction;
    transaction.address = 0x22;

    transaction.operation = I2cInterface::Operation::kWrite;
    // With a read operation, the address is shifted by 1 and LSB is set to 0.
    CHECK((kFakeDeviceAddress << 1) == transaction.GetProperAddress());
    // With a read operation, the address is shifted by 1 and LSB is set to 1.
    transaction.operation = I2cInterface::Operation::kRead;
    CHECK(((kFakeDeviceAddress << 1) | 1) == transaction.GetProperAddress());
  }
  SECTION("Read Setup")
  {
    local_i2c.CONSET = 0;
    uint8_t read_buffer[10];

    test_subject.Read(kAddress, read_buffer, sizeof(read_buffer));
    I2cInterface::Transaction_t actual_transaction =
        I2c::GetTransactionInfo(I2c::Port::kI2c0);

    CHECK(actual_transaction.address == kAddress);
    CHECK(actual_transaction.data_out == nullptr);
    CHECK(actual_transaction.out_length == 0);
    CHECK(actual_transaction.data_in == read_buffer);
    CHECK(actual_transaction.in_length == sizeof(read_buffer));
    CHECK(actual_transaction.position == 0);
    CHECK(actual_transaction.repeated == false);
    CHECK(actual_transaction.busy == true);
    CHECK(actual_transaction.status == Status::kSuccess);
    CHECK(actual_transaction.operation == I2c::Operation::kRead);
    CHECK(actual_transaction.timeout == I2c::kI2cTimeout);
    CHECK(local_i2c.CONSET == I2c::Control::kStart);
  }
  SECTION("Write Setup")
  {
    local_i2c.CONSET = 0;
    uint8_t write_buffer[10];

    test_subject.Write(kAddress, write_buffer, sizeof(write_buffer));
    I2cInterface::Transaction_t actual_transaction =
        I2c::GetTransactionInfo(I2c::Port::kI2c0);

    CHECK(actual_transaction.address == kAddress);
    CHECK(actual_transaction.data_out == write_buffer);
    CHECK(actual_transaction.out_length == sizeof(write_buffer));
    CHECK(actual_transaction.data_in == nullptr);
    CHECK(actual_transaction.in_length == 0);
    CHECK(actual_transaction.position == 0);
    CHECK(actual_transaction.repeated == false);
    CHECK(actual_transaction.busy == true);
    CHECK(actual_transaction.status == Status::kSuccess);
    CHECK(actual_transaction.operation == I2c::Operation::kWrite);
    CHECK(actual_transaction.timeout == I2c::kI2cTimeout);
    CHECK(local_i2c.CONSET == I2c::Control::kStart);
  }
  SECTION("Write and Read Setup")
  {
    local_i2c.CONSET = 0;
    uint8_t read_buffer[10];
    uint8_t write_buffer[15];

    test_subject.WriteThenRead(kAddress, write_buffer, sizeof(write_buffer),
                               read_buffer, sizeof(read_buffer));
    I2cInterface::Transaction_t actual_transaction =
        I2c::GetTransactionInfo(I2c::Port::kI2c0);

    CHECK(actual_transaction.address == kAddress);
    CHECK(actual_transaction.data_out == write_buffer);
    CHECK(actual_transaction.out_length == sizeof(write_buffer));
    CHECK(actual_transaction.data_in == read_buffer);
    CHECK(actual_transaction.in_length == sizeof(read_buffer));
    CHECK(actual_transaction.position == 0);
    CHECK(actual_transaction.repeated == true);
    CHECK(actual_transaction.busy == true);
    CHECK(actual_transaction.status == Status::kSuccess);
    CHECK(actual_transaction.operation == I2c::Operation::kWrite);
    CHECK(actual_transaction.timeout == I2c::kI2cTimeout);
    CHECK(local_i2c.CONSET == I2c::Control::kStart);
  }

#define CHECK_BITS(mask, reg) CHECK(mask == (reg & mask))

  auto setup_state_machine = [&](I2c::MasterState state) {
    local_i2c.STAT   = util::Value(state);
    local_i2c.CONSET = 0;
    local_i2c.CONCLR = 0;
    local_i2c.DAT    = 0;
  };

  SECTION("I2C State Machine: kBusError")
  {
    setup_state_machine(I2c::MasterState::kBusError);
    I2c::I2cHandler<I2c::Port::kI2c0>();
    I2cInterface::Transaction_t actual_transaction =
        I2c::GetTransactionInfo(I2c::Port::kI2c0);
    CHECK(Status::kBusError == actual_transaction.status);
    CHECK_BITS(I2c::Control::kAssertAcknowledge, local_i2c.CONSET);
    CHECK_BITS(I2c::Control::kStop, local_i2c.CONSET);
    CHECK_BITS(I2c::Control::kInterrupt, local_i2c.CONCLR);
  }
  SECTION("I2C State Machine: kStartCondition")
  {
    setup_state_machine(I2c::MasterState::kStartCondition);
    test_subject.Write(kAddress, nullptr, 0);
    I2c::I2cHandler<I2c::Port::kI2c0>();
    CHECK((kAddress << 1) == local_i2c.DAT);
    CHECK_BITS(I2c::Control::kInterrupt, local_i2c.CONCLR);
  }
  SECTION("I2C State Machine: kRepeatedStart")
  {
    setup_state_machine(I2c::MasterState::kRepeatedStart);
    test_subject.Write(kAddress, nullptr, 0);
    I2c::I2cHandler<I2c::Port::kI2c0>();
    CHECK(((kAddress << 1) | 1) == local_i2c.DAT);
    CHECK_BITS(I2c::Control::kInterrupt, local_i2c.CONCLR);
  }
  SECTION("I2C State Machine: kSlaveAddressWriteSentRecievedAck")
  {
    SECTION("Non-zero length buffer")
    {
      setup_state_machine(I2c::MasterState::kSlaveAddressWriteSentRecievedAck);
      uint8_t write_buffer[] = { 0x55, 0x22, 0xAA };
      test_subject.Write(kAddress, write_buffer, sizeof(write_buffer));
      I2c::I2cHandler<I2c::Port::kI2c0>();
      // In this state, you will send the first byte from the write buffer
      // and thus, no other value needs to be check except for [0].
      CHECK(write_buffer[0] == local_i2c.DAT);
      CHECK_BITS(I2c::Control::kStart, local_i2c.CONCLR);
      CHECK_BITS(I2c::Control::kInterrupt, local_i2c.CONCLR);
    }
    SECTION("Zero length buffer")
    {
      setup_state_machine(I2c::MasterState::kSlaveAddressWriteSentRecievedAck);

      test_subject.Write(kAddress, nullptr, 0);
      I2c::I2cHandler<I2c::Port::kI2c0>();
      I2cInterface::Transaction_t actual_transaction =
          I2c::GetTransactionInfo(I2c::Port::kI2c0);

      CHECK(!actual_transaction.busy);
      CHECK_BITS(I2c::Control::kStop, local_i2c.CONSET);
      CHECK_BITS(I2c::Control::kStart, local_i2c.CONCLR);
      CHECK_BITS(I2c::Control::kInterrupt, local_i2c.CONCLR);
    }
  }
  SECTION("I2C State Machine: kSlaveAddressWriteSentRecievedNack")
  {
    setup_state_machine(I2c::MasterState::kSlaveAddressWriteSentRecievedNack);

    I2c::I2cHandler<I2c::Port::kI2c0>();
    I2cInterface::Transaction_t actual_transaction =
        I2c::GetTransactionInfo(I2c::Port::kI2c0);

    CHECK(!actual_transaction.busy);
    CHECK_BITS(I2c::Control::kStart, local_i2c.CONCLR);
    CHECK_BITS(I2c::Control::kStop, local_i2c.CONSET);
    CHECK_BITS(I2c::Control::kInterrupt, local_i2c.CONCLR);
  }
  SECTION("I2C State Machine: kTransmittedDataRecievedAck wo/ repeat start")
  {
    setup_state_machine(I2c::MasterState::kTransmittedDataRecievedAck);
    uint8_t write_buffer[] = { 'A', 'B', 'C', 'D' };
    test_subject.Write(kAddress, write_buffer, sizeof(write_buffer));
    I2cInterface::Transaction_t actual_transaction;
    // Each iteration should be load the next byte from write_buffer into
    // the local_i2c.DAT
    for (size_t i = 0; i < sizeof(write_buffer); i++)
    {
      setup_state_machine(I2c::MasterState::kTransmittedDataRecievedAck);

      I2c::I2cHandler<I2c::Port::kI2c0>();

      actual_transaction = I2c::GetTransactionInfo(I2c::Port::kI2c0);
      CHECK(local_i2c.DAT == write_buffer[i]);
      CHECK(actual_transaction.busy);
    }

    // WriteThenRead should set the transaction to repeated
    I2c::I2cHandler<I2c::Port::kI2c0>();
    actual_transaction = I2c::GetTransactionInfo(I2c::Port::kI2c0);

    CHECK(!actual_transaction.busy);
    CHECK_BITS(I2c::Control::kStop, local_i2c.CONSET);
  }
  SECTION("I2C State Machine: kTransmittedDataRecievedAck w/ repeat start")
  {
    setup_state_machine(I2c::MasterState::kTransmittedDataRecievedAck);
    uint8_t write_buffer[] = { 'A', 'B', 'C', 'D' };
    test_subject.WriteThenRead(kAddress, write_buffer, sizeof(write_buffer),
                               nullptr, 0);
    I2cInterface::Transaction_t actual_transaction;
    // Each iteration should be load the next byte from write_buffer into
    // the local_i2c.DAT
    for (size_t i = 0; i < sizeof(write_buffer); i++)
    {
      setup_state_machine(I2c::MasterState::kTransmittedDataRecievedAck);

      I2c::I2cHandler<I2c::Port::kI2c0>();

      actual_transaction = I2c::GetTransactionInfo(I2c::Port::kI2c0);
      CHECK(local_i2c.DAT == write_buffer[i]);
      CHECK(actual_transaction.busy);
    }

    // WriteThenRead should set the transaction to repeated
    I2c::I2cHandler<I2c::Port::kI2c0>();
    actual_transaction = I2c::GetTransactionInfo(I2c::Port::kI2c0);

    CHECK(actual_transaction.operation == I2cInterface::Operation::kRead);
    CHECK(actual_transaction.position == 0);
    CHECK_BITS(I2c::Control::kStart, local_i2c.CONSET);
  }
  SECTION("I2C State Machine: kTransmittedDataRecievedNack")
  {
    setup_state_machine(I2c::MasterState::kTransmittedDataRecievedNack);

    I2c::I2cHandler<I2c::Port::kI2c0>();
    I2cInterface::Transaction_t actual_transaction =
        I2c::GetTransactionInfo(I2c::Port::kI2c0);

    CHECK(!actual_transaction.busy);
    CHECK_BITS(I2c::Control::kStop, local_i2c.CONSET);
    CHECK_BITS(I2c::Control::kInterrupt, local_i2c.CONCLR);
  }
  SECTION("I2C State Machine: kArbitrationLost")
  {
    setup_state_machine(I2c::MasterState::kArbitrationLost);

    I2c::I2cHandler<I2c::Port::kI2c0>();

    CHECK_BITS(I2c::Control::kStart, local_i2c.CONSET);
    CHECK_BITS(I2c::Control::kInterrupt, local_i2c.CONCLR);
  }
  SECTION("I2C State Machine: kSlaveAddressReadSentRecievedAck")
  {
    SECTION("Non-zero length buffer")
    {
      setup_state_machine(I2c::MasterState::kSlaveAddressReadSentRecievedAck);
      uint8_t read_buffer[3];
      test_subject.Read(kAddress, read_buffer, sizeof(read_buffer));

      I2c::I2cHandler<I2c::Port::kI2c0>();

      CHECK_BITS(I2c::Control::kStart, local_i2c.CONCLR);
      CHECK_BITS(I2c::Control::kAssertAcknowledge, local_i2c.CONSET);
      CHECK_BITS(I2c::Control::kInterrupt, local_i2c.CONCLR);
    }
    SECTION("Zero length buffer")
    {
      setup_state_machine(I2c::MasterState::kSlaveAddressReadSentRecievedAck);
      test_subject.Read(kAddress, nullptr, 0);

      I2c::I2cHandler<I2c::Port::kI2c0>();

      CHECK_BITS(I2c::Control::kStart, local_i2c.CONCLR);
      CHECK_BITS(I2c::Control::kInterrupt, local_i2c.CONCLR);
    }
  }
  SECTION("I2C State Machine: kSlaveAddressReadSentRecievedNack")
  {
    setup_state_machine(I2c::MasterState::kSlaveAddressReadSentRecievedNack);

    I2c::I2cHandler<I2c::Port::kI2c0>();
    I2cInterface::Transaction_t actual_transaction =
        I2c::GetTransactionInfo(I2c::Port::kI2c0);

    CHECK(Status::kDeviceNotFound == actual_transaction.status);
    CHECK(!actual_transaction.busy);
    CHECK_BITS(I2c::Control::kStop, local_i2c.CONSET);
    CHECK_BITS(I2c::Control::kStart, local_i2c.CONCLR);
    CHECK_BITS(I2c::Control::kInterrupt, local_i2c.CONCLR);
  }
  SECTION("I2C State Machine: kRecievedDataRecievedAck")
  {
    constexpr uint8_t kI2cReadData[]          = { 'A', 'B', 'C', 'D' };
    uint8_t read_buffer[sizeof(kI2cReadData)] = { 0 };
    test_subject.Read(kAddress, read_buffer, sizeof(read_buffer));
    I2cInterface::Transaction_t actual_transaction;
    // Each iteration should be load a new byte from kI2cReadData into the
    // read_buffer array;
    for (size_t i = 0; i < sizeof(read_buffer) - 1; i++)
    {
      setup_state_machine(I2c::MasterState::kRecievedDataRecievedAck);
      local_i2c.DAT = kI2cReadData[i];

      I2c::I2cHandler<I2c::Port::kI2c0>();

      actual_transaction = I2c::GetTransactionInfo(I2c::Port::kI2c0);
      CHECK(kI2cReadData[i] == read_buffer[i]);
      if (i < sizeof(read_buffer) - 2)
      {
        CHECK(actual_transaction.busy);
        CHECK_BITS(I2c::Control::kAssertAcknowledge, local_i2c.CONSET);
      }
      // Check for the NACK at the end of the read buffer
      else
      {
        CHECK(!actual_transaction.busy);
        CHECK_BITS(I2c::Control::kAssertAcknowledge, local_i2c.CONCLR);
      }
    }
    // Last byte should be transferred
    setup_state_machine(I2c::MasterState::kRecievedDataRecievedAck);
    constexpr size_t kEnd = sizeof(read_buffer) - 1;
    local_i2c.DAT         = kI2cReadData[kEnd];

    I2c::I2cHandler<I2c::Port::kI2c0>();

    actual_transaction = I2c::GetTransactionInfo(I2c::Port::kI2c0);
    // At this point, the limit for the length should have been reached.
    CHECK(!actual_transaction.busy);
    CHECK(kI2cReadData[kEnd] == read_buffer[kEnd]);
    CHECK_BITS(I2c::Control::kAssertAcknowledge, local_i2c.CONCLR);

    // Another is loaded into the DAT register, but it should not be found
    // in the read_buffer
    setup_state_machine(I2c::MasterState::kRecievedDataRecievedAck);
    local_i2c.DAT = 'F';
    I2c::I2cHandler<I2c::Port::kI2c0>();

    for (size_t i = 0; i < sizeof(read_buffer); i++)
    {
      CHECK(local_i2c.DAT != read_buffer[i]);
    }
    actual_transaction = I2c::GetTransactionInfo(I2c::Port::kI2c0);
    CHECK(!actual_transaction.busy);
    CHECK_BITS(I2c::Control::kAssertAcknowledge, local_i2c.CONCLR);
    CHECK_BITS(I2c::Control::kInterrupt, local_i2c.CONCLR);
  }
  SECTION("I2C State Machine: kRecievedDataRecievedNack")
  {
    setup_state_machine(I2c::MasterState::kRecievedDataRecievedNack);
    local_i2c.DAT = 0xDE;
    uint8_t read_buffer[5];
    test_subject.Read(kAddress, read_buffer, sizeof(read_buffer));

    I2c::I2cHandler<I2c::Port::kI2c0>();
    I2cInterface::Transaction_t actual_transaction =
        I2c::GetTransactionInfo(I2c::Port::kI2c0);

    CHECK(local_i2c.DAT == read_buffer[0]);
    CHECK(!actual_transaction.busy);
    CHECK_BITS(I2c::Control::kStop, local_i2c.CONSET);
    CHECK_BITS(I2c::Control::kInterrupt, local_i2c.CONCLR);
  }
  SECTION("I2C State Machine: kDoNothing")
  {
    setup_state_machine(I2c::MasterState::kDoNothing);

    I2c::I2cHandler<I2c::Port::kI2c0>();

    CHECK_BITS(I2c::Control::kInterrupt, local_i2c.CONCLR);
  }
  SECTION("I2C State Machine: Unknown/Default")
  {
    setup_state_machine(I2c::MasterState(0xFF));

    I2c::I2cHandler<I2c::Port::kI2c0>();
    CHECK_BITS(I2c::Control::kStop, local_i2c.CONCLR);
    CHECK_BITS(I2c::Control::kInterrupt, local_i2c.CONCLR);
  }
  Lpc40xxSystemController::system_controller = LPC_SC;
  I2c::i2c[util::Value(I2c::Port::kI2c0)]    = LPC_I2C0;
}
