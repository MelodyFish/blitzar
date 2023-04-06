#pragma once

namespace sxt::basdv {
class stream;
}
namespace sxt::basdv {
struct stream_handle;
}

namespace sxt::xena {
//--------------------------------------------------------------------------------------------------
// computation_handle
//--------------------------------------------------------------------------------------------------
class computation_handle {
public:
  computation_handle() noexcept = default;
  computation_handle(const computation_handle&) = delete;
  computation_handle(computation_handle&& other) noexcept;

  ~computation_handle() noexcept;

  computation_handle& operator=(const computation_handle&) = delete;
  computation_handle& operator=(computation_handle&& other) noexcept;

  void wait() noexcept;

  bool empty() const noexcept { return head_ == nullptr; }

  void add_stream(basdv::stream&& stream) noexcept;

private:
  basdv::stream_handle* head_{nullptr};
};
} // namespace sxt::xena
