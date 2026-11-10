# evs-hwfw-environment

## Context

This repository contains the environment related files needed by every project in the Aquila ecosystem.

This includes amongst others
- Docker files to build development images
- Toolchain files for cross-compilation
- VCPKG triplets
- ...

It's meant to be used as a submodule in the 'environment' folder of the project.

### Modifications instructions

Content in evs-hwfw-environment is somehow "sensitive" in the sense that it's shared amongst many projects.

So when used as a submodule of another project, agents should not by default modify things here especially when it comes to adding more tools to the Dockerfiles. It should be a last resort proposal only if brings value to everyone as modifying the environment will impact all others projects.

On the other hands, inside the project, there might be a 
 - dockers/Dockerfile.X to add project related tools that are needed to compile
 - dockers/Dockerfile.dev.X to add user personal tools (i.e. not directly needed by every user of the project).

This repository should be kept as lightweight as possible.

## Platforms

Each project uses CMake and there should be a CMakePresets.json at the root of the project that should list available configurations.
It's conventional to use <project_root>/out/build/<preset> as the build directory
