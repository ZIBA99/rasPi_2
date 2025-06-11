#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <sys/ioctl.h>
#include <stdlib.h>  // rand() 함수를 위한 헤더
#include <time.h>    // srand() 시드 설정을 위한 헤더

static const char* I2C_DEV = "/dev/i2c-1"; 	/* I2C를 위한 장치 파일 */
static const int I2C_SLAVE = 0x0703; 		/* ioctl( ) 함수에서 I2C_SLAVE 설정을 위한 값38 */

static const int LPS25H_ID = 0x5C; 		/* SenseHAT의 I2C-1의 값 */
static const int LED_MATRIX = 0x46;             /* SenseHAT LED 매트릭스의 I2C 주소 */

static const int CTRL_REG1 = 0x20; 		/* STMicroelectronics의 LPS25H 스펙 문서의 값 */
static const int CTRL_REG2 = 0x21;

static const int PRESS_OUT_XL = 0x28; 		/* STMicroelectronics의 LPS25H 스펙 문서의 값 */
static const int PRESS_OUT_L = 0x29;
static const int PRESS_OUT_H = 0x2A;

static const int PTEMP_OUT_L = 0x2B;
static const int PTEMP_OUT_H = 0x2C;

void getPressure(int fd, double *temperature, double *pressure);	/* 기압과 온도를 위한 함수 */
void setRandomLED(int led_fd);                                    /* LED 매트릭스를 랜덤하게 설정하는 함수 */

int main( )
{
    int i2c_fd, led_fd;
    double t_c = 0.0; 				/* 온도와 압력을 출력하기 위한 변수 */
    double pressure = 0.0;

    /* 랜덤 시드 초기화 */
    srand(time(NULL));

    /* I2C 장치 파일 오픈 (LPS25H 센서용) */
    if((i2c_fd = open(I2C_DEV, O_RDWR)) < 0) {
        perror("Unable to open i2c device");
        return 1;
    }

    /* I2C 장치 파일 오픈 (LED 매트릭스용) */
    if((led_fd = open(I2C_DEV, O_RDWR)) < 0) {
        perror("Unable to open i2c device for LED matrix");
        close(i2c_fd);
        return 1;
    }

    /* I2C 장치의 슬레이브(slave) 모드 설정을 위해 LPS25H를 사용 */
    if(ioctl(i2c_fd, I2C_SLAVE, LPS25H_ID) < 0) {
        perror("Unable to configure i2c slave device");
        close(i2c_fd);
        close(led_fd);
        return 1;
    }

    /* LED 매트릭스 I2C 슬레이브 설정 */
    if(ioctl(led_fd, I2C_SLAVE, LED_MATRIX) < 0) {
        perror("Unable to configure i2c slave device for LED matrix");
        close(i2c_fd);
        close(led_fd);
        return 1;
    }

    for(int i = 0; i < 10; i++) {
        /* LPS25H 장치 초기화 */
        wiringPiI2CWriteReg8(i2c_fd, CTRL_REG1, 0x00);
        wiringPiI2CWriteReg8(i2c_fd, CTRL_REG1, 0x84);
        wiringPiI2CWriteReg8(i2c_fd, CTRL_REG2, 0x01);

        /* 기압과 온도 값 획득 */
        getPressure(i2c_fd, &t_c, &pressure);

        /* 계산된 값을 출력 */
        printf("Temperature(from LPS25H) = %.2f°C\n", t_c);
        printf("Pressure = %.0f hPa\n", pressure);
        
        /* LED 매트릭스를 랜덤하게 설정 */
        setRandomLED(led_fd);
        
        delay(1000); 					/* 다시 측정하기 전에 1초(1000밀리초)간 대기 */
    }

    /* 사용이 끝난 장치를 정리 */
    wiringPiI2CWriteReg8(i2c_fd, CTRL_REG1, 0x00);

    close(i2c_fd);
    close(led_fd);

    return 0;
}

/* 기압과 온도를 가져오기 위한 함수 */
void getPressure(int fd, double *temperature, double *pressure)
{
    int result;
    unsigned char temp_out_l = 0, temp_out_h = 0; 	/* 온도를 계산하기 위한 변수 */
    unsigned char press_out_xl = 0; 			/* 기압을 계산하기 위한 변수 */
    unsigned char press_out_l = 0;
    unsigned char press_out_h = 0;
    short temp_out = 0; 				/* 온도와 압력을 저장하기 위한 변수 */
    int press_out = 0;

    /* 측정이 완료될 때까지 대기 */
    do {
        delay(25); 					/* 25밀리초 대기 */
        result = wiringPiI2CReadReg8(fd, CTRL_REG2);
    } while(result != 0);

    /* 측정된 온도값 읽기(2바이트 읽기) */
    temp_out_l = wiringPiI2CReadReg8(fd, PTEMP_OUT_L);
    temp_out_h = wiringPiI2CReadReg8(fd, PTEMP_OUT_H);

    /* 측정된 기압값 읽기(3바이트 읽기) */
    press_out_xl = wiringPiI2CReadReg8(fd, PRESS_OUT
}
