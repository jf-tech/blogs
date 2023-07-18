import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# Define the universal gravitation constant
G = 6.67430e-11  # m^3 kg^-1 s^-2

# Define the properties of the three bodies (approximate)
# 0.122
masses = np.array([1.1, 0.907, 0]) * 1.98847e30  # kg (Alpha Centauri A, B and Proxima Centauri)
dist_AB = 35.6e9  # meters, approximate average distance between A and B
dist_AC = 0 # 43000e9  # meters, approximate average distance between A and Proxima Centauri
v_AB = 22000  # m/s, velocity of B relative to A
v_AC = 5460  # m/s, velocity of Proxima relative to A

# Define the initial position vectors
r = np.array([[0, 0, 0], [dist_AB, 0, 0], [0, dist_AC, 0]], dtype='float64')

# Define the initial velocities
v = np.array([[0, 0, 0], [0, v_AB, 0], [0, 0, v_AC]], dtype='float64')

# Store the positions over time
positions = [np.copy(r)]

# Calculate the force between the bodies and update their positions and velocities
def update_system(r, v, masses, dt):
    # Compute the forces between the bodies
    a = np.zeros((3, 3))
    for i in range(3):
        for j in range(3):
            if i != j:
                rij = r[j] - r[i]
                rij_norm = np.linalg.norm(rij)
                a[i] += G * masses[j] / rij_norm**3 * rij

    # Update velocities
    v += a * dt

    # Update positions
    r += v * dt

    return r, v

# Simulation time parameters
t_start = 0
t_end = 5e15
dt = 1e12  # time step

t = np.arange(t_start, t_end, dt)

# Run the simulation
for _ in t:
    r, v = update_system(r, v, masses, dt)
    positions.append(np.copy(r))

positions = np.array(positions)

# Create a new plot for the trajectories
fig = plt.figure(figsize=[6, 6])
ax = fig.add_subplot(111, projection='3d')

# Plot the trajectories of the bodies
plots = [ax.plot(pos[:,0], pos[:,1], pos[:,2])[0] for pos in positions.transpose(1, 0, 2)]

def animate(i):
    for j, plot in enumerate(plots):
        plot.set_data(positions[:i,j,0], positions[:i,j,1])
        plot.set_3d_properties(positions[:i,j,2])  # for the z-data

print("frames: ", len(t))
ani = animation.FuncAnimation(fig, animate, frames=len(t), interval=1, blit=False)

plt.show()
