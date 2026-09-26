# mod-gather-track

AzerothCore module that combines two features for gathering professions:

1. **Dual Tracking** — allows `Find Herbs` and `Find Minerals` to reveal
   both resource types at once, provided the player has the corresponding
   profession.
2. **Gathering Speed** — overrides the cast time of Herbalism, Mining, and
   Skinning spells without editing client DBC files.

## Requirements

- AzerothCore WotLK 3.3.5a (`mod-playerbots/azerothcore-wotlk` fork).
- Windows / Linux.

## Installation

1. Clone this module into `modules/mod-gather-track`.
2. Re-run CMake.
3. Build `worldserver`.
4. Copy `conf/mod-gather-track.conf.dist` to
   `configs/modules/mod-gather-track.conf` and edit as needed.
5. Restart `worldserver`.

## Configuration

See `conf/mod-gather-track.conf.dist` for full descriptions and a
reference table of `SpellCastTimes` indexes.

## Credits

- AzerothCore module framework.
- Original dual-tracking logic: [b-wun/mod-dual-tracking](https://github.com/b-wun/mod-dual-tracking).

## License

This module is released under the GNU AGPL v3 license, consistent with
the base AzerothCore project.
