//  Usage:
//      // Two different methods of creating the Pwm object
//      Pwm p2_0 = Pwm::CreatePwm<1>();
//      Pwm p2_0(1);
//      p2_0.Initialize(500);
//      p2_0.SetDutyCycle(0.75);
//      p2_0.SetFrequeny(1'000);
#pragma once

#include <cstdint>
#include <cstdio>

#include "L0_LowLevel/LPC40xx.h"
#include "L0_LowLevel/system_controller.hpp"
#include "L1_Drivers/pin.hpp"
#include "utility/log.hpp"

class PwmInterface
{
 public:
  static constexpr uint32_t kDefaultFrequency = 1'000;
  virtual void Initialize(uint32_t frequency_hz = kDefaultFrequency) = 0;
  virtual void SetDutyCycle(float duty_cycle)                        = 0;
  virtual float GetDutyCycle()                                       = 0;
  virtual void SetFrequency(uint32_t frequency_hz)                   = 0;
  virtual uint32_t GetFrequency()                                    = 0;
  virtual void EnablePwm(bool enable = true)                         = 0;
  virtual uint32_t GetMatchRegister0()                               = 0;
};

class Pwm final : public PwmInterface, protected Lpc40xxSystemController
{
 public:
  static constexpr uint8_t kPwmFunction      = 0b001;
  inline static LPC_PWM_TypeDef * pwm1       = LPC_PWM1;
  inline static volatile uint32_t * match[7] = { &pwm1->MR0, &pwm1->MR1,
                                                 &pwm1->MR2, &pwm1->MR3,
                                                 &pwm1->MR4, &pwm1->MR5,
                                                 &pwm1->MR6 };

  enum SysClk : uint8_t
  {
    kActivatePwm0 = (1 << 5),
    kActivatePwm1 = (1 << 6)
  };
  enum PwmConfigure : uint8_t
  {
    kTimerMode     = (3 << 0),
    kCounterEnable = (1 << 0),
    kCounterReset  = (1 << 1),
    kResetMr0      = (1 << 1),
    kPwmEnable     = (1 << 3)
  };

  template <unsigned pwm_channel>
  static Pwm CreatePwm()
  {
    static_assert(pwm_channel <= 6, "Invalid PWM channel must be between 0-6.");
    return Pwm(pwm_channel);
  }

  explicit constexpr Pwm(uint8_t channel)
      : pwm_(&pwm_pin_),
        pwm_pin_(2, static_cast<uint8_t>(channel - 1)),
        channel_(channel)
  {
  }

  explicit constexpr Pwm(uint8_t channel, PinInterface * pin)
      : pwm_(pin), pwm_pin_(Pin::CreateInactivePin()), channel_(channel)
  {
  }

  void Initialize(uint32_t frequency_hz = kDefaultFrequency) override
  {
    SJ2_ASSERT_FATAL(1 <= channel_ && channel_ <= 6,
                     "Channel must be between 1 and 6.");
    // Enables PWM1 power/clock control bit
    // TODO(#): Replace direct manipulation of system clock register.
    PowerUpPeripheral(Lpc40xxSystemController::Peripherals::kPwm1);
    // Resets PWMTC on Match with MR0
    pwm1->MCR |= PwmConfigure::kResetMr0;
    pwm1->MR0 = GetPeripheralFrequency() / frequency_hz;
    // Enables PWM TC and PC for counting and enables PWM mode
    EnablePwm();
    pwm1->CTCR &= ~PwmConfigure::kTimerMode;
    // Enables PWM[channel] output
    pwm1->PCR |= PwmOutputEnable(channel_);
    pwm_->SetPinFunction(kPwmFunction);
  }

  void SetDutyCycle(float duty_cycle) override
  {
    SJ2_ASSERT_FATAL(0.0f <= duty_cycle && duty_cycle <= 1.0f,
                     "duty_cycle of Duty Cycle provided is out of bounds.");
    *match[channel_] = CalculateDutyCycle(duty_cycle);
    pwm1->LER |= (1 << channel_);
  }

  float GetDutyCycle() override
  {
    return (static_cast<float>(*match[channel_]) /
            static_cast<float>(GetMatchRegister0()));
  }

  void SetFrequency(uint32_t frequency_hz) override
  {
    SJ2_ASSERT_FATAL(frequency_hz != 0, "Pwm Frequency cannot be zero Hz.");
    // Disables PWM mode; this will reset all counters to 0
    // And allow us to update MR0
    float previous_duty_cycle = GetDutyCycle();
    EnablePwm(false);
    pwm1->MR0 = GetPeripheralFrequency() / frequency_hz;
    SetDutyCycle(previous_duty_cycle);
    EnablePwm();
  }

  uint32_t GetFrequency() override
  {
    uint32_t match_register0 = GetMatchRegister0();
    uint32_t result          = 0;
    if (match_register0 != 0)
    {
      result = GetPeripheralFrequency() / match_register0;
    }
    return result;
  }

  void EnablePwm(bool enable = true) override
  {
    if (enable)
    {
      pwm1->TCR |= PwmConfigure::kCounterReset;
      pwm1->TCR &= ~PwmConfigure::kCounterReset;
      pwm1->TCR |= PwmConfigure::kCounterEnable | PwmConfigure::kPwmEnable;
    }
    else
    {
      pwm1->TCR &= ~PwmConfigure::kPwmEnable;
    }
  }

  uint32_t GetMatchRegister0() override
  {
    return pwm1->MR0;
  }

  inline uint32_t CalculateDutyCycle(float percent)
  {
    return static_cast<uint32_t>(
        (percent * static_cast<float>(GetMatchRegister0())));
  }

  inline uint32_t PwmOutputEnable(uint8_t channel)
  {
    return (1 << (channel + 8));
  }

 private:
  PinInterface * pwm_;
  Pin pwm_pin_;

  // Will signify current PWM1 channel
  uint8_t channel_;
};
