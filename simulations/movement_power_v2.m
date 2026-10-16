% Dual Motor V2: hypothetical movement and electrical estimates.
% No electrical-to-mechanical coupling yet. All unmeasured inputs are examples.
clear;
clc;
close all;

wheel_diameter = 0.065; % m, assumed
wheel_rpm = 120;        % loaded wheel RPM, assumed
response_time = 1.5;    % s, prescribed curve, not derived from torque
motor_pack_voltage = 4.5;
boost_output = 6.0;
boost_efficiency = 0.85;
driver_drop = 2.0;      % assumed, load dependent
drive_current = 0.250;
steer_current = 0.100;  % assumes both motors active

% Assumed average currents on regulated 5 V rail.
esp_current = 0.180;
gps_current = 0.030;
ultrasonic_current = 0.015;
headlights_current = 0.010;
buzzer_current = 0.005;
driver_logic_current = 0.035;
electronics_rail = 5.0;
battery_voltage = 3.7;
electronics_efficiency = 0.85;
usable_fraction = 0.80;

motor_current = drive_current + steer_current;
direct_motor_voltage = motor_pack_voltage - driver_drop;
boosted_motor_voltage = boost_output - driver_drop;
boost_input_current = boost_output * motor_current ...
    / (motor_pack_voltage * boost_efficiency);
driver_heat = driver_drop * motor_current;
electronics_current = esp_current + gps_current + ultrasonic_current ...
    + headlights_current + buzzer_current + driver_logic_current;
electronics_power = electronics_rail * electronics_current;
battery_current = electronics_power ...
    / (battery_voltage * electronics_efficiency);
capacities_Ah = [1.000, 0.500];
runtime_hours = capacities_Ah * usable_fraction / battery_current;

top_speed = pi * wheel_diameter * wheel_rpm / 60;
t = 0:0.05:10;
speed = top_speed * (1 - exp(-t / response_time));
distance = cumtrapz(t, speed);

fprintf("EXAMPLE INPUTS ONLY; not measured vehicle performance.\n");
fprintf("Direct motor voltage estimate: %.2f V\n", direct_motor_voltage);
fprintf("Boosted motor voltage estimate: %.2f V\n", boosted_motor_voltage);
fprintf("Both motors active, output load: %.0f mA\n", motor_current * 1000);
fprintf("Motor-pack current with boost: %.0f mA\n", boost_input_current * 1000);
fprintf("Driver motor-path heat estimate: %.2f W\n", driver_heat);
fprintf("Electronics at 5 V: %.0f mA\n", electronics_current * 1000);
fprintf("Electronics battery current: %.0f mA\n", battery_current * 1000);
fprintf("1000 mAh electronics runtime: %.2f h\n", runtime_hours(1));
fprintf("500 mAh electronics runtime: %.2f h\n", runtime_hours(2));
fprintf("Assumed final wheel speed: %.2f m/s\n", top_speed);
fprintf("Distance at 10 seconds: %.2f m\n", distance(end));
fprintf("Movement is independent of voltage/current in this first model.\n");
if direct_motor_voltage < 3
    fprintf("FLAG: direct motor voltage estimate is below motor's 3 V rating.\n");
end
if motor_pack_voltage < electronics_rail
    fprintf("FLAG: direct 4.5 V motor rail is below 5 V L293D logic rail.\n");
end

subplot(2, 1, 1);
plot(t, speed, "b", "linewidth", 2);
xlabel("Time (seconds)");
ylabel("Speed (m/s)");
title("Hypothetical movement: measured RPM and torque still needed");
grid on;
subplot(2, 1, 2);
plot(t, distance, "r", "linewidth", 2);
xlabel("Time (seconds)");
ylabel("Distance (meters)");
grid on;
