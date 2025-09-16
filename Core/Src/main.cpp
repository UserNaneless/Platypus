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
#include "stm32f4xx_hal_can.h"
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
CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

SD_HandleTypeDef hsd;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SDIO_SD_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
static void MX_CAN1_Init(void);
static void MX_CAN2_Init(void);
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

class LED_Base {
    protected:
        Color state = Color::OFF;

        void setPin(GPIO_TypeDef *port, uint16_t pin, bool state) {
            if (state)
                HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
            else
                HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
        }

    public:
        virtual void setColor(Color color) {
            state = color;
        };

        virtual ~LED_Base() = default;

        virtual void Off() {
            setColor(Color::OFF);
        };

        virtual void OK() {
            setColor(state);
            wait(300);
            Off();
            wait(300);
            setColor(state);
            wait(300);
            Off();
            wait(300);
            setColor(state);
            wait(300);
            Off();
        }
};

class LED_RGB : LED_Base {
    private:
        LED_RGB_Data led;

    public:
        LED_RGB(LED_RGB_Data &led) : led(led) {}

        void setColor(Color color) {
            state = color;
            setPin(led.r.GPIO_Port, led.r.GPIO_Pin, (color >> 2) & 1);
            setPin(led.g.GPIO_Port, led.g.GPIO_Pin, (color >> 1) & 1);
            setPin(led.b.GPIO_Port, led.b.GPIO_Pin, color & 1);
        }

        void Off() {
            setColor(Color::OFF);
        }

