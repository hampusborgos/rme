## <a name='gui'>GUI</a>
[g_gui.canRedo()](#gui)  
[g_gui.canUndo()](#gui)  
[g_gui.doRedo()](#gui)  
[g_gui.doUndo()](#gui)  
[g_gui.getCenterPosition()](#gui)  
[g_gui.getCurrentEditor()](#gui)  
[g_gui.getCurrentFloor()](#gui)  
[g_gui.getDataDirectory()](#gui)  
[g_gui.getEditorAt(index)](#gui)  
[g_gui.getExecDirectory()](#gui)  
[g_gui.getExtensionsDirectory()](#gui)  
[g_gui.getLocalDataDirectory()](#gui)  
[g_gui.getLocalDirectory()](#gui)  
[g_gui.loadMap(path)](#gui)    
[g_gui.newMap()](#gui)  
[g_gui.saveCurrentMap()](#gui)  
[g_gui.setCenterPosition(position)](#gui)  
[g_gui.setCurrentEditor(editor)](#gui)  
[g_gui.setCurrentEditor(index)](#gui)  
[g_gui.setCurrentFloor(z)](#gui)  
[g_gui.setDrawingMode()](#gui)  
[g_gui.setSelectionMode()](#gui)  
[g_gui.setStatusText(text)](#gui)  
[g_gui.shouldSaveCurrentMap()](#gui)  
[g_gui.showTextBox(title, text)]()](#gui)  

## <a name='editor'>Editor</a>
[Editor(index)](#editor)  
[editor:createSelection(start, end)](#editor)  
[editor:getItemCount(itemid)](#editor_getItemCount)  
[editor:getMapDescription()](#editor)  
[editor:getMapHeight()](#editor)  
[editor:getMapWidth()](#editor)  
[editor:getSelection()](#editor)  
[editor:getTile(position)](#editor)  
[editor:getTile(x, y, z)](#editor)  
[editor:replaceItems(items)](#editor_replaceItems)  
[editor:selectTiles(tiles)](#editor)  
[editor:setMapDescription(newDescription)](#editor)  
[editor:setMapHeight(newHeight)](#editor)  
[editor:setMapWidth(newWidth)](#editor)  

## <a name='tile'>Tile</a>
[Tile(position)](#tile)  
[Tile(x, y, z)](#tile)  
[tile:getPosition()](#tile)  
[tile:isBlocking()](#tile)  
[tile:isHouse()](#tile)  
[tile:isHouseExit()](#tile)  
[tile:isModified()](#tile)  
[tile:isNoLogout()](#tile)  
[tile:isNoPvP()](#tile)  
[tile:isPZ()](#tile)  
[tile:isPvP()](#tile)  
[tile:isSelected()](#tile)  
[tile:setNoLogout(enable)](#tile)  
[tile:setNoPvP(enable)](#tile)  
[tile:setPZ(enable)](#tile)  
[tile:setPvP(enable)](#tile)  

## <a name='selection'>Selection</a>
[Selection(editor, start, end)](#selection)  
[selection:borderize()](#selection)  
[selection:destroy()](#selection)  
[selection:getMaxPosition()](#selection)  
[selection:getMinPosition()](#selection)  
[selection:getTileCount()](#selection)  
[selection:getTiles()](#selection)  
[selection:move(position)](#selection)  
[selection:offset(x[, y = 0[, z = 0]])](#selection)  
[selection:randomize()](#selection)  
[selection:replaceItems(items)](#selection_replaceItems)  
[selection:saveAsMinimap(directory, fileName)](#selection)  

## <a name='house'>House</a>
[House(editor, houseId)](#house)  
[house:getId()](#house_getId)  
[house:getName()](#house_getName)  
[house:getTownId()](#house_getTownId)  
[house:getSize()](#house_getSize)  

## <a name='editor_getItemCount'>editor:getItemCount(itemid)</a>
#### Example:
```Lua
local editor = Editor(0) -- gets the editor at first tab.
if editor then
  local items = {546, 321, 875} -- list of items to find.
  for _, id in pairs(items) do
    print('found ' .. editor:getItemCount(id) .. ' items id ' .. tostring(id))
  end
end
```

## <a name='editor_replaceItems'>editor:replaceItems(items)</a>
#### Example:
```Lua
-- from id / to id
local items = {
  [103] = 104,
  [493] = 100
}

local editor = Editor(0) -- gets the editor at first tab.
if editor then
  editor:replaceItems(items)
end
```

## <a name='selection_replaceItems'>selection:replaceItems(items)</a>
#### Example:
```Lua
-- from id / to id
local items = {
  [103] = 104,
  [493] = 100
}

local editor = Editor(0) -- gets the editor at first tab.
if editor then
  local selection = editor:getSelection() -- gets the current selected area.
  if selection then
    selection:replaceItems(items)
  end
end
```

## <a name='house_getId'>house:getId()</a>
#### Example:
```Lua
local houseId = 1

local editor = Editor(0) -- gets the editor at first tab.
if editor then
  local house = House(editor, houseId)
  if house then
    print(house:getId())
  end
end
```

## <a name='house_getName'>house:getName()</a>
#### Example:
```Lua
local houseId = 1

local editor = Editor(0) -- gets the editor at first tab.
if editor then
  local house = House(editor, houseId)
  if house then
    print(house:getName())
  end
end
```

## <a name='house_getTownId'>house:getTownId()</a>
#### Example:
```Lua
local houseId = 1

local editor = Editor(0) -- gets the editor at first tab.
if editor then
  local house = House(editor, houseId)
  if house then
    print(house:getTownId())
  end
end
```

## <a name='house_getSize'>house:getSize()</a>
#### Example:
```Lua
local houseId = 1

local editor = Editor(0) -- gets the editor at first tab.
if editor then
  local house = House(editor, houseId)
  if house then
    print(house:getSize())
  end
end
```
