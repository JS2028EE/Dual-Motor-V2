% DUAL MOTOR V2 - PORTABLE BATTERY + UPDATED BLE CONTROL MODEL
% GNU Octave / MATLAB compatible; no extra packages required.
%
% Four figures:
%   1. Steering-only pulses (left, then right)
%   2. Forward driving only
%   3. Forward + left pulse + forward + right pulse
%   4. Motor/electronics power paths and estimated losses
%
% IMPORTANT:
% This is a transparent engineering estimate, NOT measured telemetry.
% See simulations/PORTABLE_BATTERY_CONTROL_V3.md for assumptions.
% Replace assumptions only when supported by measurements/datasheets.

clear;
clc;
close all;

%% 1. BATTERIES AND CONVERTERS

% Motor branch: one rechargeable 3.7 V, 1000 mAh cell -> boost -> 6 V target
motor_cell_V = 3.70;              % nominal assumption
motor_capacity_Ah = 1.000;        % 1000 mAh
motor_usable_fraction = 0.80;     % planning assumption, not a cell guarantee
motor_source_R = 0.15;            % cell + protection + switch + wiring (assumed)

motor_boost_nominal_V = 6.00;     % intended motor rail
motor_boost_efficiency = 0.85;    % assumed, not measured
motor_boost_output_R = 0.25;      % output droop equivalent (assumed placeholder)

% Electronics branch: separate 3.7 V, 500 mAh cell -> boost -> 5 V
electronics_cell_V = 3.70;
electronics_capacity_Ah = 0.500;
electronics_usable_fraction = 0.80;
electronics_source_R = 0.15;      % cell + protection + switch + wiring (assumed)

electronics_boost_V = 5.00;
electronics_boost_efficiency = 0.85; % assumed, not measured

%% 2. L293D, MOTORS, AND CONTROL ASSUMPTIONS

% Starting current estimates retained from the previous model.
% These are assumed average currents at the 6 V motor supply rail.
drive_motor_current_A = 0.250;
steering_motor_current_A = 0.100;

% Approximate total conducting drop through the L293D motor-current path.
% This is a placeholder; actual drop depends on current, temperature,
% supply voltage, and the specific L293D device.
L293D_path_drop_V = 2.00;

% Firmware/website behavior documented for this project:
% steering direction change delay ~50 ms; steering pulse ~200 ms.
steering_delay_s = 0.050;
steering_pulse_s = 0.200;

% Steering current is treated as an average during the powered pulse.
% Do not multiply by PWM again: this 100 mA assumption already represents
% the average current during the 50%-PWM steering pulse.

% Simple speed estimate assumptions; not measured vehicle performance.
wheel_diameter_m = 0.065;
wheel_RPM_at_4V = 120;
mechanical_response_s = 1.50;

%% 3. ELECTRONICS LOAD ON THE SEPARATE 5 V RAIL

% Assumed average currents; update when measured.
ESP32_BLE_current_A = 0.180;
L293D_logic_current_A = 0.035;
other_confirmed_electronics_A = 0.000;

electronics_load_A = ESP32_BLE_current_A ...
                   + L293D_logic_current_A ...
                   + other_confirmed_electronics_A;

electronics_output_power_W = electronics_boost_V * electronics_load_A;
electronics_boost_input_power_W = ...
    electronics_output_power_W / electronics_boost_efficiency;

[electronics_battery_current_A, electronics_battery_terminal_V] = ...
    battery_input_model(electronics_cell_V, electronics_source_R, ...
                        electronics_boost_input_power_W);

%% 4. TIME BASE

dt = 0.01;
t = 0:dt:10;

%% 5. TEST 1 - STEERING ONLY

% Left command at 1.00 s, right command at 4.00 s.
% Each command waits 50 ms, then applies a 200 ms steering pulse.
steering_command = zeros(size(t));

left_start = 1.00 + steering_delay_s;
right_start = 4.00 + steering_delay_s;

left_mask = (t >= left_start) & ...
            (t < left_start + steering_pulse_s);
right_mask = (t >= right_start) & ...
             (t < right_start + steering_pulse_s);

steering_command(left_mask) = 1;
steering_command(right_mask) = -1;

steering_active = abs(steering_command);
steering_rail_current_A = steering_motor_current_A .* steering_active;

steering_rail_voltage_V = max(0, motor_boost_nominal_V ...
    - motor_boost_output_R .* steering_rail_current_A);

