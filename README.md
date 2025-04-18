# u

## Synopsis

A collection of efficient circuits to determine if an arbitrary length inputs vector is a valid unary encoding.

## Detail

### Edge-Based [e.sv](./rtl/e/e.sv):

Admit an input vector if there is at most one edge transition across the span.

### Mask-Based [u.sv](./rtl/u/u.sv):

Admit an input vector if the format is a sequence of ones followed by a sequence of zeros. 

### Incrementer-Based [p.sv](./rtl/p/p.sv):

Admit an input vector if, once incremented, the resultant vector is one-hot encoded.

## Instructions

The [Dockerfile](./.devcontainer/Dockerfile) is the recommended development environment for 'u'. Alternatively, 'u' can be built standalone using a recent version of Verilator (developed using v5.034). An additional environment variable 'VERILATOR_ROOT' pointing to the root of a Verilator installation is required. 

```shell
# Compile library for some pre-defined configuration (W=32)
cmake . --preset w32c
cmake --build build_w32c -t tb

# List available designs
./build_w32c/tb/tb --list_designs

# List available tests
./build_w32c/tb/tb --list_tests
DirectedExhaustiveTestCase
FullyRandomizedTestCase

# Run a test on design
./build_w32c/tb/tb -d -t d=u,t=DirectedExhaustiveTestCase
```

