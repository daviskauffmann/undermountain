#include "game.h"

#include "game/actor.h"
#include "game/class.h"
#include "game/map.h"
#include "game/tile.h"
#include "game/corpse.h"
#include "game/world.h"
#include "game/object.h"
#include "game/base_item.h"
#include "game/item.h"
#include "game/message.h"
#include "game/light.h"
#include "game/list.h"
#include "game/surface.h"
#include "scene.h"
#include "ui.h"
#include "scenes/game/direction.h"
#include "game/ability.h"
#include "game/race.h"
#include "game/saving_throw.h"
#include "game/spell.h"
#include <curses.h>
#include <libtcod.h>
#include <stdio.h>
#include <string.h>

#define MESSAGE_LOG_HEIGHT 5
#define STATUS_WIDTH 20

// Forward declarations
extern struct scene menu_scene;

// Viewport tracking
static int view_x = 0;
static int view_y = 0;
static int view_width = 0;
static int view_height = 0;

// Full-screen overlay state
enum overlay
{
    OVERLAY_NONE,
    OVERLAY_INVENTORY,
    OVERLAY_CHARACTER,
    OVERLAY_SPELLBOOK,
};

static enum overlay active_overlay = OVERLAY_NONE;
static int overlay_scroll = 0;

static void open_overlay(enum overlay o)
{
    if (active_overlay == o)
        active_overlay = OVERLAY_NONE;
    else
    {
        active_overlay = o;
        overlay_scroll = 0;
    }
}

static void init(const struct scene *const previous_scene)
{
    (void)previous_scene;

    // Calculate viewport size (use full terminal for now)
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    view_width = max_x;
    view_height = max_y - MESSAGE_LOG_HEIGHT;
}

static void uninit(void)
{
    // Save the game
    if (world && !world->doomed)
    {
        // TODO: Save to file when we implement save/load
    }

    if (world)
    {
        world_uninit();
    }
}

static bool try_move_player(enum direction direction)
{
    if (!world || !world->player)
    {
        return false;
    }

    int target_x, target_y;
    get_neighbor_by_direction(world->player->x, world->player->y, direction, &target_x, &target_y);

    return actor_move(world->player, target_x, target_y);
}

static struct scene *input(void)
{
    int ch = getch();
    if (ch == ERR)
        return &game_scene;

    // When an overlay is open, intercept navigation keys and ESC.
    if (active_overlay != OVERLAY_NONE)
    {
        switch (ch)
        {
        case 27: // ESC — close overlay
            active_overlay = OVERLAY_NONE;
            break;
        case KEY_UP:
        case 'k':
        case 'K':
            overlay_scroll--;
            if (overlay_scroll < 0)
                overlay_scroll = 0;
            break;
        case KEY_DOWN:
        case 'j':
        case 'J':
            overlay_scroll++;
            break;
        case KEY_PPAGE: // Page Up
            overlay_scroll -= (view_height - 2);
            if (overlay_scroll < 0)
                overlay_scroll = 0;
            break;
        case KEY_NPAGE: // Page Down
            overlay_scroll += (view_height - 2);
            break;
        // Still allow toggling overlays while one is open
        case 'i':
        case 'I':
            open_overlay(OVERLAY_INVENTORY);
            break;
        case 'c':
        case 'C':
            open_overlay(OVERLAY_CHARACTER);
            break;
        case 'v':
        case 'V':
            open_overlay(OVERLAY_SPELLBOOK);
            break;
        default:
            break; // all other keys ignored while overlay is open
        }
        return &game_scene;
    }

