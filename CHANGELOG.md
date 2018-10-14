#### 3.5

Features:

* Implements flood fill in Terrain Brush.
* Update wall brushes for 10.98
* Added Show As Minimap menu.
* Make spawns visible when placing a new spawn.

Fixed bugs:

* Fix container item crash.

#### 3.4

Features:

* New Find Item / Jump to Item dialog.
* Configurable copy position format.
* Add text ellipsis for tooltips.
* Show hook indicators for walls.
* Updated data for 10.98

Fixed bugs:

* Icon background colour; white and gray no longer work.
* Only show colors option breaks RME.

#### 3.3

Features:

* Support for tooltips in the map.
* Support for animations preview.
* Restore last position when opening a map.
* Export search result to a .txt file.
* Waypoint brush improvements.
* Better fullscreen support on macOS.

Fixed bugs:

* Items larger than 64x64 are now displayed properly.
* Fixed potential crash when using waypoint brush.
* Fixed a bug where you could not open map files by clicking them while the editor is running.
* You can now open the extensions folder on macOS.
* Fixed a bug where an item search would not display any result on macOS.
* Fixed multiple issues related to editing houses on macOS.

#### 3.2

Features:

* Export minimap by selected area.
* Search for unique id, action id, containers or writable items on selected area.
* Go to Previous Position menu. Keyboard shortcut 'P'.
* Data files for version 10.98.
* Select Raw button on the Browse Field window.

Fixed bugs:

* Text is hidden after selecting an item from the palette. Issue #144
* Search result does not sort ids. Issue #126
* Monster direction is not saved. Issue #132

#### 3.1

Features:

* In-game box improvements. Now the hidden tiles, visible tiles and player position are displayed.
* New _Zoom In_, _Zoom Out_ and _Zoom Normal_ menus.
* New keyboard shortcuts:
	- **Ctrl+Alt+Click** Select the relative brush of an item.
	- **Ctrl++** Zoom In
	- **Ctrl+-** Zoom Out
	- **Ctrl+0** Zoom Normal(100%)
* If zoom is 100%, move one tile at a time.

Fixed bugs:

* Some keyboard shortcuts not working on Linux.
* Main menu is not updated when the last map tab is closed.
* In-game box wrong height.
* UI tweaks for Import Map window.
