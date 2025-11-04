
clear all;
close all;
% >>> Ergänzung am Anfang <<<
C_max_saettigung = 300000;   % maximale Luftkonzentration in ppm (ca. 36°C)
% >>> Innerhalb der Schleife, beim Berechnen von C_t <<<
    
% Eingabewerte
gesamtmenge_ml = 290;             % Gesamtmenge AS-Lösung in ml
V_verdunstung_ml_pro_tag = 29;    % ml/Tag
konzentration_AS = 0.60;          % 60 %
beutenvolumen_l = 77;
molmasse_AS = 46.03;              % g/mol
molvolumen_l = 26.5;              % l/mol bei 36 °C
dichte_AS = 1.0;                  % g/ml
% Zeitachse
t_max = 332;                      % Gesamtzeit in Stunden (z. B. 10 Tage)
t = linspace(0, t_max, 2000);     % feine Zeitauflösung
% Luftwechselraten
luftwechsel_raten = [1, 6, 10];
farben = ['r', 'b', 'g'];
labels = {'1/h', '6/h', '10/h'};
% Dauer der aktiven Verdunstung (in Stunden)
verdunstungsdauer_h = (gesamtmenge_ml / V_verdunstung_ml_pro_tag) * 24;
% Berechne konstante Dampfzufuhr in l/h
g_AS_pro_Tag = V_verdunstung_ml_pro_tag * konzentration_AS * dichte_AS;
mol_AS_pro_Tag = g_AS_pro_Tag / molmasse_AS;
gasvolumen_l_pro_Tag = mol_AS_pro_Tag * molvolumen_l;
gasvolumen_l_pro_h = gasvolumen_l_pro_Tag / 24;
% Plot vorbereiten
figure; hold on;
% Schleife über Luftwechselraten
for i = 1:length(luftwechsel_raten)
    k = luftwechsel_raten(i);   % aktuelle Luftwechselrate
    % Gleichgewichtskonzentration für diesen k
    C_inf = (gasvolumen_l_pro_h / beutenvolumen_l) * 1e6 / k;
    % Konzentrationsverlauf berechnen
    C_t = zeros(size(t));
for j = 1:length(t)
        if t(j) <= verdunstungsdauer_h
            % Aufbauphase
            C_akt = C_inf * (1 - exp(-k * t(j)));
        else
            % Abbauphase
            t_rest = t(j) - verdunstungsdauer_h;
            C_end = C_inf * (1 - exp(-k * verdunstungsdauer_h));
            C_akt = C_end * exp(-k * t_rest);
        end
        % Sättigung prüfen
        C_t(j) = min(C_akt, C_max_saettigung);
end
    % Plot
    plot(t, C_t, 'Color', farben(i), 'LineWidth', 2, 'DisplayName', labels{i});
    fprintf('Luftwechselrate %s → C_max ≈ %.0f ppm\n', labels{i}, max(C_t));
end
% Zielbereich für Behandlung (z. B. 15.000–30.000 ppm)
yline(500, '--k', '500ppm L/R 2016');
yline(2300, '--k', '2300ppm Bolli et al. 1993');
%yline(30000, '--k', '300k approx. Luftsättigung');
% Verdunstungsende markieren
xline(verdunstungsdauer_h, '--k', 'Verdunstung endet');
% Achsen & Beschriftung
xlabel('Zeit (Stunden)');
ylabel('Konzentration (ppm)');
title('Konzentrationsverlauf bei verschiedenen Luftwechselraten');
legend('Location', 'northeast');
grid on;
