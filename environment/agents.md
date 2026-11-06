## Modifications instructions

Content in evs-hwfw-environment is somehow "sensitive" in the sense that it's shared amongst many projects.

So when used as a submodule of another project, agents should not by default modify things here especially when it comes to adding more tools to the Dockerfiles. It should be a last resort proposal as modifying the environment will impact all others projects.

On the other hands, inside the project, there might be a dockers/Dockerfile.X to add project related tools or dockers/Dockerfile.dev.X to add user personal tools (i.e. not directly needed by every user of the project).

This repository should be kept as lightweight as possible.