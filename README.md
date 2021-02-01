# MS51_Analog_PWM
 MS51_Analog_PWM

1. Initial TIMER 1 with divide 12 for analog PWM (GPIO : 0.5 , FREQ : 2K)

2. PWM freq. formula : (0xFFFF+1 - target)  / (24MHz/psc) = time base 

(65536 - counter) / (24 000 000 / psc) = time base ( 1/ freq)

psc = 1 (sys clock) or 12 (divide 12)

target timing = 500 us ( 1 / 2000 , 2K freq.)

if psc = 12 , freq = 2000 , 

(65536 - counter) / (24 000 000 / 12) = 1 / 2000 , 

(65536 - counter) = 2 000 000 / 2 000 = 1 000 

counter = 65536 - 1000

2. swap duty (50 percent , 85 percent) per 2500 ms

below is screen capture , 50 percent duty , 2K freq

![image](https://github.com/released/MS51_Analog_PWM/blob/main/capture.jpg)

