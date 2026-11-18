#include "message.h"

#include "event.h"
#include "world.h"

#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MESSAGE_HISTORY_LIMIT 200

static struct message history[MESSAGE_HISTORY_LIMIT];
static size_t history_start;
static size_t history_count;

static void record_event(const struct game_event *const event, void *const context)
{
    (void)context;

    if (!world_event_is_visible(event))
    {
        return;
    }

    struct message_data data;
    if (!message_from_event(event, &data))
    {
        return;
    }

    char *const text = message_format(&data);
    if (!text)
    {
        return;
    }

    message_add(text, data.type);
    free(text);
}

static struct event_listener message_listener = {
    .receive = record_event,
};

void message_init(void)
{
    message_clear();
    event_subscribe(&message_listener);
}

void message_clear(void)
{
    for (size_t index = 0; index < history_count; index++)
    {
        const size_t slot = (history_start + index) % MESSAGE_HISTORY_LIMIT;
        free(history[slot].text);
        history[slot].text = NULL;
    }

    history_start = 0;
    history_count = 0;
}

void message_add(const char *const text, const enum message_type type)
{
    const char *line = text;
    const char *end;
    do
    {
        end = strchr(line, '\n');
        const size_t length = end ? (size_t)(end - line) : strlen(line);
        char *const copy = malloc(length + 1);
        if (!copy)
        {
            fprintf(stderr, "Cannot allocate message log text.\n");
            return;
        }

        memcpy(copy, line, length);
        copy[length] = '\0';

        const size_t slot = (history_start + history_count) % MESSAGE_HISTORY_LIMIT;
        if (history_count == MESSAGE_HISTORY_LIMIT)
        {
            free(history[slot].text);
            history_start = (history_start + 1) % MESSAGE_HISTORY_LIMIT;
        }
        else
        {
            history_count++;
        }
        history[slot].text = copy;
        history[slot].type = type;
        printf("%s\n", copy);

        if (end)
        {
            line = end + 1;
        }
    } while (end);
}

struct color message_color(const enum message_type type)
{
    switch (type)
    {
    case MESSAGE_GAME_OVER:
    case MESSAGE_ACTOR_DIED:
    case MESSAGE_TRAP_TRIGGERED:
    case MESSAGE_ACTOR_DAMAGED:
    case MESSAGE_ACTOR_BURNED:
    case MESSAGE_ACTOR_ACIDIFIED:
    case MESSAGE_ACTOR_BOMBARDED:
    case MESSAGE_SURFACE_DAMAGED:
        return (struct color){255, 95, 80};
    case MESSAGE_ACTOR_HEALED:
    case MESSAGE_ACTOR_RESTED:
    case MESSAGE_LEVEL_GAINED:
    case MESSAGE_ABILITY_INCREASED:
        return (struct color){110, 230, 80};
    case MESSAGE_EXPERIENCE_GAINED:
    case MESSAGE_GOLD_PICKED_UP:
    case MESSAGE_TRAP_DETECTED:
        return (struct color){255, 190, 50};
    case MESSAGE_SPELL_CAST:
    case MESSAGE_SPELL_READIED:
    case MESSAGE_MANA_RECOVERED:
    case MESSAGE_ACTOR_ZAPPED:
        return (struct color){90, 160, 255};
    default:
        return (struct color){255, 255, 255};
    }
}

size_t message_count(void)
{
    return history_count;
}

const struct message *message_get(const size_t index)
{
    if (index >= history_count)
    {
        fprintf(stderr, "Message history index out of range: %zu.\n", index);
        return NULL;
    }

    return &history[(history_start + index) % MESSAGE_HISTORY_LIMIT];
}

