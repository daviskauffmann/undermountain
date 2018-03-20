#include <assert.h>
#include <libtcod/libtcod.h>
#include <math.h>

#include "engine.h"
#include "entity.h"
#include "game.h"
#include "map.h"
#include "utils.h"

void entity_init(entity_t *entity, int id, game_t *game)
{
    entity->id = id;
    entity->game = game;
}

entity_t *entity_create(game_t *game)
{
    entity_t *entity = NULL;

    for (int id = 0; id < NUM_ENTITIES; id++)
    {
        entity_t *current = &game->entities[id];

        if (current->id == ID_UNUSED)
        {
            entity_init(current, id, game);

            entity = current;

            break;
        }
    }

    assert(entity);

    return entity;
}

void entity_map_place(entity_t *entity)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (position)
    {
        map_t *map = &entity->game->maps[position->level];
        tile_t *tile = &map->tiles[position->x][position->y];

        TCOD_list_push(map->entities, entity);
        TCOD_list_push(tile->entities, entity);
    }
}

void entity_map_remove(entity_t *entity)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (position)
    {
        map_t *map = &entity->game->maps[position->level];
        tile_t *tile = &map->tiles[position->x][position->y];

        TCOD_list_remove(map->entities, entity);
        TCOD_list_remove(tile->entities, entity);
    }
}

void entity_update_flash(entity_t *entity, bool flash_blocks_turn)
{
    flash_t *flash = (flash_t *)component_get(entity, COMPONENT_FLASH);

    if (flash)
    {
        flash->fade -= (1.0f / (float)FPS) * 4.0f;

        if (flash->fade <= 0)
        {
            component_remove(entity, COMPONENT_FLASH);
        }

        if (flash_blocks_turn)
        {
            position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

            fov_t *player_fov = (fov_t *)component_get(entity->game->player, COMPONENT_FOV);

            if (position && player_fov)
            {
                if (TCOD_map_is_in_fov(player_fov->fov_map, position->x, position->y))
                {
                    entity->game->turn_available = false;
                }
            }
        }
    }
}

void entity_update_projectile(entity_t *entity)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
    projectile_t *projectile = (projectile_t *)component_get(entity, COMPONENT_PROJECTILE);

    if (position && projectile)
    {
        float x = projectile->x + projectile->dx;
        float y = projectile->y + projectile->dy;
        int next_x = (int)x;
        int next_y = (int)y;

        bool should_move = true;

        if (map_is_inside(next_x, next_y))
        {
            map_t *map = &entity->game->maps[position->level];
            tile_t *tile = &map->tiles[next_x][next_y];
            tile_info_t *tile_info = &entity->game->tile_info[tile->type];

            if (!tile_info->is_walkable)
            {
                should_move = false;
            }
            else
            {
                for (void **iterator = TCOD_list_begin(tile->entities); iterator != TCOD_list_end(tile->entities); iterator++)
                {
                    entity_t *other = *iterator;

                    if (other != projectile->shooter)
                    {
                        health_t *other_health = (health_t *)component_get(other, COMPONENT_HEALTH);

                        if (other_health)
                        {
                            should_move = false;

                            entity_attack(projectile->shooter, other);

                            inventory_t *other_inventory = (inventory_t *)component_get(other, COMPONENT_INVENTORY);

                            if (other_inventory)
                            {
                                // TODO: add the arrow to their inventory?
                                // also set a flag to not put the arrow on the ground
                            }
                        }
                    }
                }
            }
        }
        else
        {
            should_move = false;
        }

        if (should_move)
        {
            projectile->x = x;
            projectile->y = y;

            position->x = next_x;
            position->y = next_y;

            fov_t *player_fov = (fov_t *)component_get(entity->game->player, COMPONENT_FOV);

            if (player_fov)
            {
                if (TCOD_map_is_in_fov(player_fov->fov_map, position->x, position->y))
                {
                    entity->game->turn_available = false;
                }
            }
        }
        else
        {
            if (projectile->on_hit)
            {
                projectile->on_hit(projectile->on_hit_params);
            }

            // TODO:
            // destroying the arrow makes subsequent arrows travel increasingly faster... wtf?
            // SOLVED: we need to delete the arrow from the map's entity list
            // but maybe we want arrows not to be destroyed, but to fall to the ground
            // or even be added to the inventory of whatever it hit (if it has one)
            entity_destroy(entity);
            // component_remove(entity, COMPONENT_PROJECTILE);

            // entity_map_place(entity);

            // component_add(entity, COMPONENT_PICKABLE);
        }
    }
}

