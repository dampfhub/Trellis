# Trellis

## Dependencies
* glfw3
* glad
* glm
* freetype
* asio
* nlohmann_json
* sqlite3

## Quickstart Contribution Guide
* Clone repo
* Get vcpkg
  * https://github.com/microsoft/vcpkg
* Install vcpkg in the same directory as the cloned repository (follow instructions in repo)
* Install all dependencies using vcpkg:
```
vcpkg install glfw3:x64-windows-static
vcpkg install glad:x64-windows-static
vcpkg install glm:x64-windows-static
vcpkg install freetype:x64-windows-static
vcpkg install asio:x64-windows-static
vcpkg install nlohmann_json:x64-windows-static
vcpkg install sqlite3:x64-windows-static
```
