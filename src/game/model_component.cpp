#include "model_component.hpp"

ModelComponent::ModelComponent(const std::string &model)
{
  model_ = asset_manager->load_model(model);
}