steering_rail_power_W = steering_rail_voltage_V .* ...
                        steering_rail_current_A;

steering_boost_input_power_W = ...
    steering_rail_power_W ./ motor_boost_efficiency;

[steering_battery_current_A, steering_battery_terminal_V] = ...
    battery_input_model(motor_cell_V, motor_source_R, ...
                        steering_boost_input_power_W);

steering_motor_terminal_V = max(0, ...
    steering_rail_voltage_V - L293D_path_drop_V);

%% 6. TEST 2 - FORWARD DRIVING ONLY

% Forward held for 8 seconds, then STOP.
drive_command = double(t < 8.00);

drive_rail_current_A = drive_motor_current_A .* drive_command;

drive_rail_voltage_V = max(0, motor_boost_nominal_V ...
    - motor_boost_output_R .* drive_rail_current_A);

drive_rail_power_W = drive_rail_voltage_V .* drive_rail_current_A;

drive_boost_input_power_W = ...
    drive_rail_power_W ./ motor_boost_efficiency;

[drive_battery_current_A, drive_battery_terminal_V] = ...
    battery_input_model(motor_cell_V, motor_source_R, ...
                        drive_boost_input_power_W);

drive_motor_terminal_V = max(0, ...
    drive_rail_voltage_V - L293D_path_drop_V);

drive_rpm_target = wheel_RPM_at_4V .* ...
                   (drive_motor_terminal_V / 4.0) .* drive_command;

drive_speed_target_mps = pi * wheel_diameter_m .* ...
                         drive_rpm_target / 60;

drive_speed_mps = first_order_response(t, drive_speed_target_mps, ...
                                       mechanical_response_s);

drive_distance_m = cumtrapz(t, drive_speed_mps);

%% 7. TEST 3 - COMBINED FORWARD + LEFT + FORWARD + RIGHT

% 0-8 s: forward remains commanded.
% 2 s: left steering command; 50 ms delay then 200 ms pulse.
% 5 s: right steering command; 50 ms delay then 200 ms pulse.
% 8 s: STOP.
combined_drive_command = double(t < 8.00);

combined_steering_command = zeros(size(t));

combined_left_mask = (t >= 2.00 + steering_delay_s) & ...
    (t < 2.00 + steering_delay_s + steering_pulse_s);

combined_right_mask = (t >= 5.00 + steering_delay_s) & ...
    (t < 5.00 + steering_delay_s + steering_pulse_s);

combined_steering_command(combined_left_mask) = 1;
combined_steering_command(combined_right_mask) = -1;

combined_steering_active = abs(combined_steering_command);

combined_drive_current_A = drive_motor_current_A .* ...
                           combined_drive_command;

combined_steering_current_A = steering_motor_current_A .* ...
                              combined_steering_active;

combined_motor_rail_current_A = combined_drive_current_A + ...
                                combined_steering_current_A;

combined_motor_rail_voltage_V = max(0, motor_boost_nominal_V ...
    - motor_boost_output_R .* combined_motor_rail_current_A);

combined_motor_rail_power_W = combined_motor_rail_voltage_V .* ...
                              combined_motor_rail_current_A;

combined_boost_input_power_W = ...
    combined_motor_rail_power_W ./ motor_boost_efficiency;

[combined_battery_current_A, combined_battery_terminal_V] = ...
    battery_input_model(motor_cell_V, motor_source_R, ...
                        combined_boost_input_power_W);

combined_motor_terminal_V = max(0, ...
    combined_motor_rail_voltage_V - L293D_path_drop_V);

combined_rpm_target = wheel_RPM_at_4V .* ...
    (combined_motor_terminal_V / 4.0) .* combined_drive_command;

combined_speed_target_mps = pi * wheel_diameter_m .* ...
                            combined_rpm_target / 60;

combined_speed_mps = first_order_response(t, ...
    combined_speed_target_mps, mechanical_response_s);

combined_distance_m = cumtrapz(t, combined_speed_mps);

%% 8. POWER FLOW AND EFFICIENCY AT COMBINED PEAK LOAD

% Pick a sample where forward drive and steering overlap.
overlap_index = find(combined_motor_rail_current_A > ...
    drive_motor_current_A, 1, "first");

if isempty(overlap_index)
    error("Combined drive/steering overlap was not generated.");
end

