% Dual Motor V2 - electronics battery/switch/boost model.
% MATLAB and GNU Octave; no toolboxes.
% RUN mode: charging USB disconnected. All resistance/load values assumed.
clear; clc; close all;

Vcell = 3.65;          % assumed open-circuit voltage for this short example
capacity_Ah = 1.0;     % 1000 mAh
Rcell = 0.12;         % assumed battery internal resistance, ohms
Rprotection = 0.10;   % assumed charger-board protection/wiring resistance
Rswitch = 0.05;       % assumed closed-switch resistance
eta = 0.85;          % assumed boost efficiency
Vtarget = 5.0;
Vin_min = 3.1;        % model operating limit, NOT actual module specification
Iin_limit = 1.0;      % model input limit, NOT a verified hardware rating

t = 0:0.05:20;
switch_on = t >= 2 & t < 18;
Iout = 0.200 * double(switch_on);  % assumed 5 V electronics load
Iout(t >= 8 & t < 12) = 0.350;   % temporary higher load

Ibat = zeros(size(t));
Vbattery = Vcell * ones(size(t));
Vprotected = Vbattery;
Vboost_in = zeros(size(t));
Vout = zeros(size(t));
Rtotal = Rcell + Rprotection + Rswitch;

for k = 1:length(t)
    if switch_on(k)
        Pneeded = Vtarget * Iout(k) / eta;
        disc = Vcell^2 - 4 * Rtotal * Pneeded;
        if disc <= 0
            error('Requested power exceeds this model battery/path capability.');
        end
        % Stable solution of I * (Vcell - I*Rtotal) = Pneeded.
        Ibat(k) = 2 * Pneeded / (Vcell + sqrt(disc));
        Vbattery(k) = Vcell - Ibat(k) * Rcell;
        Vprotected(k) = Vbattery(k) - Ibat(k) * Rprotection;
        Vboost_in(k) = Vprotected(k) - Ibat(k) * Rswitch;
        if Vboost_in(k) < Vin_min || Ibat(k) > Iin_limit
            error('Model limit exceeded; change load/design, not hardware blindly.');
        end
        Vout(k) = Vtarget;
    end
end

Pcell_loss = Ibat.^2 * Rcell;
Pprotection = Ibat.^2 * Rprotection;
Pswitch = Ibat.^2 * Rswitch;
Pboost = Vboost_in .* Ibat - Vout .* Iout;
charge_used_mAh = trapz(t, Ibat) / 3.6;

fprintf('ASSUMED MODEL: no measured current or resistance values.\n');
fprintf('Peak battery/boost-input current: %.0f mA\n', max(Ibat)*1000);
fprintf('Peak 5 V output current: %.0f mA\n', max(Iout)*1000);
fprintf('Minimum enabled boost input: %.2f V\n', min(Vboost_in(switch_on)));
fprintf('Battery charge used in 20 s: %.2f mAh\n', charge_used_mAh);
fprintf('Fraction of nominal capacity used: %.3f percent\n', ...
    100 * charge_used_mAh / (capacity_Ah * 1000));
fprintf('Peak battery internal loss: %.3f W\n', max(Pcell_loss));
fprintf('Peak protection-path loss: %.3f W\n', max(Pprotection));
fprintf('Peak switch loss: %.3f W\n', max(Pswitch));
fprintf('Peak boost loss: %.3f W\n', max(Pboost));

figure(1);
subplot(3,1,1);
plot(t,Vbattery,t,Vprotected,t,Vboost_in,t,Vout,'LineWidth',1.5);
legend('Battery terminals','Charger OUT pair','After switch / boost IN','Boost OUT');
ylabel('Voltage (V)'); grid on;
title('Run mode: battery -> protected OUT -> switch -> boost -> 5 V load');

subplot(3,1,2);
plot(t,1000*Ibat,t,1000*Iout,'LineWidth',1.5);
legend('Battery / switch / boost input','5 V output load');
ylabel('Current (mA)'); grid on;

subplot(3,1,3);
plot(t,Pcell_loss,t,Pprotection,t,Pswitch,t,Pboost,'LineWidth',1.5);
legend('Battery internal','Protection path','Switch','Boost');
xlabel('Time (s)'); ylabel('Power lost (W)'); grid on;

% SEPARATE charging illustration: load switch OFF, USB input ON.
% Assumed constant-current phase only; not a full charge/temperature model.
% 250 mA is an EXAMPLE, not a verified setting for your battery.
Vusb = 5.0;
Ichg = 0.250;
Vcharge_cell = 3.2:0.01:4.1;
Pcharger = (Vusb - Vcharge_cell) * Ichg;
figure(2);
plot(Vcharge_cell,Pcharger,'r','LineWidth',2);
xlabel('Cell voltage during assumed constant-current phase (V)');
ylabel('Approximate charger IC heat generation (W)');
title('Separate charging mode: 5 V USB, assumed 250 mA; load OFF');
grid on;