void entity_update_light(entity_t *entity)
{
    light_t *light = (light_t *)component_get(entity, COMPONENT_LIGHT);
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (light && position)
    {
        if (light->fov_map != NULL)
        {
            TCOD_map_delete(light->fov_map);
        }

        map_t *map = &entity->game->maps[position->level];

        light->fov_map = map_to_fov_map(map, position->x, position->y, light->radius);
    }
}

void entity_update_fov(entity_t *entity, TCOD_list_t lights)
{
    fov_t *fov = (fov_t *)component_get(entity, COMPONENT_FOV);
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (fov && position)
    {
        map_t *map = &entity->game->maps[position->level];

        if (fov->fov_map)
        {
            TCOD_map_delete(fov->fov_map);
        }

        fov->fov_map = map_to_fov_map(map, position->x, position->y, fov->radius);

        TCOD_map_t los_map = map_to_fov_map(map, position->x, position->y, 0);

        for (int x = 0; x < MAP_WIDTH; x++)
        {
            for (int y = 0; y < MAP_HEIGHT; y++)
            {
                if (TCOD_map_is_in_fov(los_map, x, y))
                {
                    for (void **iterator = TCOD_list_begin(lights); iterator != TCOD_list_end(lights); iterator++)
                    {
                        entity_t *other = *iterator;

                        light_t *other_light = (light_t *)component_get(other, COMPONENT_LIGHT);
                        position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

                        if (other_light && other_position)
                        {
                            if (TCOD_map_is_in_fov(other_light->fov_map, x, y))
                            {
                                TCOD_map_set_in_fov(fov->fov_map, x, y, true);
                            }
                        }
                    }
                }
            }
        }

        TCOD_map_delete(los_map);
    }
}