static const char *const message_catalog[NUM_MESSAGE_TYPES] = {
    [MESSAGE_ITEM_DROPPED] = "{actor} drops {item}.",
    [MESSAGE_ITEM_EQUIPPED] = "{actor} equips {item}.",
    [MESSAGE_ITEM_UNEQUIPPED] = "{actor} unequips {item}.",
    [MESSAGE_ITEM_CANNOT_EQUIP] = "{actor} cannot equip {item}.",
    [MESSAGE_ITEM_TOO_LARGE] = "{item} is too large for {actor}.",
    [MESSAGE_ITEM_TOO_SMALL] = "{item} is too small for {actor}.",
    [MESSAGE_ITEM_NOT_PROFICIENT] = "{actor} is not proficient with {item}.",
    [MESSAGE_EQUIPMENT_SLOT_EMPTY] = "{actor} is not equipping anything in their {slot} slot.",
    [MESSAGE_ATTACK_MISSED] = "{actor} misses {target}.",
    [MESSAGE_ATTACK_HIT] = "{actor} hits {target} for {amount}.",
    [MESSAGE_DAMAGE_REDUCED] = "{target}'s damage reduction absorbs {amount} damage.",
    [MESSAGE_ATTACK_SWUNG] = "{actor} swings at the air!",
    [MESSAGE_NOTHING_TO_SELECT] = "{action}: nothing to select.",
    [MESSAGE_NO_EQUIPMENT] = "That equipment slot is empty.",
    [MESSAGE_ABILITY_INCREASED] = "{ability} increased to {amount}.",
    [MESSAGE_SPELL_READIED] = "{spell} readied.",
    [MESSAGE_NO_READIED_SPELL] = "Ready a spell with Z first.",
    [MESSAGE_GAME_RESUMED] = "Welcome back, {actor}!",
    [MESSAGE_GAME_STARTED] = "Hail, {actor}!",
    [MESSAGE_GAME_OVER] = "Game over! Press 'ESC' to return to the menu.",
    [MESSAGE_EXPERIENCE_GAINED] = "{actor} gains {amount} experience.",
    [MESSAGE_LEVEL_GAINED] = "{actor} has gained a level!",
    [MESSAGE_REST_ENEMIES_NEARBY] = "{actor} cannot rest while enemies are nearby.",
    [MESSAGE_REST_NO_FOOD] = "{actor} has no food to rest with.",
    [MESSAGE_ACTOR_RESTED] = "{actor} rests.",
    [MESSAGE_TRAP_TRIGGERED] = "{actor} triggers a trap and takes {amount} damage!",
    [MESSAGE_TRAP_AVOIDED] = "{actor} avoids a trap!",
    [MESSAGE_TRAP_DETECTED] = "{actor} detects a trap!",
    [MESSAGE_TRAP_NOT_DETECTED] = "{actor} senses a nearby trap, but fails to detect it!",
    [MESSAGE_ACTORS_SWAPPED] = "{actor} swaps with {target}.",
    [MESSAGE_DOOR_CANNOT_OPEN] = "{actor} can't open the door.",
    [MESSAGE_DOOR_OPENED] = "{actor} opens the door.",
    [MESSAGE_DOOR_CANNOT_CLOSE] = "{actor} can't close this.",
    [MESSAGE_DOOR_CLOSED] = "{actor} closes the door.",
    [MESSAGE_DUNGEON_END_REACHED] = "{actor} has reached the end.",
    [MESSAGE_CANNOT_DESCEND] = "{actor} can't descend here.",
    [MESSAGE_ACTOR_DESCENDED] = "{actor} descends.",
    [MESSAGE_CANNOT_GO_HIGHER] = "{actor} can't go any higher.",
    [MESSAGE_CANNOT_ASCEND] = "{actor} can't ascend here.",
    [MESSAGE_ACTOR_ASCENDED] = "{actor} ascends.",
    [MESSAGE_CHEST_CANNOT_OPEN] = "{actor} can't open the chest.",
    [MESSAGE_CHEST_OPENED] = "{actor} opens the chest.",
    [MESSAGE_CANNOT_PRAY] = "{actor} can't pray here.",
    [MESSAGE_ALTAR_PRAYED] = "{actor} prays at the altar. The gods grant {amount} {ability}.",
    [MESSAGE_CANNOT_DRINK] = "{actor} can't drink here.",
    [MESSAGE_FOUNTAIN_DRANK] = "{actor} drinks from the fountain.",
    [MESSAGE_CANNOT_SIT] = "{actor} can't sit here",
    [MESSAGE_NOTHING_TO_PICK_UP] = "{actor} cannot find anything to pick up.",
    [MESSAGE_INVENTORY_FULL] = "{actor} is carrying too many items.",
    [MESSAGE_GOLD_PICKED_UP] = "{actor} picks up {amount} gold.",
    [MESSAGE_ITEM_PICKED_UP] = "{actor} picks up {item}.",
    [MESSAGE_ITEM_CANNOT_QUAFF] = "{actor} cannot quaff {item}.",
    [MESSAGE_ITEM_QUAFFED] = "{actor} quaffs {item}.",
    [MESSAGE_ITEM_CANNOT_READ] = "{actor} cannot read {item}.",
    [MESSAGE_ITEM_READ] = "{actor} reads {item}.",
    [MESSAGE_ITEM_CANNOT_LEARN] = "{actor} cannot learn from {item}.",
    [MESSAGE_CLASS_CANNOT_LEARN] = "{class}s cannot learn spells.",
    [MESSAGE_CLASS_REQUIRES_DIVINE] = "{class}s can only learn divine spells.",
    [MESSAGE_CLASS_REQUIRES_ARCANE] = "{class}s can only learn arcane spells.",
    [MESSAGE_SPELL_ALREADY_KNOWN] = "{actor} already knows {spell}.",
    [MESSAGE_OBJECT_CANNOT_DESTROY] = "{actor} cannot destroy the {object}.",
    [MESSAGE_OBJECT_DESTROYED] = "{actor} destroys the {object}.",
    [MESSAGE_SHOOT_NO_WEAPON] = "{actor} cannot shoot without a weapon!",
    [MESSAGE_SHOOT_NOT_RANGED] = "{actor} cannot shoot without a ranged weapon!",
    [MESSAGE_SHOOT_NO_AMMUNITION] = "{actor} has no ammunition equipped!",
    [MESSAGE_SHOOT_WRONG_AMMUNITION] = "{actor} has unsuitable ammunition equipped!",
    [MESSAGE_WEAPON_BROKEN] = "{actor}'s {item} breaks!",
    [MESSAGE_SPELL_NOT_KNOWN] = "{actor} does not know {spell}.",
    [MESSAGE_SPELL_LEVEL_TOO_LOW] = "{actor} is not high enough level to cast {spell}.",
    [MESSAGE_SPELL_NOT_ENOUGH_MANA] = "{actor} does not have enough mana.",
    [MESSAGE_SPELL_INTERRUPTED] = "{actor}'s spell is interrupted.",
    [MESSAGE_SPELL_FIZZLED] = "{actor}'s spell fizzles.",
    [MESSAGE_SPELL_CAST] = "{actor} casts {spell}.",
    [MESSAGE_SPELL_RESISTED] = "{target} resists {actor}'s {spell}.",
    [MESSAGE_ACTOR_DAMAGED] = "{actor} damages {target} for {amount}.",
    [MESSAGE_ACTOR_HEALED] = "{actor} heals {target} for {amount}.",
    [MESSAGE_ACTOR_ZAPPED] = "{actor} zaps {target} for {amount}.",
    [MESSAGE_SPELL_CANNOT_CAST_HERE] = "{actor} cannot cast {spell} here.",
    [MESSAGE_ACTOR_DAZED] = "{actor} dazes {target}.",
    [MESSAGE_ACTOR_FROZEN] = "{actor} freezes {target} for {amount}.",
    [MESSAGE_MANA_RECOVERED] = "{actor} recovers {amount} mana.",
    [MESSAGE_ACTOR_DIED] = "{actor} dies.",
    [MESSAGE_ACTOR_ACIDIFIED] = "{actor} acidifies {target} for {amount} damage.",
    [MESSAGE_ACTOR_BURNED] = "{actor} burns {target} for {amount}.",
    [MESSAGE_ACTOR_BOMBARDED] = "{actor} bombards {target} for {amount} damage.",
    [MESSAGE_SURFACE_DAMAGED] = "{source} damages {target} for {amount}",
};

