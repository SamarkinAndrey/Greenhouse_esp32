/*
struct _IntervalData {
  ulong MillisFrom = 0;
  ulong MillisTo   = 0;

  float ValueFrom = 0;
  float ValueTo   = 0;

  float ValueMin = 0;
  float ValueMax = 0;
  float ValueAvg = 0;

  float ValueUp   = 0;
  float ValueDown = 0;

  float Vibration        = 0;
  float VibrationPercent = 0;

  size_t RecordCount = 0;

  char MillisFromStr[50] = "";
  char MillisToStr[50]   = "";
};

template <typename T>
struct _HistoryItem {
  static_assert(std::is_arithmetic<T>::value, "_HistoryItem: type <T> must be numeric.");

  T     Value  = T();
  ulong Millis = 0;

  _HistoryItem(T _Value = T(), ulong _Millis = 0)
      : Value(_Value), Millis(_Millis) {
  }
};

template <typename T>
class _ReadingsHistory : public _ClassType, public _UsingIniciator {
  static_assert(std::is_arithmetic<T>::value, "_ReadingsHistory: type <T> must be numeric.");

private:
  _HistoryItem<T> *_history = nullptr;

  size_t _size_max = 0;
  size_t _size     = 0;
  size_t _start    = 0;
  size_t _end      = 0;

  void _Destroy() {
    if (!_history)
      return;

    delete[] _history;

    _history = nullptr;
  }

public:
  ulong MillisFrom = 0;
  ulong MillisTo   = 0;

  char MillisFromStr[50] = "";
  char MillisToStr[50]   = "";

  _ReadingsHistory() {
  }

  ~_ReadingsHistory() {
    _Destroy();
  }

  void setEnabled(void *Iniciator, bool Value, size_t Size = READINGS_HISTORY_SIZE) {
    if ((Value == IsEnabled()) || (!Value && (_Iniciator != Iniciator)))
      return;

    _Destroy();

    Clear();

    if (Value) {
      _size_max = (Size < 1) ? 1 : (Size > READINGS_HISTORY_SIZE) ? READINGS_HISTORY_SIZE :
                                                                    Size;

      _history = new (std::nothrow) _HistoryItem<T>[_size_max];

      if (!_history) {
        _size_max = 0;

        return;
      }
    } else {
      _size_max = 0;
    }

    _Iniciator = Iniciator;

    debug.tprintf("%s.setEnabled(%s)\n", Name(), Value ? "true" : "false");
  }

  bool IsEnabled() {
    return (_history);
  }

  void putValue(T Value = T()) {
    if (!IsEnabled() || (_size_max < 1) || (!IsValidValue<T>(Value)))
      return;

    ulong _millis = millis();

    _history[_end] = {Value, _millis};
    _end           = (_end + 1) % _size_max;

    if (_size < _size_max) {
      _size++;
    } else {
      _start = (_start + 1) % _size_max;
    }

    MillisFrom = _history[_start].Millis;
    MillisTo   = _history[(_end - 1 + _size_max) % _size_max].Millis;

    MillisToTimeStr24(MillisFromStr, MillisFrom, true);
    MillisToTimeStr24(MillisToStr, MillisTo, true);
  }

  _IntervalData *getIntervalStat(ulong IntervalSec = 0) {
    if (!IsEnabled() || (_size_max < 1) || (IntervalSec < 1) || (_size < 1))
      return nullptr;

    ulong _millis   = millis();
    ulong _interval = IntervalSec * 1000ul;

    ulong _millis_to   = 0;
    ulong _millis_from = 0;

    T _value_min  = std::numeric_limits<T>::max();
    T _value_max  = std::numeric_limits<T>::lowest();
    T _value_from = T();
    T _value_to   = T();

    T _sum = T();

    size_t _pos  = (_end - 1 + _size_max) % _size_max;
    size_t _from = 0;
    size_t _to   = 0;

    int _count   = 0;
    int _checked = 0;

    while (_checked < _size && (_millis - _history[_pos].Millis) <= _interval) {
      T _value = _history[_pos].Value;

      if (IsValidValue<T>(_value)) {
        _from = _pos;
        if (_count < 1)
          _to = _from;

        _sum += _value;

        _value_min = min(_value_min, _value);
        _value_max = max(_value_max, _value);

        _count++;
      }

      _pos = (_pos - 1 + _size_max) % _size_max;

      _checked++;
    }

    if (_count < 1)
      return nullptr;

    _millis_from = _history[_from].Millis;
    _millis_to   = _history[_to].Millis;

    _value_from = _history[_from].Value;
    _value_to   = _history[_to].Value;

    _IntervalData *_Stat = new _IntervalData{};

    _Stat->MillisFrom = _millis_from;
    _Stat->MillisTo   = _millis_to;
    _Stat->ValueFrom  = _value_from;
    _Stat->ValueTo    = _value_to;
    _Stat->ValueMin   = _value_min;
    _Stat->ValueMax   = _value_max;
    _Stat->ValueAvg   = (_count > 0) ? _sum / static_cast<T>(_count) : T();

    _Stat->ValueUp          = (_value_to > _value_from) ? _value_to - _value_from : 0;
    _Stat->ValueDown        = (_value_from > _value_to) ? _value_from - _value_to : 0;
    _Stat->Vibration        = _value_max - _value_min;
    _Stat->VibrationPercent = (_value_max != 0) ? (_value_max - _value_min) * (100.0 / _value_max) : 0;

    _Stat->RecordCount = _count;

    MillisToTimeStr24(_Stat->MillisFromStr, _Stat->MillisFrom, true);
    MillisToTimeStr24(_Stat->MillisToStr, _Stat->MillisTo, true);

    return _Stat;
  }

  void Clear() {
    _start = 0;
    _end   = 0;
    _size  = 0;

    MillisFrom = 0;
    MillisTo   = 0;

    MillisFromStr[0] = '\0';
    MillisToStr[0]   = '\0';
  }

  size_t getSize() {
    return _size;
  }

  size_t getSizeMax() {
    return _size_max;
  }
};
*/