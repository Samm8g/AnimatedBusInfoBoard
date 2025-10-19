# Animated Bus Info Board

This is a C++/Qt demo application that simulates a modern, animated bus information display screen.

It is designed to be configurable and showcases several Qt features, including layouts, timers, custom widgets, and animations.

## Features

- Full-screen display with a dark theme.
- Configurable bus number, route name, and weather indicator.
- Animated upcoming stop list, loaded from `stops.csv`.
- Animated, rotating ad display, loaded from an `ads/` directory.
- Smooth progress indicators for stop and ad changes.
- Live-updating clock and date.

## Build Instructions

### NixOS (Recommended)

This project is configured to build easily in a Nix shell environment.

1.  **Enter the Shell:**
    Open your terminal in the project's root directory and run:
    ```sh
    nix-shell
    ```

2.  **Build the Project:**
    Create a build directory (if you don't have one) and run CMake and Ninja:
    ```sh
    cmake -B cmake-build-debug -S . -G Ninja
    cmake --build cmake-build-debug
    ```

### Standard Linux (Debian/Ubuntu Example)

If you are not using NixOS, you can install the dependencies manually.

1.  **Install Dependencies:**
    You will need a C++ compiler, CMake, Ninja, and the Qt6 development libraries.
    ```sh
    sudo apt update
    sudo apt install build-essential cmake ninja-build qt6-base-dev
    ```

2.  **Configure and Build:**
    ```sh
    cmake -B cmake-build-debug -S . -G Ninja
    cmake --build cmake-build-debug
    ```

## Running the Application

After a successful build, the executable and its data files will be located in the `cmake-build-debug` directory.

To run the application, execute the following command from within the `cmake-build-debug` directory:

```sh
./AnimatedBusInfoBoard
```

## Customization

This demo is designed to be easily customized without changing the source code. The master copies of all configuration files are located in the `data/` directory in the project root.

When you build the project, these files are automatically copied to the `cmake-build-debug` directory. You can edit the files in `data/` and rebuild to see your changes.

-   **Bus & Display Info (`data/config.ini`):**
    Edit this file to change the bus number, route name, and the weather text.

-   **Bus Stops (`data/stops.csv`):**
    Edit this file to change the list of bus stops. Each line represents one stop.

-   **Ad Images (`data/ads/` directory):**
    Place your ad images (`.png` or `.jpg`) inside the `data/ads/` directory. The application will automatically load and rotate through them.