# 3D Game Engine

This project is a simple 3D demo game engine developed in C++. It demonstrates basic rendering of 3D models in the OBJ file format and showcases fundamental transformations such as rotation and translation, using vector and matrix math for rendering in a console application.

## Features

- Rendering of 3D models in OBJ format
- Basic transformations (rotation and translation)
- Camera controls using arrow keys
- Vector and matrix math implementation
- Console-based rendering

## Getting Started

### Prerequisites

- C++ compiler (e.g., g++, Visual Studio)
- `olcConsoleGameEngine.h` header file

### Installation

1. Clone the repository:
    
    bash
    
    ```bash
    git clone https://github.com/AlabasterRig/3D-Engine.git
    cd 3D-Engine
    ```
    
2. Install dependencies:
    - Download the `olcConsoleGameEngine.h` file from [OneLoneCoder's repository](https://github.com/OneLoneCoder/olcPixelGameEngine)
    - Place it in the `include` directory of your project
3. Add your 3D models:
    - Place your OBJ files in the `models` directory
    - The engine can load standard OBJ files with vertex and face data
4. Build the project:
    
    bash
    
    ```bash
    # For Linux/macOS with g++
    g++ -o 3DEngine main.cpp -std=c++17
    
    # For Windows with Visual Studio
    # Open the solution file and build using Visual Studio interface
    ```
    
5. Run the application:
    
    bash
    
    ```bash
    # Linux/macOS
    ./3DEngine
    
    # Windows
    3DEngine.exe
    ```
    

### Usage

This engine can be used as a base for developing 3D applications and games. You can extend it by adding more features such as lighting and advanced physics.

#### Controls

- Arrow keys: Move camera
- W/A/S/D: Move through the scene
- Q/E: Rotate object
- Space: Reset view

## Acknowledgments

- [OneLoneCoder](https://github.com/OneLoneCoder) for the Console Game Engine
