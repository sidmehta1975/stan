#ifndef STAN__IO__TRANSFORMS__VECTOR_NOOP_HPP
#define STAN__IO__TRANSFORMS__VECTOR_NOOP_HPP

#include <stan/math/prim/scal/fun/lb_constrain.hpp>
#include <stan/math/prim/scal/fun/lb_free.hpp>

#include <stan/io/transforms/vector_transform.hpp>

namespace stan {
  namespace io {

    template <typename T>
    class vector_noop: public vector_transform<T> {
    public:
      vector_noop(size_t N): vector_transform<T>(N) {}

      size_t unconstrained_dim() { return N_; }

      static void unconstrain(const vector_t& input, std::vector<T>& output) {
        for (idx_t n = 0; n < input.size(); ++n)
          output.push_back(input[n]);
      }

      static std::vector<T> unconstrain(const vector_t& input) {
        std::vector<T> output;
        for (idx_t n = 0; n < input.size(); ++n)
          output.push_back(input[n]);
      }

      void constrain(const std::vector<T>& input, vector_t& output) {
        output = Eigen::Map<const vector_t>(&(input[0]), input.size());
      }

      void constrain(const std::vector<T>& input, vector_t& output, T& lp) {
        output = Eigen::Map<const vector_t>(&(input[0]), input.size());
      }

      vector_t constrain(const std::vector<T>& input) {
        return Eigen::Map<const vector_t>(&(input[0]), input.size());
      }

      vector_t constrain(const std::vector<T>& input, T& lp) {
        return Eigen::Map<const vector_t>(&(input[0]), input.size());
      }

    }

  }  // io
}  // stan

#endif