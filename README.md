# gmsh-reader
C++ application for reading a GMSH mesh file

## Build
This is pretty easy - install a C++ compiler and run:

    make

## Run
Again, quite nice:

    ./read_MSH_file.bin mesh_file.msh
    ./read_MSH_file.bin tutorial4Backup.msh

This code will only read 4.1 ASCII version `.msh` files, these are written by recent versions of gmsh.

## Credits
Most code is by Mohammad Haji <mhajit@gmail.com>. Snippits by Mark Stock <markjstock@gmail.com> and https://stackoverflow.com/users/763305 .
