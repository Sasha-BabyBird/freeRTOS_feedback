format long
T = 1; %шаг дискретизации
num = [1];
den = [1 2 2];
tfsys_c = tf(num, den); %система в TF-форме
tfsys_d = c2d(tfsys_c, T); %дискретизируем с шагом T
sssys_d = ss(tfsys_d); %переведём систему в SS-форму
A = sssys_d.A
B = sssys_d.B
C = sssys_d.C
D = sssys_d.D;
s_desired_observer = [-1 -1.0001]; %собственные значения для наблюдателя
s_desired_system = [-0.2 + 0.1j -0.2 - 0.1j]; %собственные значения для системы
z_desired_observer = exp(s_desired_observer*T); %дискретизация собственных значений
z_desired_system = exp(s_desired_system*T);
x0 = [5; -5]; %начальные условия системы
v0 = [0; 0]; %нулевые начальные значения наблюдателя
K = place(A, B, z_desired_system)
L_tr = place(A', C', z_desired_observer); 
L = L_tr'
Az = A - B*K - L*C %матрица, описывающая динамику наблюдателя

sim('OMC_for_RDTS.slx') 
model_u = model_u'; %результат моделирования в Simulink
model_y = model_y';
writematrix(model_u);
writematrix(model_y);
rtos_u = readmatrix("model_u_by_RTOS.txt"); %результат расчётов в RTOS
rtos_y = readmatrix("model_y_by_RTOS.txt");

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
