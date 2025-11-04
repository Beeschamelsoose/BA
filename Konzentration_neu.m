% Konzentrationsverlauf von Ameisensäure in einer Doppelzander-Beute
% 10 Tage Verdunstung (treiberbasiert) + 2 Tage Abbau
clear all; close all;

%% Konstanten & Parameter
V_beute = 0.072;            % Beutenvolumen [m^3] (Doppelzander ~ 72 L)
R = 8.314;                  % Gaskonstante [J/(mol*K)]
T = 298;                    % Temperatur [K] (~25 °C)
p = 1e5;                    % Gesamtdruck [Pa] ~ 1 bar

% Ameisensäure
rho_HCOOH = 1.22;           % Dichte [g/ml]
M_HCOOH   = 46.03e-3;       % Molare Masse [kg/mol]

% Praxis-Quelle (Reservoirverlust, NICHT direkt Gasphase!)
ml_per_day = 29;            % Verdunstungsrate aus Praxis [ml/Tag]
t_day = 24*3600;            % [s]
t_evap = 10 * t_day;        % [s] Verdunstung aktiv
t_total = 12 * t_day;       % [s] Gesamtlaufzeit (10 d + 2 d Abbau)

% Luftwechsel (ACH) und zusätzliche Senke
ACH = [1, 4, 6, 10];           % [1/h] Luftwechselraten (3 Kurven)
k_vent = ACH/3600;          % [1/s]
sink_factor = 0.0;          % Anteil der Extrasenke relativ zur Lüftung (0..1)
k_sink = sink_factor .* k_vent;  % [1/s] zusätzliche Verluste

% >>> Gasphasen-Sättigung über Lösung (Effektivparameter):
% y_star ist der effektive Volumenanteil der HCOOH im Gas im Gleichgewicht
% über der verwendeten Lösung bei T (fasst Aktivität, Mischungsgrad etc. zusammen).
y_star = 0.05;              % (z.B. 5% v/v) --> bitte bei Bedarf kalibrieren
C_sat = (y_star * p) / (R * T);   % [mol/m^3] Sättigungskonzentration in Gasphase

% --- Kalibrierung der Emission: k_emis aus anfänglichem Stoffmengenstrom ---
% Start-Massenstrom (aus 29 ml/Tag):
m_dot0_gps = ml_per_day * rho_HCOOH / t_day;  % [g/s]
n_dot0 = (m_dot0_gps / 1000) / M_HCOOH;       % [mol/s] (g->kg, dann /M)
% Bei C=0 ist Emission = k_emis * C_sat = n_dot0  -> k_emis:
k_emis = n_dot0 / C_sat;    % [1/s] effektive Emissionskonstante

% Hilfsfunktionen
ppm = @(c) c * R * T / p * 1e6;  % mol/m^3 -> ppm (Vol-ppm)
toDays = @(tsec) tsec/3600/24;

%% Zeitachse
t = linspace(0, t_total, 3000);  % [s]

%% Berechnung
C = zeros(numel(ACH), numel(t));     % mol/m^3
C_ss = zeros(size(ACH));             % stationäre Werte während Aktivphase
C_end10d = zeros(size(ACH));         % Konzentration am Ende der 10 Tage
C_ss_ppm = zeros(size(ACH));
C_end10d_ppm = zeros(size(ACH));

for i = 1:numel(ACH)
    kv = k_vent(i);
    ks = k_sink(i);
    lambda_on  = k_emis + kv + ks;   % Gesamtabklingrate in der Aktivphase
    lambda_off = kv + ks;            % nach Abschalten (nur Verluste)

    % Stationärwert während Verdunstung (t -> ∞, Quelle aktiv):
    C_ss(i) = (k_emis * C_sat) / lambda_on;     % [mol/m^3]
    C_ss_ppm(i) = ppm(C_ss(i));

    % Phase 1 (0 .. t_evap): C(t) = C_ss * (1 - exp(-lambda_on * t)), C(0)=0
    t1 = t(t <= t_evap);
    C1 = C_ss(i) * (1 - exp(-lambda_on * t1));

    % Konzentration am Ende der Aktivphase:
    C_end = C_ss(i) * (1 - exp(-lambda_on * t_evap));
    C_end10d(i) = C_end;
    C_end10d_ppm(i) = ppm(C_end);

    % Phase 2 (t_evap .. t_total): dC/dt = -lambda_off * C
    t2 = t(t > t_evap);
    C2 = C_end * exp(-lambda_off * (t2 - t_evap));

    % Zusammenfügen
    C(i,:) = [C1, C2];
end

%% Plot
figure; hold on; grid on; box on;
colors = lines(numel(ACH));

for i = 1:numel(ACH)
    plot(toDays(t), ppm(C(i,:)), 'LineWidth', 2, 'Color', colors(i,:));
    % Stationären Wert (während Aktivphase) markieren:
    yline(C_ss_ppm(i), '--', sprintf('C_{stat}≈%.0f ppm (%d/h)', C_ss_ppm(i), ACH(i)), ...
        'Color', colors(i,:), 'LineWidth', 1);
end

% Vertikale Linie: Ende der Verdunstung
xline(toDays(t_evap), 'k--', 'Verdunstung stoppt', ...
    'LineWidth', 1.2, 'LabelOrientation', 'horizontal', 'LabelVerticalAlignment', 'bottom');

xlabel('Zeit [Tage]');
ylabel('Ameisensäure-Konzentration [ppm]');
title(sprintf(['Ameisensäure in Doppelzander (V=72 L): 10 Tage Emission + 2 Tage Abbau\n',...
               'y^* = %.2f,  sink = %.0f%%%% von Lüftung, k_{emis} = %.3g s^{-1}'], ...
               y_star, sink_factor*100, k_emis));
legend(arrayfun(@(x) sprintf('%d Luftwechsel/h', x), ACH, 'UniformOutput', false), ...
    'Location', 'northeast');
xlim([0, toDays(t_total)]);

%% Konsolenausgabe
disp('--- Parameter & Kalibrierung ---');
fprintf('Beutevolumen V           = %.3f m^3\n', V_beute);
fprintf('Praxisrate               = %.1f ml/Tag  (%.3f g/s)\n', ml_per_day, m_dot0_gps);
fprintf('n_dot(0)                 = %.3g mol/s\n', n_dot0);
fprintf('C_sat (Gas, eff.)        = %.3g mol/m^3  (≈ %.0f ppm)\n', C_sat, ppm(C_sat));
fprintf('k_emis                   = %.3g 1/s\n', k_emis);
fprintf('sink_factor              = %.2f (k_sink = sink_factor * k_vent)\n\n', sink_factor);

disp('--- Ergebnisse (während Aktivphase / nach 10 Tagen) ---');
for i = 1:numel(ACH)
    fprintf('ACH = %2d/h:  C_stat ≈ %6.0f ppm,   C(10 d) ≈ %6.0f ppm\n', ...
        ACH(i), C_ss_ppm(i), C_end10d_ppm(i));
end

% Optional: Zeit bis unter z.B. 100 ppm nach Abschalten
threshold_ppm = 100;
disp(' ');
fprintf('Zeit bis unter %d ppm nach Abschalten (ab Tag 10):\n', threshold_ppm);
for i = 1:numel(ACH)
    k_off = k_vent(i) + k_sink(i);
    if C_end10d_ppm(i) <= threshold_ppm
        t_decay = 0;
    else
        t_decay = log(C_end10d_ppm(i)/threshold_ppm) / k_off; % [s]
    end
    fprintf('ACH = %2d/h:  %.2f Tage\n', ACH(i), toDays(t_decay));
end