Irail = combined_motor_rail_current_A(overlap_index);
Vrail = combined_motor_rail_voltage_V(overlap_index);
Vbattery = combined_battery_terminal_V(overlap_index);
Ibattery = combined_battery_current_A(overlap_index);

Psource = motor_cell_V * Ibattery;
Pbattery_internal_loss = Ibattery^2 * motor_source_R;
Pboost_input = combined_boost_input_power_W(overlap_index);
Pboost_loss = Pboost_input - combined_motor_rail_power_W(overlap_index);
Pboost_output_resistance_loss = ...
    (motor_boost_nominal_V - Vrail) * Irail;
PL293D_loss = L293D_path_drop_V * Irail;
Pmotor_terminals = max(0, Vrail - L293D_path_drop_V) * Irail;

motor_path_efficiency_percent = 100 * Pmotor_terminals / Psource;

electronics_source_power_W = electronics_cell_V * ...
                             electronics_battery_current_A;
electronics_battery_loss_W = ...
    electronics_battery_current_A^2 * electronics_source_R;

electronics_path_efficiency_percent = ...
    100 * electronics_output_power_W / electronics_source_power_W;

%% 9. ESTIMATED CHARGE USE AND CONTINUOUS-LOAD RUNTIME SCENARIOS

motor_charge_steering_mAh = ...
    trapz(t, steering_battery_current_A) / 3.6;

motor_charge_driving_mAh = ...
    trapz(t, drive_battery_current_A) / 3.6;

motor_charge_combined_mAh = ...
    trapz(t, combined_battery_current_A) / 3.6;

electronics_charge_mAh = ...
    electronics_battery_current_A * (t(end) - t(1)) / 3.6;

drive_continuous_I = max(drive_battery_current_A);
steering_continuous_I = max(steering_battery_current_A);
combined_continuous_I = max(combined_battery_current_A);

runtime_drive_h = motor_capacity_Ah * motor_usable_fraction / ...
                  max(drive_continuous_I, eps);

runtime_steering_h = motor_capacity_Ah * motor_usable_fraction / ...
                     max(steering_continuous_I, eps);

runtime_combined_h = motor_capacity_Ah * motor_usable_fraction / ...
                     max(combined_continuous_I, eps);

runtime_electronics_h = electronics_capacity_Ah * ...
    electronics_usable_fraction / max(electronics_battery_current_A, eps);

%% 10. PRINT MODEL INPUTS AND RESULTS

fprintf("\n====================================================\n");
fprintf("DUAL MOTOR V2 - PORTABLE BATTERY CONTROL MODEL V3\n");
fprintf("====================================================\n");

fprintf("\nMODEL STATUS\n");
fprintf("All currents, resistances, efficiencies and motor speed inputs\n");
fprintf("are assumptions unless explicitly described as user-reported.\n");

fprintf("\nBATTERY CONFIGURATION\n");
fprintf("Motor battery: %.2f V nominal, %.0f mAh\n", ...
    motor_cell_V, motor_capacity_Ah * 1000);
fprintf("Electronics battery: %.2f V nominal, %.0f mAh\n", ...
    electronics_cell_V, electronics_capacity_Ah * 1000);
fprintf("Motor boost target: %.2f V\n", motor_boost_nominal_V);
fprintf("Electronics boost target: %.2f V\n", electronics_boost_V);

fprintf("\nASSUMED MOTOR-RAIL CURRENTS\n");
fprintf("Drive only: %.0f mA\n", drive_motor_current_A * 1000);
fprintf("Steering only during pulse: %.0f mA\n", ...
    steering_motor_current_A * 1000);
fprintf("Drive + steering overlap: %.0f mA\n", ...
    (drive_motor_current_A + steering_motor_current_A) * 1000);

fprintf("\nPREDICTED MOTOR BATTERY CURRENT\n");
fprintf("Steering-only peak: %.0f mA\n", ...
    max(steering_battery_current_A) * 1000);
fprintf("Driving-only peak: %.0f mA\n", ...
    max(drive_battery_current_A) * 1000);
fprintf("Combined peak: %.0f mA\n", ...
    max(combined_battery_current_A) * 1000);
fprintf("Combined battery terminal voltage at overlap: %.2f V\n", ...
    Vbattery);

fprintf("\nMOTOR VOLTAGES AT COMBINED OVERLAP\n");
fprintf("Boost rail after assumed output droop: %.2f V\n", Vrail);
fprintf("Estimated motor terminal voltage after L293D: %.2f V\n", ...
    max(0, Vrail - L293D_path_drop_V));

