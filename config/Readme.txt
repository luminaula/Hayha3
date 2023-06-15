Hayha - Neural network game enhancement toolkit 0.2.5

Configure your desired detection library in Settings.json.
HNetCl, HNetCUDA, HNetCUDNN, and HNetCUDNNH are many times faster than HNetCPU.

To use remote detection copy all files to your desired detection daemon server.
Configure ip and change mode to "remote" in Settings.json

Edit Mouse.lua and Keyboard.lua to your liking. You are expected to be able to edit them.

System recommendations:

6 core cpu
Nvidia 1070 or AMD vega
16gb ddr4 3200mhz cl14
10gbe if using remote detection

Dependancies:

Cuda 9.2 (Optional)
CUDNN (Optional)
OpenCl (Optional)

How to use:
-Configure Settings.json
-Start Hayha executable.
-Press "Start"
-Left control is default activation key

Changelog:

0.2.5
-Added old capturing when DXGI fails
-Fixed gui crashing on launch
-Fixed loading network
-Fixed logging
-Hacked OpenCl runtime to work for AMD gpus 
-Api to version 0.0.2
-Added benchmarking
-Mouse and keyboard now process at intervals instead of random times