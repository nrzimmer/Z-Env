# envwalk

envwalk is a simple environment manager for your shell. It automatically loads `.env` files when you enter an allowed directory and unloads them when you leave.

## Features

- Automatically loads `.env` files from the current directory and its parents
- Unsets variables when you leave a directory
- Per-directory allowlist — only directories you explicitly allow are loaded
- `~` in values is expanded to `$HOME`
- Supports zsh and bash

## Installation

**Build from source:**

```sh
make
sudo cp envwalk /usr/local/bin/envwalk
```

**Arch Linux:**

```sh
make arch-install
```

**Ubuntu/Debian:**

```sh
make ubuntu
sudo dpkg -i ../envwalk_*.deb
```

## Shell setup

Add to `~/.zshrc`:

```sh
eval "$(envwalk hook zsh)"
```

Add to `~/.bashrc`:

```sh
eval "$(envwalk hook bash)"
```

## Usage

| Command                | Description                                    |
| ---------------------- | ---------------------------------------------- |
| `envwalk allow [path]` | Allow a directory to auto-load its `.env` file |
| `envwalk deny [path]`  | Remove a directory from the allowed list       |
| `envwalk list`         | Show all allowed directories                   |
| `envwalk hook <shell>` | Print the shell hook (`zsh` or `bash`)         |
| `envwalk help`         | Show help                                      |

`allow` and `deny` default to the current directory when no path is given.

## .env file format

```sh
KEY=value
QUOTED="hello world"
PATH_VAR=~/projects    # ~ is expanded to $HOME
# lines starting with # or // are ignored
# duplicate keys are ignored (first wins)
```

## How it works

On every command, envwalk traverses from the current directory up to the root, loading `.env` files from all allowed directories it finds. When you change directory, variables that belonged to directories you've left are automatically unset.

## Development

```sh
make               # debug build
make release       # optimized build
make test          # run tests (131 tests across dotenv, path, config, cli, hooks)
make clean         # remove build artifacts
make arch          # build Arch Linux package
make arch-install  # build and install Arch Linux package
make ubuntu        # build Ubuntu/Debian package
make package       # build both packages
```

### Releasing

Use the release script to bump the version. It updates `packaging/arch/PKGBUILD` (version and SHA256 checksums), `packaging/arch/CHANGELOG`, and `packaging/ubuntu/changelog`, then commits, tags, pushes, and creates a GitHub release.

> **Note:** the SHA256 checksums in `PKGBUILD` must be kept in sync with the source files. The release script handles this automatically — do not update them manually.
>
> **Before running `make arch`, `make arch-install`, or `make package` after modifying any source files**, run `scripts/release.sh --preview <type>` first to regenerate the SHA256 checksums in `packaging/arch/PKGBUILD`. Without this step, `makepkg` will reject the build due to checksum mismatch.

```sh
scripts/release.sh <major|minor|fix>   # bump version, commit, tag, push
scripts/release.sh --preview <type>    # apply changes locally without committing
scripts/release.sh --dry-run <type>    # preview diff and revert
```

Contributions are welcome!
