#pragma once

#include "common/range/view.h"
#include "model/entity.h"

namespace hz::model {
    class world;

    class world_component {

    };

    class world {
    public:
        template<typename... Args>
        auto add_entity(Args&&... args) -> world & {
            entities.emplace_back(args...);
            return *this;
        }

        auto get_entities() noexcept -> range::contiguous_view<entity> {
            return entities;
        }
        auto get_entities() const noexcept -> range::contiguous_view<entity const> {
            return entities;
        }

    private:
        std::vector<entity> entities;
        std::vector<world_component> components;
    };
}