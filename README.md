# freeRTOS_feedback
 An example for a feedback control model implemented by using FreeRTOS routines such as tasks and queues. MATLAB code performs the computatuin of matrices as well as the comparison between FreeRTOS and Simulink output vectors. Completed in order to meet the requirements of the Real-Time Digital Systems course. 

# Usage Guide
1. Download the latest FreeRTOS version: https://freertos.org/a00104.html.
2. In FreeRTOS\Demo\WIN32-MSVC, open WIN32.sln with Microsoft Visual Studio.
3. Replace main.c, main_blinky.c and FreeRTOSConfig.h from the demo with corresponding files from the repo.
4. Run the FreeRTOS project.
5. In MATLAB, run RTDS2022.m. SLX- and TXT-files must be in the same folder.
6. model_u.txt, model_y.txt and RTOS_vs_Simulink.fig are just for demonstration purposes.