fprintf("\nELECTRONICS BRANCH\n");
fprintf("Assumed 5 V load: %.0f mA\n", electronics_load_A * 1000);
fprintf("Predicted electronics battery current: %.0f mA\n", ...
    electronics_battery_current_A * 1000);
fprintf("Predicted electronics battery terminal voltage: %.2f V\n", ...
    electronics_battery_terminal_V);

fprintf("\nCHARGE USED IN EACH 10-SECOND TEST\n");
fprintf("Steering-only: %.3f mAh\n", motor_charge_steering_mAh);
fprintf("Driving-only: %.3f mAh\n", motor_charge_driving_mAh);
fprintf("Combined: %.3f mAh\n", motor_charge_combined_mAh);
fprintf("Electronics branch over 10 s: %.3f mAh\n", electronics_charge_mAh);

fprintf("\nCONTINUOUS-LOAD RUNTIME SCENARIOS (NOT DRIVE CYCLE PREDICTIONS)\n");
fprintf("Motor battery, continuous drive assumption: %.2f h\n", runtime_drive_h);
fprintf("Motor battery, continuous steering assumption: %.2f h\n", runtime_steering_h);
fprintf("Motor battery, continuous combined assumption: %.2f h\n", runtime_combined_h);
fprintf("Electronics battery, continuous assumed load: %.2f h\n", runtime_electronics_h);

fprintf("\nPOWER FLOW AT DRIVE + STEERING OVERLAP\n");
fprintf("Battery source power: %.3f W\n", Psource);
fprintf("Battery internal/wiring loss: %.3f W\n", Pbattery_internal_loss);
fprintf("Boost converter loss: %.3f W\n", Pboost_loss);
fprintf("Boost output droop-equivalent loss: %.3f W\n", ...
    Pboost_output_resistance_loss);
fprintf("L293D motor-path loss: %.3f W\n", PL293D_loss);
fprintf("Power at motor terminals: %.3f W\n", Pmotor_terminals);
fprintf("Estimated motor-branch source-to-motor efficiency: %.1f%%\n", ...
    motor_path_efficiency_percent);
fprintf("Estimated electronics source-to-5V-load efficiency: %.1f%%\n", ...
    electronics_path_efficiency_percent);

fprintf("\nLIMITATIONS\n");
fprintf("- No measured motor current, stall current, torque, or RPM is available.\n");
fprintf("- Battery discharge rating and actual internal resistance are unverified.\n");
fprintf("- Boost efficiency and output droop are assumptions, not module tests.\n");
fprintf("- L293D voltage drop varies with current and temperature.\n");
fprintf("- Startup surges, thermal behavior, PWM switching, and motor inductance\n");
fprintf("  are not modeled.\n");
fprintf("- Capacitors are not credited with sustained power; exact values/ESR\n");
fprintf("  and transient placement are not confirmed here.\n");
fprintf("- TP4056 charging behavior is not modeled during operation.\n");
fprintf("- GPS/ultrasonic sensors are excluded until their installed loads are known.\n");
fprintf("- Speed is a simplified voltage-to-RPM estimate, not a validated\n");
fprintf("  vehicle dynamics model.\n");

fprintf("\n====================================================\n");
fprintf("FOUR FIGURES GENERATED\n");
fprintf("====================================================\n");

%% 11. FIGURE 1 - STEERING ONLY

figure(1);

subplot(3,1,1);
stairs(t, steering_command, "linewidth", 1.8);
ylim([-1.4 1.4]);
yticks([-1 0 1]);
yticklabels({"Right", "Off", "Left"});
ylabel("Direction");
title("Figure 1 - Steering only: left and right pulses");
grid on;

subplot(3,1,2);
plot(t, steering_rail_current_A * 1000, "linewidth", 1.8);
ylabel("Motor-rail current (mA)");
title("Assumed steering motor current");
grid on;

subplot(3,1,3);
plot(t, steering_battery_current_A * 1000, "linewidth", 1.8);
hold on;
plot(t, steering_battery_terminal_V, "linewidth", 1.8);
xlabel("Time (s)");
ylabel("Current (mA) / Voltage (V)");
legend("Motor battery current", "Motor battery terminal voltage");
title("Motor battery response during steering");
grid on;

%% 12. FIGURE 2 - DRIVING ONLY

