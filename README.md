# u

## Synopsis

A collection of efficient circuits to determine if an arbitrary lengthed input vector is a valid unary encoding.

## Detail

#### Mask-Based [u.sv](./rtl/u/u.sv):

Admit an input vector if the format is a sequence of ones followed by a sequence of zeros.

Compute a lo- and hi- mask of a contiguous span of ones and a contiguous span of zeros. Match on spans for each possible transition location in the input vector. Redundant recomputation on lo- and hi- masks can be efficiently eliminated by Common Subexpression Elimination during synthesis, but still logic area may be high for large W. 

#### Edge-Based [e.sv](./rtl/e/e.sv):

Admit an input vector if there is at most one edge transition across the span.

Edge detection between adjacent bits is performed efficiently using an XOR. 1-hot detection is sub-linear complexity on W. Some additional qualification is required on final decision to determine final result. This does not represent a timing concern.

#### Incrementer-Based [p.sv](./rtl/p/p.sv): 

Admit an input vector if, once incremented, the resultant vector is one-hot encoded.

An increment operation performed by a CLA has a sub-linear complexity on W. The 1-hot detection at the output of the incrementer has sub-linear complexity on W. A conditional invert is required on the input to the incrementer. The CLA can be performed efficiently in logic for large W, but the implicit serialization of the prioritization operations may present a timing concern.

#### Prefix-based [c.sv](./rtl/c/c.sv)

Admit an input vector if all prefix sub-vectors are themselves valid partial unary encodings.

An array of cells is used to detect a valid unary encoding from LSB to MSB. The LSB of the vector must be a valid value for a unary encoding: 1'b1 in the normal-form case, 1'b0 in the complimented-form case. Detect presence of edges with increasing index. The presence of duplicate edges in the vector kills the match operation. Exactly one edge must be present to match a valid unary encoding. The final index must correspond to a valid terminal value for the unary encoding: 1'b0 in the normal-form case, 1'b1 in the complimented-form for case. Circuit complexity is linear on W and is therefore inefficient when compared against the other solutions. Solution is noteworthy due to its non-trivial circuit implementation.

## Instructions

The [Dockerfile](./.devcontainer/Dockerfile) is the recommended development environment for 'u'. Alternatively, 'u' can be built standalone using a recent version of Verilator (developed using v5.034). In this case, an additional environment variable 'VERILATOR_ROOT' pointing to the root of a Verilator installation is required. 

```shell
# Compile library for some pre-defined configuration (W=32)
cmake . --preset w32c
cmake --build build_w32c -t tb

# List available designs
./build_w32c/tb/tb --list_designs
e
u
c
p

# List available tests
./build_w32c/tb/tb --list_tests
DirectedExhaustiveTestCase
FullyRandomizedTestCase

# Run a test on design
./build_w32c/tb/tb -d -t d=u,t=DirectedExhaustiveTestCase
```

