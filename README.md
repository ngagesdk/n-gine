# N-RPG

A simple game engine in the style of the 16-bit JRPGs of the 1990s.

## Documentation

### General map design rules

- If a transition is to be created from one map to another, these two
  maps must have the same edge length on the respective side.

- A tileset image is an ordinary `bmp` file with the RGB subtype
  `RGB565`.  Transparency for maps consisting of several layers can be
  achieved via the key colour `#ff00ff`.

- Maps must be saved uncompressed (Tile Layer Format CSV) and in JSON
  format with the tileset embedded.  The file name of the map that is
  loaded first is `1.tmj`.

- All resources must be combined into a packed file system with the file
  name `data.pfs`.  To do this, all assets used must be in the same
  directory as the maps.

### Properties

Tbd.

## Licence and Credits

- Packed file loader by [Daniel
  Monteiro](https://montyontherun.itch.io/).

- CyberPop - Interior Tiles by
  [MalibuDarby](https://malibudarby.itch.io/cyberpop-interior-tiles).

- This project is licensed under the "The MIT License".  See the file
  [LICENSE.md](LICENSE.md) for details.

- The [N-Gage SDK logo](media/) by [Dan Whelan](https://danwhelan.ie) is
  licensed under a Creative Commons [Attribution-ShareAlike 4.0
  International (CC BY-SA
  4.0)](https://creativecommons.org/licenses/by-sa/4.0/) license.