void entity_ai(entity_t *entity)
{
    ai_t *ai = (ai_t *)component_get(entity, COMPONENT_AI);

    if (ai)
    {
        ai->energy += ai->energy_per_turn;

        while (ai->energy >= 1.0f)
        {
            ai->energy -= 1.0f;

            position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

            if (position)
            {
                fov_t *fov = (fov_t *)component_get(entity, COMPONENT_FOV);

                map_t *map = &entity->game->maps[position->level];

                bool took_action = false;

                if (!took_action)
                {
                    alignment_t *alignment = (alignment_t *)component_get(entity, COMPONENT_ALIGNMENT);

                    if (alignment && fov)
                    {
                        for (void **iterator = TCOD_list_begin(map->entities); iterator != TCOD_list_end(map->entities); iterator++)
                        {
                            entity_t *other = *iterator;

                            alignment_t *other_alignment = (alignment_t *)component_get(other, COMPONENT_ALIGNMENT);
                            position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

                            if (other_alignment && other_position)
                            {
                                if (TCOD_map_is_in_fov(fov->fov_map, other_position->x, other_position->y) &&
                                    other_alignment->type != alignment->type)
                                {
                                    took_action = true;

                                    ai->last_seen_x = other_position->x;
                                    ai->last_seen_y = other_position->y;

                                    if (distance(position->x, position->y, other_position->x, other_position->y) < 2.0f)
                                    {
                                        entity_attack(entity, other);
                                    }
                                    else
                                    {
                                        entity_path_towards(entity, other_position->x, other_position->y);
                                    }

                                    break;
                                }
                            }
                        }
                    }
                }

                if (!took_action)
                {
                    if (ai->last_seen_x != -1 && ai->last_seen_y != -1)
                    {
                        if (position->x == ai->last_seen_x && position->y == ai->last_seen_y)
                        {
                            ai->last_seen_x = -1;
                            ai->last_seen_y = -1;
                        }
                        else
                        {
                            took_action = true;

                            entity_path_towards(entity, ai->last_seen_x, ai->last_seen_y);
                        }
                    }
                }

                if (!took_action)
                {
                    if (fov && ai->follow_target)
                    {
                        position_t *follow_position = (position_t *)component_get(ai->follow_target, COMPONENT_POSITION);

                        if (follow_position)
                        {
                            if (!TCOD_map_is_in_fov(fov->fov_map, follow_position->x, follow_position->y) ||
                                distance(position->x, position->y, follow_position->x, follow_position->y) > 5.0f)
                            {
                                took_action = true;

                                entity_path_towards(entity, follow_position->x, follow_position->y);
                            }
                        }
                    }
                }

                if (!took_action)
                {
                    tile_t *tile = &map->tiles[position->x][position->y];

                    switch (tile->type)
                    {
                    case TILE_STAIR_DOWN:
                    {
                        took_action = entity_descend(entity);
                    }
                    break;
                    case TILE_STAIR_UP:
                    {
                        took_action = entity_ascend(entity);
                    }
                    break;
                    }
                }

                if (!took_action)
                {
                    int x = position->x + TCOD_random_get_int(NULL, -1, 1);
                    int y = position->y + TCOD_random_get_int(NULL, -1, 1);

                    entity_move(entity, x, y);
                }
            }
        }
    }
}

void entity_path_towards(entity_t *entity, int x, int y)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (position)
    {
        map_t *map = &entity->game->maps[position->level];

        TCOD_map_t TCOD_map = map_to_TCOD_map(map);
        TCOD_map_set_properties(TCOD_map, x, y, TCOD_map_is_transparent(TCOD_map, x, y), true);

        TCOD_path_t path = TCOD_path_new_using_map(TCOD_map, 1.0f);
        TCOD_path_compute(path, position->x, position->y, x, y);

        {
            int next_x, next_y;
            if (!TCOD_path_is_empty(path) && TCOD_path_walk(path, &next_x, &next_y, false))
            {
                entity_move(entity, next_x, next_y);
            }
            else
            {
                entity_move_towards(entity, x, y);
            }
        }

        TCOD_path_delete(path);

        TCOD_map_delete(TCOD_map);
    }
}

void entity_move_towards(entity_t *entity, int x, int y)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (position)
    {
        int dx = x - position->x;
        int dy = y - position->y;
        float d = distance(position->x, position->y, x, y);

        if (d > 0.0f)
        {
            dx = (int)roundf(dx / d);
            dy = (int)roundf(dy / d);

            entity_move(entity, position->x + dx, position->y + dy);
        }
    }
}

bool entity_move(entity_t *entity, int x, int y)
{
    bool success = false;

    if (map_is_inside(x, y))
    {
        position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

        if (position)
        {
            map_t *map = &entity->game->maps[position->level];
            tile_t *tile = &map->tiles[x][y];
            tile_info_t *tile_info = &entity->game->tile_info[tile->type];

            bool skip_move = false;

            if (!skip_move)
            {
                if (tile->type == TILE_DOOR_CLOSED)
                {
                    skip_move = true;
                    success = true;

                    entity_open_door(entity, tile);
                }
            }

            if (!skip_move)
            {
                if (!tile_info->is_walkable)
                {
                    skip_move = true;
                }
            }

            if (!skip_move)
            {
                for (void **iterator = TCOD_list_begin(tile->entities); iterator != TCOD_list_end(tile->entities); iterator++)
                {
                    entity_t *other = *iterator;

                    component_t *other_solid = component_get(other, COMPONENT_SOLID);

                    if (other_solid)
                    {
                        skip_move = true;

                        health_t *other_health = (health_t *)component_get(other, COMPONENT_HEALTH);

                        if (other_health)
                        {
                            alignment_t *alignment = (alignment_t *)component_get(entity, COMPONENT_ALIGNMENT);

                            alignment_t *other_alignment = (alignment_t *)component_get(other, COMPONENT_ALIGNMENT);

                            if (alignment && other_alignment &&
                                alignment->type == other_alignment->type)
                            {
                                // TODO: only the player can swap?
                                if (entity == entity->game->player)
                                {
                                    success = true;

                                    entity_swap(entity, other);
                                }
                            }
                            else
                            {
                                success = true;

                                entity_attack(entity, other);
                            }
                        }

                        break;
                    }
                }
            }

            if (!skip_move)
            {
                entity_map_remove(entity);
                position->x = x;
                position->y = y;
                entity_map_place(entity);

                success = true;
            }
        }
    }

    return success;
}

