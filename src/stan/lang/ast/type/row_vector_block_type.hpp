#ifndef STAN_LANG_AST_ROW_VECTOR_BLOCK_TYPE_HPP
#define STAN_LANG_AST_ROW_VECTOR_BLOCK_TYPE_HPP

#include <stan/lang/ast/node/expression.hpp>
#include <stan/lang/ast/node/range.hpp>

namespace stan {
namespace lang {

/**
 * Row vector block var type.
 */
struct row_vector_block_type {
  /**
   * Bounds constraints
   */
  range bounds_;

  /**
   * Row vector length
   */
  expression N_;

  /**
   * Construct a block var type with default values.
   */
  row_vector_block_type();

  /**
   * Construct a block var type with specified values.
   * Arg `N` should be int expression - constructor doesn't check.
   *
   * @param bounds variable upper and/or lower bounds
   * @param N num columns for row vector
   */
  row_vector_block_type(const range& bounds, const expression& N);

  /**
   * Get bounds.
   */
  range bounds() const;

  /**
   * Get N (num cols).
   */
  expression N() const;
};
}  // namespace lang
}  // namespace stan
#endif
