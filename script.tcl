open_project lowpass-to-phase
set_top lowpass_to_phase
add_files src/tophase.cpp
add_files src/tophase.hpp
add_files -tb src/tb.cpp -cflags "-Wno-unknown-pragmas"
open_solution "solution1"
set_part {xczu28dr-ffvg1517-2-e}
create_clock -period 550MHz -name default
config_export -description {Resize decimated resIQ stream and convert to phase} -display_name "Lowpass to Phase" -format ip_catalog -library mkidgen3 -rtl verilog -vendor MazinLab -version 1.0
csim_design
csynth_design
#cosim_design -tool xsim

