#include "math.hpp"

namespace dc::math
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

BoundingBox::BoundingBox(const glm::vec3 &min, const glm::vec3 &max)
    : min_(glm::min(min, max)),
      max_(glm::max(min, max))
{
}

BoundingBox::BoundingBox(const glm::vec3 *points, size_t numPoints)
{
  glm::vec3 vmin(std::numeric_limits<float>::max());
  glm::vec3 vmax(std::numeric_limits<float>::lowest());

  for (size_t i = 0; i != numPoints; i++)
  {
    vmin = glm::min(vmin, points[i]);
    vmax = glm::max(vmax, points[i]);
  }
  min_ = vmin;
  max_ = vmax;
}

glm::vec3 BoundingBox::size() const
{
  return glm::vec3(max_[0] - min_[0], max_[1] - min_[1], max_[2] - min_[2]);
}

glm::vec3 BoundingBox::center() const
{
  return 0.5f *
         glm::vec3(max_[0] + min_[0], max_[1] + min_[1], max_[2] + min_[2]);
}

void BoundingBox::transform(const glm::mat4 &t)
{
  glm::vec3 corners[] = {
      glm::vec3(min_.x, min_.y, min_.z),
      glm::vec3(min_.x, max_.y, min_.z),
      glm::vec3(min_.x, min_.y, max_.z),
      glm::vec3(min_.x, max_.y, max_.z),
      glm::vec3(max_.x, min_.y, min_.z),
      glm::vec3(max_.x, max_.y, min_.z),
      glm::vec3(max_.x, min_.y, max_.z),
      glm::vec3(max_.x, max_.y, max_.z),
  };
  for (auto &v : corners)
    v = glm::vec3(t * glm::vec4(v, 1.0f));
  *this = BoundingBox(corners, 8);
}

BoundingBox BoundingBox::transformed(const glm::mat4 &t) const
{
  BoundingBox b = *this;
  b.transform(t);
  return b;
}

void BoundingBox::combine_point(const glm::vec3 &p)
{
  min_ = glm::min(min_, p);
  max_ = glm::max(max_, p);
}

int calc_mipmap_levels_2d(int width, int height)
{
  int levels{1};
  while ((width | height) >> levels)
  {
    levels += 1;
  }
  return levels;
}

} // namespace math
