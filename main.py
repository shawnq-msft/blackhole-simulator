#!/usr/bin/env python3
"""
Black Hole Simulator with CUDA acceleration and high-performance 3D rendering.
This application simulates gravitational lensing and other relativistic effects
around a black hole using GPU acceleration.
"""

import sys
import argparse
import time
import numpy as np
import torch
import moderngl
import moderngl_window as mglw
from moderngl_window.context.glfw import Window
from moderngl_window.timers.clock import Timer

from python_implementation.physics.black_hole import BlackHole
from python_implementation.rendering.renderer import BlackHoleRenderer
from python_implementation.utils.camera import Camera


class BlackHoleSimulatorApp(mglw.WindowConfig):
    """Main application class for the Black Hole Simulator."""
    
    gl_version = (4, 3)
    title = "Black Hole Simulator"
    window_size = (1280, 720)
    aspect_ratio = None
    resizable = True
    samples = 4
    resource_dir = 'assets'
    
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        
        # Check for CUDA availability
        self.cuda_available = torch.cuda.is_available()
        if self.cuda_available:
            print(f"CUDA available: {torch.cuda.get_device_name(0)}")
            self.device = torch.device("cuda")
        else:
            print("CUDA not available, falling back to CPU")
            self.device = torch.device("cpu")
        
        # Initialize black hole physics
        self.black_hole = BlackHole(
            mass=1.0,  # Mass in solar masses
            spin=0.9,  # Dimensionless spin parameter (0 to 1)
            position=np.array([0.0, 0.0, 0.0]),
            device=self.device
        )
        
        # Initialize camera
        self.camera = Camera(
            position=np.array([0.0, 0.0, 30.0]),
            target=np.array([0.0, 0.0, 0.0]),
            up=np.array([0.0, 1.0, 0.0]),
            fov=60.0,
            aspect=self.aspect_ratio or (self.window_size[0] / self.window_size[1]),
            near=0.1,
            far=1000.0
        )
        
        # Initialize renderer
        self.renderer = BlackHoleRenderer(
            ctx=self.ctx,
            black_hole=self.black_hole,
            camera=self.camera,
            window_size=self.window_size,
            device=self.device
        )
        
        # Initialize timing and input state
        self.timer = Timer()
        self.last_time = time.time()
        self.keys = {}
        self.mouse_pos = (0, 0)
        self.mouse_delta = (0, 0)
        self.mouse_pressed = False
        
        # Camera movement speed
        self.camera_speed = 5.0
        self.mouse_sensitivity = 0.2
        
        # Register event handlers
        self.wnd.mouse_position_event_func = self.on_mouse_position
        self.wnd.mouse_press_event_func = self.on_mouse_press
        self.wnd.mouse_release_event_func = self.on_mouse_release
        self.wnd.key_event_func = self.on_key_event
        self.wnd.resize_func = self.on_resize
        
        print("Black Hole Simulator initialized successfully")
    
    def on_resize(self, width, height):
        """Handle window resize events."""
        self.camera.aspect = width / height
        self.renderer.resize(width, height)
    
    def on_mouse_position(self, x, y, dx, dy):
        """Handle mouse movement events."""
        self.mouse_pos = (x, y)
        self.mouse_delta = (dx, dy)
        
        if self.mouse_pressed:
            # Rotate camera based on mouse movement
            self.camera.rotate(
                -dx * self.mouse_sensitivity,
                -dy * self.mouse_sensitivity
            )
    
    def on_mouse_press(self, x, y, button):
        """Handle mouse press events."""
        self.mouse_pressed = True
    
    def on_mouse_release(self, x, y, button):
        """Handle mouse release events."""
        self.mouse_pressed = False
    
    def on_key_event(self, key, action, modifiers):
        """Handle keyboard events."""
        if action == self.wnd.keys.ACTION_PRESS:
            self.keys[key] = True
            
            # Handle specific key presses
            if key == self.wnd.keys.ESCAPE:
                self.wnd.close()
        
        elif action == self.wnd.keys.ACTION_RELEASE:
            self.keys[key] = False
    
    def process_input(self, time_delta):
        """Process input for camera movement."""
        # Calculate movement speed with time delta for consistent movement
        move_speed = self.camera_speed * time_delta
        
        # Forward/backward movement
        if self.keys.get(self.wnd.keys.W):
            self.camera.move_forward(move_speed)
        if self.keys.get(self.wnd.keys.S):
            self.camera.move_forward(-move_speed)
        
        # Left/right movement
        if self.keys.get(self.wnd.keys.A):
            self.camera.move_right(-move_speed)
        if self.keys.get(self.wnd.keys.D):
            self.camera.move_right(move_speed)
        
        # Up/down movement
        if self.keys.get(self.wnd.keys.Q):
            self.camera.move_up(move_speed)
        if self.keys.get(self.wnd.keys.E):
            self.camera.move_up(-move_speed)
        
        # Adjust camera speed
        if self.keys.get(self.wnd.keys.PAGE_UP):
            self.camera_speed *= 1.1
        if self.keys.get(self.wnd.keys.PAGE_DOWN):
            self.camera_speed *= 0.9
    
    def render(self, time, frame_time):
        """Main render loop."""
        # Clear the framebuffer
        self.ctx.clear(0.0, 0.0, 0.0, 1.0)
        self.ctx.enable(moderngl.DEPTH_TEST | moderngl.CULL_FACE)
        
        # Process input
        self.process_input(frame_time)
        
        # Update camera matrices
        self.camera.update()
        
        # Render the black hole
        self.renderer.render()
        
        # Display FPS
        current_time = time.time()
        if current_time - self.last_time >= 1.0:
            fps = 1.0 / frame_time if frame_time > 0 else 0
            self.wnd.title = f"Black Hole Simulator - FPS: {fps:.1f}"
            self.last_time = current_time


def main():
    """Entry point for the application."""
    parser = argparse.ArgumentParser(description="Black Hole Simulator")
    parser.add_argument("--fullscreen", action="store_true", help="Run in fullscreen mode")
    parser.add_argument("--resolution", type=str, default="1280x720", help="Window resolution (e.g., 1920x1080)")
    parser.add_argument("--mass", type=float, default=1.0, help="Black hole mass in solar masses")
    parser.add_argument("--spin", type=float, default=0.9, help="Black hole spin parameter (0-1)")
    
    args = parser.parse_args()
    
    # Parse resolution
    try:
        width, height = map(int, args.resolution.split("x"))
    except ValueError:
        print(f"Invalid resolution format: {args.resolution}. Using default 1280x720.")
        width, height = 1280, 720
    
    # Configure window settings
    mglw.settings.WINDOW['class'] = Window
    mglw.settings.WINDOW['size'] = (width, height)
    mglw.settings.WINDOW['fullscreen'] = args.fullscreen
    mglw.settings.WINDOW['vsync'] = True
    mglw.settings.WINDOW['resizable'] = True
    mglw.settings.WINDOW['title'] = "Black Hole Simulator"
    
    # Run the application
    try:
        mglw.run_window_config(BlackHoleSimulatorApp)
    except Exception as e:
        print(f"Error running application: {e}")
        return 1
    
    return 0


if __name__ == "__main__":
    sys.exit(main())