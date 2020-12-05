# vulkan_st
Vulkan studying repo

# Build

## Common

I decided to include binary assets `assets/` into the repository as well although it's not good
practice to include such things into git. But I will keep it minimally, and to get benefit of convinence
in quickly getting start and building demos without a need to externally download from elsewhere.

## Windows

Download and install vulkansdk from [vulkan.lunarg.com](https://vulkan.lunarg.com/) to your machine.
Make sure to add `PATH` of its `Bin/` to your environment variable.

Execute `win.bat` located at each sub-project directory.

## Linux (Debian/Ubuntu)

Install vulkan onto your machine using this [guide](https://linuxconfig.org/install-and-test-vulkan-on-linux)
as it quite depends on which graphics driver, and vendors you have and want to install from.

Install `libglwf3-dev` by executing `sudo apt install libglwf3-dev`.
