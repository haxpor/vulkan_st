@echo off

rem Require glslc.exe (bundled with vulkansdk).
rem Add vulkansdk's Bin path into your environment variable PATH.
glslc.exe main.vert -o vert.spv
glslc.exe main.frag -o frag.spv
