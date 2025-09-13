/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fatfs.h"
#include "ff.h"
#include "stm32f4xx_hal_gpio.h"
#include <cstdint>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define wait HAL_Delay
#define SD_INIT_ERROR 0
#define DISK_INIT_ERROR 1
#define MOUNT_ERROR 2
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SD_HandleTypeDef hsd;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SDIO_SD_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
typedef struct LED_Color_Data {
        GPIO_TypeDef *GPIO_Port;
        uint16_t GPIO_Pin;
} LED_Color_Data;

typedef struct LED_RGB_Data {
        LED_Color_Data r;
        LED_Color_Data g;
        LED_Color_Data b;
} LED_RGB_Data;

enum Color {
    WHITE = 0b111,
    BLUE = 0b001,
    GREEN = 0b010,
    RED = 0b100,
    CYAN = 0b011,
    MAGENTA = 0b101,
    YELLOW = 0b110,
    OFF = 0b000
};

class LED {
    private:
        LED_RGB_Data led;
        Color ledRGB = Color::OFF;

        void setPin(GPIO_TypeDef *port, uint16_t pin, bool state) {
            if (state)
                HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
            else
                HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
        }

    public:
        LED(LED_RGB_Data &led) : led(led) {
        }

        void setColor(Color color) {
            ledRGB = color;
            setPin(led.r.GPIO_Port, led.r.GPIO_Pin, (color >> 2) & 1);
            setPin(led.g.GPIO_Port, led.g.GPIO_Pin, (color >> 1) & 1);
            setPin(led.b.GPIO_Port, led.b.GPIO_Pin, color & 1);
        }

        void Off() {
            setColor(Color::OFF);
        }

        void OK() {
            setColor(ledRGB);
            wait(300);
            Off();
            wait(300);
            setColor(ledRGB);
            wait(300);
            Off();
            wait(300);
            setColor(ledRGB);
            wait(300);
            Off();
        }

        void OK(Color color) {
            setColor(color);
            wait(300);
            Off();
            wait(300);
            setColor(color);
            wait(300);
            Off();
            wait(300);
            setColor(color);
            wait(300);
            Off();
        }
};

class SD_Card {
    private:
        bool inserted = false;
        bool opened = false;
        FATFS FatFS{};
        FIL fil{};

    public:
        SD_Card() {
            checkInsertion();
        }

        ~SD_Card() {
            if (opened) {
                close();
            }
            unmount();
        }

        bool isInserted() {
            return inserted;
        }

        bool checkInsertion() {
            bool inserted_new = (BSP_SD_IsDetected() == SD_PRESENT);
            if (inserted_new) {
                inserted = BIT();
                if (inserted) {
                    mount();
                }
                return inserted;
            }
            inserted = false;
            return false;
        }

        bool BIT() {
            if (BSP_SD_Init() == MSD_OK) {
                if (disk_initialize(0) == RES_OK) {
                    return true;
                }
            }
            return false;
        }

        FRESULT mount() {
            if (!inserted)
                return FR_DISK_ERR;
            return f_mount(&FatFS, "0:", 1);
        }

        FRESULT unmount() {
            if (!inserted)
                return FR_DISK_ERR;
            return f_mount(NULL, "0:", 0);
        }

        bool write_once(char *name, char *buf) {
            if (!inserted)
                return false;
            FIL fil;
            if (f_open(&fil, name, FA_OPEN_ALWAYS | FA_OPEN_APPEND) == FR_OK) {
                f_puts(buf, &fil);
            } else {
                f_close(&fil);
                return false;
            }
            return true;
            f_close(&fil);
        }

        void open(char *name) {
            if (!inserted)
                return;
            if (f_open(&fil, name, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
                opened = true;
            };
        }

        void write(char *buf) {
            if (!inserted)
                return;
            f_puts(buf, &fil);
        }

        FRESULT close() {
            if (!inserted)
                return FR_DISK_ERR;
            return f_close(&fil);
        }
};

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_SDIO_SD_Init();
    MX_USB_OTG_FS_PCD_Init();
    MX_FATFS_Init();
    /* USER CODE BEGIN 2 */

