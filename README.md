This repository contains the 7700R code used during the 2025-2026 VEX Push Back season. This program utilizes the LemLib library with many major changes to the program. Using this program, 7700R was able to qualify and compete at the 2026 VEX World Championship, placing 5th overall in the technology division. This code is open-source and available to anyone; you can contact me on Discord or via email.

Discord: josh.7700r

Email: roe.joshua1015@gmail.com

Movement functions
- RAMSETE
- 2D Special Euclidean (SE2) move to point
- Linear Time-Varying (LTV) Unicycle controller
- Proportional Integral Derivative (PID) control

Furthermore, this program is equipped with a complex intake system that uses a task to run detection on the intake motors simultaneously. This task specifically includes:
- Anti-jam (Torque)
- Color sort
- Button rules
- Piston delays

Work-in-progress algorithms that never made it to the competition field:
- Monte-Carlo Localization (MCL)
- Model Predictive Control (MPC)
