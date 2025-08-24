#!/bin/bash

mkdir -pv externals
cd externals

wget https://raw.githubusercontent.com/YosysHQ/picorv32/refs/heads/main/picorv32.v
wget https://raw.githubusercontent.com/t-weber/electro/refs/heads/main/fpga_sv/mem/ram_2port.sv
wget https://raw.githubusercontent.com/t-weber/electro/refs/heads/main/fpga_sv/mem/memcpy.sv
wget https://raw.githubusercontent.com/t-weber/electro/refs/heads/main/fpga_sv/mem/memsel.sv
