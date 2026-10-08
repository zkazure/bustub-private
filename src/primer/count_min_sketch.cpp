//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// count_min_sketch.cpp
//
// Identification: src/primer/count_min_sketch.cpp
//
// Copyright (c) 2015-2025, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "primer/count_min_sketch.h"

#include <cstdint>
#include <cmath>
#include <limits>
#include <queue>
#include <stdexcept>
#include <string>

namespace bustub {

/**
 * Constructor for the count-min sketch.
 *
 * @param width The width of the sketch matrix.
 * @param depth The depth of the sketch matrix.
 * @throws std::invalid_argument if width or depth are zero.
 */
template <typename KeyType>
CountMinSketch<KeyType>::CountMinSketch(uint32_t width, uint32_t depth) : width_(width), depth_(depth) {
  /** @TODO(student) Implement this function! */
  if (this->width_ <= 0 || this->depth_ <= 0) {
    throw std::invalid_argument("width_ or depth_ must greater than 0.");
  }

  counts_ = std::make_unique<std::atomic<uint32_t>[]>(this->width_ * this->depth_);
  for (uint32_t i = 0; i < this->width_ * this->depth_; i++) {
    counts_[i].store(0);
  }

  /** @fall2025 PLEASE DO NOT MODIFY THE FOLLOWING */
  // Initialize seeded hash functions
  hash_functions_.reserve(depth_);
  for (size_t i = 0; i < depth_; i++) {
    hash_functions_.push_back(this->HashFunction(i));
  }
}

template <typename KeyType>
CountMinSketch<KeyType>::CountMinSketch(CountMinSketch &&other) noexcept : width_(other.width_), depth_(other.depth_) {
  /** @TODO(student) Implement this function! */
  this->hash_functions_ = std::move(other.hash_functions_);
  this->counts_ = std::move(other.counts_);
}

template <typename KeyType>
auto CountMinSketch<KeyType>::operator=(CountMinSketch &&other) noexcept -> CountMinSketch & {
  /** @TODO(student) Implement this function! */
  this->width_ = std::move(other.width_);
  this->depth_ = std::move(other.depth_);
  this->hash_functions_ = std::move(other.hash_functions_);

  this->counts_ = std::move(other.counts_);

  return *this;
}

template <typename KeyType>
void CountMinSketch<KeyType>::Insert(const KeyType &item) {
  /** @TODO(student) Implement this function! */
  for (uint32_t i = 0; i < depth_; ++i) {
    counts_[hash_functions_[i](item) + i * width_].fetch_add(1);
  }
}

template <typename KeyType>
void CountMinSketch<KeyType>::Merge(const CountMinSketch<KeyType> &other) {
  if (width_ != other.width_ || depth_ != other.depth_) {
    throw std::invalid_argument("Incompatible CountMinSketch dimensions for merge.");
  }
  /** @TODO(student) Implement this function! */
  for (uint32_t i = 0; i < this->width_ * this->depth_; ++i) {
    this->counts_[i].fetch_add(other.counts_[i].load());
  }
}

template <typename KeyType>
auto CountMinSketch<KeyType>::Count(const KeyType &item) const -> uint32_t {
  uint32_t res = std::numeric_limits<uint32_t>::max();
  for (uint32_t i = 0; i < depth_; ++i) {
    uint32_t tmp = counts_[width_ * i + hash_functions_[i](item)];
    res = tmp < res ? tmp : res;
  }
  return res;
}

template <typename KeyType>
void CountMinSketch<KeyType>::Clear() {
  /** @TODO(student) Implement this function! */
  for (uint32_t i = 0; i < this->width_ * this->depth_; ++i) {
    counts_[i].store(0);
  }
}

template <typename KeyType>
auto CountMinSketch<KeyType>::TopK(uint16_t k, const std::vector<KeyType> &candidates)
    -> std::vector<std::pair<KeyType, uint32_t>> {
  /** @TODO(student) Implement this function! */
  using CountKey = std::pair<uint32_t, KeyType>;

  std::priority_queue<CountKey, std::vector<CountKey>> max_heap;
  for (auto &c : candidates) {
    max_heap.push({this->Count(c), c});
  }

  std::vector<std::pair<KeyType, uint32_t>> res(k);
  for (uint16_t i = 0; i < k; ++i) {
    res[i] = {max_heap.top().second, max_heap.top().first};
    max_heap.pop();
  }

  return res;
}

// Explicit instantiations for all types used in tests
template class CountMinSketch<std::string>;
template class CountMinSketch<int64_t>;  // For int64_t tests
template class CountMinSketch<int>;      // This covers both int and int32_t
}  // namespace bustub