    // Normal (no overlay) input
    switch (ch)
    {
    case 27: // ESC
        return &menu_scene;

    // Arrow keys - cardinal directions
    case KEY_UP:
        world->player->took_turn = world_can_player_take_turn() && try_move_player(DIRECTION_N);
        break;
    case KEY_DOWN:
        world->player->took_turn = world_can_player_take_turn() && try_move_player(DIRECTION_S);
        break;
    case KEY_LEFT:
        world->player->took_turn = world_can_player_take_turn() && try_move_player(DIRECTION_W);
        break;
    case KEY_RIGHT:
        world->player->took_turn = world_can_player_take_turn() && try_move_player(DIRECTION_E);
        break;

    // Vi keys - hjkl for cardinal, yubn for diagonal
    case 'h':
    case 'H':
        world->player->took_turn = world_can_player_take_turn() && try_move_player(DIRECTION_W);
        break;
    case 'j':
    case 'J':
        world->player->took_turn = world_can_player_take_turn() && try_move_player(DIRECTION_S);
        break;
    case 'k':
    case 'K':
        world->player->took_turn = world_can_player_take_turn() && try_move_player(DIRECTION_N);
        break;
    case 'l':
    case 'L':
        world->player->took_turn = world_can_player_take_turn() && try_move_player(DIRECTION_E);
        break;
    case 'y':
    case 'Y':
        world->player->took_turn = world_can_player_take_turn() && try_move_player(DIRECTION_NW);
        break;
    case 'u':
    case 'U':
        world->player->took_turn = world_can_player_take_turn() && try_move_player(DIRECTION_NE);
        break;
    case 'b':
    case 'B':
        world->player->took_turn = world_can_player_take_turn() && try_move_player(DIRECTION_SW);
        break;
    case 'n':
    case 'N':
        world->player->took_turn = world_can_player_take_turn() && try_move_player(DIRECTION_SE);
        break;

    // Wait/rest (period or numpad 5)
    case '.':
    case '5':
        world->player->took_turn = world_can_player_take_turn();
        break;

    // Numpad support
    case '1':
        world->player->took_turn = world_can_player_take_turn() && try_move_player(DIRECTION_SW);
        break;
    case '2':
        world->player->took_turn = world_can_player_take_turn() && try_move_player(DIRECTION_S);
        break;
    case '3':
        world->player->took_turn = world_can_player_take_turn() && try_move_player(DIRECTION_SE);
        break;
    case '4':
        world->player->took_turn = world_can_player_take_turn() && try_move_player(DIRECTION_W);
        break;
    case '6':
        world->player->took_turn = world_can_player_take_turn() && try_move_player(DIRECTION_E);
        break;
    case '7':
        world->player->took_turn = world_can_player_take_turn() && try_move_player(DIRECTION_NW);
        break;
    case '8':
        world->player->took_turn = world_can_player_take_turn() && try_move_player(DIRECTION_N);
        break;
    case '9':
        world->player->took_turn = world_can_player_take_turn() && try_move_player(DIRECTION_NE);
        break;

    case 'i':
    case 'I':
        open_overlay(OVERLAY_INVENTORY);
        break;

    case 'c':
    case 'C':
        open_overlay(OVERLAY_CHARACTER);
        break;

    case 'v':
    case 'V':
        open_overlay(OVERLAY_SPELLBOOK);
        break;

    case KEY_PPAGE: // Page Up — no overlay, ignore
    case KEY_NPAGE: // Page Down — no overlay, ignore
        break;

    case 't':
        if (world_can_player_take_turn())
        {
            if (world->player->light_type == LIGHT_TYPE_TORCH)
            {
                world->player->light_type = LIGHT_TYPE_PLAYER;
            }
            else
            {
                world->player->light_type = LIGHT_TYPE_TORCH;
            }

            world->player->took_turn = true;
        }
        break;
    case 'T':
        if (world_can_player_take_turn())
        {
            if (world->player->light_type == LIGHT_TYPE_GLOW)
            {
                world->player->light_type = LIGHT_TYPE_PLAYER;
            }
            else
            {
                world->player->light_type = LIGHT_TYPE_GLOW;
            }

            world->player->took_turn = true;
        }
    }

    return &game_scene;
}

