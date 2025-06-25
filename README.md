# Black Hole Simulator

A high-performance black hole simulator that demonstrates gravitational lensing and other relativistic effects using CUDA acceleration and modern 3D rendering techniques.

## Features

- **Physically accurate simulation** based on general relativity
- **Gravitational lensing** showing how light bends around black holes
- **Accretion disk visualization** with relativistic effects
- **CUDA-accelerated physics** for real-time performance
- **High-quality rendering** using modern OpenGL (via ModernGL)
- **Interactive camera** for exploring the black hole from different angles

## Physics Simulation

The simulator implements several key aspects of black hole physics:

1. **Gravitational Lensing**: Light paths are bent by the curvature of spacetime around the black hole, calculated using the geodesic equation.

2. **Event Horizon**: The boundary beyond which nothing can escape, visualized as a black sphere.

3. **Accretion Disk**: A disk of hot, glowing matter spiraling into the black hole, with temperature-dependent coloration.

4. **Relativistic Effects**:
   - Doppler shift (redshift/blueshift) as matter orbits the black hole
   - Relativistic beaming that makes the approaching side of the disk appear brighter
   - Frame dragging around rotating black holes

## Implementation Details

### Python Implementation

- **CUDA Acceleration**: Uses PyTorch and CUDA for high-performance physics calculations
- **Modern OpenGL**: Utilizes ModernGL for efficient rendering
- **GLSL Shaders**: Custom shaders for gravitational lensing and visual effects
- **Physically Based**: Implements both Schwarzschild metric (non-rotating black holes) and Kerr metric (rotating black holes)

### Directory Structure

- `/python_implementation/physics/`: Black hole physics implementation
- `/python_implementation/rendering/`: Rendering components
- `/python_implementation/utils/`: Utility classes
- `/python_implementation/shaders/`: GLSL shaders for visual effects

## Requirements

- Python 3.8+
- CUDA-capable GPU (for optimal performance)
- Dependencies listed in `requirements.txt`

## Installation

1. Clone the repository:
   ```
   git clone https://github.com/novaAIdesigner/blackhole-simulator.git
   cd blackhole-simulator
   ```

2. Install dependencies:
   ```
   pip install -r requirements.txt
   ```

## Usage

Run the simulator:

```
python main.py
```

### Command-line options:

- `--fullscreen`: Run in fullscreen mode
- `--resolution WIDTHxHEIGHT`: Set window resolution (default: 1280x720)
- `--mass VALUE`: Set black hole mass in solar masses (default: 1.0)
- `--spin VALUE`: Set black hole spin parameter from 0 to 1 (default: 0.9)

### Controls:

- **WASD**: Move camera position
- **Mouse**: Look around
- **Q/E**: Move up/down
- **Mouse wheel**: Adjust movement speed
- **ESC**: Exit the application

## Scientific Background

The simulation is based on the mathematics of general relativity, particularly:

- The Schwarzschild metric for non-rotating black holes
- The Kerr metric for rotating black holes
- Gravitational lensing equations
- Relativistic Doppler and gravitational redshift

## References

- Luminet, J.-P. (1979). "Image of a spherical black hole with thin accretion disk"
- James et al. (2015). "Gravitational Lensing by Spinning Black Holes in Astrophysics, and in the Movie Interstellar"
- Thorne, K. S. (2014). "The Science of Interstellar"
- Bronzwaer et al. (2021). "RAPTOR II: polarized radiative transfer in curved spacetime"

## License

MIT License