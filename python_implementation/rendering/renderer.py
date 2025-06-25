"""
Renderer class for visualizing the black hole using high-performance 3D rendering.
"""

import os
import numpy as np
import torch
import moderngl
from pathlib import Path
from PIL import Image
import pyrr
from pyrr import Matrix44

class BlackHoleRenderer:
    """
    Renderer class for visualizing black holes with gravitational lensing effects.
    Uses ModernGL for high-performance GPU-accelerated rendering.
    """
    
    def __init__(self, ctx, black_hole, camera, window_size, device=None):
        """
        Initialize the black hole renderer.
        
        Args:
            ctx: ModernGL context
            black_hole: BlackHole instance
            camera: Camera instance
            window_size: Tuple of (width, height)
            device: Torch device (cuda or cpu)
        """
        self.ctx = ctx
        self.black_hole = black_hole
        self.camera = camera
        self.window_size = window_size
        self.device = device if device is not None else torch.device("cuda" if torch.cuda.is_available() else "cpu")
        
        # Initialize time
        self.time = 0.0
        
        # Load shaders
        self._load_shaders()
        
        # Create geometry
        self._create_geometry()
        
        # Create textures
        self._create_textures()
        
        # Create framebuffers
        self._create_framebuffers()
    
    def _load_shaders(self):
        """Load and compile shader programs."""
        # Get shader directory
        shader_dir = Path(__file__).parent.parent / "shaders"
        
        # Load vertex shader
        with open(shader_dir / "blackhole.vert") as f:
            vertex_shader = f.read()
        
        # Load fragment shaders
        with open(shader_dir / "blackhole.frag") as f:
            blackhole_fragment = f.read()
        
        with open(shader_dir / "starfield.frag") as f:
            starfield_fragment = f.read()
        
        with open(shader_dir / "accretion_disk.frag") as f:
            accretion_disk_fragment = f.read()
        
        # Compile shader programs
        self.blackhole_program = self.ctx.program(
            vertex_shader=vertex_shader,
            fragment_shader=blackhole_fragment
        )
        
        self.starfield_program = self.ctx.program(
            vertex_shader=vertex_shader,
            fragment_shader=starfield_fragment
        )
        
        self.accretion_disk_program = self.ctx.program(
            vertex_shader=vertex_shader,
            fragment_shader=accretion_disk_fragment
        )
    
    def _create_geometry(self):
        """Create geometry for rendering."""
        # Create a full-screen quad for post-processing
        vertices = np.array([
            # Position (x, y, z), Texture coordinates (u, v)
            -1.0, -1.0, 0.0, 0.0, 0.0,
             1.0, -1.0, 0.0, 1.0, 0.0,
             1.0,  1.0, 0.0, 1.0, 1.0,
            -1.0,  1.0, 0.0, 0.0, 1.0
        ], dtype='f4')
        
        indices = np.array([
            0, 1, 2,
            0, 2, 3
        ], dtype='i4')
        
        # Create vertex buffer
        self.quad_vbo = self.ctx.buffer(vertices)
        
        # Create index buffer
        self.quad_ibo = self.ctx.buffer(indices)
        
        # Create vertex array
        self.quad_vao = self.ctx.vertex_array(
            self.blackhole_program,
            [
                (self.quad_vbo, '3f 2f', 'in_position', 'in_texcoord_0')
            ],
            self.quad_ibo
        )
        
        # Create a sphere for the event horizon
        event_horizon_data = self.black_hole.get_event_horizon_mesh(resolution=64)
        
        # Convert to numpy arrays
        positions = event_horizon_data['positions'].cpu().numpy()
        indices = event_horizon_data['indices'].cpu().numpy()
        
        # Create vertex buffer
        self.horizon_vbo = self.ctx.buffer(positions.astype('f4'))
        
        # Create index buffer
        self.horizon_ibo = self.ctx.buffer(indices.astype('i4'))
        
        # Create vertex array
        self.horizon_vao = self.ctx.vertex_array(
            self.blackhole_program,
            [
                (self.horizon_vbo, '3f', 'in_position')
            ],
            self.horizon_ibo
        )
    
    def _create_textures(self):
        """Create textures for rendering."""
        # Generate starfield texture
        starfield_size = 2048
        starfield_data = np.zeros((starfield_size, starfield_size, 4), dtype=np.float32)
        
        # Simple procedural starfield (will be replaced by shader)
        for i in range(10000):
            x = np.random.randint(0, starfield_size)
            y = np.random.randint(0, starfield_size)
            brightness = np.random.random() * 0.8 + 0.2
            size = np.random.randint(1, 4)
            
            for dx in range(-size, size + 1):
                for dy in range(-size, size + 1):
                    px = (x + dx) % starfield_size
                    py = (y + dy) % starfield_size
                    
                    # Gaussian falloff
                    dist = np.sqrt(dx*dx + dy*dy)
                    if dist <= size:
                        intensity = brightness * np.exp(-dist*dist / (2*size*size))
                        
                        # Random star color
                        if np.random.random() < 0.1:
                            # Blue-white
                            color = np.array([0.8, 0.9, 1.0, 1.0])
                        elif np.random.random() < 0.2:
                            # Yellow
                            color = np.array([1.0, 0.9, 0.6, 1.0])
                        elif np.random.random() < 0.1:
                            # Red
                            color = np.array([1.0, 0.5, 0.5, 1.0])
                        else:
                            # White
                            color = np.array([1.0, 1.0, 1.0, 1.0])
                        
                        starfield_data[py, px] += color * intensity
        
        # Clamp values
        starfield_data = np.clip(starfield_data, 0.0, 1.0)
        
        # Create starfield texture
        self.starfield_texture = self.ctx.texture(
            (starfield_size, starfield_size),
            4,
            data=starfield_data.tobytes()
        )
        self.starfield_texture.filter = (moderngl.LINEAR, moderngl.LINEAR)
        self.starfield_texture.repeat_x = True
        self.starfield_texture.repeat_y = True
        
        # Generate accretion disk texture
        disk_size = 2048
        disk_data = np.zeros((disk_size, disk_size, 4), dtype=np.float32)
        
        # Create accretion disk texture (will be generated by shader)
        self.accretion_disk_texture = self.ctx.texture(
            (disk_size, disk_size),
            4,
            data=disk_data.tobytes()
        )
        self.accretion_disk_texture.filter = (moderngl.LINEAR, moderngl.LINEAR)
        self.accretion_disk_texture.repeat_x = False
        self.accretion_disk_texture.repeat_y = True
    
    def _create_framebuffers(self):
        """Create framebuffers for rendering."""
        # Create color attachment
        self.color_texture = self.ctx.texture(self.window_size, 4)
        self.color_texture.filter = (moderngl.LINEAR, moderngl.LINEAR)
        
        # Create depth attachment
        self.depth_texture = self.ctx.depth_texture(self.window_size)
        self.depth_texture.filter = (moderngl.LINEAR, moderngl.LINEAR)
        
        # Create framebuffer
        self.framebuffer = self.ctx.framebuffer(
            color_attachments=[self.color_texture],
            depth_attachment=self.depth_texture
        )
    
    def resize(self, width, height):
        """
        Resize the renderer to match the new window size.
        
        Args:
            width: New width in pixels
            height: New height in pixels
        """
        self.window_size = (width, height)
        
        # Recreate framebuffers
        self._create_framebuffers()
    
    def render(self):
        """Render the black hole scene."""
        # Update time
        self.time += 0.01
        
        # Get camera matrices
        view_matrix = self.camera.get_view_matrix()
        projection_matrix = self.camera.get_projection_matrix()
        
        # Render to framebuffer
        self.framebuffer.use()
        self.ctx.clear(0.0, 0.0, 0.0, 1.0)
        
        # Render starfield background
        self.starfield_program['time'] = self.time
        self.starfield_program['resolution'] = self.window_size
        self.starfield_program['model'] = Matrix44.identity()
        self.starfield_program['view'] = view_matrix
        self.starfield_program['projection'] = projection_matrix
        
        self.quad_vao.render()
        
        # Render accretion disk
        self.accretion_disk_program['time'] = self.time
        self.accretion_disk_program['resolution'] = self.window_size
        self.accretion_disk_program['inner_radius'] = float(3.0 * self.black_hole.rs)
        self.accretion_disk_program['outer_radius'] = float(20.0 * self.black_hole.rs)
        self.accretion_disk_program['spin'] = float(self.black_hole.spin)
        self.accretion_disk_program['model'] = Matrix44.identity()
        self.accretion_disk_program['view'] = view_matrix
        self.accretion_disk_program['projection'] = projection_matrix
        
        self.ctx.enable(moderngl.BLEND)
        self.ctx.blend_func = moderngl.SRC_ALPHA, moderngl.ONE_MINUS_SRC_ALPHA
        self.quad_vao.render()
        
        # Render black hole
        self.blackhole_program['time'] = self.time
        self.blackhole_program['camera_position'] = tuple(self.camera.get_position())
        self.blackhole_program['black_hole_position'] = tuple(self.black_hole.position)
        self.blackhole_program['black_hole_radius'] = float(self.black_hole.rh)
        self.blackhole_program['schwarzschild_radius'] = float(self.black_hole.rs)
        self.blackhole_program['spin'] = float(self.black_hole.spin)
        
        # Set textures
        self.starfield_texture.use(0)
        self.blackhole_program['starfield_texture'] = 0
        
        self.accretion_disk_texture.use(1)
        self.blackhole_program['accretion_disk_texture'] = 1
        
        # Set matrices
        model_matrix = Matrix44.identity()
        self.blackhole_program['model'] = model_matrix
        self.blackhole_program['view'] = view_matrix
        self.blackhole_program['projection'] = projection_matrix
        
        # Render event horizon
        self.horizon_vao.render()
        
        # Render gravitational lensing effects
        self.quad_vao.render()
        
        # Blit framebuffer to screen
        self.framebuffer.color_attachments[0].use()
        self.ctx.screen.use()
        self.quad_vao.render()
    
    def capture_screenshot(self, filename):
        """
        Capture a screenshot of the current render.
        
        Args:
            filename: Output filename
        """
        # Read pixels from framebuffer
        data = self.framebuffer.read(components=4, attachment=0)
        
        # Convert to image
        image = Image.frombytes('RGBA', self.window_size, data)
        image = image.transpose(Image.FLIP_TOP_BOTTOM)
        
        # Save image
        image.save(filename)