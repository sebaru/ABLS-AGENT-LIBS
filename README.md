# abls-satellite-libs

Reusable C shared library for ABLS satellite lifecycle orchestration.

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

## RPM

```sh
cd build
cpack -G RPM
```

Produces runtime and devel RPM packages.
