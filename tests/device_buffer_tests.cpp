/*
 * Copyright (c) 2019, NVIDIA CORPORATION.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gtest/gtest.h"

#include <rmm/device_buffer.hpp>
#include <rmm/mr/cnmem_memory_resource.hpp>
#include <rmm/mr/cuda_memory_resource.hpp>
#include <rmm/mr/default_memory_resource.hpp>
#include <rmm/mr/device_memory_resource.hpp>
#include <rmm/mr/managed_memory_resource.hpp>

#include <cuda_runtime_api.h>
#include <cstddef>
#include <random>

template <typename MemoryResourceType>
struct DeviceBufferTest : public ::testing::Test {
  cudaStream_t stream{};
  std::size_t size{};
  MemoryResourceType mr{};

  DeviceBufferTest() {
    std::default_random_engine generator;
    std::uniform_int_distribution<std::size_t> distribution(1000, 100000);
    size = distribution(generator);
  }

  void SetUp() override { EXPECT_EQ(cudaSuccess, cudaStreamCreate(&stream)); }

  void TearDown() override {
    EXPECT_EQ(cudaSuccess, cudaStreamDestroy(stream));
  };
};

using resources = ::testing::Types<rmm::mr::cuda_memory_resource,
                                   rmm::mr::managed_memory_resource,
                                   rmm::mr::cnmem_memory_resource>;

TYPED_TEST_CASE(DeviceBufferTest, resources);

TYPED_TEST(DeviceBufferTest, DefaultMemoryResource) {
  rmm::device_buffer buff(this->size);
  EXPECT_NE(nullptr, buff.data());
  EXPECT_EQ(this->size, buff.size());
  EXPECT_EQ(rmm::mr::get_default_resource(), buff.memory_resource());
  EXPECT_EQ(0, buff.stream());
}

TYPED_TEST(DeviceBufferTest, DefaultMemoryResourceStream) {
  rmm::device_buffer buff(this->size, this->stream);
  EXPECT_EQ(cudaSuccess, cudaStreamSynchronize(this->stream));
  EXPECT_NE(nullptr, buff.data());
  EXPECT_EQ(this->size, buff.size());
  EXPECT_EQ(rmm::mr::get_default_resource(), buff.memory_resource());
  EXPECT_EQ(this->stream, buff.stream());
}

TYPED_TEST(DeviceBufferTest, ExplicitMemoryResource) {
  rmm::device_buffer buff(this->size, 0, &this->mr);
  EXPECT_NE(nullptr, buff.data());
  EXPECT_EQ(this->size, buff.size());
  EXPECT_EQ(&this->mr, buff.memory_resource());
  EXPECT_TRUE(this->mr.is_equal(*buff.memory_resource()));
  EXPECT_EQ(0, buff.stream());
}

TYPED_TEST(DeviceBufferTest, ExplicitMemoryResourceStream) {
  rmm::device_buffer buff(this->size, this->stream, &this->mr);
  EXPECT_EQ(cudaSuccess, cudaStreamSynchronize(this->stream));
  EXPECT_NE(nullptr, buff.data());
  EXPECT_EQ(this->size, buff.size());
  EXPECT_EQ(&this->mr, buff.memory_resource());
  EXPECT_TRUE(this->mr.is_equal(*buff.memory_resource()));
  EXPECT_EQ(this->stream, buff.stream());
}

TYPED_TEST(DeviceBufferTest, CopyConstructor) {
  rmm::device_buffer buff(this->size, 0, &this->mr);
  // Can't do this until RMM cmake is setup to build cuda files
  //thrust::sequence(thrust::device, static_cast<signed char *>(buff.data()),
  //                 static_cast<signed char *>(buffer.data()) + buff.size(), 0);
  rmm::device_buffer buff_copy(buff);
  EXPECT_NE(nullptr, buff_copy.data());
  EXPECT_NE(buff.data(), buff_copy.data());
  EXPECT_EQ(buff.size(), buff_copy.size());
  EXPECT_EQ(buff.memory_resource(), buff_copy.memory_resource());
  EXPECT_TRUE(buff.memory_resource()->is_equal(*buff_copy.memory_resource()));
  EXPECT_EQ(buff.stream(), buff_copy.stream());

  //EXPECT_TRUE(
  //    thrust::equal(thrust::device, static_cast<signed char *>(buff.data()),
  //                  static_cast<signed char *>(buff.data()) + buff.size(),
  //                  static_cast<signed char *>(buff_copy.data())));
}