static int get_wall_glyph(const struct map *const map, const int x, const int y)
{
    static const int glyphs[] = {
        0x25A0, //  0 - none = ■
        0x2502, //  1 - N    = │
        0x2500, //  2 - E    = ─
        0x2514, //  3 - NE   = └
        0x2502, //  4 - S    = │
        0x2502, //  5 - NS   = │
        0x250C, //  6 - SE   = ┌
        0x251C, //  7 - NES  = ├
        0x2500, //  8 - W    = ─
        0x2518, //  9 - NW   = ┘
        0x2500, // 10 - EW   = ─
        0x2534, // 11 - NEW  = ┴
        0x2510, // 12 - SW   = ┐
        0x2524, // 13 - NSW  = ┤
        0x252C, // 14 - ESW  = ┬
        0x253C, // 15 - NESW = ┼
    };

    int index = 0;
    if (y > 0 && map->tiles[x][y - 1].type == TILE_TYPE_WALL)
    {
        index |= 1 << 0;
    }
    if (x < MAP_WIDTH - 1 && map->tiles[x + 1][y].type == TILE_TYPE_WALL)
    {
        index |= 1 << 1;
    }
    if (y < MAP_HEIGHT - 1 && map->tiles[x][y + 1].type == TILE_TYPE_WALL)
    {
        index |= 1 << 2;
    }
    if (x > 0 && map->tiles[x - 1][y].type == TILE_TYPE_WALL)
    {
        index |= 1 << 3;
    }

    return glyphs[index];
}