figure(2);

subplot(3,1,1);
stairs(t, drive_command, "linewidth", 1.8);
ylim([-0.2 1.2]);
ylabel("Forward command");
title("Figure 2 - Forward driving only");
grid on;

subplot(3,1,2);
plot(t, drive_speed_mps, "linewidth", 1.8);
ylabel("Estimated speed (m/s)");
title("Simplified speed response");
grid on;

subplot(3,1,3);
plot(t, drive_battery_current_A * 1000, "linewidth", 1.8);
hold on;
plot(t, drive_motor_terminal_V .* drive_command, "linewidth", 1.8);
xlabel("Time (s)");
ylabel("Current (mA) / Voltage (V)");
legend("Motor battery current", "Motor terminal voltage");
title("Driving electrical response");
grid on;

%% 13. FIGURE 3 - COMBINED DRIVE AND STEERING

figure(3);

subplot(4,1,1);
stairs(t, combined_drive_command, "linewidth", 1.8);
hold on;
stairs(t, combined_steering_command, "linewidth", 1.8);
ylim([-1.4 1.4]);
ylabel("Command");
legend("Forward", "Steering direction");
title("Figure 3 - Forward, left, forward, then right");
grid on;

subplot(4,1,2);
plot(t, combined_speed_mps, "linewidth", 1.8);
ylabel("Estimated speed (m/s)");
title("Estimated forward speed while steering");
grid on;

subplot(4,1,3);
plot(t, combined_motor_rail_current_A * 1000, "linewidth", 1.8);
ylabel("Motor-rail current (mA)");
title("Combined drive and steering current");
grid on;

subplot(4,1,4);
plot(t, combined_battery_current_A * 1000, "linewidth", 1.8);
hold on;
plot(t, combined_battery_terminal_V, "linewidth", 1.8);
xlabel("Time (s)");
ylabel("Current (mA) / Voltage (V)");
legend("Motor battery current", "Motor battery terminal voltage");
title("Motor battery response during combined control");
grid on;

%% 14. FIGURE 4 - POWER BREAKDOWN AND EFFICIENCY

figure(4);

subplot(2,1,1);
loss_and_output_W = [ ...
    Pbattery_internal_loss, ...
    Pboost_loss, ...
    Pboost_output_resistance_loss, ...
    PL293D_loss, ...
    Pmotor_terminals];

bar(loss_and_output_W);
set(gca, "xtick", 1:5);
set(gca, "xticklabel", { ...
    "Battery/wiring loss", ...
    "Boost loss", ...
    "Boost output droop", ...
    "L293D loss", ...
    "Motor terminal power"});
ylabel("Power (W)");
title("Motor branch power flow at drive + steering overlap");
grid on;

subplot(2,1,2);
efficiencies = [ ...
    motor_boost_efficiency * 100, ...
    motor_path_efficiency_percent, ...
    electronics_path_efficiency_percent];

bar(efficiencies);
set(gca, "xtick", 1:3);
set(gca, "xticklabel", { ...
    "Motor boost assumed", ...
    "Motor battery to motor", ...
    "Electronics battery to 5 V load"});
ylabel("Efficiency (%)");
ylim([0 100]);
title("Estimated conversion/path efficiencies");
grid on;

%% 15. LOCAL FUNCTIONS

function [I, Vterminal] = battery_input_model(Voc, R, Pload)
    % Solve Pload = I * (Voc - I*R), using the lower-current root.
    % Pload may be a scalar or vector.
    I = zeros(size(Pload));
    Vterminal = Voc .* ones(size(Pload));

    for n = 1:numel(Pload)
        P = Pload(n);

        if P <= 0
            I(n) = 0;
            Vterminal(n) = Voc;
        else
            discriminant = Voc^2 - 4 * R * P;

            if discriminant <= 0
                error(["Requested power exceeds this simplified battery ", ...
                       "model's maximum-power point. Recheck assumptions."]);
            end

            I(n) = (Voc - sqrt(discriminant)) / (2 * R);
            Vterminal(n) = Voc - I(n) * R;
        end
    end
end

function y = first_order_response(t, target, tau)
    % First-order lag used only as a simple illustrative speed response.
    y = zeros(size(t));

    for n = 2:numel(t)
        dt_local = t(n) - t(n-1);
        y(n) = y(n-1) + ...
            (target(n) - y(n-1)) * dt_local / tau;
    end
end
