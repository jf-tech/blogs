import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# Define the universal gravitational constant
G = 6.67430e-11  # m^3 kg^-1 s^-2

# Define the properties of the two bodies
mass1 = 5.972e24  # kg (mass of the Earth)
mass2 = 7.348e22  # kg (mass of the Moon)

# Define the initial position vectors
r1 = np.array([0, 0, 0], dtype='float64')
r2 = np.array([3.8e8, 0, 0], dtype='float64')  # approx. Earth-Moon distance

# Define the initial velocities
v1 = np.array([0, 0, 0], dtype='float64')
v2 = np.array([0, 1022, 0], dtype='float64')  # approx. velocity of Moon

# Simulation time parameters
t_start = 0
t_end = 3600 * 24 * 30 * 4 # about 4 months
dt = 3600 # time step, 1 hr


# Calculate the force between the bodies and update their positions and velocities
def update_system(r1, r2, v1, v2):
    # The position vector between the bodies
    r21 = r2 - r1

    # Compute the distance (aka norm) between the bodies
    r = np.linalg.norm(r21, 2)

    # Compute the force
    F = G * mass1 * mass2 * (r2-r1) / r**3

    # Compute the acceleration of each body
    a1 = F / mass1
    a2 = -F / mass2 # Note the negative sign due to Newton's 3rd law.

    # Update velocities
    v1 += a1 * dt
    v2 += a2 * dt

    # Update positions
    r1 += v1 * dt
    r2 += v2 * dt

    return r1, r2, v1, v2


# Store the positions over time
positions1 = []
positions2 = []

# Run the simulation
t = np.arange(t_start, t_end, dt)
for _ in t:
    r1, r2, v1, v2 = update_system(r1, r2, v1, v2)
    positions1.append(np.copy(r1)) # list stores references, thus we have to make copes of r1, r2
    positions2.append(np.copy(r2))

positions1 = np.array(positions1)
positions2 = np.array(positions2)

# Create a new plot for the trajectories
fig = plt.figure(figsize=[6, 6])
ax = fig.add_subplot(111, projection='3d')

# Plot the trajectories of the bodies
plot1, = ax.plot(positions1[:,0], positions1[:,1], positions1[:,2])
plot2, = ax.plot(positions2[:,0], positions2[:,1], positions2[:,2])

def animate(i):
    plot1.set_data(positions1[:i,0], positions1[:i,1])
    plot1.set_3d_properties(positions1[:i,2])  # for the z-data
    plot2.set_data(positions2[:i,0], positions2[:i,1])
    plot2.set_3d_properties(positions2[:i,2])  # for the z-data
    return plot1, plot2,

ani = animation.FuncAnimation(fig, animate, frames=len(t), interval=1, blit=True)
# ani.save('earth-moon.mp4', writer = animation.FFMpegWriter(fps=30))

plt.show()