bool entity_interact(entity_t *entity, int x, int y, action_t action)
{
    bool success = false;

    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (position)
    {
        map_t *map = &entity->game->maps[position->level];
        tile_t *tile = &map->tiles[x][y];

        switch (action)
        {
        case ACTION_DESCEND:
        {
            success = entity_descend(entity);
        }
        break;
        case ACTION_ASCEND:
        {
            success = entity_ascend(entity);
        }
        break;
        case ACTION_CLOSE_DOOR:
        {
            success = entity_close_door(entity, tile);
        }
        break;
        case ACTION_OPEN_DOOR:
        {
            success = entity_open_door(entity, tile);
        }
        break;
        }
    }

    return success;
}

bool entity_descend(entity_t *entity)
{
    bool success = false;

    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (position)
    {
        map_t *map = &entity->game->maps[position->level];
        tile_t *tile = &map->tiles[position->x][position->y];

        if (tile->type == TILE_STAIR_DOWN)
        {
            if (position->level < NUM_MAPS)
            {
                map_t *next_map = &entity->game->maps[position->level + 1];

                entity_map_remove(entity);
                position->level++;
                position->x = next_map->stair_up_x;
                position->y = next_map->stair_up_y;
                entity_map_place(entity);

                {
                    appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

                    if (appearance)
                    {
                        game_log(entity->game, position, TCOD_white, "%s descends", appearance->name);
                    }
                }

                success = true;
            }
            else
            {
                appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

                if (appearance)
                {
                    game_log(entity->game, position, TCOD_white, "%s has reached the end", appearance->name);
                }
            }
        }
        else
        {
            appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

            if (appearance)
            {
                game_log(entity->game, position, TCOD_white, "%s can't descend here", appearance->name);
            }
        }
    }

    return success;
}

bool entity_ascend(entity_t *entity)
{
    bool success = false;

    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (position)
    {
        map_t *map = &entity->game->maps[position->level];
        tile_t *tile = &map->tiles[position->x][position->y];

        if (tile->type == TILE_STAIR_UP)
        {
            if (position->level > 0)
            {
                map_t *next_map = &entity->game->maps[position->level - 1];

                entity_map_remove(entity);
                position->level--;
                position->x = next_map->stair_down_x;
                position->y = next_map->stair_down_y;
                entity_map_place(entity);

                {
                    appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

                    if (appearance)
                    {
                        game_log(entity->game, position, TCOD_white, "%s ascends", appearance->name);
                    }
                }

                success = true;
            }
            else
            {
                // TODO: end game
                appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

                if (appearance)
                {
                    game_log(entity->game, position, TCOD_white, "%s can't go any higher", appearance->name);
                }
            }
        }
        else
        {
            appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

            if (appearance)
            {
                game_log(entity->game, position, TCOD_white, "%s can't ascend here", appearance->name);
            }
        }
    }

    return success;
}

