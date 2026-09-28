#pragma once

#include "_common.h"
#include "_classtype.h"
#include <Arduino.h>
#include <PID_v1.h>

#define PID_WINDOW_MIN      60000ul
#define PID_SAMPLE_MIN      10000ul
#define PID_DUTY_MIN_ON     2000ul
#define PID_DUTY_MIN_OFF    2000ul
#define PID_WINDOW_DEFAULT  600000ul
#define PID_SAMPLE_DEFAULT  60000ul

class _PIDControl : public _ClassType {
private:
  double _Input    = 0;
  double _Output   = 0;
  double _Setpoint = 0;

  PID _PID;

  ulong _SampleTime = 60000ul;
  ulong _Window     = 600000ul;

  ulong _OnTime  = 0;
  ulong _OffTime = 0;

  bool _LastComputeValid = false;

  void _updateDuty() {
    float _duty = (float)_Output / 100.0f;
    if (_duty < 0.0f)
      _duty = 0.0f;
    if (_duty > 1.0f)
      _duty = 1.0f;

    _OnTime  = (ulong)(_Window * _duty);
    _OffTime = _Window - _OnTime;

    if (_OnTime > 0 && _OnTime < PID_DUTY_MIN_ON)
      _OnTime = PID_DUTY_MIN_ON;

    if (_OffTime > 0 && _OffTime < PID_DUTY_MIN_OFF)
      _OffTime = PID_DUTY_MIN_OFF;
  }

public:
  _PIDControl(_SubType subType = _SubType::Undefined)
      : _ClassType(_MainType::Undefined, subType),
        _PID(&_Input, &_Output, &_Setpoint, 0.0, 0.0, 0.0, P_ON_M, DIRECT) {
    _PID.SetOutputLimits(0, 100);
    _PID.SetSampleTime((int)_SampleTime);
    _PID.SetMode(AUTOMATIC);
  }

  void setTunings(float Kp, float Ki, float Kd) {
    _PID.SetTunings((double)Kp, (double)Ki, (double)Kd);
  }

  void setSetpoint(float Setpoint) {
    _Setpoint = (double)Setpoint;
  }

  void setSampleTime(ulong SampleTime) {
    if (SampleTime < PID_SAMPLE_MIN)
      SampleTime = PID_SAMPLE_MIN;
    _SampleTime = SampleTime;
    _PID.SetSampleTime((int)_SampleTime);
  }

  void setWindow(ulong Window) {
    if (Window < PID_WINDOW_MIN)
      Window = PID_WINDOW_MIN;
    _Window = Window;
    _updateDuty();
  }

  float getOutputPercent() const {
    return (float)_Output;
  }

  ulong getOnTime() const {
    return _OnTime;
  }

  ulong getOffTime() const {
    return _OffTime;
  }

  bool LastComputeValid() const {
    return _LastComputeValid;
  }

  bool Tick(float ProcessValue, bool IsValid) {
    if (!IsValid) {
      _PID.SetMode(MANUAL);
      _Output = 0.0;
      _OnTime = 0;
      _OffTime = 0;
      _LastComputeValid = false;
      return false;
    }

    _PID.SetMode(AUTOMATIC);
    _Input = (double)ProcessValue;

    if (!_PID.Compute()) {
      _LastComputeValid = false;
      return false;
    }

    _updateDuty();
    _LastComputeValid = true;

    debug.tprintf("PID %s: PV=%.2f SP=%.2f out=%.1f%% on=%lu off=%lu\n",
                  Name(),
                  (float)_Input,
                  (float)_Setpoint,
                  (float)_Output,
                  _OnTime,
                  _OffTime);

    return true;
  }
};
