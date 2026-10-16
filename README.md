## hyprshutdown

a graceful shutdown/logout utility for hyprland, personalized for the caelestia desktop environment.

![](./assets/preview.png)

## usage

run `hyprshutdown` to close running applications gracefully and shut down hyprland.

see `hyprshutdown -h` for options.

## changes

- dynamic caelestia material 3 theme loader reading active scheme colors from `scheme.json` (with fallbacks to `current.conf` / `current.lua`) and appearance tokens from `shell.json`.
- redesigned shutdown modal with centered `surfacecontainer` card (540x480, 28px rounding, 1px `outlinevariant` border).
- translucent fullscreen scrim (`background` color with shell `transparency.base` opacity).
- header row with shutdown title and dynamic live active application count pill badge (`secondarycontainer`).
- scrollable app list container (`surfacecontainerlow`, 16px rounding) with per-app material 3 surface cards (`surfacecontainerhigh`) displaying application class and title.
- pango markup escaping preventing text formatting parse failures on window titles.
- tonal material 3 action buttons: cancel (`secondarycontainer`) and force quit (`errorcontainer`).
- keyboard shortcut: escape key triggers graceful cancellation.

## notes

`hyprshutdown` shuts down hyprland sessions and does not power off hardware directly.
