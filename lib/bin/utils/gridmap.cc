#pragma once
#include <unordered_map>
#include <vector>
#include <memory>
#include <cmath>
#include <cstdint>

template <typename T>
class GridMap {
public:
    using EntityID = uint32_t;
    using EntityPtr = std::shared_ptr<T>;

private:
    struct EntityData {
        float x = 0, y = 0;
        float proximity = 0;
        EntityPtr ptr = nullptr;
        int cellX = 0, cellY = 0;
        bool initialized = false;
    };

    std::unordered_map<EntityID, EntityData> entities;
    std::unordered_map<int64_t, std::vector<EntityID>> grid;

    int cellSize;

    inline int64_t cell_key(int cx, int cy) const {
        return (static_cast<int64_t>(cx) << 32) ^ static_cast<int64_t>(cy);
    }

public:
    GridMap(int cell_size = 32)
        : cellSize(cell_size)
    {}

    // ----------------------------------------------------------
    // REMOVE ENTITY
    // ----------------------------------------------------------
    void remove_entity(EntityID id) {
        auto it = entities.find(id);
        if (it == entities.end()) return;

        int cx = it->second.cellX;
        int cy = it->second.cellY;
        int64_t key = cell_key(cx, cy);

        auto g = grid.find(key);
        if (g != grid.end()) {
            auto &vec = g->second;
            for (size_t i = 0; i < vec.size(); ++i) {
                if (vec[i] == id) {
                    vec[i] = vec.back();
                    vec.pop_back();
                    break;
                }
            }
            if (vec.empty()) grid.erase(key);
        }

        entities.erase(it);
    }

    // ----------------------------------------------------------
    // UPDATE ENTITY (CALL EVERY FRAME)
    // ----------------------------------------------------------
    void update_entity(
        EntityID id,
        float x, float y,
        float interaction_proximity,
        EntityPtr ptr
    ) {
        auto& e = entities[id];

        // FIRST TIME REGISTRATION
        if (!e.initialized) {
            e.x = x;
            e.y = y;
            e.proximity = interaction_proximity;
            e.ptr = ptr;
            e.cellX = static_cast<int>(x) / cellSize;
            e.cellY = static_cast<int>(y) / cellSize;
            grid[cell_key(e.cellX, e.cellY)].push_back(id);
            e.initialized = true;
            return;
        }

        // Update proximity if changed
        e.proximity = interaction_proximity;

        // Check if position unchanged
        if (e.x == x && e.y == y) {
            return;
        }

        e.x = x;
        e.y = y;

        int newCX = static_cast<int>(x) / cellSize;
        int newCY = static_cast<int>(y) / cellSize;

        // Cell changed → move entry
        if (newCX != e.cellX || newCY != e.cellY) {
            int64_t oldKey = cell_key(e.cellX, e.cellY);
            int64_t newKey = cell_key(newCX, newCY);

            auto &oldVec = grid[oldKey];
            for (size_t i = 0; i < oldVec.size(); ++i) {
                if (oldVec[i] == id) {
                    oldVec[i] = oldVec.back();
                    oldVec.pop_back();
                    break;
                }
            }
            if (oldVec.empty()) grid.erase(oldKey);

            grid[newKey].push_back(id);

            e.cellX = newCX;
            e.cellY = newCY;
        }
    }

    // ----------------------------------------------------------
    // GET NEARBY ENTITIES BASED ON PROXIMITY
    // ----------------------------------------------------------
    std::vector<EntityPtr> get_nearby_entities(EntityID id) {
        std::vector<EntityPtr> result;

        auto it = entities.find(id);
        if (it == entities.end()) return result;

        const EntityData& e = it->second;

        float px = e.x, py = e.y;
        float prox = e.proximity;
        float proxSq = prox * prox;

        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {

                int cx = e.cellX + dx;
                int cy = e.cellY + dy;
                int64_t key = cell_key(cx, cy);

                auto g = grid.find(key);
                if (g == grid.end()) continue;

                const auto& vec = g->second;
                for (EntityID otherID : vec) {
                    if (otherID == id) continue;

                    const auto& o = entities.at(otherID);

                    float ddx = o.x - px;
                    float ddy = o.y - py;
                    float distSq = ddx * ddx + ddy * ddy;

                    float allowed = prox;
                    float allowedSq = allowed * allowed;

                    if (distSq <= allowedSq)
                        result.push_back(o.ptr);
                }
            }
        }

        return result;
    }

    // ----------------------------------------------------------
    // BUILD INTERACTION MAP: ID → vector<shared_ptr<T>>
    // ----------------------------------------------------------
    std::unordered_map<EntityPtr, std::vector<EntityPtr>> build_interaction_map() {
        std::unordered_map<EntityPtr, std::vector<EntityPtr>> finalMap;
        finalMap.reserve(entities.size());

        for (auto& kv : entities) {
            EntityPtr id = kv.second.ptr;
            finalMap[id] = get_nearby_entities( kv.first );
        }

        return finalMap;
    }
};

