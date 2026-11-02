`HEADJOBTests` is built by CMake. It checks parameter-state roundtrip, relative harmonic balance, finite/bounded output and bypass processing at 44.1/48/96/192 kHz, independent silent channel behavior, and exact latency-compensated clean null at zero controls.

`AUSmoke.cpp` is an optional macOS bundle test. Compile with `clang++ -std=c++17 Tests/AUSmoke.cpp -o au-smoke -framework AudioToolbox -framework CoreFoundation`, then pass the absolute .component path. It calls the exported AU factory interface directly, including initialize and stereo render. It deliberately opens without a system component registration; this is a bundle smoke test, not Apple's auval or a DAW integration test.

Full AU validation after installing: `auval -v aufx HjR4 Hjob -strict`.
Full VST3 validation: use Steinberg's VST3 SDK validator against the built bundle.
