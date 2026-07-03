# abls-agent-libs

Reusable C shared library for ABLS agent lifecycle orchestration.

## Scope

- Thread_Init / Thread_Start / Thread_End
- Signal-aware stop requests
- Optional MQTT bridge wrappers
- Config bootstrap precedence through Json_read_config

Effective configuration precedence:
1. Environment variables (ABLS_*)
2. Config file (/etc/abls-habitat-agent.conf)
3. Built-in defaults

## Build

```sh
./install_deps.sh
./build.sh
sudo ./install.sh
```

## Test

```sh
cd build
ctest --output-on-failure
```

## Packaging RPM

```sh
./build_rpm.sh
```

Produces runtime and devel RPM packages in `build/`.

## Packaging DEB

```sh
./build_apt.sh --dist bookworm --no-sign
./build_apt.sh --dist trixie --no-sign
```

Produces runtime and dev DEB packages and copies normalized artifacts to:

- `build/deb/<suite>/<arch>/`

## Release bump + publication

```sh
./bump.sh 1.2.3
```

The release flow:

- tags `v1.2.3` from `trunk`
- merges `trunk` into `main`
- builds RPM + DEB packages
- copies RPM to `../ABLS-PKGS/public/rpms/<arch>/`
- copies DEB to `../ABLS-PKGS/deb-packages/<suite>/`
