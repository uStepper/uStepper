clc;
clear all;
close all;

digits = 2;
v = 1000;
v0 = 0;
a = 0.1;
f = 32800;
m = a/(f^2);
S= 10000;

Sa = floor((v^2 - v0^2)/(2*a));
Sd = Sa;
Sc = 0;

if S/2 < Sa
    Sa = floor(S/2);
    Sd = Sa;
    S = S - (S - Sa - Sd);
else
    Sc = S - Sa - Sd;
end

p1(1) = f/sqrt(v0^2 + 2*a);
p2(1) = f/sqrt(v0^2 + 2*a);
for i=1:Sa-1
   p1(i+1) = p1(i)-round(m,12)*round(round(round(p1(i),digits)*round(p1(i),digits),digits)*round(p1(i),digits),digits);
end
for i=Sa:Sa + Sc - 1
   p1(i+1) = p1(i);
end
for i=Sa + Sc:Sa + Sc + Sd - 1
   p1(i+1) = p1(i)+round(m,12)*round(round(round(p1(i),digits)*round(p1(i),digits),digits)*round(p1(i),digits),digits);
end

for i=1:Sa-1
   p2(i+1) = p2(i)-m*p2(i)^3;
end
for i=Sa:Sa + Sc - 1
   p2(i+1) = p2(i);
end
for i=Sa + Sc:Sa + Sc + Sd - 1
   p2(i+1) = p2(i)+m*p2(i)^3;
end

p1(S)
p2(S)

%%
clc;
clear all;
close all;

digits = 2;
v = 0;
v0 = 2000;
a = 0.1;
f = 32000;
m = a/(f^2);

S = floor((v^2 - v0^2)/(-2*a));
p1(1) = f/sqrt(v0^2 + -2*a);
p2(1) = f/sqrt(v0^2 + -2*a);
for i=1:S-1
   p1(i+1) = p1(i)+round(m,15)*round(round(round(p1(i),digits)*round(p1(i),digits),digits)*round(p1(i),digits),digits);
end

for i=1:S-1
   p2(i+1) = p2(i)+m*p2(i)*p2(i)*p2(i);
end

p1(S)
p2(S)
m