bool entity_close_door(entity_t *entity, tile_t *tile)
{
    bool success = false;

    if (tile->type == TILE_DOOR_OPEN)
    {
        success = true;

        tile->type = TILE_DOOR_CLOSED;

        appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);
        position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

        if (appearance && position)
        {
            game_log(entity->game, position, TCOD_white, "%s closes the door", appearance->name);
        }
    }
    else
    {
        appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);
        position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

        if (appearance && position)
        {
            game_log(entity->game, position, TCOD_white, "%s can't close that", appearance->name);
        }
    }

    return success;
}

bool entity_open_door(entity_t *entity, tile_t *tile)
{
    bool success = false;

    if (tile->type == TILE_DOOR_CLOSED)
    {
        success = true;

        tile->type = TILE_DOOR_OPEN;

        appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);
        position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

        if (appearance && position)
        {
            game_log(entity->game, position, TCOD_white, "%s opens the door", appearance->name);
        }
    }
    else
    {
        appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);
        position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

        if (appearance && position)
        {
            game_log(entity->game, position, TCOD_white, "%s can't open that", appearance->name);
        }
    }

    return success;
}

void entity_swap(entity_t *entity, entity_t *other)
{
    if (entity != other)
    {
        position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

        position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

        if (position && other_position)
        {
            int temp_x = position->x;
            int temp_y = position->y;

            entity_map_remove(entity);
            position->x = other_position->x;
            position->y = other_position->y;
            entity_map_place(entity);

            entity_map_remove(other);
            other_position->x = temp_x;
            other_position->y = temp_y;
            entity_map_place(other);

            {
                appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

                appearance_t *other_appearance = (appearance_t *)component_get(other, COMPONENT_APPEARANCE);

                if (appearance && other_appearance)
                {
                    game_log(entity->game, position, TCOD_white, "%s swaps with %s", appearance->name, other_appearance->name);
                }
            }
        }
    }
}

void entity_pick(entity_t *entity, entity_t *other)
{
    inventory_t *inventory = (inventory_t *)component_get(entity, COMPONENT_INVENTORY);
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    pickable_t *other_pickable = (pickable_t *)component_get(other, COMPONENT_PICKABLE);
    position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

    if (inventory && position && other_pickable && other_position)
    {
        {
            map_t *other_map = &other->game->maps[other_position->level];
            tile_t *other_tile = &other_map->tiles[other_position->x][other_position->y];

            TCOD_list_remove(other_tile->entities, other);
            TCOD_list_push(inventory->items, other);
        }

        component_remove(other, COMPONENT_POSITION);

        {
            appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

            appearance_t *other_appearance = (appearance_t *)component_get(other, COMPONENT_APPEARANCE);

            if (appearance && other_appearance)
            {
                game_log(entity->game, position, TCOD_white, "%s picks up %s", appearance->name, other_appearance->name);
            }
        }
    }
}

bool entity_swing(entity_t *entity, int x, int y)
{
    bool success = false;

    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (position)
    {
        success = true;

        map_t *map = &entity->game->maps[position->level];
        tile_t *other_tile = &map->tiles[x][y];

        bool hit = false;

        for (void **iterator = TCOD_list_begin(other_tile->entities); iterator != TCOD_list_end(other_tile->entities); iterator++)
        {
            entity_t *other = *iterator;

            health_t *other_health = (health_t *)component_get(other, COMPONENT_HEALTH);

            if (other_health)
            {
                hit = true;

                entity_attack(entity, other);

                break;
            }
        }

        if (!hit)
        {
            appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

            if (appearance)
            {
                game_log(entity->game, position, TCOD_white, "%s swings at the air", appearance->name);
            }
        }
    }

    return success;
}

