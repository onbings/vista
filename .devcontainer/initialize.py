#!/usr/bin/env python3
from __future__ import annotations

import pathlib
import sys

SCRIPT_DIR = pathlib.Path(__file__).parent.resolve()

sys.path.insert(1, str(SCRIPT_DIR.parent / "environment"))
import env_launch

def write_dev_docker_compose_if_needed(path: pathlib.Path):
    """
    Write the dev docker-compose file if it does not already exist.

    Arguments
        path : The path to the dev docker-compose file.
    """
    if not path.exists():
        with open(path, "w") as f:
            f.write("services:\n")
            f.write("  dev:\n")
            f.write("    # Extra/overridden bind mounts (see docker-compose.yml for reference)\n")
            f.write("    volumes: []\n")
            f.write("    # Extra/overridden environment vars (see docker-compose.yml for reference)\n")
            f.write("    environment: []\n")

def write_dev_docker_file_if_needed(path: pathlib.Path):
    """
    Write the dev Dockerfile if it does not already exist.

    Arguments
        path : The path to the dev Dockerfile.
    """
    if not path.exists():
        with open(path, "w") as f:
            f.write("ARG BASE_IMAGE\n")
            f.write("FROM ${BASE_IMAGE} AS image_devel\n")
            f.write("\n")
            f.write("# Add your custom things here namely to debug things or customize your environment\n")
            f.write("# Pay attention that any modification made here will be yours only. No CI or others\n")
            f.write("# developper will never see them. So if your build rely on some feature placed here\n")
            f.write("# they are misplaced and should belong to the non .dev. Dockerfile.\n")
            f.write("\n")

def extract_name(argv: list[str]) -> str:
    """
    Extract the value of the -n/--name argument forwarded to env_launch.py.

    Arguments
        argv : The arguments forwarded to env_launch.py

    Returns
        str : The environment name
    """
    for flag in ("-n", "--name"):
        if flag in argv:
            return argv[argv.index(flag) + 1]

    raise ValueError("Missing required -n/--name argument")

def main(argv: list[str]):
    """
    Entry point of the script.

    Arguments
        argv : The arguments forwarded to env_launch.py

    Returns
        int : 0 if the operation was successful, non-zero otherwise.
    """
    name           = extract_name(argv)
    docker_dir     = SCRIPT_DIR / ".." / "dockers"
    dev_dockerfile = docker_dir / f"Dockerfile.dev.{name}"

    # Make sure docker directory exists
    docker_dir.mkdir(parents=True, exist_ok=True)

    write_dev_docker_compose_if_needed(SCRIPT_DIR / "docker-compose.dev.yml")
    write_dev_docker_file_if_needed   (dev_dockerfile)

    return env_launch.main(argv)

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
