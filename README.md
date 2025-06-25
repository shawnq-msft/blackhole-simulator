# Black Hole Simulator with DirectX

A physically accurate black hole simulator that demonstrates gravitational lensing and other relativistic effects using DirectX for rendering.

## Features

- Realistic simulation of black hole physics based on general relativity
- Gravitational lensing effect showing how light bends around massive objects
- Accretion disk simulation with relativistic Doppler and beaming effects
- Interactive camera controls to explore the black hole from different angles
- Support for both non-rotating (Schwarzschild) and rotating (Kerr) black holes

## Physics

The simulator implements several key aspects of black hole physics:

1. **Gravitational Lensing**: Light paths are bent by the curvature of spacetime around the black hole, calculated using the geodesic equation.

2. **Event Horizon**: The boundary beyond which nothing can escape, visualized as a black sphere.

3. **Accretion Disk**: A disk of hot, glowing matter spiraling into the black hole, with temperature-dependent coloration.

4. **Relativistic Effects**:
   - Doppler shift (redshift/blueshift) as matter orbits the black hole
   - Relativistic beaming that makes the approaching side of the disk appear brighter
   - Frame dragging around rotating black holes

## Implementation Details

- Written in C++ using DirectX 11 for rendering
- HLSL shaders for gravitational lensing and other visual effects
- Implements both Schwarzschild metric (non-rotating black holes) and Kerr metric (rotating black holes)
- Ray tracing techniques to calculate light paths in curved spacetime

## Controls

- WASD: Move camera position
- Mouse: Look around
- Q/E: Move up/down
- Mouse wheel: Adjust movement speed
- ESC: Exit the application

## Building the Project

### Prerequisites

- Windows operating system
- Visual Studio with C++ development tools
- DirectX SDK

### Build Steps

1. Clone the repository
2. Open the project in Visual Studio
3. Build the solution
4. Run the executable

## References

- Luminet, J.-P. (1979). "Image of a spherical black hole with thin accretion disk"
- James et al. (2015). "Gravitational Lensing by Spinning Black Holes in Astrophysics, and in the Movie Interstellar"
- Thorne, K. S. (2014). "The Science of Interstellar"