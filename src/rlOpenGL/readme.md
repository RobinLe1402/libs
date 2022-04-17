# rlOpenGL
A library to be used as a basic framework for OpenGL-based applications.

## Usage
To use the rlOpenGL framework, follow these steps:

 1. Import the code
* add `rlOpenGL.lib` to the project
* `#include "rl/lib/rlOpenGL/Core.hpp"`
* (optional) `#include "rl/lib/rlOpenGL/Texture.hpp"`

 2. Use the framework
 
 __Class implementation__
* Create a frame graph type (possibly a struct)
* Implement the `rl::OpenGL::IApplication` interface
* Implement the `rl::OpenGL::IRenderer` interface
* [Optional] Create a class derived of `rl::OpenGL::Window`

 __Create objects, run__
* Create an object of your `IRenderer` and the `Window` class
* Create an object of your `IApplication` implementation with the other objects as constructor parameters
* Create an object of type `rl::OpenGL::AppConfig`, customize the settings
* Call `IApplication.execute` Using this `AppConfig` object





## Internal workings
The main idea was to seperate the logic from the graphics and the window, while still working together as well as possible.

The `IApplication` and `IRenderer` classes run on the same thread (currently, might change in the future) but are not supposed to communicate directly. Instead, `const void*` is used to pass the information to render from the `IApplication` to the `IRenderer`.

The `Window` class runs in it's own thread. But when receiving one of the following Windows messages, the `Window` thread asks the `IApplication` thread to handle the message before continuing:
* `WM_SIZE` (the window has been resized and now has the final new size)
* `WM_SIZING` (the window is currently getting resized)
* `WM_SETFOCUS` (the window got the keyboard focus)
* `WM_KILLFOCUS` (the window lost the keyboard focus)


| Message | Meaning |
|---------|---------|
|`WM_SIZE`|the window has been resized and now has the final new size|
|`WM_SIZING`|the window is currently getting resized|
|`WM_SETFOCUS`|the window got the keyboard focus|
|`WM_KILLFOCUS`|the window lost the keyboard focus|

When the `IApplication` thread receives one of these messages, it calls these methods of `IApplication`:
* `OnResize`
* `OnGainFocus`
* `OnLoseFocus`
* `cacheGraph`

This means that whenever one of these methods is called, no function that synchronously interacts with the window (like `ShowWindow`, `SendMessage` etc.) can be called - otherwise, the application locks up.