#include "diskio.h"
#include "ff.h"
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
