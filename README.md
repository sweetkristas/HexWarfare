HexWarfare
=====


Warfare game based on hexes

Some images are from the Battle for Wesnoth project and are licensed under GPLv2.
https://github.com/wesnoth/wesnoth


Building Instructions

You need to have the dependencies already built and installed. At this time we are dependent on SDL2 (base+image+mixer+ttf), google protobufs, enet, boost. Boost is mostly required since certain C++11 things like regex are broken in some compilers still.

If using ubuntu (or derivative) these can be downloaded with the command:
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev libprotobuf-dev libenet-dev libboost-1.55-all-dev
This assumes you have a development environment set up already.

Clone the repository with git clone https://github.com/sweetkristas/HexWarfare
Update the submodules.
The first time: git submodule init && git submodule update
Change into the HexWarfare directory and type make
