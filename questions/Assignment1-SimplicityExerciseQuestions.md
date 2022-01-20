


>
Author: Khyati Satta
ECEN 5823 Internet of Things Embedded Firmware
Date: 19 January 2022

  

**1. How much current does the system draw (instantaneous measurement) when a single LED is on with the GPIO pin set to StrongAlternateStrong?**

Answer: 5.38 mA

  
  

**2. How much current does the system draw (instantaneous measurement) when a single LED is on with the GPIO pin set to WeakAlternateWeak?**

Answer: 5.21 mA

  
  

**3. Is there a meaningful difference in current between the answers for question 1 and 2? Please explain your answer,referencing the [Mainboard Schematic](https://www.silabs.com/documents/public/schematic-files/WSTK-Main-BRD4001A-A01-schematic.pdf) and [AEM Accuracy](https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf) section of the user's guide where appropriate. Extra credit is avilable for this question and depends on your answer.**

Answer: No. There is not any significant difference between the two since the driving current for the StrongAlternateStrong is 10 mA and WeakAlternateWeak is around 1 mA according to the reference manual. Since, the load connected is an LED, the difference in the two is not quite distinct.

  
  

**4. With the WeakAlternateWeak drive strength setting, what is the average current for 1 complete on-off cycle for 1 LED with an on-off duty cycle of 50% (approximately 1 sec on, 1 sec off)?**

Answer: 5.08 mA

  
  

**5. With the WeakAlternateWeak drive strength setting, what is the average current for 1 complete on-off cycle for 2 LEDs (both on at the time same and both off at the same time) with an on-off duty cycle of 50% (approximately 1 sec on, 1 sec off)?**

Answer: 5.31 mA