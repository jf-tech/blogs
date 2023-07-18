# Importing Packages
import numpy as np
from scipy.integrate import odeint
import matplotlib.pyplot as plt

# CR3BP Model
def model_CR3BP(state, t):
    x = state[0]
    y = state[1]
    z = state[2]
    x_dot = state[3]
    y_dot = state[4]
    z_dot = state[5]
    x_ddot = x+2*y_dot-((1-mu)*(x+mu))/((x+mu)**2+y**2+z**2)**(3/2)\
             -(mu*(x-(1-mu)))/((x-(1-mu))**2+y**2+z**2)**(3/2)
    y_ddot = y-2*x_dot-((1-mu)*y)/((x+mu)**2+y**2+z**2)**(3/2)\
             -(mu*y)/((x-(1-mu))**2+y**2+z**2)**(3/2)
    z_ddot = -((1-mu)*z)/((x+mu)**2+y**2+z**2)**(3/2)\
             -(mu*z)/((x-(1-mu))**2+y**2+z**2)**(3/2)
    dstate_dt = [x_dot, y_dot, z_dot, x_ddot, y_ddot, z_ddot]
    return dstate_dt

# Defining ND Parameters
G = 6.67408E-20  # Univ. Gravitational Constant [km3 kg-1 s-2]
mEarth = 5.97219E+24  # Mass of the Earth [kg]
mMoon = 7.34767E+22  # Mass of the Moon [kg]
a = 3.844E+5  # Semi-major axis of Earth and Moon [km]
m1 = mEarth
m2 = mMoon
Mstar = m1+m2  # ND Mass Parameter
Lstar = a  # ND Length Parameter
Tstar = (Lstar**3/(G*Mstar))**(1/2)  # ND Time Parameter
mu = m2/Mstar
print('\u03BC = ' + str(mu))

# Initial Conditions [Initial State Vector]
X_0 = 50000/Lstar  # ND x
Y_0 = 0            # ND y
Z_0 = 0            # ND z
VX_0 = 1.08*Tstar/Lstar  # ND x_dot
VY_0 = 3.18*Tstar/Lstar  # ND y_dot
VZ_0 = 0.68*Tstar/Lstar  # ND z_dot
state_0 = [X_0, Y_0, Z_0, VX_0, VY_0, VZ_0]  # ND ICs

# Time Array
t = np.linspace(0, 15, 1000)  # ND Time

# Numerically Integrating
sol = odeint(model_CR3BP, state_0, t)

# Rotational Frame Position Time History
X_rot = sol[:, 0]
Y_rot = sol[:, 1]
Z_rot = sol[:, 2]

# Inertial Frame Position Time History
X_Iner = sol[:, 0]*np.cos(t) - sol[:, 1]*np.sin(t)
Y_Iner = sol[:, 0]*np.sin(t) + sol[:, 1]*np.cos(t)
Z_Iner = sol[:, 2]

# Constant m1 and m2 Rotational Frame Locations for CR3BP Primaries
m1_loc = [-mu, 0, 0]
m2_loc = [(1-mu), 0, 0]

# Moving m1 and m2 Inertial Locations for CR3BP Primaries
X_m1 = m1_loc[0]*np.cos(t) - m1_loc[1]*np.sin(t)
Y_m1 = m1_loc[0]*np.sin(t) + m1_loc[1]*np.cos(t)
Z_m1 = m1_loc[2]*np.ones(len(t))
X_m2 = m2_loc[0]*np.cos(t) - m2_loc[1]*np.sin(t)
Y_m2 = m2_loc[0]*np.sin(t) + m2_loc[1]*np.cos(t)
Z_m2 = m2_loc[2]*np.ones(len(t))

# Rotating Frame Plot
fig = plt.figure()
ax = plt.axes(projection='3d')

# Adding Figure Title and Labels
ax.set_title('Rotating Frame CR3BP Orbit (\u03BC = ' + str(round(mu, 6)) + ')')
ax.set_xlabel('x [ND]')
ax.set_ylabel('y [ND]')
ax.set_zlabel('z [ND]')

# Plotting Rotating Frame Positions
ax.plot3D(X_rot, Y_rot, Z_rot, c='green')
ax.plot3D(m1_loc[0], m1_loc[1], m1_loc[2], c='blue', marker='o')
ax.plot3D(m2_loc[0], m2_loc[1], m2_loc[2], c='black', marker='o')

# Setting Axis Limits
xyzlim = np.array([ax.get_xlim3d(), ax.get_ylim3d(), ax.get_zlim3d()]).T
XYZlim = np.asarray([min(xyzlim[0]), max(xyzlim[1])])
ax.set_xlim3d(XYZlim)
ax.set_ylim3d(XYZlim)
ax.set_zlim3d(XYZlim * 3 / 4)

# Inertial Frame Plot
fig = plt.figure()
ax = plt.axes(projection='3d')

# Adding Figure Title and Labels
ax.set_title('Inertial Frame CR3BP Orbit (\u03BC = ' + str(round(mu, 6)) + ')')
ax.set_xlabel('X [ND]')
ax.set_ylabel('Y [ND]')
ax.set_zlabel('Z [ND]')

# Plotting Inertial Frame Positions
ax.plot3D(X_Iner, Y_Iner, Z_Iner, c='green')
ax.plot3D(X_m1, Y_m1, Z_m1, c='blue')
ax.plot3D(X_m2, Y_m2, Z_m2, c='black')

# Setting Axis Limits
ax.set_xlim3d(XYZlim)
ax.set_ylim3d(XYZlim)
ax.set_zlim3d(XYZlim * 3 / 4)
plt.show()
