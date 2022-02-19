#include "math.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "glm/gtc/epsilon.hpp"
#include "glm/trigonometric.hpp"

#include <glm/gtx/matrix_decompose.hpp>

namespace math
{

bool decompose_transform(const glm::mat4 &transform,
                         glm::vec3       &translation,
                         glm::vec3       &rotation,
                         glm::vec3       &scale)
{
  // from glm::decompose in matrix_decompose.inl

  glm::mat4 local_matrix{transform};

  // normalize matrix
  if (glm::epsilonEqual(local_matrix[3][3], 0.0f, glm::epsilon<float>()))
  {
    return false;
  }

  // first, isolate perspective
  if (glm::epsilonNotEqual(local_matrix[0][3], 0.0f, glm::epsilon<float>()) ||
      glm::epsilonNotEqual(local_matrix[1][3], 0.0f, glm::epsilon<float>()) ||
      glm::epsilonNotEqual(local_matrix[2][3], 0.0f, glm::epsilon<float>()))
  {
    // clear the perspective partition
    local_matrix[0][3] = local_matrix[1][3] = local_matrix[2][3] = 0.0f;
    local_matrix[3][3]                                           = 1.0f;
  }

  // next take care of translation
  translation     = local_matrix[3];
  local_matrix[3] = glm::vec4{0.0f, 0.0f, 0.0f, local_matrix[3].w};

  glm::vec3 row[3]{};

  // now get scale and shear
  for (glm::length_t i = 0; i < 3; ++i)
  {
    for (glm::length_t j = 0; j < 3; ++j)
    {
      row[i][j] = local_matrix[i][j];
    }
  }

  // compute x scale factor and normalize first row
  scale.x  = glm::length(row[0]);
  row[0]   = glm::detail::scale(row[0], 1.0f);
  scale.y  = glm::length(row[1]);
  row[1]   = glm::detail::scale(row[1], 1.0f);
  scale.z  = glm::length(row[2]);
  row[2]   = glm::detail::scale(row[2], 1.0f);

  // at this point, the matrix (in rows[]) is orthonormal.
  // check for a coordinate system flip. If the determinant
  // is -1, then negate the matrix and the scaling factors.

  rotation.y = glm::asin(-row[0][2]);
  if (glm::cos(rotation.y) != 0.0f)
  {
    rotation.x = atan2(row[1][2], row[2][2]);
    rotation.z = atan2(row[0][1], row[0][0]);
  }
  else
  {
    rotation.x = atan2(row[2][0], row[1][1]);
    rotation.z = 0.0f;
  }

  return true;
}
} // namespace math