void entity_shoot(entity_t *entity, int x, int y, void (*on_hit)(void *on_hit_params), void *on_hit_params)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (position)
    {
        if (position->x == x && position->y == y)
        {
        }
        else
        {
            entity_t *arrow = entity_create(entity->game);

            projectile_t *arrow_projectile = (projectile_t *)component_add(arrow, COMPONENT_PROJECTILE);
            arrow_projectile->x = (float)position->x;
            arrow_projectile->y = (float)position->y;
            // TODO: this is somewhat inaccurate
            float dx = (float)x - (float)position->x;
            float dy = (float)y - (float)position->y;
            float d = distance(position->x, position->y, x, y);
            if (d > 0.0f)
            {
                dx /= d;
                dy /= d;
            }
            arrow_projectile->dx = dx;
            arrow_projectile->dy = dy;
            arrow_projectile->shooter = entity;
            arrow_projectile->on_hit = on_hit;
            arrow_projectile->on_hit_params = on_hit_params;

            appearance_t *arrow_appearance = (appearance_t *)component_add(arrow, COMPONENT_APPEARANCE);
            arrow_appearance->name = "Arrow";
            arrow_appearance->color = TCOD_white;
            arrow_appearance->layer = LAYER_0;
            float a = angle(position->x, position->y, x, y);
            if ((a >= 0.0f && a <= 30.0f) ||
                (a >= 150.0f && a <= 180.0f) ||
                (a >= 180.0f && a <= 210.0f) ||
                (a >= 330.0f && a <= 360.0f))
            {
                arrow_appearance->glyph = '-';
            }
            else if ((a >= 30.0f && a <= 60.0f) ||
                     (a >= 210.0f && a <= 240.0f))
            {
                arrow_appearance->glyph = '/';
            }
            else if ((a >= 60.0f && a <= 90.0f) ||
                     (a >= 90.0f && a <= 120.0f) ||
                     (a >= 240.0f && a <= 270.0f) ||
                     (a >= 270.0f && a <= 300.0f))
            {
                arrow_appearance->glyph = '|';
            }
            else if ((a >= 120.0f && a <= 150.0f) ||
                     (a >= 300.0f && a <= 330.0f))
            {
                arrow_appearance->glyph = '\\';
            }

            position_t *arrow_position = (position_t *)component_add(arrow, COMPONENT_POSITION);
            arrow_position->level = position->level;
            arrow_position->x = position->x;
            arrow_position->y = position->y;
            entity_map_place(arrow);
        }
    }
}

void entity_attack(entity_t *entity, entity_t *other)
{
    appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    appearance_t *other_appearance = (appearance_t *)component_get(other, COMPONENT_APPEARANCE);
    health_t *other_health = (health_t *)component_get(other, COMPONENT_HEALTH);
    position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

    if (appearance && position && other_appearance && other_health && other_position)
    {
        int attack_roll = roll(1, 20);
        int attack_bonus = 1;
        int total_attack = attack_roll + attack_bonus;
        int other_armor_class = 5;
        bool hit = attack_roll == 1
                       ? false
                       : attack_roll == 20
                             ? true
                             : total_attack >= other_armor_class;

        if (hit)
        {
            // 1d8 19-20x2
            int weapon_a = 1;
            int weapon_x = 8;
            int weapon_threat_range = 19;
            int weapon_crit_multiplier = 2;

            int damage_rolls = 1;

            bool crit = false;
            if (attack_roll >= weapon_threat_range)
            {
                int threat_roll = roll(1, 20);
                int total_threat = threat_roll + attack_bonus;

                if (total_threat >= other_armor_class)
                {
                    crit = true;
                    damage_rolls *= weapon_crit_multiplier;
                }
            }

            int total_damage = 0;
            int damage_bonus = 0;
            for (int i = 0; i < damage_rolls; i++)
            {
                int damage_roll = roll(weapon_a, weapon_x);
                int damage = damage_roll + damage_bonus;

                total_damage += damage;
            }

            game_log(entity->game, position, crit ? TCOD_yellow : TCOD_white, "%s %s %s for %d", appearance->name, crit ? "crits" : "hits", other_appearance->name, total_damage);

            other_health->current -= total_damage;

            flash_t *other_flash = (flash_t *)component_add(other, COMPONENT_FLASH);
            other_flash->color = TCOD_red;
            other_flash->fade = 1.0f;

            if (other_health->current <= 0)
            {
                entity_die(other, entity);
            }
        }
        else
        {
            game_log(entity->game, position, TCOD_white, "%s misses", appearance->name);
        }
    }
}

