mkdir -p build
g++ ../source/AlternateGame.cpp -o build/AlternateGame -O3 -std=c++17
g++ ../source/OnePlayerGame.cpp -o build/OnePlayerGame -O3 -std=c++17
g++ ../source/SimultaneousGame.cpp -o build/SimultaneousGame -O3 -std=c++17
