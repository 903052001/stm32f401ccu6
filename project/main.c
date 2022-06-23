/**
  ******************************************************************************
  * @file    Templates/Src/main.c
  * @author  MCD Application Team
  * @brief   Main program body
  ******************************************************************************
  */

//#include "FreeRTOSConfig.h"
//#include "FreeRTOS.h"
//#include "task.h"


#include <stdio.h>
#include "stm32f4xx.h"


#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, int line)
{
    /* User can add his own implementation to report the file name and line number */
    printf("Wrong parameters value: file %s on line %d\r\n", file, line);

    /* Infinite loop */
    while (1);
}
#endif

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 84000000
  *            HCLK(Hz)                       = 84000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 336
  *            PLL_P                          = 4
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale2 mode
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ = 7;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        /* Initialization Error */
        assert_failed(__FILE__, __LINE__);
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        /* Initialization Error */
        assert_failed(__FILE__, __LINE__);
    }
}

void hw_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Configure GPIO pins */
    GPIO_InitStruct.Pin   = GPIO_PIN_13;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    return;
}

UART_HandleTypeDef UART1_Handle;
void hw_uart_init(void)
{
    UART1_Handle.Instance          = USART1;
    UART1_Handle.Init.BaudRate     = 115200;
    UART1_Handle.Init.WordLength   = UART_WORDLENGTH_8B;
    UART1_Handle.Init.StopBits     = UART_STOPBITS_1;
    UART1_Handle.Init.Parity       = UART_PARITY_NONE;
    UART1_Handle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    UART1_Handle.Init.Mode         = UART_MODE_TX_RX;
    UART1_Handle.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&UART1_Handle) != HAL_OK)
    {

    }
}

#ifdef __GNUC__
    #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
    #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif
PUTCHAR_PROTOTYPE
{
    //具体哪个串口可以更改UART1_Handle为其它串口
    HAL_UART_Transmit(&UART1_Handle, (uint8_t *)&ch, 1, 0xffff);
    return ch;

}



// void vTask( void *pvParameters )
// {
//     int num = (int)pvParameters;

//     (void)num;

//     for( ;; )
//     {
//         vTaskDelay(1000);
//         HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
//         printf("------------task---------\r\n");
//     }
// }


#include "unity.h"
void  test_fun1(void)
{
    printf("------------fun 1---------\r\n");
    int a = 10;
    TEST_ASSERT_EQUAL_INT(10, a);
    printf("------------fun 2---------\r\n");
    TEST_ASSERT_GREATER_THAN(1, a);
    printf("------------fun 3---------\r\n");
}



/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(int argc, char *argv[])
{
    /* STM32F4xx HAL library initialization:
         - Configure the Flash prefetch, Flash preread and Buffer caches
         - Systick timer is configured by default as source of time base, but user
               can eventually implement his proper time base source (a general purpose
               timer for example or other time source), keeping in mind that Time base
               duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
               handled in milliseconds basis.
         - Low Level Initialization */
    HAL_Init();

    /* Configure the System clock to have a frequency of 84 MHz */
    SystemClock_Config();

    /* Add your application code here */
    //Fn_device_auto_init();
    hw_gpio_init();
    hw_uart_init();
    (void)argc;
    (void)argv;

    extern void MX_USB_DEVICE_Init(void);
    extern void usb_printf(const char *format, uint32_t Len);
    MX_USB_DEVICE_Init();

   RUN_TEST(test_fun1, 0);


//    xTaskCreate( vTask, "Task1", 1000, NULL, 1, NULL );
//    vTaskStartScheduler();


    /* Infinite loop */
    while (1)
    {
        HAL_Delay(1000);
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        //HAL_UART_Transmit_DMA(&UART1_Handle, "hello world!\r\n", sizeof("hello world!\r\n"));
        usb_printf("hello usbd\r\n", sizeof("hello usbd\r\n"));
    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