static const char *const attack_messages[2][2][2] = {
    {
        {"{actor} hits {target} for {amount}.",
         "{actor} hits {target} for {amount} (sneak)."},
        {"{actor} hits {target} for {amount} (opportunity).",
         "{actor} hits {target} for {amount} (opportunity) (sneak)."},
    },
    {
        {"{actor} crits {target} for {amount}.",
         "{actor} crits {target} for {amount} (sneak)."},
        {"{actor} crits {target} for {amount} (opportunity).",
         "{actor} crits {target} for {amount} (opportunity) (sneak)."},
    },
};

bool message_from_event(const struct game_event *const event, struct message_data *const data)
{
    if (!event || !data || event->type < 0 || event->type >= NUM_EVENT_TYPES)
    {
        fprintf(stderr, "Cannot translate message: invalid event or output.\n");
        return false;
    }

    static const enum message_type messages[NUM_EVENT_TYPES] = {
        [EVENT_ITEM_DROPPED] = MESSAGE_ITEM_DROPPED,
        [EVENT_ITEM_EQUIPPED] = MESSAGE_ITEM_EQUIPPED,
        [EVENT_ITEM_UNEQUIPPED] = MESSAGE_ITEM_UNEQUIPPED,
        [EVENT_ITEM_CANNOT_EQUIP] = MESSAGE_ITEM_CANNOT_EQUIP,
        [EVENT_ITEM_TOO_LARGE] = MESSAGE_ITEM_TOO_LARGE,
        [EVENT_ITEM_TOO_SMALL] = MESSAGE_ITEM_TOO_SMALL,
        [EVENT_ITEM_NOT_PROFICIENT] = MESSAGE_ITEM_NOT_PROFICIENT,
        [EVENT_EQUIPMENT_SLOT_EMPTY] = MESSAGE_EQUIPMENT_SLOT_EMPTY,
        [EVENT_ATTACK_MISSED] = MESSAGE_ATTACK_MISSED,
        [EVENT_ATTACK_HIT] = MESSAGE_ATTACK_HIT,
        [EVENT_DAMAGE_REDUCED] = MESSAGE_DAMAGE_REDUCED,
        [EVENT_ATTACK_SWUNG] = MESSAGE_ATTACK_SWUNG,
        [EVENT_ABILITY_INCREASED] = MESSAGE_ABILITY_INCREASED,
        [EVENT_SPELL_READIED] = MESSAGE_SPELL_READIED,
        [EVENT_GAME_RESUMED] = MESSAGE_GAME_RESUMED,
        [EVENT_GAME_STARTED] = MESSAGE_GAME_STARTED,
        [EVENT_GAME_OVER] = MESSAGE_GAME_OVER,
        [EVENT_EXPERIENCE_GAINED] = MESSAGE_EXPERIENCE_GAINED,
        [EVENT_LEVEL_GAINED] = MESSAGE_LEVEL_GAINED,
        [EVENT_REST_ENEMIES_NEARBY] = MESSAGE_REST_ENEMIES_NEARBY,
        [EVENT_REST_NO_FOOD] = MESSAGE_REST_NO_FOOD,
        [EVENT_ACTOR_RESTED] = MESSAGE_ACTOR_RESTED,
        [EVENT_TRAP_TRIGGERED] = MESSAGE_TRAP_TRIGGERED,
        [EVENT_TRAP_AVOIDED] = MESSAGE_TRAP_AVOIDED,
        [EVENT_TRAP_DETECTED] = MESSAGE_TRAP_DETECTED,
        [EVENT_TRAP_NOT_DETECTED] = MESSAGE_TRAP_NOT_DETECTED,
        [EVENT_ACTORS_SWAPPED] = MESSAGE_ACTORS_SWAPPED,
        [EVENT_DOOR_CANNOT_OPEN] = MESSAGE_DOOR_CANNOT_OPEN,
        [EVENT_DOOR_OPENED] = MESSAGE_DOOR_OPENED,
        [EVENT_DOOR_CANNOT_CLOSE] = MESSAGE_DOOR_CANNOT_CLOSE,
        [EVENT_DOOR_CLOSED] = MESSAGE_DOOR_CLOSED,
        [EVENT_DUNGEON_END_REACHED] = MESSAGE_DUNGEON_END_REACHED,
        [EVENT_CANNOT_DESCEND] = MESSAGE_CANNOT_DESCEND,
        [EVENT_ACTOR_DESCENDED] = MESSAGE_ACTOR_DESCENDED,
        [EVENT_CANNOT_GO_HIGHER] = MESSAGE_CANNOT_GO_HIGHER,
        [EVENT_CANNOT_ASCEND] = MESSAGE_CANNOT_ASCEND,
        [EVENT_ACTOR_ASCENDED] = MESSAGE_ACTOR_ASCENDED,
        [EVENT_CHEST_CANNOT_OPEN] = MESSAGE_CHEST_CANNOT_OPEN,
        [EVENT_CHEST_OPENED] = MESSAGE_CHEST_OPENED,
        [EVENT_CANNOT_PRAY] = MESSAGE_CANNOT_PRAY,
        [EVENT_ALTAR_PRAYED] = MESSAGE_ALTAR_PRAYED,
        [EVENT_CANNOT_DRINK] = MESSAGE_CANNOT_DRINK,
        [EVENT_FOUNTAIN_DRANK] = MESSAGE_FOUNTAIN_DRANK,
        [EVENT_CANNOT_SIT] = MESSAGE_CANNOT_SIT,
        [EVENT_NOTHING_TO_PICK_UP] = MESSAGE_NOTHING_TO_PICK_UP,
        [EVENT_INVENTORY_FULL] = MESSAGE_INVENTORY_FULL,
        [EVENT_GOLD_PICKED_UP] = MESSAGE_GOLD_PICKED_UP,
        [EVENT_ITEM_PICKED_UP] = MESSAGE_ITEM_PICKED_UP,
        [EVENT_ITEM_CANNOT_QUAFF] = MESSAGE_ITEM_CANNOT_QUAFF,
        [EVENT_ITEM_QUAFFED] = MESSAGE_ITEM_QUAFFED,
        [EVENT_ITEM_CANNOT_READ] = MESSAGE_ITEM_CANNOT_READ,
        [EVENT_ITEM_READ] = MESSAGE_ITEM_READ,
        [EVENT_ITEM_CANNOT_LEARN] = MESSAGE_ITEM_CANNOT_LEARN,
        [EVENT_CLASS_CANNOT_LEARN] = MESSAGE_CLASS_CANNOT_LEARN,
        [EVENT_CLASS_REQUIRES_DIVINE] = MESSAGE_CLASS_REQUIRES_DIVINE,
        [EVENT_CLASS_REQUIRES_ARCANE] = MESSAGE_CLASS_REQUIRES_ARCANE,
        [EVENT_SPELL_ALREADY_KNOWN] = MESSAGE_SPELL_ALREADY_KNOWN,
        [EVENT_OBJECT_CANNOT_DESTROY] = MESSAGE_OBJECT_CANNOT_DESTROY,
        [EVENT_OBJECT_DESTROYED] = MESSAGE_OBJECT_DESTROYED,
        [EVENT_SHOOT_NO_WEAPON] = MESSAGE_SHOOT_NO_WEAPON,
        [EVENT_SHOOT_NOT_RANGED] = MESSAGE_SHOOT_NOT_RANGED,
        [EVENT_SHOOT_NO_AMMUNITION] = MESSAGE_SHOOT_NO_AMMUNITION,
        [EVENT_SHOOT_WRONG_AMMUNITION] = MESSAGE_SHOOT_WRONG_AMMUNITION,
        [EVENT_WEAPON_BROKEN] = MESSAGE_WEAPON_BROKEN,
        [EVENT_SPELL_NOT_KNOWN] = MESSAGE_SPELL_NOT_KNOWN,
        [EVENT_SPELL_LEVEL_TOO_LOW] = MESSAGE_SPELL_LEVEL_TOO_LOW,
        [EVENT_SPELL_NOT_ENOUGH_MANA] = MESSAGE_SPELL_NOT_ENOUGH_MANA,
        [EVENT_SPELL_INTERRUPTED] = MESSAGE_SPELL_INTERRUPTED,
        [EVENT_SPELL_FIZZLED] = MESSAGE_SPELL_FIZZLED,
        [EVENT_SPELL_CAST] = MESSAGE_SPELL_CAST,
        [EVENT_SPELL_RESISTED] = MESSAGE_SPELL_RESISTED,
        [EVENT_ACTOR_DAMAGED] = MESSAGE_ACTOR_DAMAGED,
        [EVENT_ACTOR_HEALED] = MESSAGE_ACTOR_HEALED,
        [EVENT_ACTOR_ZAPPED] = MESSAGE_ACTOR_ZAPPED,
        [EVENT_SPELL_CANNOT_CAST_HERE] = MESSAGE_SPELL_CANNOT_CAST_HERE,
        [EVENT_ACTOR_DAZED] = MESSAGE_ACTOR_DAZED,
        [EVENT_ACTOR_FROZEN] = MESSAGE_ACTOR_FROZEN,
        [EVENT_MANA_RECOVERED] = MESSAGE_MANA_RECOVERED,
        [EVENT_ACTOR_DIED] = MESSAGE_ACTOR_DIED,
        [EVENT_ACTOR_ACIDIFIED] = MESSAGE_ACTOR_ACIDIFIED,
        [EVENT_ACTOR_BURNED] = MESSAGE_ACTOR_BURNED,
        [EVENT_ACTOR_BOMBARDED] = MESSAGE_ACTOR_BOMBARDED,
        [EVENT_SURFACE_DAMAGED] = MESSAGE_SURFACE_DAMAGED,
    };

    *data = (struct message_data){
        .type = messages[event->type],
        .actor = event->actor,
        .target = event->target,
        .item = event->item,
        .slot = event->slot,
        .spell = event->spell,
        .class_name = event->class_name,
        .object = event->object,
        .ability = event->ability,
        .source = event->source,
        .amount = event->amount,
        .critical = event->critical,
        .opportunity = event->opportunity,
        .sneak = event->sneak,
    };

    return data->type != MESSAGE_NONE;
}