        void OK() {
            setColor(state);
            wait(300);
            Off();
            wait(300);
            setColor(state);
            wait(300);
            Off();
            wait(300);
            setColor(state);
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

class LED : LED_Base {
    private:
        LED_Color_Data led;

    public:
        LED(LED_Color_Data &led) : led(led) {}

        void setColor(Color color) {
            state = color;
            setPin(led.GPIO_Port, led.GPIO_Pin, color);
        }

        void On() {
            setColor(Color::WHITE);
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
                    if (mount() != FR_OK) {
                        inserted = false;
                        return false;
                    }
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
                return FR_NOT_READY;
            return f_mount(&FatFS, "0:", 1);
        }

        FRESULT unmount() {
            if (!inserted)
                return FR_NOT_READY;
            return f_mount(NULL, "0:", 0);
        }

        FRESULT write_once(char *name, char *buf, size_t size, BYTE mode = FA_WRITE | FA_CREATE_ALWAYS) {
            if (!inserted)
                return FR_NOT_READY;
            FIL fil;
            if (f_open(&fil, name, mode) == FR_OK) {
                UINT bw;
                if (f_write(&fil, buf, size, &bw) != FR_OK) {
                    f_close(&fil);
                    return FR_DISK_ERR;
                }
            }
            return f_close(&fil);
        }

        bool open(char *name, BYTE mode = FA_WRITE | FA_CREATE_ALWAYS) {
            if (!inserted)
                return false;
            if (f_open(&fil, name, mode) == FR_OK) {
                opened = true;
                return true;
            };
            return false;
        }

        FRESULT write(char *buf, size_t size) {
            if (!inserted)
                return FR_NOT_READY;
            if (!opened)
                return FR_NO_FILE;
            return f_write(&fil, buf, size, NULL);
        }

        FRESULT close() {
            if (!inserted)
                return FR_NOT_READY;
            if (!opened)
                return FR_NO_FILE;
            return f_close(&fil);
        }
};
uint32_t txMailbox;

void CAN1_SendTest(void) {
    CAN_TxHeaderTypeDef txHeader;
    uint8_t txData[8] = { 0x11, 0x22, 0x33, 0x44 };

    txHeader.StdId = 0x123;      // Standard ID
    txHeader.IDE = CAN_ID_STD;   // Use standard ID
    txHeader.RTR = CAN_RTR_DATA; // Data frame
    txHeader.DLC = 4;            // 4 data bytes

    HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox);
}

bool CAN1_ReceivePolling(void) {
    CAN_RxHeaderTypeDef rxHeader;
    uint8_t rxData[8];

    // if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) > 0)
    // {
    if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rxHeader, rxData) == HAL_OK) {
        // rxHeader.StdId -> should be 0x123
        // rxData[0..3]   -> should be {0x11,0x22,0x33,0x44}
        return true;
    }
    // }
    return false;
}

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
    MX_CAN1_Init();
    MX_CAN2_Init();

    HAL_CAN_Start(&hcan1);
    HAL_CAN_Start(&hcan2);
    /* USER CODE BEGIN 2 */

    LED_RGB_Data LED1_RGB_Data = {
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

    LED_RGB_Data LED2_RGB_Data = {
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

    LED_Color_Data CAN1_LED_Data = {
        .GPIO_Port = CAN1_LED_GPIO_Port,
        .GPIO_Pin = CAN1_LED_Pin,
    };

    LED_Color_Data CAN2_LED_Data = {
        .GPIO_Port = CAN2_LED_GPIO_Port,
        .GPIO_Pin = CAN2_LED_Pin,
    };

    LED_RGB Led1(LED1_RGB_Data);
    LED_RGB Led2(LED2_RGB_Data);
    LED CAN1_LED(CAN1_LED_Data);
    LED CAN2_LED(CAN2_LED_Data);
    SD_Card SD;

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        CAN1_SendTest();
        wait(100);
        if (HAL_CAN_IsTxMessagePending(&hcan1, txMailbox)) {
            Led2.setColor(Color::YELLOW);
            wait(1000);
        } else {
            Led2.setColor(Color::OFF);
        }
        if (CAN1_ReceivePolling()) {
            Led1.setColor(Color::GREEN);
            wait(10000);
        } else {
            Led1.setColor(Color::RED);
        }
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
 * @brief CAN1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_CAN1_Init(void) {

    /* USER CODE BEGIN CAN1_Init 0 */

    /* USER CODE END CAN1_Init 0 */

    /* USER CODE BEGIN CAN1_Init 1 */

    /* USER CODE END CAN1_Init 1 */
    hcan1.Instance = CAN1;
    hcan1.Init.Prescaler = 16;
    hcan1.Init.Mode = CAN_MODE_NORMAL;
    hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan1.Init.TimeSeg1 = CAN_BS1_1TQ;
    hcan1.Init.TimeSeg2 = CAN_BS2_1TQ;
    hcan1.Init.TimeTriggeredMode = DISABLE;
    hcan1.Init.AutoBusOff = DISABLE;
    hcan1.Init.AutoWakeUp = DISABLE;
    hcan1.Init.AutoRetransmission = DISABLE;
    hcan1.Init.ReceiveFifoLocked = DISABLE;
    hcan1.Init.TransmitFifoPriority = DISABLE;

    hcan1.Init.Mode = CAN_MODE_LOOPBACK;
    if (HAL_CAN_Init(&hcan1) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN CAN1_Init 2 */
    CAN_FilterTypeDef filterConfig;
    filterConfig.FilterBank = 0;
    filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    filterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    filterConfig.FilterIdHigh = 0x0000;
    filterConfig.FilterIdLow = 0x0000;
    filterConfig.FilterMaskIdHigh = 0x0000;
    filterConfig.FilterMaskIdLow = 0x0000;
    filterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    filterConfig.FilterActivation = ENABLE;

    HAL_CAN_ConfigFilter(&hcan1, &filterConfig);

    /* USER CODE END CAN1_Init 2 */
}

/**
 * @brief CAN2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_CAN2_Init(void) {

    /* USER CODE BEGIN CAN2_Init 0 */

    /* USER CODE END CAN2_Init 0 */

    /* USER CODE BEGIN CAN2_Init 1 */

    /* USER CODE END CAN2_Init 1 */
    hcan2.Instance = CAN2;
    hcan2.Init.Prescaler = 16;
    hcan2.Init.Mode = CAN_MODE_NORMAL;
    hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan2.Init.TimeSeg1 = CAN_BS1_1TQ;
    hcan2.Init.TimeSeg2 = CAN_BS2_1TQ;
    hcan2.Init.TimeTriggeredMode = DISABLE;
    hcan2.Init.AutoBusOff = DISABLE;
    hcan2.Init.AutoWakeUp = DISABLE;
    hcan2.Init.AutoRetransmission = DISABLE;
    hcan2.Init.ReceiveFifoLocked = DISABLE;
    hcan2.Init.TransmitFifoPriority = DISABLE;
    if (HAL_CAN_Init(&hcan2) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN CAN2_Init 2 */
    CAN_FilterTypeDef filterConfig = { 0 };
    filterConfig.FilterBank = 14;
    filterConfig.SlaveStartFilterBank = 14;
    filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    filterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    filterConfig.FilterIdHigh = 0x0000;
    filterConfig.FilterIdLow = 0x0000;
    filterConfig.FilterMaskIdHigh = 0x0000;
    filterConfig.FilterMaskIdLow = 0x0000;
    filterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    filterConfig.FilterActivation = ENABLE;

    HAL_CAN_ConfigFilter(&hcan2, &filterConfig);

    /* USER CODE END CAN2_Init 2 */
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
    hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
    hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd.Init.ClockDiv = 0;
    /* USER CODE BEGIN SDIO_Init 2 */
    hsd.Init.BusWide = SDIO_BUS_WIDE_4B;
    hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd.Init.ClockDiv = 6;
    hsd.Init.BusWide = SDIO_BUS_WIDE_1B;

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
    HAL_GPIO_WritePin(GPIOB, LED2_RED_Pin | LED1_RED_Pin | LED1_GREEN_Pin | LED2_GREEN_Pin | CAN1_LED_Pin, GPIO_PIN_SET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOC, LED1_BLUE_Pin | LED2_BLUE_Pin, GPIO_PIN_SET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(CAN2_LED_GPIO_Port, CAN2_LED_Pin, GPIO_PIN_SET);

    /*Configure GPIO pin : SD_PRESENCE_Pin */
    GPIO_InitStruct.Pin = SD_PRESENCE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(SD_PRESENCE_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : LED2_RED_Pin LED1_RED_Pin LED1_GREEN_Pin LED2_GREEN_Pin
                             CAN1_LED_Pin */
    GPIO_InitStruct.Pin = LED2_RED_Pin | LED1_RED_Pin | LED1_GREEN_Pin | LED2_GREEN_Pin | CAN1_LED_Pin;
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

    /*Configure GPIO pin : CAN2_LED_Pin */
    GPIO_InitStruct.Pin = CAN2_LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(CAN2_LED_GPIO_Port, &GPIO_InitStruct);

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
