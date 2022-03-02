#include "name_component.hpp"
#include "serialization.hpp"

void NameComponent::save(FILE *file) const { write_string(file, name_); }

void NameComponent::read(FILE *file) { read_string(file, name_); }
