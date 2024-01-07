#pragma once

#include "map.h"

typedef struct tagRESOURCES {
    BYTE bLoadFlags;

    LPPALMAP lpMap; // current loaded map
    LPSPRITE* lppEventObjectSprites; // event object sprites
    int nEventObject; // number of event objects

    LPSPRITE rglpPlayerSprite[MAX_PLAYABLE_PLAYER_ROLES]; // player sprites
} RESOURCES, *LPRESOURCES;

/**
 * @brief the loader for game
 *
 */
class PalLoader {
public:
    PalLoader() = default;
    ~PalLoader() = default;

    bool load();

private:
    /**
     * @brief Initialze the resource manager.
     * 
     */
    void initResources();

    /**
     * @brief Set flags to load resources.
     * 
     * @param flag 
     */
    void setLoadFlags(BYTE bFlags);

    /**
     * @brief Load the game resources if needed.
     * 
     */
    void loadResources();

    /**
     * @brief Free all sprites of event objects on the scene.
     *
     */
    void freeEventObjectSprites();
    /**
     * @brief Free all player sprites.
     *
     */
    void freePlayerSprites();
    LPRESOURCES gpResources = nullptr;
};