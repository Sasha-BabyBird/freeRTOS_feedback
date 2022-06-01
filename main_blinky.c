
/* Standard includes. */
#include <stdio.h>
#include <conio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Priorities at which the tasks are created. */
#define	main_priority	( tskIDLE_PRIORITY + 1 )

/* The number of items the queue can hold at once. */
#define queue_length					( 2 )

/*-----------------------------------------------------------*/

/*
 * The tasks as described in the comments at the top of this file.
 */
static void object( void *pvParameters );
static void controller( void *pvParameters );
static void plotting( void* pvParameters );


/*-----------------------------------------------------------*/

/* The queue used by both tasks. */
static QueueHandle_t y_queue = NULL;
static QueueHandle_t u_queue = NULL;
static TaskHandle_t object_task, controller_task, plotting_task;

/*-----------------------------------------------------------*/

/* Initial conditions */

static long double x0_11 = 5.0;
static long double x0_21 = -5.0;

static long double z0_11 = 0.0;
static long double z0_21 = 0.0;

/* Matrices of the system and the observer */

static long double a_11 = 0.397532220692826;
static long double a_12 = -0.270670566473225;
static long double a_21 = 0.5;
static long double a_22 = 0.0;

static long double b_11 = 0.5;
static long double b_21 = 0;

static long double c_11 = 0.491674014000475;
static long double c_12 = 0.492258097086624;

static long double k_11 = 2.463497596829576;
static long double k_12 = -2.139939051196105;

static long double l_11 = -0.393178004288917;
static long double l_21 = -0.294305911509129;

static long double az_11 = 1.822596426693042;
static long double az_12 = -1.147095035863699;
static long double az_21 = 0.644702568855762;
static long double az_22 = 0.144874467960828;

long double y_vector[50];
long double u_vector[50];
int time_vector[50];

void main_blinky( void )
{

	/* Create the queues. */
    y_queue = xQueueCreate(queue_length, sizeof(double));
    u_queue = xQueueCreate(queue_length, sizeof(double));

	if( y_queue != NULL && u_queue != NULL )
	{
		xTaskCreate( object,			/* The function that implements the task. */
					"object", 							/* The text name assigned to the task - for debug only as it is not used by the kernel. */
					configMINIMAL_STACK_SIZE, 		/* The size of the stack to allocate to the task. */
					NULL, 							/* The parameter passed to the task - not used in this simple case. */
					main_priority,/* The priority assigned to the task. */
					&object_task );							/* The task handle. */

		xTaskCreate( controller, "controller", configMINIMAL_STACK_SIZE, NULL, main_priority, &controller_task );
        xTaskCreate( plotting, "plot", configMINIMAL_STACK_SIZE, NULL, main_priority, &plotting_task );

		vTaskStartScheduler();
	}

	/* If all is well, the scheduler will now be running, and the following
	line will never be reached.  If the following line does execute, then
	there was insufficient FreeRTOS heap memory available for the idle and/or
	timer tasks	to be created.  See the memory management section on the
	FreeRTOS web site for more details. */
	for( ;; );
}
/*-----------------------------------------------------------*/

static void object( void *pvParameters )
{
	/* Prevent the compiler warning about the unused parameter. */
	( void ) pvParameters;

    volatile long double u;
    volatile long double x_11 = x0_11;
    volatile long double x_21 = x0_21;
    volatile long double x_11_new;
    volatile long double x_21_new;
    volatile long double y;
    
    int counter = 0;
	for( ;; )
	{
        if (eTaskGetState(plotting_task) != eSuspended)
        {
            vTaskSuspend(plotting_task);
        }
        if (counter == 50)
        {
            vTaskDelete(controller_task);
            vTaskResume(plotting_task);
        }
        
        if (!counter)
        {
            u = 0.0;
        }
        else
        {
            xQueueReceive(u_queue, &u, portMAX_DELAY);
        }
        printf("Counter: %i\r\n", counter);
        printf("Value received by object: %lf\r\n", u);

        y = c_11 * x_11 + c_12 * x_21;
        x_11_new = a_11 * x_11 + a_12 * x_21 + b_11 * u;
        x_21_new = a_21 * x_11 + a_22 * x_21 + b_21 * u;
        x_11 = x_11_new;
        x_21 = x_21_new;

        //printf("x_11 = %lf\r\n", x_11);
        //printf("x_21 = %lf\r\n", x_21);
	
        u_vector[counter] = u;
        y_vector[counter] = y;
        time_vector[counter] = counter;
        counter++;

        printf("Value sent by object: %lf\r\n", y);
        xQueueSend(y_queue, &y, 0);
	}
}
/*-----------------------------------------------------------*/

static void controller( void *pvParameters )
{
	/* Prevent the compiler warning about the unused parameter. */
	( void ) pvParameters;

    volatile long double y;
    volatile long double z_11 = z0_11;
    volatile long double z_21 = z0_21;
    volatile long double z_11_new;
    volatile long double z_21_new;
    volatile long double u;

	for( ;; )
	{
		xQueueReceive(y_queue, &y, portMAX_DELAY);
        printf("Value received by controller: %lf\r\n", y);
        
        u = k_11 * z_11 + k_12 * z_21;
        z_11_new = az_11 * z_11 + az_12 * z_21 + l_11 * y;
        z_21_new = az_21 * z_11 + az_22 * z_21 + l_21 * y;
        z_11 = z_11_new;
        z_21 = z_21_new;

        //printf("z_11 = %lf\r\n", z_11);
        //printf("z_21 = %lf\r\n", z_21);
        
        printf("Value sent by controller: %lf\r\n\n", u);
        xQueueSend(u_queue, &u, 0);
	}
}
/*-----------------------------------------------------------*/

static void plotting(void* pvParameters)
{
    /* Prevent the compiler warning about the unused parameter. */
    (void)pvParameters;

    vTaskDelete(object_task);

    FILE* model_y_by_RTOS;
    model_y_by_RTOS = fopen("model_y_by_RTOS.txt", "wb");
    FILE* model_u_by_RTOS;
    model_u_by_RTOS = fopen("model_u_by_RTOS.txt", "wb");
    printf("\r\nVector of u:\r\n");
    for (int i = 0; i < 50; i++)
    {
        printf("%lf", u_vector[i]);
        fprintf(model_u_by_RTOS, "%lf", u_vector[i]);
        if (i < 49)
        {
            printf(", ");
            fprintf(model_u_by_RTOS, ",");
        }
        else
            printf(".\n");
    }
    printf("\r\nVector of y:\r\n");
    for (int i = 0; i < 50; i++)
    {
        printf("%lf", y_vector[i]);
        fprintf(model_y_by_RTOS, "%lf", y_vector[i]);
        if (i < 49)
        {
            printf(", ");
            fprintf(model_y_by_RTOS, ",");
        }
        else
            printf(".\n");
    }
    printf("\r\nVector of time:\r\n");
    for (int i = 0; i < 50; i++)
    {
        printf("%i", time_vector[i]);
        if (i < 49)
            printf(", ");
        else
            printf(".");
    }
 
    fclose(model_y_by_RTOS);
    fclose(model_u_by_RTOS);
    vTaskDelete(NULL);
}