    LED_RGB_Data Led1_RGB = {
        {
            .GPIO_Port = LED1_RED_GPIO_Port,
            .GPIO_Pin = LED1_RED_Pin,
        },
        {
            .GPIO_Port = LED1_GREEN_GPIO_Port,
            .GPIO_Pin = LED1_GREEN_Pin,
        },
        {
            .GPIO_Port = LED1_BLUE_GPIO_Port,
            .GPIO_Pin = LED1_BLUE_Pin,
        },
    };

    LED_RGB_Data Led2_RGB = {
        {
            .GPIO_Port = LED2_RED_GPIO_Port,
            .GPIO_Pin = LED2_RED_Pin,
        },
        {
            .GPIO_Port = LED2_GREEN_GPIO_Port,
            .GPIO_Pin = LED2_GREEN_Pin,
        },
        {
            .GPIO_Port = LED2_BLUE_GPIO_Port,
            .GPIO_Pin = LED2_BLUE_Pin,
        },
    };

    LED Led1(Led1_RGB);
    LED Led2(Led2_RGB);
    SD_Card SD;

    SD.checkInsertion();
    if (SD.isInserted()) {
        char name[] = "test.txt";
        char buf[] = "Hello from SD Class";
        if (SD.write_once(name, buf)) {
            Led1.OK(Color::GREEN);
        };
    }

    bool once = false;

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 168;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief SDIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_SDIO_SD_Init(void) {

    /* USER CODE BEGIN SDIO_Init 0 */

    /* USER CODE END SDIO_Init 0 */

    /* USER CODE BEGIN SDIO_Init 1 */

    /* USER CODE END SDIO_Init 1 */
    hsd.Instance = SDIO;
    hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
    hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
    hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
    hsd.Init.BusWide = SDIO_BUS_WIDE_4B;
    hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd.Init.ClockDiv = 6;
    hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
    /* USER CODE BEGIN SDIO_Init 2 */

    /* USER CODE END SDIO_Init 2 */
}

/**
 * @brief USB_OTG_FS Initialization Function
 * @param None
 * @retval None
 */
static void MX_USB_OTG_FS_PCD_Init(void) {

    /* USER CODE BEGIN USB_OTG_FS_Init 0 */

    /* USER CODE END USB_OTG_FS_Init 0 */

    /* USER CODE BEGIN USB_OTG_FS_Init 1 */

    /* USER CODE END USB_OTG_FS_Init 1 */
    hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
    hpcd_USB_OTG_FS.Init.dev_endpoints = 4;
    hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
    hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
    hpcd_USB_OTG_FS.Init.Sof_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.vbus_sensing_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
    if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN USB_OTG_FS_Init 2 */

    /* USER CODE END USB_OTG_FS_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    /* USER CODE BEGIN MX_GPIO_Init_1 */

    /* USER CODE END MX_GPIO_Init_1 */

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB, LED2_RED_Pin | LED1_RED_Pin | LED1_GREEN_Pin | LED2_GREEN_Pin, GPIO_PIN_SET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOC, LED1_BLUE_Pin | LED2_BLUE_Pin, GPIO_PIN_SET);

    /*Configure GPIO pin : SD_PRESENCE_Pin */
    GPIO_InitStruct.Pin = SD_PRESENCE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(SD_PRESENCE_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : LED2_RED_Pin LED1_RED_Pin LED1_GREEN_Pin LED2_GREEN_Pin */
    GPIO_InitStruct.Pin = LED2_RED_Pin | LED1_RED_Pin | LED1_GREEN_Pin | LED2_GREEN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pins : LED1_BLUE_Pin LED2_BLUE_Pin */
    GPIO_InitStruct.Pin = LED1_BLUE_Pin | LED2_BLUE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* USER CODE BEGIN MX_GPIO_Init_2 */

    /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line
       number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
       line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