char *message_format(const struct message_data *const data)
{
    if (!data || data->type <= MESSAGE_NONE || data->type >= NUM_MESSAGE_TYPES || !message_catalog[data->type])
    {
        fprintf(stderr, "Cannot format message: invalid message type.\n");
        return NULL;
    }

    const char *const template = data->type == MESSAGE_ATTACK_HIT ? attack_messages[data->critical][data->opportunity][data->sneak] : message_catalog[data->type];
    char amount[32];
    snprintf(amount, sizeof(amount), "%d", data->amount);
    const struct
    {
        const char *token;
        const char *value;
    } arguments[] = {
        {"{actor}", data->actor},
        {"{target}", data->target},
        {"{item}", data->item},
        {"{slot}", data->slot},
        {"{spell}", data->spell},
        {"{class}", data->class_name},
        {"{object}", data->object},
        {"{ability}", data->ability},
        {"{action}", data->action},
        {"{source}", data->source},
        {"{amount}", amount},
    };

    char *text = NULL;
    size_t size = 0;
    for (int pass = 0; pass < 2; pass++)
    {
        const char *cursor = template;
        size_t offset = 0;
        while (*cursor)
        {
            const char *value = cursor;
            size_t length = 1;
            size_t consumed = 1;
            if (*cursor == '{')
            {
                value = NULL;
                for (size_t i = 0; i < sizeof(arguments) / sizeof(arguments[0]); i++)
                {
                    const size_t token_length = strlen(arguments[i].token);
                    if (strncmp(cursor, arguments[i].token, token_length) == 0)
                    {
                        value = arguments[i].value;
                        consumed = token_length;
                        break;
                    }
                }
                if (!value)
                {
                    fprintf(stderr, "Cannot format message %d: missing or unknown argument at '%s'.\n",
                            data->type, cursor);
                    free(text);
                    return NULL;
                }
                length = strlen(value);
            }
            if (length >= SIZE_MAX - offset)
            {
                fprintf(stderr, "Cannot format message %d: text is too long.\n", data->type);
                free(text);
                return NULL;
            }
            if (text)
            {
                memcpy(text + offset, value, length);
            }
            offset += length;
            cursor += consumed;
        }
        if (pass == 0)
        {
            size = offset;
            text = malloc(size + 1);
            if (!text)
            {
                fprintf(stderr, "Cannot allocate text for message %d.\n", data->type);
                return NULL;
            }
        }
    }
    text[size] = '\0';
    return text;
}
