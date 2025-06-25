"""
Black hole physics simulation using CUDA acceleration.
"""

import numpy as np
import torch
import math

# Physical constants
G = 6.67430e-11  # Gravitational constant (m^3 kg^-1 s^-2)
C = 299792458.0  # Speed of light (m/s)
SOLAR_MASS = 1.989e30  # Solar mass in kg

class BlackHole:
    """
    Class representing a black hole with relativistic physics calculations.
    Uses CUDA acceleration for ray tracing and gravitational lensing calculations.
    """
    
    def __init__(self, mass=1.0, spin=0.0, position=None, device=None):
        """
        Initialize a black hole with given parameters.
        
        Args:
            mass: Mass of the black hole in solar masses
            spin: Dimensionless spin parameter (0 to 1)
            position: 3D position of the black hole
            device: Torch device (cuda or cpu)
        """
        self.device = device if device is not None else torch.device("cuda" if torch.cuda.is_available() else "cpu")
        
        # Convert mass to SI units
        self.mass = mass * SOLAR_MASS
        
        # Validate and set spin parameter (0 ≤ a ≤ 1)
        self.spin = max(0.0, min(1.0, spin))
        
        # Set position
        self.position = np.array([0.0, 0.0, 0.0]) if position is None else np.array(position)
        
        # Calculate Schwarzschild radius (event horizon for non-rotating black hole)
        self.rs = 2.0 * G * self.mass / (C * C)
        
        # Calculate event horizon radius for Kerr black hole
        self.rh = self.rs / 2 * (1 + np.sqrt(1 - self.spin**2))
        
        # Calculate photon sphere radius
        self.photon_sphere_radius = 1.5 * self.rs
        
        # Initialize CUDA kernels for ray tracing
        self._init_cuda_kernels()
    
    def _init_cuda_kernels(self):
        """Initialize CUDA kernels for physics calculations."""
        # Move calculations to the selected device
        self.rs_tensor = torch.tensor(self.rs, device=self.device, dtype=torch.float32)
        self.spin_tensor = torch.tensor(self.spin, device=self.device, dtype=torch.float32)
        self.position_tensor = torch.tensor(self.position, device=self.device, dtype=torch.float32)
    
    def calculate_deflection(self, ray_origins, ray_directions):
        """
        Calculate the deflection of light rays due to gravitational lensing.
        
        Args:
            ray_origins: Tensor of ray origin positions (batch_size, 3)
            ray_directions: Tensor of ray direction vectors (batch_size, 3)
            
        Returns:
            Tensor of deflected ray directions (batch_size, 3)
        """
        # Ensure inputs are on the correct device
        ray_origins = ray_origins.to(self.device)
        ray_directions = ray_directions.to(self.device)
        
        # Normalize ray directions
        ray_directions = ray_directions / torch.norm(ray_directions, dim=1, keepdim=True)
        
        # Calculate ray parameters relative to black hole position
        rel_origins = ray_origins - self.position_tensor
        
        # Calculate impact parameter (closest approach distance if the ray were undeflected)
        # This is a simplified calculation for demonstration
        dot_product = torch.sum(rel_origins * ray_directions, dim=1, keepdim=True)
        closest_points = rel_origins - dot_product * ray_directions
        impact_parameters = torch.norm(closest_points, dim=1, keepdim=True)
        
        # Calculate deflection angle using the Schwarzschild metric approximation
        # θ = 4GM/(c²b) where b is the impact parameter
        deflection_angles = 4.0 * G * self.mass / (C * C * impact_parameters)
        
        # Apply deflection to ray directions (simplified model)
        # In a real simulation, we would solve the geodesic equations
        deflection_vectors = torch.cross(ray_directions, 
                                         torch.cross(rel_origins, ray_directions, dim=1), 
                                         dim=1)
        deflection_vectors = deflection_vectors / torch.norm(deflection_vectors, dim=1, keepdim=True)
        
        # Apply deflection based on angle and impact parameter
        deflection_mask = (impact_parameters > self.rs_tensor * 1.5).float()  # No deflection inside photon sphere
        deflection_factor = deflection_angles * deflection_mask
        
        # Calculate new directions
        new_directions = ray_directions + deflection_vectors * deflection_factor
        new_directions = new_directions / torch.norm(new_directions, dim=1, keepdim=True)
        
        return new_directions
    
    def ray_trace(self, ray_origins, ray_directions, max_steps=100, step_size=0.1):
        """
        Trace rays through the curved spacetime around the black hole.
        
        Args:
            ray_origins: Tensor of ray origin positions (batch_size, 3)
            ray_directions: Tensor of ray direction vectors (batch_size, 3)
            max_steps: Maximum number of integration steps
            step_size: Size of each integration step
            
        Returns:
            Dictionary containing ray tracing results
        """
        batch_size = ray_origins.shape[0]
        
        # Initialize results
        intersected = torch.zeros(batch_size, 1, device=self.device, dtype=torch.bool)
        final_positions = torch.zeros_like(ray_origins)
        final_directions = torch.zeros_like(ray_directions)
        
        # Current state
        positions = ray_origins.clone()
        directions = ray_directions.clone() / torch.norm(ray_directions, dim=1, keepdim=True)
        
        # Ray tracing loop
        for step in range(max_steps):
            # Calculate distance to black hole center
            rel_positions = positions - self.position_tensor
            distances = torch.norm(rel_positions, dim=1, keepdim=True)
            
            # Check for intersection with event horizon
            new_intersected = distances < self.rs_tensor * 1.1  # Slightly larger than event horizon
            intersected = intersected | new_intersected
            
            # Update positions for non-intersected rays
            active_rays = ~intersected.squeeze()
            if not torch.any(active_rays):
                break
                
            # Calculate deflection for active rays
            deflected_directions = self.calculate_deflection(
                positions[active_rays], 
                directions[active_rays]
            )
            
            # Update directions and positions
            directions[active_rays] = deflected_directions
            positions[active_rays] = positions[active_rays] + directions[active_rays] * step_size
            
            # Store final state
            final_positions = positions.clone()
            final_directions = directions.clone()
        
        return {
            'intersected': intersected,
            'positions': final_positions,
            'directions': final_directions
        }
    
    def calculate_redshift(self, observer_velocity, emission_position):
        """
        Calculate relativistic redshift/blueshift of light from the accretion disk.
        
        Args:
            observer_velocity: Velocity vector of the observer
            emission_position: Position where light is emitted (e.g., on accretion disk)
            
        Returns:
            Redshift factor (z)
        """
        # Convert inputs to tensors on the correct device
        observer_velocity = torch.tensor(observer_velocity, device=self.device, dtype=torch.float32)
        emission_position = torch.tensor(emission_position, device=self.device, dtype=torch.float32)
        
        # Calculate orbital velocity at emission position (simplified model)
        rel_position = emission_position - self.position_tensor
        distance = torch.norm(rel_position)
        
        # Keplerian orbital velocity
        orbital_speed = torch.sqrt(G * self.mass / distance)
        
        # Direction of orbital velocity (perpendicular to radius vector)
        orbital_direction = torch.tensor([
            -rel_position[1], 
            rel_position[0], 
            0.0
        ], device=self.device, dtype=torch.float32)
        orbital_direction = orbital_direction / torch.norm(orbital_direction)
        
        # Orbital velocity vector
        orbital_velocity = orbital_direction * orbital_speed
        
        # Calculate relativistic Doppler shift
        # z = γ(1 - v·n/c) - 1 where γ = 1/√(1-v²/c²)
        v_squared = torch.sum(orbital_velocity**2)
        gamma = 1.0 / torch.sqrt(1.0 - v_squared / (C*C))
        
        # Direction from emission to observer (simplified as radial)
        direction_to_observer = -rel_position / distance
        
        # Doppler factor
        doppler_factor = gamma * (1.0 - torch.sum(orbital_velocity * direction_to_observer) / C)
        
        # Gravitational redshift
        # z_grav = 1/√(1-2GM/rc²) - 1
        gravitational_factor = 1.0 / torch.sqrt(1.0 - self.rs_tensor / distance)
        
        # Combined redshift
        redshift = doppler_factor * gravitational_factor - 1.0
        
        return redshift
    
    def generate_accretion_disk(self, inner_radius=None, outer_radius=None, resolution=1024):
        """
        Generate an accretion disk model around the black hole.
        
        Args:
            inner_radius: Inner radius of accretion disk (defaults to ISCO)
            outer_radius: Outer radius of accretion disk
            resolution: Resolution of the disk texture
            
        Returns:
            Dictionary containing accretion disk data
        """
        # Calculate innermost stable circular orbit (ISCO)
        if self.spin == 0:
            # Schwarzschild black hole
            isco_radius = 6.0 * G * self.mass / (C * C)
        else:
            # Kerr black hole (simplified)
            z1 = 1.0 + (1.0 - self.spin**2)**(1/3) * ((1.0 + self.spin)**(1/3) + (1.0 - self.spin)**(1/3))
            z2 = torch.sqrt(3.0 * self.spin**2 + z1**2)
            isco_radius = self.rs * (3.0 + z2 - torch.sqrt((3.0 - z1) * (3.0 + z1 + 2.0 * z2)))
        
        # Set default radii if not provided
        if inner_radius is None:
            inner_radius = float(isco_radius)
        if outer_radius is None:
            outer_radius = 20.0 * self.rs
        
        # Create a grid of points for the accretion disk
        theta = torch.linspace(0, 2*math.pi, resolution, device=self.device)
        r = torch.linspace(inner_radius, outer_radius, resolution//2, device=self.device)
        
        # Create meshgrid
        theta_grid, r_grid = torch.meshgrid(theta, r, indexing='ij')
        
        # Convert to Cartesian coordinates
        x = r_grid * torch.cos(theta_grid)
        y = r_grid * torch.sin(theta_grid)
        z = torch.zeros_like(x)  # Thin disk approximation
        
        # Calculate temperature profile (simplified)
        # T ∝ r^(-3/4) for a standard thin disk
        temperature = torch.pow(r_grid / inner_radius, -0.75)
        
        # Normalize temperature to range [0, 1]
        temperature = (temperature - temperature.min()) / (temperature.max() - temperature.min())
        
        # Create position tensor
        positions = torch.stack([x, y, z], dim=-1)
        
        return {
            'positions': positions,
            'temperature': temperature,
            'inner_radius': inner_radius,
            'outer_radius': outer_radius
        }
    
    def get_event_horizon_mesh(self, resolution=64):
        """
        Generate a mesh representing the event horizon.
        
        Args:
            resolution: Number of vertices around the equator
            
        Returns:
            Dictionary containing vertex and index data for rendering
        """
        # For a Kerr black hole, the event horizon is oblate
        # This is a simplified model that doesn't fully capture the ergosphere
        
        # Generate spherical coordinates
        theta = torch.linspace(0, math.pi, resolution, device=self.device)
        phi = torch.linspace(0, 2*math.pi, resolution, device=self.device)
        
        # Create meshgrid
        theta_grid, phi_grid = torch.meshgrid(theta, phi, indexing='ij')
        
        # Calculate radius based on angle (simplified Kerr metric)
        radius = self.rh * (1 + 0.5 * self.spin**2 * torch.sin(theta_grid)**2)
        
        # Convert to Cartesian coordinates
        x = radius * torch.sin(theta_grid) * torch.cos(phi_grid)
        y = radius * torch.sin(theta_grid) * torch.sin(phi_grid)
        z = radius * torch.cos(theta_grid)
        
        # Create position tensor
        positions = torch.stack([x, y, z], dim=-1)
        
        # Generate indices for triangle mesh
        indices = []
        for i in range(resolution-1):
            for j in range(resolution-1):
                # Calculate vertex indices
                v0 = i * resolution + j
                v1 = i * resolution + (j + 1)
                v2 = (i + 1) * resolution + j
                v3 = (i + 1) * resolution + (j + 1)
                
                # Add two triangles
                indices.append([v0, v1, v2])
                indices.append([v1, v3, v2])
        
        indices = torch.tensor(indices, device=self.device, dtype=torch.int32)
        
        return {
            'positions': positions.reshape(-1, 3),
            'indices': indices
        }