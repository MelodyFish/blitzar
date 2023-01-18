#pragma once

#include "sxt/proof/inner_product/driver.h"

namespace sxt::prfip {
//--------------------------------------------------------------------------------------------------
// cpu_driver
//--------------------------------------------------------------------------------------------------
class cpu_driver final : public driver {
public:
  // driver
  std::unique_ptr<workspace>
  make_workspace(const proof_descriptor& descriptor,
                 basct::cspan<s25t::element> a_vector) const noexcept override;

  void commit_to_fold(rstt::compressed_element& l_value, rstt::compressed_element& r_value,
                      workspace& ws) const noexcept override;

  void fold(workspace& ws, const s25t::element& x) const noexcept override;

  void compute_expected_commitment(rstt::compressed_element& commit,
                                   const proof_descriptor& descriptor,
                                   basct::cspan<rstt::compressed_element> l_vector,
                                   basct::cspan<rstt::compressed_element> r_vector,
                                   basct::cspan<s25t::element> x_vector,
                                   const s25t::element& ap_value) const noexcept override;
};
} // namespace sxt::prfip