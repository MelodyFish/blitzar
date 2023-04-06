#pragma once

#include <cassert>
#include <concepts>

#include "sxt/algorithm/base/identity_mapper.h"
#include "sxt/algorithm/base/mapper.h"
#include "sxt/algorithm/base/reducer.h"
#include "sxt/algorithm/reduction/kernel_fit.h"
#include "sxt/algorithm/reduction/thread_reduction.h"
#include "sxt/base/device/memory_utility.h"
#include "sxt/base/device/stream.h"
#include "sxt/execution/async/future.h"
#include "sxt/execution/async/synchronization.h"
#include "sxt/execution/kernel/kernel_dims.h"
#include "sxt/execution/kernel/launch.h"
#include "sxt/memory/management/managed_array.h"
#include "sxt/memory/resource/async_device_resource.h"
#include "sxt/memory/resource/pinned_resource.h"

namespace sxt::algr {
//--------------------------------------------------------------------------------------------------
// reduction_kernel
//--------------------------------------------------------------------------------------------------
template <algb::reducer Reducer, unsigned int BlockSize, algb::mapper Mapper>
  requires std::same_as<typename Reducer::value_type, typename Mapper::value_type>
__global__ void reduction_kernel(typename Reducer::value_type* out, Mapper mapper, unsigned int n) {
  using T = typename Reducer::value_type;
  auto thread_index = threadIdx.x;
  auto block_index = blockIdx.x;
  auto index = block_index * (BlockSize * 2) + thread_index;
  auto step = BlockSize * 2 * gridDim.x;
  __shared__ T shared_data[BlockSize];
  thread_reduce<Reducer, BlockSize>(out + block_index, shared_data, mapper, n, step, thread_index,
                                    index);
}

//--------------------------------------------------------------------------------------------------
// reduce
//--------------------------------------------------------------------------------------------------
template <algb::reducer Reducer, class Mapper>
xena::future<typename Reducer::value_type> reduce(basdv::stream&& stream, Mapper mapper,
                                                  unsigned n) noexcept {
  using T = typename Reducer::value_type;
  auto dims = fit_reduction_kernel(n);

  memr::async_device_resource resource{stream};

  // kernel computation
  memmg::managed_array<T> out_array{dims.num_blocks, &resource};
  xenk::launch_kernel(dims.block_size, [&]<unsigned BlockSize>(
                                           std::integral_constant<unsigned, BlockSize>) noexcept {
    reduction_kernel<Reducer, BlockSize>
        <<<dims.num_blocks, BlockSize, 0, stream>>>(out_array.data(), mapper, n);
  });
  memmg::managed_array<T> result_array{dims.num_blocks, memr::get_pinned_resource()};
  basdv::async_copy_device_to_host(result_array, out_array, stream);

  // future
  return xena::await_and_own_stream(std::move(stream), std::move(result_array))
      .then([num_blocks = dims.num_blocks](memmg::managed_array<T>&& result_array) noexcept {
        auto res = result_array[0];
        for (unsigned int i = 1; i < num_blocks; ++i) {
          Reducer::accumulate(res, result_array[i]);
        }
        return res;
      });
}
} // namespace sxt::algr