static void render(void)
{
    if (!world || !world->player)
    {
        return;
    }

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    clear();

    // Recompute view dimensions each frame
    view_width = max_x;
    view_height = max_y - MESSAGE_LOG_HEIGHT;

    // Center view on player
    const struct actor *player = world->player;
    view_x = player->x - view_width / 2;
    view_y = player->y - view_height / 2;

    // Clamp view to map bounds
    if (view_x + view_width > MAP_WIDTH)
    {
        view_x = MAP_WIDTH - view_width;
    }
    if (view_x < 0)
    {
        view_x = 0;
    }
    if (view_y + view_height > MAP_HEIGHT)
    {
        view_y = MAP_HEIGHT - view_height;
    }
    if (view_y < 0)
    {
        view_y = 0;
    }

    // Get current map
    const struct map *map = &world->maps[player->floor];

    // Draw tiles
    for (int x = view_x; x < view_x + view_width && x < MAP_WIDTH; x++)
    {
        for (int y = view_y; y < view_y + view_height && y < MAP_HEIGHT; y++)
        {
            if (!map_is_inside(x, y))
            {
                continue;
            }

            const struct tile *tile = &map->tiles[x][y];
            const struct tile_data *tile_data = &tile_database[tile->type];

            // Only draw explored tiles
            if (tile->explored)
            {
                int glyph = tile_data->glyph;
                int screen_x = x - view_x;
                int screen_y = y - view_y;

                // Select appropriate wall graphic
                if (tile->type == TILE_TYPE_WALL)
                {
                    glyph = get_wall_glyph(map, x, y);
                }

                // Check if in FOV (visible now)
                bool in_fov = TCOD_map_is_in_fov(player->fov, x, y);

                if (in_fov)
                {
                    if (tile->actor)
                    {
                        const struct actor *actor = tile->actor;
                        const struct class_data *class_data = &class_database[actor->class];
                        glyph = class_data->glyph;
                    }
                    else if (tile->object && tile->object->trap_detection_state == OBJECT_TRAP_DETECTION_STATE_DETECTED)
                    {
                        const struct object_data *object_data = &object_database[tile->object->type];

                        glyph = object_data->glyph;
                    }

                    else if (tile->item)
                    {
                        const struct item_data *const item_data = &item_database[tile->item->type];
                        const struct base_item_data *const base_item_data = &base_item_database[item_data->type];

                        glyph = base_item_data->glyph;
                    }
                    else if (tile->corpse)
                    {
                        glyph = corpse_metadata.glyph;
                    }
                    else if (tile->surface)
                    {
                        const struct surface_data *const surface_data = &surface_database[tile->surface->type];

                        if (surface_data->glyph)
                        {
                            glyph = surface_data->glyph;
                        }
                    }

                    // Draw in bright (visible)
                    attron(A_BOLD);
                    mvaddch(screen_y, screen_x, glyph);
                    attroff(A_BOLD);
                }
                else
                {
                    // Draw dimmed (remembered but not currently visible)
                    mvaddch(screen_y, screen_x, glyph);
                }
            }
        }
    }

    // ── Overlay rendering ─────────────────────────────────────────────────
    // Helpers shared by all three overlays.

#define OVL_SROW(lrow_, scroll_) ((lrow_) - (scroll_) + 2)
#define OVL_VIS(lrow_, scroll_) (OVL_SROW(lrow_, scroll_) >= 2 && OVL_SROW(lrow_, scroll_) <= (max_y - 2))

    // Write a label on the left and a value right-aligned within [col, col+w).
    // Defined as a local lambda-style macro, used across all overlay functions.
#define OVL_LR(col_, w_, label_, val_buf_)                                 \
    do                                                                     \
    {                                                                      \
        mvprintw(OVL_SROW(lrow, scroll), (col_), "%-.*s", (w_), (label_)); \
        int _vl = (int)strlen(val_buf_);                                   \
        int _vc = (col_) + (w_) - _vl;                                     \
        if (_vc >= (col_))                                                 \
            mvprintw(OVL_SROW(lrow, scroll), _vc, "%s", (val_buf_));       \
        lrow++;                                                            \
    } while (0)

#define OVL_HDR(col_, w_, text_)                                              \
    do                                                                        \
    {                                                                         \
        if (OVL_VIS(lrow, scroll))                                            \
        {                                                                     \
            attron(A_BOLD | A_UNDERLINE);                                     \
            mvprintw(OVL_SROW(lrow, scroll), (col_), "%-.*s", (w_), (text_)); \
            attroff(A_BOLD | A_UNDERLINE);                                    \
        }                                                                     \
        lrow++;                                                               \
    } while (0)

#define OVL_FULL(col_, w_, fmt_, ...)                                                \
    do                                                                               \
    {                                                                                \
        if (OVL_VIS(lrow, scroll))                                                   \
            mvprintw(OVL_SROW(lrow, scroll), (col_), "%-.*s", (w_), (char[128]){0}); \
        if (OVL_VIS(lrow, scroll))                                                   \
            mvprintw(OVL_SROW(lrow, scroll), (col_), fmt_, ##__VA_ARGS__);           \
        lrow++;                                                                      \
    } while (0)

#define OVL_BLANK() \
    do              \
    {               \
        lrow++;     \
    } while (0)

    // Draw a centred title, erase map content, and add the scroll hint.
#define OVERLAY_FRAME(title_, hint_) \
    do                               \
    {                                \
        ui_screen_frame(title_);     \
        ui_footer_hint(hint_);       \
    } while (0)

#define OVERLAY_DIVIDER(dcol_)                    \
    do                                            \
    {                                             \
        for (int _dy = 1; _dy < max_y - 1; _dy++) \
            mvaddch(_dy, (dcol_), ACS_VLINE);     \
    } while (0)

#define OVERLAY_CLAMP_AND_ARROWS(total_)               \
    do                                                 \
    {                                                  \
        int _ms = (total_) - (max_y - 3);              \
        if (_ms < 0)                                   \
            _ms = 0;                                   \
        if (overlay_scroll > _ms)                      \
            overlay_scroll = _ms;                      \
        if (overlay_scroll < 0)                        \
            overlay_scroll = 0;                        \
        if (overlay_scroll > 0)                        \
            mvaddch(2, max_x - 1, ACS_UARROW);         \
        if ((total_) - overlay_scroll > max_y - 3)     \
            mvaddch(max_y - 2, max_x - 1, ACS_DARROW); \
    } while (0)

    if (active_overlay == OVERLAY_INVENTORY)
    {
        OVERLAY_FRAME("Inventory", "[↑↓/jk] scroll  [PgUp/PgDn] page  [ESC] close");

        // Split: equipment on left, carried items on right
        int div_col = max_x / 2;
        int left_w = div_col - 3;          // col 2 .. div_col-1
        int right_w = max_x - div_col - 3; // div_col+2 .. max_x-2
        OVERLAY_DIVIDER(div_col);

        // Left column — Equipment
        {
            int lrow = 0, scroll = overlay_scroll;
            OVL_HDR(2, left_w, "Equipment");
            for (enum equip_slot slot = EQUIP_SLOT_NONE + 1; slot < NUM_EQUIP_SLOTS; slot++)
            {
                if (OVL_VIS(lrow, scroll))
                {
                    const struct item *eq = world->player->equipment[slot];
                    char val[64];
                    snprintf(val, sizeof(val), "%s", eq ? item_database[eq->type].name : "(empty)");
                    OVL_LR(2, left_w, equip_slot_database[slot].name, val);
                }
                else
                    lrow++;
            }
            OVERLAY_CLAMP_AND_ARROWS(lrow);
        }

        // Right column — Carried items
        {
            int lrow = 0, scroll = overlay_scroll;
            int rcol = div_col + 2;
            OVL_HDR(rcol, right_w, "Carried Items");
            if (world->player->items->size == 0)
            {
                OVL_FULL(rcol, right_w, "(nothing)");
            }
            else
            {
                for (size_t ii = 0; ii < world->player->items->size; ii++)
                {
                    if (OVL_VIS(lrow, scroll))
                    {
                        const struct item *it = list_get(world->player->items, ii);
                        const struct item_data *id = &item_database[it->type];
                        char buf[128];
                        if (it->stack > 1)
                            snprintf(buf, sizeof(buf), "%c) %s (%d)", (char)('a' + ii), id->name, it->stack);
                        else
                            snprintf(buf, sizeof(buf), "%c) %s", (char)('a' + ii), id->name);
                        mvprintw(OVL_SROW(lrow, scroll), rcol, "%-.*s", right_w, buf);
                    }
                    lrow++;
                }
            }
            // right column drives scroll clamping (usually longer than equipment list)
            OVERLAY_CLAMP_AND_ARROWS(lrow);
        }
    }
    else if (active_overlay == OVERLAY_CHARACTER)
    {
        OVERLAY_FRAME("Character Sheet", "[↑↓/jk] scroll  [PgUp/PgDn] page  [ESC] close");

        // Two-column layout: identity+abilities on left, combat+saves on right
        int div_col = max_x / 2;
        int left_w = div_col - 3;
        int right_w = max_x - div_col - 3;
        int rcol = div_col + 2;
        OVERLAY_DIVIDER(div_col);

        // We advance lrow_l and lrow_r in tandem for each "section" so the
        // vertical rhythm of both columns is independent.

        // ── Left column ───────────────────────────────────────────────────
        {
            int lrow = 0, scroll = overlay_scroll;
            char val[64];

            OVL_HDR(2, left_w, "Identity");
            snprintf(val, sizeof(val), "%s", world->player->name);
            OVL_LR(2, left_w, "Name", val);
            snprintf(val, sizeof(val), "%s", race_database[world->player->race].name);
            OVL_LR(2, left_w, "Race", val);
            snprintf(val, sizeof(val), "%s", class_database[world->player->class].name);
            OVL_LR(2, left_w, "Class", val);
            snprintf(val, sizeof(val), "%d", world->player->level);
            OVL_LR(2, left_w, "Level", val);
            snprintf(val, sizeof(val), "%d / %d", world->player->experience,
                     actor_get_experience_for_level(world->player->level + 1));
            OVL_LR(2, left_w, "XP", val);
            snprintf(val, sizeof(val), "%d gp", world->player->gold);
            OVL_LR(2, left_w, "Gold", val);
            snprintf(val, sizeof(val), "%.1f / %.1f lb",
                     actor_get_carry_weight(world->player),
                     actor_get_max_carry_weight(world->player));
            OVL_LR(2, left_w, "Weight", val);
            if (world->player->ability_points > 0)
            {
                snprintf(val, sizeof(val), "%d (unspent!)", world->player->ability_points);
                OVL_LR(2, left_w, "Ability pts", val);
            }

            OVL_BLANK();
            OVL_HDR(2, left_w, "Abilities");
            for (enum ability ab = ABILITY_NONE + 1; ab < NUM_ABILITIES; ab++)
            {
                snprintf(val, sizeof(val), "%d  (%+d)",
                         actor_get_ability_score(world->player, ab),
                         actor_get_ability_modifer(world->player, ab));
                OVL_LR(2, left_w, ability_database[ab].name, val);
            }

            OVERLAY_CLAMP_AND_ARROWS(lrow);
        }

        // ── Right column ──────────────────────────────────────────────────
        {
            int lrow = 0, scroll = overlay_scroll;
            char val[64];

            OVL_HDR(rcol, right_w, "Combat");
            snprintf(val, sizeof(val), "%d / %d",
                     world->player->hit_points, actor_get_max_hit_points(world->player));
            OVL_LR(rcol, right_w, "HP", val);
            {
                int max_mana = actor_get_max_mana(world->player);
                if (max_mana > 0)
                {
                    snprintf(val, sizeof(val), "%d / %d", world->player->mana, max_mana);
                    OVL_LR(rcol, right_w, "MP", val);
                }
            }
            snprintf(val, sizeof(val), "%d", actor_get_armor_class(world->player));
            OVL_LR(rcol, right_w, "Armor Class", val);
            {
                const struct item *const armor = world->player->equipment[EQUIP_SLOT_ARMOR];
                if (armor)
                    snprintf(val, sizeof(val), "%d",
                             base_item_database[item_database[armor->type].type].max_dexterity_bonus);
                else
                    snprintf(val, sizeof(val), "None");
                OVL_LR(rcol, right_w, "Max Dex Bonus", val);
            }
            snprintf(val, sizeof(val), "%d%%",
                     (int)(actor_get_arcane_spell_failure(world->player) * 100));
            OVL_LR(rcol, right_w, "Spell Failure", val);
            {
                int bab = actor_get_base_attack_bonus(world->player);
                int apr = actor_get_attacks_per_round(world->player);
                char bab_str[32] = {0};
                snprintf(bab_str, sizeof(bab_str), "+%d", bab);
                for (int ai = 1; ai < apr; ai++)
                {
                    char extra[16];
                    snprintf(extra, sizeof(extra), " / +%d", bab - ai * 5);
                    strncat(bab_str, extra, sizeof(bab_str) - strlen(bab_str) - 1);
                }
                OVL_LR(rcol, right_w, "Base Attack", bab_str);
            }
            snprintf(val, sizeof(val), "%+d", actor_get_attack_bonus(world->player));
            OVL_LR(rcol, right_w, "Attack Bonus", val);
            {
                const int threat = actor_get_threat_range(world->player);
                if (threat == 20)
                    snprintf(val, sizeof(val), "%s+%d (x%d)",
                             actor_get_damage(world->player),
                             actor_get_damage_bonus(world->player),
                             actor_get_critical_multiplier(world->player));
                else
                    snprintf(val, sizeof(val), "%s+%d (%d-20/x%d)",
                             actor_get_damage(world->player),
                             actor_get_damage_bonus(world->player),
                             threat,
                             actor_get_critical_multiplier(world->player));
                OVL_LR(rcol, right_w, "Damage", val);
            }
            {
                float speed = actor_get_speed(world->player);
                bool overburdened =
                    actor_get_carry_weight(world->player) > actor_get_max_carry_weight(world->player);
                if (overburdened)
                    snprintf(val, sizeof(val), "%.2f (overburdened)", speed);
                else
                    snprintf(val, sizeof(val), "%.2f", speed);
                OVL_LR(rcol, right_w, "Speed", val);
            }

            OVL_BLANK();
            OVL_HDR(rcol, right_w, "Saving Throws");
            for (enum saving_throw st = SAVING_THROW_NONE + 1; st < NUM_SAVING_THROWS; st++)
            {
                snprintf(val, sizeof(val), "%+d", actor_get_saving_throw(world->player, st));
                OVL_LR(rcol, right_w, saving_throw_database[st].name, val);
            }

            OVERLAY_CLAMP_AND_ARROWS(lrow);
        }
    }
    else if (active_overlay == OVERLAY_SPELLBOOK)
    {
        OVERLAY_FRAME("Spellbook", "[↑↓/jk] scroll  [PgUp/PgDn] page  [ESC] close");

        int col = 2;
        int content_w = max_x - 4;
        // Column widths: spell name 55%, level 10%, MP cost remainder
        int name_w = content_w * 55 / 100;
        int level_w = content_w * 10 / 100;
        int cost_col = col + name_w + level_w;

        int lrow = 0, scroll = overlay_scroll;

        // Table header
        if (OVL_VIS(lrow, scroll))
        {
            attron(A_BOLD);
            mvprintw(OVL_SROW(lrow, scroll), col, "%-*.*s", name_w, name_w, "Spell");
            mvprintw(OVL_SROW(lrow, scroll), col + name_w, "%-*.*s", level_w, level_w, "Lvl");
            mvprintw(OVL_SROW(lrow, scroll), cost_col, "%s", "MP");
            attroff(A_BOLD);
        }
        lrow++;
        if (OVL_VIS(lrow, scroll))
            mvhline(OVL_SROW(lrow, scroll), col, ACS_HLINE, content_w);
        lrow++;

        bool any_spell = false;
        const struct actor_spells player_spells = actor_get_spells(world->player);
        for (enum spell_type sp = SPELL_TYPE_NONE + 1; sp < NUM_SPELL_TYPES; sp++)
        {
            if (player_spells.has[sp])
            {
                if (OVL_VIS(lrow, scroll))
                {
                    const struct spell_data *sd = &spell_database[sp];
                    int cost = actor_get_spell_mana_cost(world->player, sp);
                    mvprintw(OVL_SROW(lrow, scroll), col, "%-*.*s", name_w, name_w, sd->name);
                    mvprintw(OVL_SROW(lrow, scroll), col + name_w, "%-*d", level_w, sd->level);
                    mvprintw(OVL_SROW(lrow, scroll), cost_col, "%d", cost);
                }
                lrow++;
                any_spell = true;
            }
        }
        if (!any_spell)
        {
            OVL_FULL(col, content_w, "(no spells known)");
        }

        OVERLAY_CLAMP_AND_ARROWS(lrow);
    }

#undef OVL_SROW
#undef OVL_VIS
#undef OVL_LR
#undef OVL_HDR
#undef OVL_FULL
#undef OVL_BLANK
#undef OVERLAY_FRAME
#undef OVERLAY_DIVIDER
#undef OVERLAY_CLAMP_AND_ARROWS

    // ── HUD separator and bottom panels ───────────────────────────────────
    if (active_overlay == OVERLAY_NONE)
    {
        // Horizontal separator above HUD
        attron(A_DIM);
        for (int x = 0; x < max_x; x++)
        {
            mvaddch(view_height, x, ACS_HLINE);
        }
        attroff(A_DIM);

        // --- Status panel (left STATUS_WIDTH columns) ---
        {
            const int max_hit_points = actor_get_max_hit_points(world->player);
            int row = view_height + 1;

            // HP bar label
            mvprintw(row++, 0, "HP: %d/%d", world->player->hit_points, max_hit_points);

            // MP bar (only for spellcasters)
            if (class_database[world->player->class].spellcasting_ability != ABILITY_NONE)
            {
                const int max_mana = actor_get_max_mana(world->player);
                mvprintw(row++, 0, "MP: %d/%d", world->player->mana, max_mana);
            }

            // Character level (* if ability points to spend)
            if (world->player->ability_points > 0)
            {
                mvprintw(row++, 0, "CL: %d*", world->player->level);
            }
            else
            {
                mvprintw(row++, 0, "CL: %d", world->player->level);
            }

            // Dungeon level
            mvprintw(row++, 0, "DL: %d", world->player->floor + 1);
        }

        // Vertical separator between status and message log
        for (int y = view_height; y < max_y; y++)
        {
            mvaddch(y, STATUS_WIDTH, y == view_height ? ACS_TTEE : ACS_VLINE);
        }

        // --- Message log (right of STATUS_WIDTH) ---
        if (world->messages && world->messages->size > 0)
        {
            int msg_width = max_x - STATUS_WIDTH - 1;
            int first_msg = (int)world->messages->size - MESSAGE_LOG_HEIGHT + 1;
            if (first_msg < 0)
            {
                first_msg = 0;
            }

            int log_y = view_height + 1;
            for (int i = first_msg; i < (int)world->messages->size && log_y < max_y; i++, log_y++)
            {
                const struct message *msg = list_get(world->messages, i);
                mvprintw(log_y, STATUS_WIDTH + 1, "%.*s", msg_width, msg->text);
                clrtoeol();
            }
        }
    }

    refresh();
}

static struct scene *update(const float delta_time)
{
    // Initialize player reference if needed
    if (!world->player)
    {
        world->player = world->hero;
    }

    // Handle input first so took_turn is set before world_update processes turns
    struct scene *next = input();
    if (next != &game_scene)
    {
        return next;
    }

    // Update world (processes AI turns, FOV, and tile exploration)
    world_update(delta_time);

    // Render the game state
    render();

    return &game_scene;
}

struct scene game_scene = {
    &init,
    &uninit,
    &update};
