# cca
Cellular automaton with a few features (cellular competition, mobility, etc) in C++ (sfml). Run from terminal on Linux (./model [preset]). Pseudo-interface in polish. Prepared for a presentation about cyclic cellular automata on a 

Predefined presets: 
* big - grid 200x200, 7 populations, cyclic competiton, discrete time
* superbig - grid 400x400, 7 populations, cyclic competiton, discrete time
* superbigc - grid 400x400, 7 populations, cyclic competiton, continuous time
* 4 - grid 150x150, 3 populations, cyclic competition, continuous time, at each step one of three actions (randomly chosen from: colonization, swap, elimination) is performed on a pair of neighbouring cells
* and others.
