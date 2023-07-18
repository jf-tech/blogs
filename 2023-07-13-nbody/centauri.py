import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# the universal gravitation constant
G = 6.6743e-11  # m^3 kg^-1 s^-2

# N-body masses
# [0]: Alpha Centauri A, 2.15e30 (kg)
# [1]: Alpha Centauri B, 1.81e30 (kg)
# [2]: Proxima Centauri , 2.43e29 (kg)
#masses = np.array([2.15e30, 1.81e30, 2.43e29], dtype='float64')
#masses = np.array([2.15e30, 1.81e30], dtype='float64')
masses = np.array([1.989e30, 5.972e24], dtype='float64') #  sun earth

# N-body position vectors
# [0]: Alpha Centauri A, center
# [1]: Alpha Centauri B, 5.4e12 (m)  (A, B distance from 11.2 to 35.6 AU, here we use the 35.6 AU)
# [2]: Proxima Centauri , 1.9e15 (m) (Proxima to AB distance max 13,000 AU)
#positions = np.array([[0, 0], [5.4e12, 0], [2.7e12, 1.9e15]], dtype='float64')
#positions = np.array([[0, 0], [5.4e12, 0]], dtype='float64')
positions = np.array([[0, 0], [1.5e11, 0]], dtype='float64') # sun earth

# N-body velocity vectors
# [0]: Alpha Centauri A, -2 (km/s)   A as a radial
# [1]: Alpha Centauri B, +2 (km/s)
# [2]: Proxima Centauri 600 m/s
# velocities = np.array([[0, -2e3], [0, 2e3], [600, 0]], dtype='float64')
#velocities = np.array([[0, -3e3], [0, 3e3]], dtype='float64')
velocities = np.array([[0, 0], [0, 29800]], dtype='float64') # sun earth

n = len(velocities)

# Simulation time parameters
t_start = 0
t_end = 2e9
dt = 2e6  # time step

# Animation parameters
tail_len = 80

# Store the positions history over time
positions_hist = []

# Calculate the force between the bodies and update their positions and velocities
def update_system():
    global positions, velocities
    acc = np.zeros((n, 2))
    for i in range(n):
        for j in range(n):
            if i != j:
                rij = positions[j] - positions[i]
                rij_norm = np.linalg.norm(rij)
                acc[i] += G * masses[j] / rij_norm**3 * rij

    velocities += acc * dt
    positions += velocities * dt

t = np.arange(t_start, t_end, dt)

# Run the simulation
for _ in t:
    update_system()
    positions_hist.append([np.copy(p) for p in positions])

positions_hist = np.array(positions_hist)

# Create a new plot for the trajectories
fig = plt.figure(figsize=[7, 7])
ax = fig.add_subplot(111)

# Plot the trajectories of the N bodies
plots = []
for i in range(n):
    plot, = ax.plot(positions_hist[:,i,0], positions_hist[:,i,1])
    plots.append(plot)

def animate(frame):
    start = 0 if tail_len <= 0 or frame < tail_len else frame - tail_len
    for i in range(n):
        plots[i].set_data(positions_hist[start:frame,i,0], positions_hist[start:frame,i,1])
    return plots

ani = animation.FuncAnimation(fig, animate, frames=len(t), interval=1, blit=True)
# ani.save('2body.mp4', writer = animation.FFMpegWriter(fps=3000))

plt.show()
