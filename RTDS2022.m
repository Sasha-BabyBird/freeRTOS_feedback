%format long

%model construction
T = 1; %sample time
num = 1;
den = [1 2 2];
tfsys_c = tf(num, den); %model in TF form
tfsys_d = c2d(tfsys_c, T); %discrete model in TF form
sssys_d = ss(tfsys_d); %discrete model in SS form
A = sssys_d.A
B = sssys_d.B
C = sssys_d.C
D = sssys_d.D;

%target eigenvalues 
s_desired_observer = [-1 -1.0001]; 
s_desired_system = [-0.2 + 0.1j -0.2 - 0.1j]; 
z_desired_observer = exp(s_desired_observer*T); 
z_desired_system = exp(s_desired_system*T);

%initial conditions and computed matrices
x0 = [5; -5]; 
v0 = [0; 0]; 
K = place(A, B, z_desired_system)
L_tr = place(A', C', z_desired_observer); 
L = L_tr'
Az = A - B*K - L*C %matrix which describes the dynamics of the observer

%Simulink modeling result
sim('OMC_for_RDTS.slx') 
model_u = model_u'; 
model_y = model_y';
writematrix(model_u);
writematrix(model_y);

%FreeRTOS computation result
rtos_u = readmatrix("model_u_by_RTOS.txt"); 
rtos_y = readmatrix("model_y_by_RTOS.txt");

%MATLAB computation result for verification
matlab_u = zeros(1, 50);
matlab_y = zeros(1, 50);
x = x0;
z = v0;
u = 0;
for i = 1:50
    x_new = A*x + B*u;
    y = C*x;
    x = x_new;
    matlab_y(i) = y;
    z_new = Az*z + L*y;
    u = -K*z;
    z = z_new;
    matlab_u(i) = u;
end

%discrepancy between MATLAB and FreeRTOS computation results
norm1 = norm(matlab_u(1:49) - rtos_u(2:end));
norm2 = norm(matlab_y - rtos_y);
matlab_vs_RTOS_dscr = max(norm1, norm2)

%plotting
tiledlayout(2, 2);

u_plot = nexttile;
u_plot_X = 0:48;
u_plot_Y = rtos_u(2:end);
hold on
stairs(u_plot, u_plot_X, u_plot_Y);
u_plot_Y = model_u(1:end-2);
stairs(u_plot, u_plot_X, u_plot_Y);
hold off
title(u_plot, 'u(t)');
legend(u_plot, {'RTOS Model', 'Simulink Model'});

u_err_plot = nexttile;
stairs(u_err_plot, 0:48, rtos_u(2:end) - model_u(1:end-2));
title(u_err_plot, 'u(t) error');

y_plot = nexttile;
y_plot_X = 0:49;
y_plot_Y = rtos_y;
hold on
stairs(y_plot, y_plot_X, y_plot_Y);
y_plot_Y = model_y(1:end-1);
stairs(y_plot, y_plot_X, y_plot_Y);
hold off
title(y_plot, 'y(t)');
legend(y_plot, {'RTOS Model', 'Simulink Model'});

y_err_plot = nexttile;
stairs(y_err_plot, 0:49, rtos_y - model_y(1:end-1));
title(y_err_plot, 'y(t) error');

% u0 = 0.0;
% x1 = A*x0 + B*u0
% y0 = C*x0
% v1 = Az*v0 + L*y0
% u1 = -K*v0
% x2 = A*x1 + B*u1
% y1 = C*x1
% v2 = Az*v1 + L*y1
% u2 = -K*v1
% x3 = A*x2 + B*u2
% y2 = C*x2
% v3 = Az*v2 + L*y2
% u3 = -K*v2
