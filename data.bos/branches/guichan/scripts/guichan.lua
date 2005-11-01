--            ____
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  )
--        /_____/\____/____/
--
--      Invasion - Battle of Survival
--       A GPL'd futuristic RTS game
--
--      guichan.lua - The main UI lua script.
--
--      (c) Copyright 2005 by Fran�ois Beerten
--
--      This program is free software; you can redistribute it and/or modify
--      it under the terms of the GNU General Public License as published by
--      the Free Software Foundation; only version 2 of the License.
--
--      This program is distributed in the hope that it will be useful,
--      but WITHOUT ANY WARRANTY; without even the implied warranty of
--      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--      GNU General Public License for more details.
--
--      You should have received a copy of the GNU General Public License
--      along with this program; if not, write to the Free Software
--      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
--      02111-1307, USA.
--
--      $Id$

-- Global usefull objects for menus  ----------
dark = Color(8, 8, 38, 130)
clear = Color(200, 200, 120)
black = Color(0, 0, 0)

bckground = CGraphic:New("graphics/screens/menu.png")
bckground:Load()
bckground:Resize(Video.Width, Video.Height)
backgroundWidget = ImageWidget(bckground)

normalImage = CGraphic:New("graphics/button.png", 200, 24)
pressedImage = CGraphic:New("graphics/pressed.png", 200, 24)
normalImage:Load() -- FIXME remove when immediatly loaded
pressedImage:Load() -- idem

local guichanadd = Container.add
Container.add = function(self, widget, x, y)
  -- ugly hack, should be done in some kind of constructor
  if not self._addedWidgets then
     self._addedWidgets = {}
  end
  self._addedWidgets[widget] = true
  guichanadd(self, widget, x, y)
end

function BosMenu()
   local menu
   local exitButton

   menu = MenuScreen()

   menu:add(backgroundWidget, 0, 0)

   exitButton = ImageButton("Exit", normalImage, pressedImage)
   exitButton:setActionCallback(function() menu:stop() end)
   menu:add(exitButton, Video.Width / 2 - 100, Video.Height - 100)

   return menu
end

-- Default configurations -------
Widget:setGlobalFont(CFont:Get("large"))


-- Define the different menus ----------

function RunSubMenu(s)
  local menu
  menu = BosMenu()
  menu:run()
end

function RunStartGameMenu(s)
  local menu
  menu = BosMenu()

  local mapslist
  mapslist = {}
  local u
  u = 1
  local fileslist = ListDirectory("maps/")
  for i,f in fileslist do
    if(string.find(f, "^C.*%.smp$")) then
      print("Added smp file:" .. f .. "--" )
      mapslist[u] = f
      u = u + 1
    end
  end
  print(mapslist)
  print(mapslist[1])

  local bq
  bq = ListBoxWidget()
  bq:setList(mapslist)
  bq:setBaseColor(black)
  bq:setForegroundColor(clear)
  bq:setBackgroundColor(dark)
  bq:setFont(CFont:Get("game"))
  menu:add(bq, 300, 100)

  function startgamebutton(s)
    print("Starting map -------")
    StartMap("maps/" .. mapslist[bq:getSelected() + 1])
  end

  local bb
  bb = ImageButton("Start", normalImage, pressedImage)
  bb:setActionCallback(startgamebutton)
  menu:add(bb, 100, 300)

  menu:run()
end

function RunWidgetsMenu(s)
  local menu
  local b
  menu = BosMenu()

  b = Label("a label, sir.")
  b:setFont(CFont:Get("game"))
  menu:add(b, 20, 10)

  b = RadioButton("Platoon", "dumgroup", true)
  b:setActionCallback(function() print("one") end)
  b:setBaseColor(dark)
  b:setForegroundColor(clear)
  b:setBackgroundColor(dark)
  menu:add(b, 20, 50)
  b = RadioButton("Army", "dumgroup")
  b:setActionCallback(function() print("two") end)
  b:setBaseColor(dark)
  b:setForegroundColor(clear)
  b:setBackgroundColor(dark)
  menu:add(b, 150, 50)

  b = TextField("text widget")
  b:setActionCallback(function() print("field") end)
  b:setFont(CFont:Get("game"))
  b:setBaseColor(clear)
  b:setForegroundColor(clear)
  b:setBackgroundColor(dark)
  menu:add(b, 20, 100)

  b = Slider(0, 1)
  b:setActionCallback(function() print("slider") end)
  menu:add(b, 20, 150)
  b:setWidth(60)
  b:setHeight(20)
  b:setBaseColor(dark)
  b:setForegroundColor(clear)
  b:setBackgroundColor(clear)

  ik = CGraphic:New("units/assault/ico_assault.png")
  ik:Load()
  b = ImageWidget(ik)
  menu:add(b, 20, 250)

  b = DropDownWidget()
  b:setFont(CFont:Get("game"))
  b:setList({"line1", "line2"})
  b:setActionCallback(function(s) print("dropdown ".. b:getSelected()) end)
  b:setBaseColor(dark)
  b:setForegroundColor(clear)
  b:setBackgroundColor(dark)
  menu:add(b, 20, 350)

  win = Windows("Test", 200, 200)
  win:setBaseColor(dark)
  win:setForegroundColor(dark)
  win:setBackgroundColor(dark)
  menu:add(win, 450, 40)
  win2 = Windows("", 50, 50)
  win:add(win2, 0, 0)

  menu:run()
end

function RunMainMenu(s)
  menu = BosMenu() 

  sb = ImageButton("SubMenu", normalImage, pressedImage)
  sb:setActionCallback(RunSubMenu)
  menu:add(sb, 300, 250)

  wb = ImageButton("WidgetsMenu", normalImage, pressedImage)
  wb:setActionCallback(RunWidgetsMenu)
  menu:add(wb, 300, 200)

  sg = ImageButton("Start Game", normalImage, pressedImage)
  sg:setActionCallback(RunStartGameMenu)
  menu:add(sg, 300, 150)

  menu:run()
end


RunMainMenu()



