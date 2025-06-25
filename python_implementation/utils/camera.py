"""
Camera class for navigating the 3D space around the black hole.
"""

import numpy as np
import math
import pyrr
from pyrr import Matrix44, Vector3, Quaternion


class Camera:
    """
    Camera class for 3D navigation and view control.
    Provides functionality for moving and rotating the camera in 3D space.
    """
    
    def __init__(self, position=None, target=None, up=None, 
                 fov=60.0, aspect=16/9, near=0.1, far=1000.0):
        """
        Initialize the camera with position and orientation.
        
        Args:
            position: Camera position as a 3D vector
            target: Look-at target as a 3D vector
            up: Up vector for camera orientation
            fov: Field of view in degrees
            aspect: Aspect ratio (width/height)
            near: Near clipping plane distance
            far: Far clipping plane distance
        """
        # Set default values if not provided
        self.position = np.array([0.0, 0.0, 5.0]) if position is None else np.array(position)
        self.target = np.array([0.0, 0.0, 0.0]) if target is None else np.array(target)
        self.up = np.array([0.0, 1.0, 0.0]) if up is None else np.array(up)
        
        # Camera parameters
        self.fov = fov
        self.aspect = aspect
        self.near = near
        self.far = far
        
        # Derived vectors
        self.forward = self._normalize(self.target - self.position)
        self.right = self._normalize(np.cross(self.forward, self.up))
        self.up = self._normalize(np.cross(self.right, self.forward))
        
        # View and projection matrices
        self.view_matrix = None
        self.projection_matrix = None
        
        # Initialize matrices
        self.update()
    
    def _normalize(self, vector):
        """Normalize a vector to unit length."""
        norm = np.linalg.norm(vector)
        if norm < 1e-10:
            return vector
        return vector / norm
    
    def update(self):
        """Update view and projection matrices based on current camera state."""
        # Update derived vectors
        self.forward = self._normalize(self.target - self.position)
        self.right = self._normalize(np.cross(self.forward, self.up))
        self.up = self._normalize(np.cross(self.right, self.forward))
        
        # Create view matrix
        self.view_matrix = Matrix44.look_at(
            self.position,
            self.target,
            self.up
        )
        
        # Create projection matrix
        self.projection_matrix = Matrix44.perspective_projection(
            self.fov,
            self.aspect,
            self.near,
            self.far
        )
    
    def move_forward(self, distance):
        """
        Move camera forward along the view direction.
        
        Args:
            distance: Distance to move
        """
        self.position += self.forward * distance
        self.target += self.forward * distance
        self.update()
    
    def move_right(self, distance):
        """
        Move camera right along the right vector.
        
        Args:
            distance: Distance to move
        """
        self.position += self.right * distance
        self.target += self.right * distance
        self.update()
    
    def move_up(self, distance):
        """
        Move camera up along the up vector.
        
        Args:
            distance: Distance to move
        """
        self.position += self.up * distance
        self.target += self.up * distance
        self.update()
    
    def rotate(self, yaw, pitch):
        """
        Rotate the camera by yaw and pitch angles.
        
        Args:
            yaw: Rotation angle around the up axis in degrees
            pitch: Rotation angle around the right axis in degrees
        """
        # Convert to radians
        yaw_rad = math.radians(yaw)
        pitch_rad = math.radians(pitch)
        
        # Calculate view direction vector
        view_dir = self.target - self.position
        
        # Create rotation quaternions
        yaw_rotation = Quaternion.from_axis_rotation(self.up, yaw_rad)
        pitch_rotation = Quaternion.from_axis_rotation(self.right, pitch_rad)
        
        # Apply rotations
        view_dir = pyrr.quaternion.apply_to_vector(yaw_rotation, view_dir)
        view_dir = pyrr.quaternion.apply_to_vector(pitch_rotation, view_dir)
        
        # Update target position
        self.target = self.position + view_dir
        
        # Update camera vectors
        self.update()
    
    def get_view_matrix(self):
        """Get the current view matrix."""
        return self.view_matrix
    
    def get_projection_matrix(self):
        """Get the current projection matrix."""
        return self.projection_matrix
    
    def get_position(self):
        """Get the current camera position."""
        return self.position
    
    def get_view_direction(self):
        """Get the current view direction."""
        return self.forward
    
    def set_position(self, position):
        """
        Set the camera position.
        
        Args:
            position: New camera position as a 3D vector
        """
        self.position = np.array(position)
        self.update()
    
    def set_target(self, target):
        """
        Set the camera target.
        
        Args:
            target: New camera target as a 3D vector
        """
        self.target = np.array(target)
        self.update()
    
    def set_aspect_ratio(self, aspect):
        """
        Set the camera aspect ratio.
        
        Args:
            aspect: New aspect ratio (width/height)
        """
        self.aspect = aspect
        self.update()
    
    def generate_ray_directions(self, width, height):
        """
        Generate ray directions for each pixel in the viewport.
        
        Args:
            width: Viewport width in pixels
            height: Viewport height in pixels
            
        Returns:
            Array of ray directions for each pixel
        """
        # Calculate field of view in radians
        fov_rad = math.radians(self.fov)
        
        # Calculate viewport dimensions
        viewport_height = 2.0 * math.tan(fov_rad / 2.0)
        viewport_width = viewport_height * self.aspect
        
        # Calculate pixel size
        pixel_width = viewport_width / width
        pixel_height = viewport_height / height
        
        # Generate ray directions
        ray_directions = np.zeros((height, width, 3))
        
        for y in range(height):
            for x in range(width):
                # Calculate normalized device coordinates
                ndc_x = (x + 0.5) / width * 2.0 - 1.0
                ndc_y = 1.0 - (y + 0.5) / height * 2.0
                
                # Calculate ray direction in camera space
                ray_dir_camera = np.array([
                    ndc_x * viewport_width / 2.0,
                    ndc_y * viewport_height / 2.0,
                    -1.0  # Forward is negative z in camera space
                ])
                
                # Transform to world space
                ray_dir_world = np.array([
                    ray_dir_camera[0] * self.right +
                    ray_dir_camera[1] * self.up +
                    ray_dir_camera[2] * self.forward
                ])
                
                # Normalize
                ray_directions[y, x] = self._normalize(ray_dir_world)
        
        return ray_directions