import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.animation import FuncAnimation

# Create figure
fig = plt.figure(figsize=(10, 10))
ax = fig.add_subplot(111, projection='3d')

# Earth parameters
r_earth = 1
u = np.linspace(0, 2 * np.pi, 20)  # Less dense wireframe
v = np.linspace(0, np.pi, 20)      # Less dense wireframe

# Orbit parameters
inclination_deg = 51
inclination = np.deg2rad(inclination_deg)
r_orbit = 1.4

# Define orbit segments for vertical split (y=0 plane)
t_seg1 = np.linspace(0, np.pi/2, 25)         # First solid segment (y >= 0)
t_seg2 = np.linspace(3*np.pi/2, 2*np.pi, 25)  # Second solid segment (y >= 0)
t_dotted = np.linspace(np.pi/2, 3*np.pi/2, 50)  # Dotted segment (y < 0)

# Solid line segments (y >= 0)
x_orbit_seg1 = r_orbit * np.cos(t_seg1)
y_orbit_seg1 = r_orbit * np.sin(t_seg1) * np.cos(inclination)
z_orbit_seg1 = r_orbit * np.sin(t_seg1) * np.sin(inclination)

x_orbit_seg2 = r_orbit * np.cos(t_seg2)
y_orbit_seg2 = r_orbit * np.sin(t_seg2) * np.cos(inclination)
z_orbit_seg2 = r_orbit * np.sin(t_seg2) * np.sin(inclination)

# Dotted line segment (y < 0)
x_orbit_dotted = r_orbit * np.cos(t_dotted)
y_orbit_dotted = r_orbit * np.sin(t_dotted) * np.cos(inclination)
z_orbit_dotted = r_orbit * np.sin(t_dotted) * np.sin(inclination)

# Equator parameters
t_equator = np.linspace(0, 2 * np.pi, 100)
x_equator = r_earth * np.cos(t_equator)
y_equator = r_earth * np.sin(t_equator)
z_equator = np.zeros_like(t_equator)  # z=0 for equator

# Launch site parameters (fixed point on Earth's surface)
site_latitude_deg = 28
theta_site = np.deg2rad(90 - site_latitude_deg)  # Convert latitude to colatitude
phi_site = 0  # Starting longitude
x_site = r_earth * np.sin(theta_site) * np.cos(phi_site)
y_site = r_earth * np.sin(theta_site) * np.sin(phi_site)
z_site = r_earth * np.cos(theta_site)

# Initial plot objects
earth_plot = ax.plot_wireframe(
    r_earth * np.outer(np.cos(u), np.sin(v)),
    r_earth * np.outer(np.sin(u), np.sin(v)),
    r_earth * np.outer(np.ones(np.size(u)), np.cos(v)),
    color='blue',
    alpha=0.2  # See-through wireframe
)
orbit_plot_seg1 = ax.plot(x_orbit_seg1, y_orbit_seg1, z_orbit_seg1, color='red', linewidth=2, linestyle='-')[0]  # Solid segment 1
orbit_plot_seg2 = ax.plot(x_orbit_seg2, y_orbit_seg2, z_orbit_seg2, color='red', linewidth=2, linestyle='-')[0]  # Solid segment 2
orbit_plot_dotted = ax.plot(x_orbit_dotted, y_orbit_dotted, z_orbit_dotted, color='red', linewidth=2, linestyle=':')[0]  # Dotted segment
equator_plot = ax.plot(x_equator, y_equator, z_equator, color='black', linewidth=2, linestyle='-')[0]  # Solid black equator
site_plot = ax.scatter([x_site], [y_site], [z_site], color='lime', s=100)  # Bright green circle

# Set equal aspect ratio and limits
ax.set_box_aspect([1, 1, 1])
ax.set_xlim(-1.5, 1.5)
ax.set_ylim(-1.5, 1.5)
ax.set_zlim(-1.5, 1.5)

# Labels
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')
ax.set_title(f'{inclination_deg}° Inclined Orbit (Press Space to Pause/Resume)')

# Set initial viewing angle ONCE
ax.view_init(elev=20, azim=45)

# Legend
ax.plot([], [], 'b-', label='Earth')
ax.plot([], [], 'r-', label=f'Orbit ({inclination_deg}° inclination, solid)')
ax.plot([], [], 'r:', label=f'Orbit ({inclination_deg}° inclination, dotted)')
ax.plot([], [], 'k-', label='Equator')  # Black solid line for equator
ax.scatter([], [], c='lime', s=100, label='Launch Site')
ax.legend()

# Animation state
paused = False

def update(frame):
    if paused:
        return ax,  # Skip update if paused
    
    # Clear the axes completely
    ax.cla()
    
    # Recreate limits and labels (but not view angle)
    ax.set_box_aspect([1, 1, 1])
    ax.set_xlim(-1.5, 1.5)
    ax.set_ylim(-1.5, 1.5)
    ax.set_zlim(-1.5, 1.5)
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_zlabel('Z')
    ax.set_title(f'{inclination_deg}° Inclined Orbit (Press Space to Pause/Resume)')
    
    # Calculate rotation angle
    angle = frame * 2 * np.pi / 300
    
    # Rotate Earth coordinates
    x_rot = r_earth * np.outer(np.cos(u + angle), np.sin(v))
    y_rot = r_earth * np.outer(np.sin(u + angle), np.sin(v))
    z_rot = r_earth * np.outer(np.ones(np.size(u)), np.cos(v))
    
    # Rotate site coordinates (around Z-axis)
    x_site_rot = r_earth * np.sin(theta_site) * np.cos(phi_site + angle)
    y_site_rot = r_earth * np.sin(theta_site) * np.sin(phi_site + angle)
    z_site_rot = r_earth * np.cos(theta_site)
    
    # Plot rotated Earth, static orbit (split into solid and dotted), equator, and rotated site
    ax.plot_wireframe(x_rot, y_rot, z_rot, color='blue', alpha=0.2)
    ax.plot(x_orbit_seg1, y_orbit_seg1, z_orbit_seg1, color='red', linewidth=2, linestyle='-')  # Solid segment 1
    ax.plot(x_orbit_seg2, y_orbit_seg2, z_orbit_seg2, color='red', linewidth=2, linestyle='-')  # Solid segment 2
    ax.plot(x_orbit_dotted, y_orbit_dotted, z_orbit_dotted, color='red', linewidth=2, linestyle=':')  # Dotted segment
    ax.plot(x_equator, y_equator, z_equator, color='black', linewidth=2, linestyle='-')  # Solid black equator
    ax.scatter([x_site_rot], [y_site_rot], [z_site_rot], color='lime', s=100)
    
    # Redraw legend
    ax.plot([], [], 'b-', label='Earth')
    ax.plot([], [], 'r-', label=f'Orbit ({inclination_deg}° inclination, solid)')
    ax.plot([], [], 'r:', label=f'Orbit ({inclination_deg}° inclination, dotted)')
    ax.plot([], [], 'k-', label='Equator')
    ax.scatter([], [], c='lime', s=100, label='Launch Site')
    ax.legend()
    
    return ax,

# Create animation and store it explicitly
anim = FuncAnimation(fig, update, frames=300, interval=50, blit=False)

# Pausing functionality
def toggle_pause(event):
    global paused
    if event.key == ' ':
        paused = not paused
        print("Animation paused" if paused else "Animation resumed")

# Connect the key press event
fig.canvas.mpl_connect('key_press_event', toggle_pause)

# Enable interactive mode
plt.ion()

# Show the plot
plt.show(block=True)