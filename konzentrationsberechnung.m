close all;
clear all;

format longg;
    e = exp(1);
V_acid_ges = 290;
D_ges = 10;
V_pro_D = V_acid_ges/D_ges;

conc_ams = 0.6;
V_Zander = 77;
m_mol = 46.03;
v_mol = 22.4; %näherung, für 20°C, obwohl eigentlich um die 36°C

V_ex_low = 1;
V_ex_mid = 5;
V_ex_high = 10;

conc_start = 0;

V_dampf_tag = V_acid_ges/D_ges*conc_ams / m_mol*v_mol;
C_inf = V_dampf_tag/V_Zander*10^6;

fprintf('Dampfvolumen: %f l/Tag\n',V_dampf_tag );
fprintf('Gleichgewischtskonzentration: %f ppm',C_inf);

conc_low=dsolve()
    figure;
    plot(conc_low,'bo');
    grid;
    xlabel('t in s');
    