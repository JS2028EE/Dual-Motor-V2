% Dual Motor V2: intended power and GPIO-command behavior.
% Hypothetical model; no measured torque/current/RPM or backfeed simulation.
% Charger USB is disconnected. Correct wiring is assumed.
clear;
clc;
close all;

motor_battery_V = 4.68;       % user-reported earlier; constant in model
electronics_battery_V = 3.65; % user-reported earlier; constant in model
motor_boost_V = 6.0;
electronics_boost_V = 5.0;
boost_efficiency = 0.85;      % assumed for both converters
driver_drop_V = 2.0;         % assumed conducting loss
battery_capacity_Ah = 1.0;
usable_fraction = 0.80;

% Assumed average 5 V load currents, NOT measured specifications.
electronics_base_A = 0.180 + 0.030 + 0.015 + 0.035;
headlights_A = 0.010;
buzzer_A = 0.005;
drive_running_A = 0.250;
steering_running_A = 0.100;

wheel_diameter_m = 0.065; % assumed
wheel_rpm_at_4V = 120;   % assumed loaded wheel RPM at 4 V
response_time_s = 1.5;  % assumed mechanical response

t = 0:0.05:20;
electronics_on = t < 18;
motor_switch_on = t < 18;
command = zeros(size(t));
command(t >= 2 & t < 8) = 0.60;
command(t >= 10 & t < 16) = -0.50;
enable = (command ~= 0) & electronics_on & motor_switch_on;
input1 = enable & (command > 0);
input2 = enable & (command < 0);
duty = abs(command) .* enable;
headlights_on = electronics_on;
buzzer_on = electronics_on & (t >= 8 & t < 9);
steering_active = enable & (t >= 4 & t < 5);

motor_on_voltage = max(0, motor_boost_V - driver_drop_V);
% Approximation only; PWM freewheel/current dynamics omitted.
motor_average_V = sign(command) .* duty * motor_on_voltage;
electronics_output_A = electronics_base_A .* electronics_on ...
    + headlights_A .* headlights_on + buzzer_A .* buzzer_on;
electronics_input_A = electronics_boost_V .* electronics_output_A ...
    ./ (electronics_battery_V * boost_efficiency);
% Constant current when enabled: not proportional to duty in this model.
drive_A = drive_running_A .* enable;
steering_A = steering_running_A .* steering_active;
motor_pack_A = motor_boost_V .* (drive_A + steering_A) ...
    ./ (motor_battery_V * boost_efficiency);
driver_heat_W = driver_drop_V .* (drive_A + steering_A);

target_rpm = wheel_rpm_at_4V .* motor_average_V / 4;
target_speed = pi * wheel_diameter_m .* target_rpm / 60;
speed = zeros(size(t));
for k = 2:length(t)
    dt = t(k) - t(k-1);
    speed(k) = speed(k-1) ...
        + (target_speed(k) - speed(k-1)) * dt / response_time_s;
end
position = cumtrapz(t, speed);
travelled = trapz(t, abs(speed));
electronics_used_mAh = trapz(t, electronics_input_A) / 3.6;
motor_used_mAh = trapz(t, motor_pack_A) / 3.6;
steady_output_A = electronics_base_A + headlights_A;
steady_input_A = electronics_boost_V * steady_output_A ...
    / (electronics_battery_V * boost_efficiency);
electronics_runtime_h = battery_capacity_Ah * usable_fraction ...
    / steady_input_A;

fprintf("EXAMPLE MODEL: assumed currents, RPM, and driver loss.\n\n");
fprintf("Motor voltage during PWM ON pulses: %.2f V\n", motor_on_voltage);
fprintf("Estimated peak electronics battery current: %.0f mA\n", ...
    max(electronics_input_A) * 1000);
fprintf("Estimated peak motor-pack current: %.0f mA\n", ...
    max(motor_pack_A) * 1000);
fprintf("Estimated peak driver conduction heat: %.2f W\n", max(driver_heat_W));
fprintf("Electronics charge used over 20 s: %.2f mAh\n", electronics_used_mAh);
fprintf("Motor-pack charge used over 20 s: %.2f mAh\n", motor_used_mAh);
fprintf("1000 mAh electronics runtime, steady example: %.2f h\n", ...
    electronics_runtime_h);
fprintf("Total distance travelled: %.2f m\n", travelled);
fprintf("Final signed position: %.2f m\n", position(end));
fprintf("\nStartup surges, battery sag, charging, and backfeed omitted.\n");

subplot(3, 1, 1);
stairs(t, command, "b", "linewidth", 2);
hold on;
stairs(t, double(enable), "k--", "linewidth", 1);
legend("Signed PWM command", "Enable");
ylabel("Command / enable");
title("Forward, stop, reverse, stop; power off at 18 s");
grid on;
subplot(3, 1, 2);
plot(t, speed, "r", "linewidth", 2);
ylabel("Speed (m/s)");
title("Hypothetical speed; negative means reverse");
grid on;
subplot(3, 1, 3);
plot(t, electronics_input_A * 1000, "b", "linewidth", 2);
hold on;
plot(t, motor_pack_A * 1000, "r", "linewidth", 2);
legend("Electronics battery", "Motor battery pack");
xlabel("Time (seconds)");
ylabel("Battery current (mA)");
grid on;
