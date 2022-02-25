# Design

Language & Framework

- C/C++ and ImGUI

Links:
- https://github.com/mnesarco/bawr (fonts, icons, svg)

General Idea (follow ZSA Oryx):

- Create a new layout object that can handle keyboard layouts (data driven)
- Create a new widget that can render a keyboard key (1u, 2u) similar to what ZSA Oryx does
- Have the App consist of two windows, one with a container that can display the keyboard layers and
  the other with properties of the layer or the key that is selected.

Currently these are not straightforward:

- Rotate a key (got a prototype)

What are the properties of a key?

- The index of the key in the layer
- Enabled/Disabled, Hidden/Visible
- The keycode when tapped
- The keycode when held / double tapped
- The textual and icon representation information of the key that is displayed in the key widget
- The LED glow color of the key
- The color of the key (just for visual purposes and rendering the SVG)
- The size of the key (1u, 2u)

What are the properties of a layer?

- The keys in the layer
- The size of the layer
- The color of the layer (just for visual purposes and perhaps RGB layer lighting)

What are the properties of the keyboard?

- The layers of the keyboard
- The name of the keyboard

Main Window:

- The keyboard layers in the form of TabPanels (add/remove layers)
- The container displaying the keyboard layer

Keyboard/Layer/Key Property Window:

- The properties of the keyboard (rules.mk and config.h?)
- The properties of the active layer
- The properties of the selected key

