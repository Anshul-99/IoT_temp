Author: Khyati Satta

1. What is the average current per period?
   Answer: 31.76 uA
   <br>Screenshot:  
   ![Avg_current_per_period](/screenshots/avg_current_per_period.jpg?raw=true)  

2. What is the average current when the Si7021 is Powered Off?
   Answer: 9.42 uA
   <br>Screenshot:  
   ![Avg_current_LPM_Off](/screenshots/avg_current_lpm_off.jpg?raw=true)  

3. What is the average current when the Si7021 is Powered On?
   Answer: 734.10 uA
   <br>Screenshot:  
   ![Avg_current_LPM_On](/screenshots/avg_current_lpm_on.jpg?raw=true)  

4. How long is the Si7021 Powered On for 1 temperature reading?
   Answer: 111.50 ms
   <br>Screenshot:  
   ![duration_lpm_on](/screenshots/avg_current_lpm_on.jpg?raw=true)  

5. Compute what the total operating time of your design for assignment 4 would be in hours, assuming a 1000mAh battery power supply?
   Answer: Battery life = (Battery capacity (in mAh)/ Device Consumption (in mA)) = (1000/0.031) = ~ 32,258 hours
   
6. How has the power consumption performance of your design changed since the previous assignment?
   Answer: The average power consumption in the previous assignment due to polling approach for both timers and I2C transfers was around 505.17 uW and now in this assignment due to interrupt-based approach for both, the average power consumption has gone down to around 95 uW resulting in a five-fold decrease.

7. Describe how you tested your code for EM1 during I2C transfers.
   Answer: The code was designed in such a way that the I2C transfers occur in EM1 mode. The interrupt-based approach pulls the MCU out of EM1 to EM0 when the I2C transfer has to occur using the handler.The transfer itself occurs in EM1 mode. The following has been explained in the screenshot attached below:
    <br>Screenshot: 
   ![I2C_Transfer](/screenshots/I2C_Transfer.jpg?raw=true)  

   