void entity_cast_spell(entity_t *entity)
{
    caster_t *caster = (caster_t *)component_get(entity, COMPONENT_CASTER);

    if (caster)
    {
        spell_t *spell = &caster->spells[caster->current];

        switch (spell->type)
        {
        case SPELL_HEAL_SELF:
        {
            health_t *health = (health_t *)component_get(entity, COMPONENT_HEALTH);

            if (health)
            {
                int heal_amount = health->max - health->current;

                health->current += heal_amount;

                flash_t *flash = (flash_t *)component_add(entity, COMPONENT_FLASH);
                flash->color = TCOD_green;
                flash->fade = 1.0f;

                appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);
                position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

                if (appearance != NULL && position != NULL)
                {
                    game_log(entity->game, position, TCOD_purple, "%s casts Heal Self, restoring %d health", appearance->name, heal_amount);
                }
            }
        }
        break;
        case SPELL_INSTAKILL:
        {
            appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);
            fov_t *fov = (fov_t *)component_get(entity, COMPONENT_FOV);
            position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
            targeting_t *targeting = (targeting_t *)component_get(entity, COMPONENT_TARGETING);

            if (appearance && fov && targeting && position)
            {
                tile_t *tile = &entity->game->maps[position->level].tiles[targeting->x][targeting->y];

                entity_t *target = NULL;

                for (void **iterator = TCOD_list_begin(tile->entities); iterator != TCOD_list_end(tile->entities); iterator++)
                {
                    entity_t *other = *iterator;

                    position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

                    if (other_position)
                    {
                        if (TCOD_map_is_in_fov(fov->fov_map, other_position->x, other_position->y))
                        {
                            target = other;
                        }
                    }
                }

                if (target)
                {
                    if (target == entity)
                    {
                        game_log(entity->game, position, TCOD_white, "%s thinks that's a bad idea!", appearance->name);
                    }
                    else
                    {
                        entity_die(target, entity);
                    }
                }
                else
                {
                    game_log(entity->game, position, TCOD_purple, "%s casts Instakill", appearance->name);
                }
            }
        }
        break;
        }
    }
}

void entity_die(entity_t *entity, entity_t *killer)
{
    appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (appearance && position)
    {
        game_log(entity->game, position, TCOD_red, "%s dies", appearance->name);

        appearance->glyph = '%';
        appearance->layer = LAYER_0;

        component_remove(entity, COMPONENT_ALIGNMENT);
        component_remove(entity, COMPONENT_AI);
        component_remove(entity, COMPONENT_HEALTH);
        component_remove(entity, COMPONENT_SOLID);

        if (killer)
        {
            appearance_t *killer_appearance = (appearance_t *)component_get(killer, COMPONENT_APPEARANCE);
            position_t *killer_position = (position_t *)component_get(killer, COMPONENT_POSITION);

            if (killer_appearance && killer_position)
            {
                game_log(killer->game, killer_position, TCOD_azure, "%s gains %d experience", killer_appearance->name, TCOD_random_get_int(NULL, 50, 100));
            }
        }

        if (entity == entity->game->player)
        {
            entity->game->game_over = true;

            TCOD_sys_delete_file("../saves/save.gz");

            game_log(entity->game, position, TCOD_green, "Game over! Press 'r' to restart");
        }
    }
}

void entity_destroy(entity_t *entity)
{
    if (entity && entity->id != ID_UNUSED)
    {
        for (component_type_t component_type = 0; component_type < NUM_COMPONENTS; component_type++)
        {
            component_remove(entity, component_type);
        }

        entity_reset(entity);
    }
}

void entity_reset(entity_t *entity)
{
    entity->id = ID_UNUSED;
    entity->game = NULL;
}
