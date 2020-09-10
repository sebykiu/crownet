/*
 * Entry.h
 *
 *  Created on: Sep 9, 2020
 *      Author: sts
 */

#pragma once

#include <memory>
#include <set>
#include <string>

template <typename VALUE>
class EntryCtor {
 public:
  virtual ~EntryCtor() = default;
  virtual std::shared_ptr<VALUE> entry() = 0;
  virtual std::shared_ptr<VALUE> localEntry() = 0;
};

template <typename VALUE>
class EntryDefaultCtorImpl : public EntryCtor<VALUE> {
 public:
  std::shared_ptr<VALUE> entry() override { return std::make_shared<VALUE>(); }

  std::shared_ptr<VALUE> localEntry() override {
    return std::make_shared<VALUE>();
  }
};

template <typename K, typename T>
class IEntry {
 public:
  using key_type = K;
  using time_type = T;
  IEntry();
  IEntry(int, time_type&, time_type&);
  virtual ~IEntry() = default;
  virtual void reset() {
    count = 0;
    _valid = false;
  }
  virtual const bool valid() const { return _valid; }
  virtual void incrementCount(const time_type& t);

  virtual const time_type& getMeasureTime() const;
  virtual const time_type& getReceivedTime() const;
  virtual const int getCount() const;

  virtual int compareMeasureTime(const IEntry& other) const;
  virtual int compareReceivedTime(const IEntry& other) const;

  virtual std::string csv(std::string delimiter) const = 0;
  virtual std::string str() const = 0;

 protected:
  int count;
  time_type measurement_time;
  time_type received_time;
  bool _valid;
};

/// implementation IEntry<K, T>

template <typename K, typename T>
IEntry<K, T>::IEntry()
    : count(0), measurement_time(), received_time(), _valid(false) {}

template <typename K, typename T>
IEntry<K, T>::IEntry(int count, time_type& m_t, time_type& r_t)
    : count(count), measurement_time(m_t), received_time(r_t), _valid(true) {}

template <typename K, typename T>
void IEntry<K, T>::incrementCount(const time_type& t) {
  count++;
  measurement_time = t;
  received_time = t;
  _valid = true;
}

template <typename K, typename T>
const typename IEntry<K, T>::time_type& IEntry<K, T>::getMeasureTime() const {
  return measurement_time;
}

template <typename K, typename T>
const typename IEntry<K, T>::time_type& IEntry<K, T>::getReceivedTime() const {
  return received_time;
}

template <typename K, typename T>
const int IEntry<K, T>::getCount() const {
  return count;
}

template <typename K, typename T>
int IEntry<K, T>::compareMeasureTime(const IEntry& other) const {
  if (measurement_time == other.measurement_time) return 0;
  if (measurement_time < other.measurement_time) {
    return -1;
  } else {
    return 1;
  }
}

template <typename K, typename T>
int IEntry<K, T>::compareReceivedTime(const IEntry& other) const {
  if (received_time == other.received_time) return 0;
  if (received_time < other.received_time) {
    return -1;
  } else {
    return 1;
  }
}