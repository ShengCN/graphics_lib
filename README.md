# General OpenGL Rendering Libs for R & D 
## Env
```Bash
# glfw in unix
sudo apt install xorg-dev
sudo apt-get install -y libboost-all-dev 
```
## Minimal tutorial 

```c++
// main.cpp
#include <graphics_lib.h>
#include <otb_window.h>

int main() {
    otb_window wnd;
    wnd.create_window();

    wnd.show();
    return 0;
}
```


``` c++

std::string default_config = "Configs/default.json";
std::shared_ptr<render_engine> otb_window::m_engine = std::make_shared<render_engine>(default_config);

// After initialize opengl
m_engine->init_ogl_states();

```

