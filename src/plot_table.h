#pragma once

#include <Table.h>
#include <vector>

#define TABLE_USE_FOLD // костыль для новой версии Table

#define HIMIDITY_PLOT_SIZE          500
#define HIMIDITY_PLOT_EPSILON       0.01f
#define HIMIDITY_PLOT_SAVE_INTERVAL 60ul * 1000ul
#define RDP_STACK_SIZE              128

class _Plot {
private:
  Table _data;
  Table _point;

  size_t _maxSize      = HIMIDITY_PLOT_SIZE;
  float  _epsilon      = HIMIDITY_PLOT_EPSILON;
  ulong  _saveInterval = HIMIDITY_PLOT_SAVE_INTERVAL;
  ulong  _saveMillis   = 0;

  char file_name[50] = "/plot.tbl";


  inline float perpendicularDistance(const uint64_t &time1, float value1,
                                     const uint64_t &time2, float value2,
                                     const uint64_t &timeP, float valueP) const {
    const float dx = static_cast<float>(time2 - time1);
    if (dx == 0)
      return fabs(valueP - value1);

    const float slope     = (value2 - value1) / dx;
    const float intercept = value1 - slope * time1;
    return fabs(slope * timeP - valueP + intercept) / sqrt(slope * slope + 1.0f);
  }

  void rdpCompressImpl(Table &src, std::vector<bool> &keep, float epsilon) {
    struct Segment {
      size_t start, end;

      Segment(size_t s, size_t e) : start(s), end(e) {}
    };

    std::vector<Segment> stack;
    stack.reserve(RDP_STACK_SIZE);
    stack.emplace_back(0, src.rows() - 1);

    while (!stack.empty() && stack.size() < RDP_STACK_SIZE) {
      auto segment = stack.back();
      stack.pop_back();

      if (segment.end <= segment.start + 1)
        continue;

      uint64_t timeStart  = src[segment.start][0];
      float    valueStart = src[segment.start][1];
      uint64_t timeEnd    = src[segment.end][0];
      float    valueEnd   = src[segment.end][1];

      float  maxDist  = 0;
      size_t maxIndex = segment.start;

      for (size_t i = segment.start + 1; i < segment.end; ++i) {
        uint64_t timeP  = src[i][0];
        float    valueP = src[i][1];
        float    dist   = perpendicularDistance(timeStart, valueStart, timeEnd, valueEnd, timeP, valueP);
        if (dist > maxDist) {
          maxDist  = dist;
          maxIndex = i;
        }
      }

      if (maxDist > epsilon) {
        keep[maxIndex] = true;
        stack.emplace_back(segment.start, maxIndex);
        stack.emplace_back(maxIndex, segment.end);
      }
    }
  }

  void cutData(Table &data) {
    while (data.rows() > _maxSize)
      data.remove(0);
  }

public:
  _Plot(size_t maxSize = HIMIDITY_PLOT_SIZE, float epsilon = HIMIDITY_PLOT_EPSILON) {
    _data.init(2, cell_t::Uint64, cell_t::Float);
    _point.init(2, cell_t::Uint64, cell_t::Float);

    _data.clearChanged();

    setMaxSize(maxSize);
    setEpsilon(epsilon);
  }

  void Compress(Table &data) {
    if ((_epsilon <= 0) || (data.rows() <= 2) || !data.changed())
      return;

    std::vector<bool> keep(data.rows(), false);
    keep.front() = true;
    keep.back()  = true;

    rdpCompressImpl(data, keep, _epsilon);

    Table compressed(0, 2, cell_t::Uint64, cell_t::Float);

    for (size_t i = 0; i < data.rows(); ++i) {
      if (keep[i]) {
        uint64_t time  = data[i][0];
        float    value = data[i][1];

        compressed.append(time, value);
      }
    }

    cutData(compressed);

    data.removeAll();
    for (size_t i = 0; i < compressed.rows(); ++i) {
      uint64_t time  = compressed[i][0];
      float    value = compressed[i][1];

      data.append(time, value);
    }
    data.clearChanged();
  }

  void putValue(float value) {
    if (!std::isfinite(value) || !sett.rtc.synced())
      return;

    uint64_t _unixMs = sett.rtc.getUnixMs();

    _point.append(_unixMs, value);

    cutData(_point);

    if (_maxSize < 1)
      return;

    _data.append(_unixMs, value);

    if (_data.rows() > _maxSize)
      Compress(_data);
  }

  bool Save() {
    if (!_FS_Initialized || !file_name || !*file_name || _data.rows() < 1)
      return false;

    File f = LittleFS.open(file_name, FILE_WRITE);
    if (!f)
      return false;

    Compress(_data);

    return _data.writeTo(f);
  }

  bool Load() {
    if (!_FS_Initialized || !file_name || !*file_name || _maxSize < 1)
      return false;

    File f = LittleFS.open(file_name, FILE_READ);
    if (!f)
      return false; 

    if (!_data.readFrom(f, f.size()))
      return false;

    Compress(_data);

    return true;
  }

  void Tick() {
    if ((_maxSize > 0) && (_saveInterval > 0) && ((millis() - _saveMillis) >= _saveInterval)) {
      _saveMillis = millis();

      if (!_loaded)
        Load();

      Save();
    }
  }

  void Clear() {
    _data.removeAll();

    _saveMillis = millis();
  }

  size_t MaxSize() const {
    return _maxSize;
  }

  float Epsilon() const {
    return _epsilon;
  }

  ulong SaveInterval() const {
    return _saveInterval;
  }

  Table &Data() {
    Compress(_data);

    return _data;
  }

  Table &Point() {
    return _point;
  }

  size_t CurrentSize() {
    return _data.rows();
  }

  void setMaxSize(size_t size) {
    _maxSize = size;

    if (_maxSize < 1) {
      Clear();

      return;
    }

    if (_data.rows() > _maxSize)
      Compress(_data);
  }

  void setEpsilon(float epsilon) {
    _epsilon = epsilon;
  }

  void setSaveInterval(ulong interval) {
    _saveInterval = interval;
  }
};

/* main.cpp

_Plot p;

// в build()
b.Plot(H(plot), p.Data(), "", false);


// в update()
p.putValue((float)kek); // добавляем точку
sett.updater().updatePlot(H(plot), p.Point(), true);

*/
